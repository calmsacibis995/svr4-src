/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:proc/prsubr.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/inline.h"
#include "sys/kmem.h"
#include "sys/mman.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/uio.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/session.h"

#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/disp.h"
#include "sys/class.h"
#include "sys/ts.h"
#include "sys/bitmap.h"

#include "sys/fault.h"
#include "sys/syscall.h"
#include "sys/procfs.h"

#include "vm/as.h"
#include "vm/rm.h"
#include "vm/seg.h"
#include "vm/faultcatch.h"

#include "prdata.h"

struct prnode prrootnode;

extern u_int timer_resolution;

void
prinit(vswp, fstype)
	register struct vfssw *vswp;
	int fstype;
{
	register int dev;

	procfstype = fstype;
	ASSERT(procfstype != 0);
	/*
	 * Associate VFS ops vector with this fstype.
	 */
	vswp->vsw_vfsops = &prvfsops;

	/*
	 * Assign a unique "device" number (used by stat(2)).
	 */
	if ((dev = getudev()) == -1) {
		cmn_err(CE_WARN, "prinit: can't get unique device number");
		dev = 0;
	}
	procdev = makedevice(dev, 0);
	prmounted = 0;
}

/*
 * Called from a hook in freeproc() when a traced process is removed
 * from the process table.  The proc-table pointers of all associated
 * /proc vnodes are cleared to indicate that the process has gone away.
 */
void
prexit(p)
	struct proc *p;
{
	register struct vnode *vp;
	register struct prnode *pnp;

	ASSERT(p->p_trace);
	for (vp = p->p_trace; vp != NULL; vp = pnp->pr_vnext) {
		pnp = VTOP(vp);
		ASSERT(pnp->pr_proc);
		pnp->pr_proc = NULL;
	}
	wakeprocs((caddr_t)p->p_trace, PRMPT);
}

/*
 * Ensure that the underlying object file is readable.
 */
int
prisreadable(p, cr)
	register proc_t *p;
	register struct cred *cr;
{
	register struct seg *seg;
	register struct vnode *vp;
	register int error = 0;

	if ((p->p_flag & SSYS) == 0 && p->p_as && (vp = p->p_exec) != NULL)
		error = VOP_ACCESS(vp, VREAD, 0, cr);

	return error;
}

/*
 * Called from hooks in exec-related code when a traced process
 * attempts to exec(2) a setuid/setgid program or an unreadable
 * file.  Rather than fail the exec we invalidate the associated
 * /proc vnode so that subsequent attempts to use it will fail.
 *
 * Old (invalid) vnodes are retained on a linked list (rooted at
 * p_trace in the process table entry) until last close.
 *
 * A controlling process must re-open the /proc file in order to
 * regain control.
 */
void
prinvalidate(up)
	register struct user *up;
{
	register proc_t *p = up->u_procp;
	register struct vnode *vp = p->p_trace;

	/*
	 * Invalidate the currently active /proc vnode.
	 */
	if (vp != NULL)
		VTOP(vp)->pr_flags |= PRINVAL;

	/*
	 * If any tracing flags are in effect and the vnode is open for
	 * writing then set the requested-stop and run-on-last-close flags.
	 * Otherwise, clear all tracing flags and wake up any tracers.
	 */
	for (; vp != NULL; vp = VTOP(vp)->pr_vnext)
		if (VTOP(vp)->pr_writers)	/* some writer exists */
			break;

	if ((p->p_flag & SPROCTR) && vp != NULL)
		p->p_flag |= (SPRSTOP|SRUNLCL);
	else {
		premptyset(&up->u_entrymask);		/* syscalls */
		premptyset(&up->u_exitmask);
		up->u_systrap = 0;
		premptyset(&p->p_sigmask);		/* signals */
		premptyset(&p->p_fltmask);		/* faults */
		p->p_flag &= ~(SPRSTOP|SRUNLCL|SPROCTR|SPRFORK);
		if (p->p_trace)
			wakeprocs((caddr_t)p->p_trace, PRMPT);
	}
}

STATIC proc_t *prpidlock, *prpidwant;

/*
 * Lock a process.  Until prunlock() is performed, this
 * (a) serializes use of /proc and (b) prevents the process
 * from running again if it is not a system process.
 * Returns 0 on success, a non-zero error number on failure.
 *
 * 'zdisp' is ZYES or ZNO to indicate whether encountering a
 * zombie process is to be considered an error.
 *
 * If 'force' is true, we don't execute the deadlock prevention code.
 * This is used only when nothing more than the process's proc table
 * entry and/or its u-block will be accessed.  This is necessary, in
 * particular, in prclose(), which cannot be interrupted.
 */
int
prlock(pnp, zdisp, force)
	register struct prnode *pnp;
	register int zdisp;
	int force;
{
	register int s = splhi();
	register proc_t *p;

again:
	/*
	 * Return immediately if there is no process.
	 */
	if ((p = pnp->pr_proc) == NULL
	  || (zdisp == ZNO && p->p_stat == SZOMB)) {
		splx(s);
		return ENOENT;
	}

	ASSERT(p->p_stat != 0 && p->p_stat != SIDL);
	ASSERT(p->p_trace != NULL);

	/*
	 * Sleep until we can grab prpidlock.
	 * This serializes the use of /proc.
	 */
	if (prpidlock) {
		prpidwant = u.u_procp;
		sleep((caddr_t)&prpidlock, PZERO);
		goto again;
	}

	/*
	 * Deadlock prevention:  If the process is not ourself and is
	 * not a system process but is sleeping at high priority or at
	 * pseudo-high priority (used by RFS), then we sleep expecting
	 * to be awakened by the process when it is awakened and runs.
	 * This requires cooperation from sleep().
	 *
	 * Special case: a process sleeping above on &prpidlock is safe.
	 * This eliminates mutual deadlock problems.
	 *
	 * If 'force' is true, we skip this deadlock-prevention code
	 * and go on to lock the process immediately.  This can be set
	 * by the caller if it is known that the operation being performed
	 * is "safe" (i.e. that it will not require access to a high-priority
	 * resource being held by the target process).  Exception: if the
	 * target is holding any swap locks the operation is "unsafe"
	 * regardless of 'force'.
	 */
	if ((!force || (p->p_flag & SSWLOCKS))
	  && p != u.u_procp
	  && !(p->p_flag & SSYS)
	  && (p->p_flag & (SNWAKE|SNOSTOP))
	  && p->p_wchan != (caddr_t)&prpidlock) {
		/*
		 * We ourself are safe wrt SPROCIO since we sleep at
		 * interruptible priority.  Sleeping at noninterruptible
		 * priority here would introduce mutual deadlock problems.
		 */
		p->p_flag |= SPRWAKE;
		if (sleep((caddr_t)&p->p_trace, (PZERO+1)|PCATCH)) {
			splx(s);
			return EINTR;
		}
		goto again;
	}

	ASSERT(prpidlock == NULL);
	prpidlock = u.u_procp;
	if (p != u.u_procp) {
		/*
		 * Interlock; non-system process will not run while this
		 * flag is on.  If we set this on a zombie it's ineffective
		 * but does no harm.
		 */
		p->p_flag |= SPROCIO;
		if (p->p_stat == SRUN && (p->p_flag & (SLOAD|SSYS)) == SLOAD)
			dq_srundec(p->p_pri);
	}

	splx(s);
	return 0;
}

/*
 * Cooperation with /proc deadlock prevention code, called
 * from sleep() by a /proc-traced process on waking up from
 * a high-priority or PNOSTOP sleep.
 *
 * Check to see if a controlling process is sleeping in prlock()
 * waiting for us to wake up.  If so, wake up the controlling
 * process and arrange for our preemption when the time comes.
 */
void
prawake(p)
	register proc_t *p;
{
	if (p->p_flag & SPRWAKE) {
		p->p_flag &= ~SPRWAKE;
		wakeprocs((caddr_t)&p->p_trace, PRMPT);
		runrun = 1;
	}
}

/*
 * Undo prlock.
 */
void
prunlock(pnp)
	struct prnode *pnp;
{
	register int s = splhi();
	register proc_t *p = pnp->pr_proc;

	if (p && (p->p_flag & SPROCIO)) {
		p->p_flag &= ~SPROCIO;
		if (p->p_stat == SRUN && (p->p_flag & (SLOAD|SSYS)) == SLOAD) {
			dq_sruninc(p->p_pri);
			if (p->p_pri > curpri)
				runrun++;
		}
	}

	ASSERT(prpidlock == u.u_procp);
	prpidlock = 0;
	if (prpidwant) {
		prpidwant = 0;
		wakeprocs((caddr_t)&prpidlock, PRMPT);
	}
	splx(s);
}

/*
 * Return process status.  The u-block is mapped in by this routine and
 * unmapped at the end.
 */
int
prgetstatus(p, sp)
	register proc_t *p;
	register prstatus_t *sp;
{
	register long flags;
	register user_t *up;
	instr_t instr;
	struct uio uio;
	struct iovec iov;
	u_long restonano;
	int error;

	bzero((caddr_t)sp, sizeof(*sp));

	up = prumap(p);
	CATCH_FAULTS(CATCH_SEGU_FAULT) {
		if (p->p_whystop == PR_FAULTED)
			bcopy((caddr_t)&up->u_siginfo, (caddr_t)&sp->pr_info,
			  sizeof(k_siginfo_t));
		else if (p->p_curinfo)
			bcopy((caddr_t)&p->p_curinfo->sq_info, (caddr_t)&sp->pr_info,
			  sizeof(k_siginfo_t));
		sp->pr_altstack = up->u_sigaltstack;
		prgetaction(p, up, p->p_cursig, &sp->pr_action);
		prgetregs(up, sp->pr_reg);
	}
	prunmap(p);
	if ((error = END_CATCH()) != 0)
		return error;

	flags = 0L;
	if (p->p_stat == SSTOP) {
		flags |= PR_STOPPED;
		if ((p->p_flag & SPSTART) == 0)
			flags |= PR_ISTOP;
	}
	if (p->p_flag & SPRSTOP)
		flags |= PR_DSTOP;
	if ((p->p_flag & SASLEEP)
	  || (p->p_stat == SSLEEP && (p->p_flag & (SNWAKE|SNOSTOP)) == 0))
		flags |= PR_ASLEEP;
	if (p->p_flag & SPRFORK)
		flags |= PR_FORK;
	if (p->p_flag & SRUNLCL)
		flags |= PR_RLC;
	if (p->p_flag & STRC)
		flags |= PR_PTRACE;
	sp->pr_flags   = flags;
	sp->pr_why     = p->p_whystop;
	sp->pr_what    = p->p_whatstop;

	sp->pr_cursig  = p->p_cursig;
	prassignset(&sp->pr_sigpend, &p->p_sig);
	prassignset(&sp->pr_sighold, &p->p_hold);
	sp->pr_pid   = p->p_pid;
	sp->pr_ppid  = p->p_ppid;
	sp->pr_pgrp  = p->p_pgrp;
	sp->pr_sid   = p->p_sessp->s_sid;
	restonano = 1000000000 / timer_resolution;
	sp->pr_utime.tv_sec = p->p_utime / timer_resolution;
	sp->pr_utime.tv_nsec = (p->p_utime % timer_resolution) * restonano;
	sp->pr_stime.tv_sec = p->p_stime / timer_resolution;
	sp->pr_stime.tv_nsec = (p->p_stime % timer_resolution) * restonano;
	sp->pr_cutime.tv_sec = p->p_cutime / timer_resolution;
	sp->pr_cutime.tv_nsec = (p->p_cutime % timer_resolution) * restonano;
	sp->pr_cstime.tv_sec = p->p_cstime / timer_resolution;
	sp->pr_cstime.tv_nsec = (p->p_cstime % timer_resolution) * restonano;
	bcopy(class[p->p_cid].cl_name, sp->pr_clname,
	  min(sizeof(class[0].cl_name), sizeof(sp->pr_clname)-1));

	/*
	 * Fetch the next instruction, if not a system process.  To avoid
	 * potential deadlocks we don't attempt this unless the process
	 * is stopped.
	 */
	if (p->p_flag & SSYS) {
		sp->pr_instr = 0;
		sp->pr_flags |= (PR_ISSYS|PR_PCINVAL);
	} else if (p->p_stat != SSTOP) {
		sp->pr_instr = 0;
		sp->pr_flags |= PR_PCINVAL;
	} else {
		iov.iov_base = (caddr_t) &instr;
		uio.uio_resid = iov.iov_len = sizeof(instr);
		uio.uio_offset = (off_t) prgetpc(sp->pr_reg);
		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_segflg = UIO_SYSSPACE;
		if (prusrio(p, UIO_READ, &uio) == 0)
			sp->pr_instr = instr;
		else {
			sp->pr_instr = 0;
			sp->pr_flags |= PR_PCINVAL;
		}
	}

	return 0;
}

/*
 * Get the sigaction structure for the specified signal.  The u-block
 * must already have been mapped in by the caller.
 */
void
prgetaction(p, up, sig, sp)
	register proc_t *p;
	register user_t *up;
	register u_int sig;
	register struct sigaction *sp;
{
	sp->sa_handler = SIG_DFL;
	premptyset(&sp->sa_mask);
	sp->sa_flags = 0;
	/* LINTED */
	if (sig > 0 && sig < NSIG) {
		sp->sa_handler = up->u_signal[sig-1];
		prassignset(&sp->sa_mask, &up->u_sigmask[sig-1]);
		if (sigismember(&up->u_sigonstack, sig))
			sp->sa_flags |= SA_ONSTACK;
		if (sigismember(&up->u_sigresethand, sig))
			sp->sa_flags |= SA_RESETHAND;
		if (sigismember(&up->u_sigrestart, sig))
			sp->sa_flags |= SA_RESTART;
		if (sigismember(&p->p_siginfo, sig))
			sp->sa_flags |= SA_SIGINFO;
		if (sigismember(&up->u_signodefer, sig))
			sp->sa_flags |= SA_NODEFER;
		if (sig == SIGCLD) {
			if (p->p_flag & SNOWAIT)
				sp->sa_flags |= SA_NOCLDWAIT;
			if ((p->p_flag & SJCTL) == 0)
				sp->sa_flags |= SA_NOCLDSTOP;
		}
	}
}

/*
 * Count the number of segments in this process's address space.
 * We always return 0 for a system process.
 */
int
prnsegs(p)
	proc_t *p;
{
	int n;
	caddr_t saddr, addr, eaddr;
	register struct seg *seg, *sseg;
	register struct as *as;

	n = 0;
	if ((p->p_flag & SSYS) == 0
	  && (as = p->p_as) != NULL
	  && (seg = sseg = as->a_segs) != NULL) {
		do {
			saddr = seg->s_base;
			eaddr = seg->s_base + seg->s_size;
			do { 
				(void)as_getprot(as, saddr, &addr);
				n++;
			} while ((saddr = addr) != eaddr);
		} while ((seg = seg->s_next) != sseg);
	}

	return n;
}

/*
 * Fill an array of structures with memory map information.  The array
 * has already been zero-filled by the caller.
 */
void
prgetmap(p, prmapp)
	proc_t *p;
	prmap_t *prmapp;
{
	register prmap_t *mp;
	register struct seg *seg, *sseg;
	register struct as *as;
	struct seg *brkseg, *stkseg;
	caddr_t saddr, addr, eaddr;
	int prot;

	if ((p->p_flag & SSYS)
	  || (as = p->p_as) == NULL
	  || (seg = sseg = as->a_segs) == NULL)
		return;

	mp = prmapp;
	stkseg = as_segat(as, p->p_stkbase);
	brkseg = as_segat(as, p->p_brkbase + p->p_brksize  -1);
	do {
		saddr = seg->s_base;
		eaddr = seg->s_base + seg->s_size;
		do { 
			mp->pr_vaddr = saddr;
			mp->pr_off = 0L;
			mp->pr_mflags = 0;
			prot = as_getprot(as, saddr, &addr);
			mp->pr_size = addr - saddr;
			if (prot & PROT_READ)
				mp->pr_mflags |= MA_READ;
			if (prot & PROT_WRITE)
				mp->pr_mflags |= MA_WRITE;
			if (prot & PROT_EXEC)
				mp->pr_mflags |= MA_EXEC;
			mp->pr_off = (*seg->s_ops->getoffset)(seg, saddr);
			if ((*seg->s_ops->gettype)(seg, saddr) == MAP_SHARED)
				mp->pr_mflags |= MA_SHARED;
			if (seg == brkseg)
				mp->pr_mflags |= MA_BREAK;
			else if (seg == stkseg)
				mp->pr_mflags |= MA_STACK;
			mp++;
		} while ((saddr = addr) != eaddr);
	} while ((seg = seg->s_next) != sseg);
}

/*
 * Return a reference to the underlying vnode (if any) associated with the
 * given segment.  For this purpose we only report regular files (not, in
 * particular, devices).
 */
/* ARGSUSED */
vnode_t *
prvnode(p, seg, addr)
	proc_t *p;
	struct seg *seg;
	caddr_t addr;
{
	struct vnode *vp;

	if ((*seg->s_ops->getvp)(seg, addr, &vp) == 0
	  && vp->v_type == VREG) {
		VN_HOLD(vp);
		return vp;
	}
	return NULL;
}

/*
 * Return information used by ps(1).
 */
int
prgetpsinfo(p, psp)
	register proc_t *p;
	register struct prpsinfo *psp;
{
	register char c, state;
	user_t *up;
	long hztime;
	dev_t d;

	bzero((caddr_t)psp, sizeof(struct prpsinfo));
	switch (state = p->p_stat) {
	case SSLEEP:	c = 'S';	break;
	case SRUN:	c = 'R';	break;
	case SZOMB:	c = 'Z';	break;
	case SSTOP:	c = 'T';	break;
	case SIDL:	c = 'I';	break;
	case SONPROC:	c = 'O';	break;
	case SXBRK:	c = 'X';	break;
	default:	c = '?';	break;
	}
	psp->pr_state = state;
	psp->pr_sname = c;
	psp->pr_zomb = (state == SZOMB);
	psp->pr_pri = p->p_pri;
	psp->pr_cpu = p->p_cpu;
	psp->pr_flag = p->p_flag;
	psp->pr_uid = p->p_cred->cr_ruid;
	psp->pr_gid = p->p_cred->cr_rgid;
	psp->pr_pid = p->p_pid;
	psp->pr_ppid = p->p_ppid;
	psp->pr_pgrp = p->p_pgrp;
	psp->pr_sid = p->p_sessp->s_sid;
	psp->pr_addr = prgetpsaddr(p);
	psp->pr_size = btoc(rm_assize(p->p_as));
	psp->pr_rssize = rm_asrss(p->p_as);
	psp->pr_wchan = p->p_wchan;
	hztime = p->p_utime + p->p_stime;
	psp->pr_time.tv_sec = hztime / timer_resolution;
	psp->pr_time.tv_nsec =
	  (hztime % timer_resolution) * (1000000000 / timer_resolution);
	if (state == SZOMB) {
		psp->pr_lttydev = PRNODEV;
		psp->pr_ottydev = PRNODEV;
	} else {
		int	error;

		bcopy(class[p->p_cid].cl_name, psp->pr_clname,
		  min(sizeof(class[0].cl_name), sizeof(psp->pr_clname)-1));
		if (strcmp(class[p->p_cid].cl_name, "TS") == 0) {
			psp->pr_oldpri = v.v_maxsyspri - p->p_pri;
			psp->pr_nice = ((tsproc_t *)p->p_clproc)->ts_nice;
		} else {
			psp->pr_oldpri = 0;
			psp->pr_nice = 0;
		}
		psp->pr_lttydev = ((d = cttydev(p)) == NODEV) ? PRNODEV : d;
		psp->pr_ottydev = cmpdev(d);
		up = prumap(p);
		CATCH_FAULTS(CATCH_SEGU_FAULT) {
			psp->pr_start.tv_sec = up->u_start;
			psp->pr_start.tv_nsec = 0L;
			bcopy(up->u_comm, psp->pr_fname,
			  min(sizeof(up->u_comm), sizeof(psp->pr_fname)-1));
			bcopy(up->u_psargs, psp->pr_psargs,
			  min(PRARGSZ-1, PSARGSZ));
		}
		prunmap(p);
		if ((error = END_CATCH()) != 0)
			return error;
	}

	return 0;
}

/*
 * Determine whether a set is empty.
 */
int
setisempty(sp, n)
	register u_long *sp;
	register unsigned n;
{
	while (n--)
		if (*sp++)
			return 0;
	return 1;
}
