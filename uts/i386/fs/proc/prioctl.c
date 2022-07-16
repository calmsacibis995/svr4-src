/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:proc/prioctl.c	1.3.2.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/immu.h"
#include "sys/inline.h"
#include "sys/kmem.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/vfs.h"
#include "sys/vnode.h"

#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/user.h"

#include "sys/fault.h"
#include "sys/syscall.h"
#include "sys/procfs.h"

#include "vm/as.h"
#include "vm/seg.h"
#include "vm/faultcatch.h"

#include "prdata.h"

STATIC int	isprwrioctl();
STATIC int	prsetrun();

/*
 * Control operations (lots).
 */
int
prioctl(vp, cmd, arg, flag, cr, rvalp)
	struct vnode *vp;
	int cmd;
	int arg;
	int flag;
	struct cred *cr;
	int *rvalp;
{
	caddr_t cmaddr = (caddr_t)arg;
	register proc_t *p;
	register user_t *up;
	register struct prnode *pnp = VTOP(vp);
	register int error;

	if (vp->v_type == VDIR)
		return EINVAL;

	/*
	 * Fail all ioctls if a security violation has occurred.
	 * This test alone is not sufficient; it must be repeated
	 * after any sleep(), including the sleep() in prlock().
	 */
	if (pnp->pr_flags & PRINVAL)
		return EAGAIN;

	/*
	 * Fail ioctls which are logically "write" requests unless
	 * the user has write permission.
	 */
	if ((flag & FWRITE) == 0 && isprwrioctl(cmd))
		return EBADF;

	if (cmd != PIOCSTOP && cmd != PIOCWSTOP) {
		int zdisp = (cmd == PIOCPSINFO || cmd == PIOCGETPR) ?
		  ZYES : ZNO;
		int force = 1;

		/*
		 * Identify ioctls that require deadlock prevention.
		 * (Most don't, and the operation is faster without it.)
		 */
		switch (cmd) {
		case PIOCOPENM:
			force = 0;
		}
		if (error = prlock(pnp, zdisp, force))
			return error;
		/*
		 * Fail if a security violation invalidated the vnode while
		 * we were waiting.
		 */
		if (pnp->pr_flags & PRINVAL) {
			prunlock(pnp);
			return EAGAIN;
		}
	}

	error = 0;
	if ((p = pnp->pr_proc) == NULL)
		return ENOENT;

	switch (cmd) {

	default:
		error = EINVAL;
		break;

	case PIOCGETPR:		/* read struct proc */
	{
		proc_t pr;

		/*
		 * Make a copy of the proc table entry before
		 * copying it out to the user, to guard against
		 * it changing underfoot (a possibility since
		 * this operation is permitted on zombies).
		 */
		pr = *p;
		if (copyout((caddr_t) &pr, cmaddr, sizeof(proc_t)))
			error = EFAULT;
		break;
	}

	case PIOCGETU:		/* read struct user */
		up = prumap(p);
		error = ucopyout((caddr_t)up, cmaddr, ctob(USIZE),
					CATCH_SEGU_FAULT);
		prunmap(p);
		break;

	case PIOCOPENM:		/* open mapped object for reading */
	{
		register struct seg *seg;
		int n;
		struct vnode *xvp;
		caddr_t va;

		/*
		 * By fiat, a system process has no address space.
		 */
		if ((p->p_flag & SSYS) || p->p_as == NULL)
			error = EINVAL;
		else if (cmaddr) {
			if (copyin(cmaddr, (caddr_t) &va, sizeof(caddr_t)))
				error = EFAULT;
			else if ((seg = as_segat(p->p_as, va)) == NULL
			  || (xvp = prvnode(p, seg, va)) == NULL)
				error = EINVAL;
		} else if ((xvp = p->p_exec) == NULL)
			error = EINVAL;
		else
			VN_HOLD(xvp);
		if (error == 0) {
			if ((error = VOP_ACCESS(xvp, VREAD, 0, cr)) == 0)
				error = fassign(&xvp, FREAD, &n);
			if (error) {
				VN_RELE(xvp);
			} else
				*rvalp = n;
		}
		break;
	}

	case PIOCSTOP:		/* stop process from running */
		/*
		 * Can't apply to a system process.
		 */
		if (p->p_flag & SSYS) {
			error = EBUSY;
			break;
		}
		/*
		 * If sleeping at low priority, stop immediately;
		 * if already stopped, do nothing; otherwise flag
		 * it to be stopped the next time it tries to run.
		 *
		 * Take care to cooperate with ptrace(2): if the process
		 * is ptraced and stopped on a signal which is not in the
		 * traced signal set or if we have previously set the process
		 * running and ptrace() hasn't done so yet, flag it to be
		 * stopped the next time ptrace() starts it.  Likewise for
		 * a process that is stopped on PR_JOBCONTROL.  We sleep
		 * below, in PIOCWSTOP, for the process to stop again.
		 */
		if (p->p_stat == SSLEEP
		  && ((p->p_flag & (SNWAKE|SNOSTOP)) == 0)
		  && stop(p, PR_REQUESTED, 0, 0))
			p->p_flag |= SASLEEP;
		else if (p->p_stat != SSTOP || (p->p_flag & SPSTART))
			p->p_flag |= SPRSTOP;

		/* FALLTHROUGH */

	case PIOCWSTOP:		/* wait for process to stop */
		/*
		 * Can't apply to a system process.
		 */
		if (p->p_flag & SSYS) {
			error = EBUSY;
			break;
		}

		/*
		 * Sleep until the process stops, but cooperate with ptrace(2):
		 * Don't wake up if the process is ptraced and stops on a
		 * signal which is not in the traced signal set or if we
		 * previously started it and ptrace() hasn't done so yet.
		 *
		 * Likewise for PR_JOBCONTROL stopped process: don't wake up
		 * until it stops for some other reason.
		 */
		while (pnp->pr_proc != NULL && p->p_stat != SZOMB
		  && (pnp->pr_flags & PRINVAL) == 0
		  && (p->p_stat != SSTOP || (p->p_flag & SPSTART))) {
			if (sleep((caddr_t)p->p_trace, (PZERO+1)|PCATCH)) {
				error = EINTR;
				break;
			}
		}
		if (error == 0) {
			if (pnp->pr_proc == NULL || p->p_stat == SZOMB)
				error = ENOENT;
			else if (pnp->pr_flags & PRINVAL)
				error = EAGAIN;
			else if (cmaddr && (error = prlock(pnp, ZNO, 0)) == 0) {
				prstatus_t prstat;
	
				/*
				 * Return process status information.
				 */
				if (pnp->pr_flags & PRINVAL)
					error = EAGAIN;
				else {
					error = prgetstatus(p, &prstat);
					if (error == 0 &&
					    copyout((caddr_t)&prstat,
						    cmaddr, sizeof(prstat)))
						error = EFAULT;
				}
				prunlock(pnp);
			}
		}
		break;

	case PIOCRUN:		/* make process runnable */
	{
		prrun_t prrun;

		if (p->p_stat != SSTOP || (p->p_flag & SPSTART)) {
			error = EBUSY;
			break;
		}
		if (cmaddr) {
			if (copyin(cmaddr, (caddr_t) &prrun, sizeof(prrun))) {
				error = EFAULT;
				break;
			}
			if ((error = prsetrun(p, &prrun)) != 0)
				break;
		}
		p->p_flag |= SPSTART;
		setrun(p);
		break;
	}

	case PIOCGTRACE:	/* get signal trace mask */
	{
		sigset_t smask;

		prassignset(&smask, &p->p_sigmask);
		if (copyout((caddr_t) &smask, cmaddr, sizeof(smask)))
			error = EFAULT;
		break;
	}

	case PIOCSTRACE:	/* set signal trace mask */
	{
		sigset_t smask;
		k_sigset_t sigmask;

		if (copyin(cmaddr, (caddr_t) &smask, sizeof(smask)))
			error = EFAULT;
		else {
			sigutok(&smask, &sigmask);
			sigdelset(&sigmask, SIGKILL);
			if (!sigisempty(&sigmask))
				p->p_flag |= SPROCTR;
			else if (prisempty(&p->p_fltmask)) {
				up = prumap(p);
				CATCH_FAULTS(CATCH_SEGU_FAULT) {
					if (up->u_systrap == 0)
						p->p_flag &= ~SPROCTR;
				}
				prunmap(p);
				if ((error = END_CATCH()) != 0)
					break;
			}
			p->p_sigmask = sigmask;
		}
		break;
	}

	case PIOCSSIG:		/* set current signal */
	{
		siginfo_t info;

		info.si_signo = 0;
		if (cmaddr && copyin(cmaddr, (caddr_t) &info, sizeof(info)))
			error = EFAULT;
		else if (info.si_signo < 0 || info.si_signo >= NSIG)
			/* Zero allowed here */
			error = EINVAL;
		else if (p->p_cursig == SIGKILL)
			/* "can't happen", but just in case */
			error = EBUSY;
		else if ((p->p_cursig = (u_char) info.si_signo) == 0) {
			/*
			 * Discard current siginfo_t, if any.
			 */
			if (p->p_curinfo) {
				kmem_free((caddr_t)p->p_curinfo,
				  sizeof(*p->p_curinfo));
				p->p_curinfo = NULL;
			}
		} else {
			/*
			 * If no current siginfo_t, allocate one.
			 */
			if (p->p_curinfo == NULL)
				p->p_curinfo = (sigqueue_t *)
				  kmem_alloc(sizeof(sigqueue_t), KM_SLEEP);
			/*
			 * Copy contents of info to current siginfo_t.
			 */
			bcopy((caddr_t)&info, (caddr_t)&p->p_curinfo->sq_info,
			  sizeof(p->p_curinfo->sq_info));
			/*
			 * Set signalled sleeping process running.
			 */
			if (p->p_stat == SSLEEP
			  && (p->p_flag & (SNWAKE|SNOSTOP)) == 0)
				setrun(p);
			/*
			 * If SIGKILL, set stopped process running.
			 * It should proceed to exit() w/o stopping.
			 */
			if (p->p_stat == SSTOP && p->p_cursig == SIGKILL) {
				p->p_flag |= SXSTART|SPSTART;
				setrun(p);
			}
		}
		break;
	}

	case PIOCKILL:		/* send signal */
	{
		int signo;
		sigsend_t v;
		extern int sigsendproc();

		if (copyin(cmaddr, (caddr_t) &signo, sizeof(int)))
			error = EFAULT;
		else if (signo <= 0 || signo >= NSIG)
			error = EINVAL;
		else {
			v.sig = signo;
			v.perm = 0;
			v.checkperm = 0;
			error = sigsendproc(p, &v);
			if (error == 0 && v.perm == 0)
				error = EPERM;
		}
		break;
	}

	case PIOCUNKILL:	/* delete a signal */
	{
		int signo;
		sigqueue_t *infop;

		if (copyin(cmaddr, (caddr_t) &signo, sizeof(int)))
			error = EFAULT;
		else if (signo <= 0 || signo >= NSIG || signo == SIGKILL)
			error = EINVAL;
		else {
			prdelset(&p->p_sig, signo);
			/*
			 * Dequeue and discard first (if any) siginfo
			 * for the designated signal.  sigdeq() will
			 * repost the signal bit to p->p_sig if any
			 * siginfo's for this signal remain.
			 */
			sigdeq(p, signo, &infop);
			if (infop)
				kmem_free((caddr_t)infop, sizeof(*infop));
		}
		break;
	}

	case PIOCNICE:		/* set nice priority */
	{
		int n;

		if (copyin(cmaddr, (caddr_t) &n, sizeof(int)))
			error = EFAULT;
		else
			error = donice(p, cr, n, (int *)NULL);
		break;
	}

	case PIOCGENTRY:	/* get syscall entry bit mask */
	case PIOCGEXIT:		/* get syscall exit bit mask */
	{
		sysset_t prmask;

		up = prumap(p);
		CATCH_FAULTS(CATCH_SEGU_FAULT) {
			if (cmd == PIOCGENTRY) {
				prassignset(&prmask, &up->u_entrymask);
			} else {
				prassignset(&prmask, &up->u_exitmask);
			}
		}
		prunmap(p);
		if ((error = END_CATCH()) == 0)
			break;

		if (copyout((caddr_t)&prmask, cmaddr, sizeof(prmask)))
			error = EFAULT;
		break;
	}

	case PIOCSENTRY:	/* set syscall entry bit mask */
	case PIOCSEXIT:		/* set syscall exit bit mask */
	{
		sysset_t prmask;
		int emptymask;

		if (copyin(cmaddr, (caddr_t)&prmask, sizeof(prmask))) {
			error = EFAULT;
			break;
		}

		emptymask = prisempty(&prmask);

		up = prumap(p);
		CATCH_FAULTS(CATCH_SEGU_FAULT) {
			if (cmd == PIOCSENTRY) {
				if (emptymask)
					emptymask = prisempty(&up->u_exitmask);
				prassignset(&up->u_entrymask, &prmask);
			} else {
				if (emptymask)
					emptymask = prisempty(&up->u_entrymask);
				prassignset(&up->u_exitmask, &prmask);
			}
			if ((up->u_systrap = !emptymask)) {
				p->p_flag |= SPROCTR;
			} else {
				if (sigisempty(&p->p_sigmask)
				  && prisempty(&p->p_fltmask))
					p->p_flag &= ~SPROCTR;
			}
		}
		prunmap(p);
		error = END_CATCH();
		break;
	}

	case PIOCSRLC:		/* set running on last /proc close */
		p->p_flag |= SRUNLCL;
		break;

	case PIOCRRLC:		/* reset run-on-last-close flag */
		p->p_flag &= ~SRUNLCL;
		break;

	case PIOCSFORK:		/* set inherit-on-fork flag */
		p->p_flag |= SPRFORK;
		break;

	case PIOCRFORK:		/* reset inherit-on-fork flag */
		p->p_flag &= ~SPRFORK;
		break;

	case PIOCGREG:		/* get general registers */
	{
		gregset_t regs;

		up = prumap(p);
		CATCH_FAULTS(CATCH_SEGU_FAULT)
			prgetregs(up, regs);
		prunmap(p);
		if ((error = END_CATCH()) != 0)
			break;
		if (copyout((caddr_t) regs, cmaddr, sizeof (gregset_t)))
			error = EFAULT;
		break;
	}

	case PIOCSREG:		/* set general registers */
	{
		gregset_t regs;

		if (p->p_stat != SSTOP || (p->p_flag & SPSTART))
			error = EBUSY;
		else if (copyin(cmaddr, (caddr_t) regs, sizeof (gregset_t)))
			error = EFAULT;
		else {
			up = prumap(p);
			CATCH_FAULTS(CATCH_SEGU_FAULT)
				prsetregs(up, regs);
			prunmap(p);
			error = END_CATCH();
		}
		break;
	}

	case PIOCGFPREG:	/* get floating-point registers */
	{
		fpregset_t fpregs;

		if (prhasfp()) {
			if ((error = prgetfpregs(p, &fpregs)) != 0)
				break;
			if (copyout((caddr_t)&fpregs, cmaddr, sizeof fpregs))
				error = EFAULT;
		} else
			error = EINVAL;	/* No FP support */
		break;
	}

	case PIOCSFPREG:	/* set floating-point registers */
	{
		fpregset_t fpregs;

		if (p->p_stat != SSTOP || (p->p_flag & SPSTART))
			error = EBUSY;
		else if (prhasfp()) {
			if (copyin(cmaddr, (caddr_t) &fpregs, sizeof(fpregs)))
				error = EFAULT;
			else
				error = prsetfpregs(p, &fpregs);
		} else
			error = EINVAL;	/* No FP support */
		break;
	}

	case PIOCSTATUS:	/* get process status */
	{
		prstatus_t prstat;

		if ((error = prgetstatus(p, &prstat)) != 0)
			break;
		if (copyout((caddr_t)&prstat, cmaddr, sizeof(prstat)))
			error = EFAULT;
		break;
	}

	case PIOCPSINFO:	/* get ps(1) information */
	{
		prpsinfo_t prps;

		if ((error = prgetpsinfo(p, &prps)) != 0)
			break;
		if (copyout((caddr_t) &prps, cmaddr, sizeof(prps)))
			error = EFAULT;
		break;
	}

	case PIOCMAXSIG:	/* get maximum signal number */
	{
		int n = NSIG-1;

		if (copyout((caddr_t)&n, cmaddr, sizeof(int)))
			error = EFAULT;
		break;
	}

	case PIOCACTION:	/* get signal action structures */
	{
		struct sigaction sigact;
		register u_int sig;

		up = prumap(p);
		for (sig = 1; sig < NSIG; sig++) {
			CATCH_FAULTS(CATCH_SEGU_FAULT)
				prgetaction(p, up, sig, &sigact);
			if ((error = END_CATCH()) != 0)
				break;
			if (copyout((caddr_t)&sigact, cmaddr, sizeof(sigact))) {
				error = EFAULT;
				break;
			}
			cmaddr += sizeof(sigact);
		}
		prunmap(p);

		break;
	}

	case PIOCGHOLD:		/* get signal-hold mask */
	{
		sigset_t holdmask;

		sigktou(&p->p_hold, &holdmask);
		if (copyout((caddr_t) &holdmask, cmaddr, sizeof(holdmask)))
			error = EFAULT;
		break;
	}

	case PIOCSHOLD:		/* set signal-hold mask */
	{
		sigset_t holdmask;

		if (copyin(cmaddr, (caddr_t) &holdmask, sizeof(holdmask)))
			error = EFAULT;
		else {
			sigutok(&holdmask, &p->p_hold);
			sigdiffset(&p->p_hold, &cantmask);
			if (p->p_stat == SSLEEP
			  && (p->p_flag & (SNWAKE|SNOSTOP)) == 0
			  && fsig(p))
				setrun(p);
		}
		break;
	}

	case PIOCNMAP:		/* get number of memory mappings */
	{
		int n;

		n = prnsegs(p);
		if (copyout((caddr_t) &n, cmaddr, sizeof(int)))
			error = EFAULT;
		break;
	}

	case PIOCMAP:		/* get memory map information */
	{
		register int n;
		register prmap_t *prmapp;

		/*
		 * Determine the number of segments and allocate storage
		 * to hold the array of prmap structures.  In addition
		 * to the "real" segments we need space for the zero-filled 
		 * entry which terminates the list.
		 */
		n = prnsegs(p) + 1;
		prmapp = (prmap_t *) kmem_zalloc(n*sizeof(prmap_t), KM_SLEEP);
		prgetmap(p, prmapp);
		if (copyout((caddr_t) prmapp, cmaddr, n*sizeof(prmap_t)))
			error = EFAULT;
		kmem_free((caddr_t) prmapp, n*sizeof(prmap_t));
		break;
	}

	case PIOCGFAULT:	/* get mask of traced faults */
	{
		fltset_t fltmask;

		prassignset(&fltmask, &p->p_fltmask);
		if (copyout((caddr_t) &fltmask, cmaddr, sizeof(fltmask)))
			error = EFAULT;
		break;
	}

	case PIOCSFAULT:	/* set mask of traced faults */
	{
		fltset_t fltmask;

		if (copyin(cmaddr, (caddr_t) &fltmask, sizeof(fltmask)))
			error = EFAULT;
		else {
			if (!prisempty(&fltmask))
				p->p_flag |= SPROCTR;
			else if (sigisempty(&p->p_sigmask)) {
				up = prumap(p);
				CATCH_FAULTS(CATCH_SEGU_FAULT) {
					if (up->u_systrap == 0)
						p->p_flag &= ~SPROCTR;
				}
				prunmap(p);
				if ((error = END_CATCH()) != 0)
					break;
			}
			prassignset(&p->p_fltmask, &fltmask);
		}
		break;
	}

	case PIOCCFAULT:	/* clear current fault */
		p->p_curflt = 0;
		break;

	case PIOCCRED:		/* get process credentials */
	{
		register struct cred *cp = p->p_cred;
		prcred_t prcred;

		prcred.pr_euid = cp->cr_uid;
		prcred.pr_ruid = cp->cr_ruid;
		prcred.pr_suid = cp->cr_suid;
		prcred.pr_egid = cp->cr_gid;
		prcred.pr_rgid = cp->cr_rgid;
		prcred.pr_sgid = cp->cr_sgid;
		prcred.pr_ngroups = cp->cr_ngroups;

		if (copyout((caddr_t) &prcred, cmaddr, sizeof(prcred)))
			error = EFAULT;
		break;
	}

	case PIOCGROUPS:	/* get supplementary groups */
	{
		register struct cred *cp = p->p_cred;
		int n = max(cp->cr_ngroups, 1);

		if (copyout((caddr_t) &cp->cr_groups[0], cmaddr,
		  n*sizeof(cp->cr_groups[0])))
			error = EFAULT;
		break;
	}

#ifdef i386
	case PIOCGDBREG:	/* get debug registers */
	{
		dbregset_t dbregs;

		if ((error = prgetdbregs(p, &dbregs)) != 0)
			break;
		if (copyout((caddr_t)&dbregs, cmaddr, sizeof dbregs))
			error = EFAULT;
		break;
	}

	case PIOCSDBREG:	/* set debug registers */
	{
		dbregset_t dbregs;

		if (p->p_stat != SSTOP || (p->p_flag & SPSTART))
			error = EBUSY;
		else {
			if (copyin(cmaddr, (caddr_t) &dbregs, sizeof(dbregs)))
				error = EFAULT;
			else
				error = prsetdbregs(p, &dbregs);
		}
		break;
	}
#endif /* i386 */

	}

	if (cmd != PIOCSTOP && cmd != PIOCWSTOP)
		prunlock(pnp);

	return error;
}

/*
 * Distinguish "writeable" ioctl requests from others.
 */
STATIC int
isprwrioctl(cmd)
	int cmd;
{
	switch (cmd) {
	case PIOCSTOP:
	case PIOCRUN:
	case PIOCSTRACE:
	case PIOCSSIG:
	case PIOCKILL:
	case PIOCUNKILL:
	case PIOCNICE:
	case PIOCSENTRY:
	case PIOCSEXIT:
	case PIOCSRLC:
	case PIOCRRLC:
	case PIOCSREG:
	case PIOCSFPREG:
	case PIOCSHOLD:
	case PIOCSFAULT:
	case PIOCCFAULT:
	case PIOCSFORK:
	case PIOCRFORK:
		return 1;
	}
	return 0;
}

/*
 * Apply PIOCRUN options.
 */
STATIC int
prsetrun(p, prp)
	register struct proc *p;
	register struct prrun *prp;
{
	register long flags = prp->pr_flags;
	register user_t *up;
	register int umapped = 0;
	int error, systrap;

	if (flags & (PRSABORT|PRSTEP|PRSVADDR|PRSTRACE|PRSFAULT)) {
		umapped++;
		up = prumap(p);
		CATCH_FAULTS(CATCH_SEGU_FAULT) {
			if (flags & PRSABORT)
				up->u_sysabort = 1;
			if (flags & PRSTEP)
				prstep(p, up);
			if (flags & PRSVADDR)
				prsvaddr(p, up, prp->pr_vaddr);
			systrap = up->u_systrap;
		}
		if ((error = END_CATCH()) != 0) {
			prunmap(p);
			return error;
		}
	}
	if (flags & PRSTRACE) {
		prdelset(&prp->pr_trace, SIGKILL);
		prassignset(&p->p_sigmask, &prp->pr_trace);
		if (!sigisempty(&p->p_sigmask))
			p->p_flag |= SPROCTR;
		else if (prisempty(&p->p_fltmask)) {
			if (systrap == 0)
				p->p_flag &= ~SPROCTR;
		}
	}
	if (flags & PRSFAULT) {
		prassignset(&p->p_fltmask, &prp->pr_fault);
		if (!prisempty(&p->p_fltmask))
			p->p_flag |= SPROCTR;
		else if (sigisempty(&p->p_sigmask)) {
			if (systrap == 0)
				p->p_flag &= ~SPROCTR;
		}
	}
	if (umapped)
		prunmap(p);
	if ((flags & PRCSIG) && p->p_cursig != SIGKILL) {
		/*
		 * Discard current siginfo_t, if any.
		 */
		p->p_cursig = 0;
		if (p->p_curinfo) {
			kmem_free(p->p_curinfo, sizeof(*p->p_curinfo));
			p->p_curinfo = NULL;
		}
	}
	if (flags & PRSHOLD) {
		sigutok(&prp->pr_sighold, &p->p_hold);
		sigdiffset(&p->p_hold, &cantmask);
	}
	if (flags & PRCFAULT)
		p->p_curflt = 0;
	if (flags & PRSTOP)
		p->p_flag |= SPRSTOP;
	return 0;
}
