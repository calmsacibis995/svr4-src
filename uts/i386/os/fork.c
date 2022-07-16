/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:fork.c	1.3.2.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/immu.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/map.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/fp.h"
#ifdef WEITEK
#include "sys/weitek.h"
#endif
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/acct.h"
#include "sys/tuneable.h"
#include "sys/inline.h"
#include "sys/disp.h"
#include "sys/class.h"
#include "sys/kmem.h"
#include "sys/session.h"
#include "sys/fault.h"
#include "sys/syscall.h"
#include "sys/ucontext.h"
#include "sys/procfs.h"
#include "sys/seg.h"
#include "sys/vmsystm.h"
#ifdef VPIX
#include "sys/v86.h"
#endif

#include "vm/hat.h"
#include "vm/seg.h"
#include "vm/as.h"
#include "vm/seg_u.h"

#if defined(__STDC__)
STATIC int procdup(proc_t *, proc_t *, int, int);
STATIC void setuctxt(proc_t *, user_t *);
STATIC int fork1(char *, rval_t *, int);
#else
STATIC int procdup();
STATIC void setuctxt();
STATIC int fork1();
#endif


/*
 * fork system call.
 */

fork(uap, rvp)
	char *uap;
	rval_t *rvp;
{
	return(fork1(uap, rvp, 0));
}

vfork(uap, rvp)
	char *uap;
	rval_t *rvp;
{
	return(fork1(uap, rvp, 1));
}

/* ARGSUSED */
STATIC int
fork1(uap, rvp, isvfork)
	char *uap;
	rval_t *rvp;
	int isvfork;
{
	register npcond;
	int error = 0; 
	pid_t	newpid;

	sysinfo.sysfork++;

	/*
	 * Disallow if
	 *	- no processes at all, or
	 *	- not super-user and too many procs owned, or
	 *	- not super-user and would take last slot.
	 * Check done in pid_assign().
	 */

	npcond = NP_FAILOK | (isvfork ? NP_VFORK : 0)
	  | ((u.u_cred->cr_uid && u.u_cred->cr_ruid) ? NP_NOLAST : 0);
	
	switch (newproc(npcond, &newpid, &error)) {
	case 1:	/* child -- successful newproc */
		rvp->r_val1 = u.u_procp->p_ppid;
		rvp->r_val2 = 1;	/* child */
		u.u_start = hrestime.tv_sec;
		u.u_ticks = lbolt;
		u.u_mem = rm_assize(u.u_procp->p_as);
		u.u_ior = u.u_iow = u.u_ioch = 0;
		u.u_procvirt = 0;
		u.u_uservirt = 0;
		u.u_acflag = AFORK;
		u.u_lock = 0;
		nfc_forkch(&error);	/* added for OpenNET NFA */
		break;
	case 0: /* parent */
		rvp->r_val1 = (int) newpid;
		rvp->r_val2 = 0;	/* parent */
		nfc_forkpar(&error);	/* added for OpenNET NFA */
		break;
	default:	/* couldn't fork */
		error = EAGAIN;
		break;
	}
	return error;
}

/*
 * Create a new process -- internal version of sys fork().
 *
 * This changes the new proc structure and
 * alters only the u.u_procp of its u-area.
 *
 * It returns 1 in the new process, 0 in the old.
 */

int
newproc(cond, pidp, perror)
	int cond;
	pid_t *pidp;
	int *perror;
{
	extern void shmfork();
	extern int xsemfork();
	extern int xsdfork();

	register proc_t *pp, *cp; 
	register int n;
	proc_t *cpp;
	pid_t newpid;
	file_t *fp;

#ifdef VPIX
	extern	char v86procflag;
#endif

	if ((newpid = pid_assign(cond, &cpp)) == -1) {
		/* no proc table entry is available */
		if (cond & NP_FAILOK) {
			return -1; 	/* out of memory or proc slot */
		} else
			cmn_err(CE_PANIC, "newproc - no procs");
		
	} 
	
	/*
	 * Make proc entry for new proc.
	 */
	
	cp = cpp;
	pp = u.u_procp;
	cp->p_cstime = 0;
	cp->p_stime = 0;
	cp->p_cutime = 0;
	cp->p_utime = 0;
	cp->p_italarm[0] = NULL;
	cp->p_italarm[1] = NULL;
	cp->p_uid = pp->p_uid;
	cp->p_cred = pp->p_cred;
	crhold(pp->p_cred);
#ifdef VPIX
	cp->p_v86 = NULL;
#endif
#ifdef MERGE386
	cp->p_vm86p = NULL;
#endif	/* MERGE386 */
	cp->p_ignore = pp->p_ignore;
	cp->p_hold = pp->p_hold;
	cp->p_siginfo = pp->p_siginfo;
	cp->p_stat = SIDL;
	cp->p_clktim = 0;
	cp->p_flag = SLOAD | (pp->p_flag & (SJCTL|SNOWAIT));

	/* Enforce per-user licensing */
	if (!(cp->p_user_license = (pp->p_user_license & PU_LIM_OK)) &&
	    cp->p_cred->cr_uid && cp->p_cred->cr_ruid &&
	    cttydev(pp) != NODEV &&
	    (pp->p_pgrp == 0)) {
		if (enable_user_alloc(EUA_FORK) != 0) { /* Can we register it? */
			cmn_err (CE_NOTE, 
			"Un-registered user attempted to fork, uid = %d, pid = %d",
				cp->p_cred->cr_uid, pp->p_pid);
			crfree(cp->p_cred);
			pid_exit(cp);	/* free the proc table entry */
			return -1;
		}
		cp->p_user_license |= PU_LOGIN_PROC|PU_LIM_OK;
	}

	cp->p_sessp = pp->p_sessp;
	SESS_HOLD(pp->p_sessp);
	pgjoin(cp, pp->p_pgidp);

	if (cond & (NP_SYSPROC | NP_INIT)) {
		cp->p_exec = NULL;
		cp->p_flag |= (SSYS | SLOCK);
	} else {
		cp->p_exec = pp->p_exec;
	}

	cp->p_brkbase = pp->p_brkbase;
	cp->p_brksize = pp->p_brksize;
	cp->p_stkbase = pp->p_stkbase;
	cp->p_stksize = pp->p_stksize;
	cp->p_swlocks = 0;
	cp->p_segacct = 0;
	if (cond & NP_VFORK)
		cp->p_flag |= SVFORK;
	cp->p_pid = newpid;
	if (newpid <= SHRT_MAX)
		cp->p_opid = (o_pid_t)newpid;
	else
		cp->p_opid = (o_pid_t)NOPID;
	cp->p_epid = newpid;
	cp->p_ppid = pp->p_pid;
	cp->p_oppid = pp->p_opid;
	cp->p_cpu = 0;
	cp->p_pri = pp->p_pri;

	/*
	 * If inherit-on-fork, copy /proc tracing flags to child.
	 * New system processes never inherit tracing flags.
	 */
	if ((pp->p_flag & (SPROCTR|SPRFORK)) == (SPROCTR|SPRFORK)
	  && !(cond & (NP_SYSPROC|NP_INIT))) {
		cp->p_flag |= (SPROCTR|SPRFORK);
		cp->p_sigmask = pp->p_sigmask;
		cp->p_fltmask = pp->p_fltmask;
	} else {
		sigemptyset(&cp->p_sigmask);
		premptyset(&cp->p_fltmask);
		/*
		 * Syscall tracing flags are in the u-block.
		 * They are cleared when the child begins execution, below.
		 */
	}
	cp->p_cid = pp->p_cid;
	cp->p_clfuncs = pp->p_clfuncs;
	cp->p_usize = pp->p_usize;
	if (CL_FORK(pp, pp->p_clproc, cp, &cp->p_stat, &cp->p_pri,
                    &cp->p_flag, &cp->p_cred, &cp->p_clproc)) {
		crfree(cp->p_cred);
		pid_exit(cp);	/* free the proc table entry */
		return -1;
	}

	/*
	** Initialize child process' async request count
	*/
	cp->p_aiocount = 0;
	cp->p_aiowcnt = 0;

#ifdef ASYNCIO
	/* 
	 * Initialize child process' raw disk async I/O count
	 */
	cp->p_raiocnt = 0;
#endif /* ASYNC IO */

	/*
	 * Link up to parent-child-sibling chain.  No need to lock
	 * in general since only a call to freeproc() (done by the
	 * same parent as newproc()) diddles with the child chain.
	 */
	cp->p_sibling = pp->p_child;
	cp->p_parent = pp;
	pp->p_child = cp;

	cp->p_nextorph = pp->p_orphan;
	cp->p_nextofkin = pp;
	pp->p_orphan = cp;

	cp->p_sysid = pp->p_sysid;	/* RFS HOOK */

	/*
	 * Make duplicate entries where needed.
	 */
	for (n = 0; n < u.u_nofiles; n++) {
		if (getf(n, &fp) == 0)
			fp->f_count++;
	}

	VN_HOLD(u.u_cdir);
	if (u.u_rdir)
		VN_HOLD(u.u_rdir);
	
	/* 
	 * save the floating point state for this process
	 */
	if (pp == fp_proc)
		fpsave();
#ifdef WEITEK
	if (pp == weitek_proc) {
		weitek_save();
	}
#endif

	/*
	 * Copy process.
	 */
	switch (procdup(cp, pp, (cond & (NP_VFORK|NP_SHARE)),
				(cond & NP_SYSPROC))) {
	case 0:
		/* Successful copy */
		break;
	case -1:
		if (!(cond & NP_FAILOK))
			cmn_err(CE_PANIC, "newproc - fork failed\n");

		/* Reset all incremented counts. */

		pexit();
		CL_EXITCLASS(cp, cp->p_clproc);

		{
		/*
		 * Clean up parent-child-sibling pointers.  No lock
		 * necessary since nobody else could be diddling with
		 * them here.
		 */
		/* The parent could have been sleeping in procdup and
		 * its children could have exit()'ed.  Exit() would  
		 *  then change the parent's p_orphan list and or 
		 * p_child list (for the init process).  So do not
		 * assume that the new child is on the top of the
		 * list but search the list like freeproc().
		 */

		/*
		pp->p_child = cp->p_sibling;
		pp->p_orphan = cp->p_nextorph;
		*/
			proc_t *p, *q;

			p = cp;
			if ((q = p->p_nextofkin)->p_orphan == p)
				q->p_orphan = p->p_nextorph;
			else {
				for (q = q->p_orphan; q; q = q->p_nextorph)
					if (q->p_nextorph == p)
		                                break;
				ASSERT(q && q->p_nextorph == p);
				q->p_nextorph = p->p_nextorph;
			}
		
			if ((q = p->p_parent)->p_child == p)
				q->p_child = p->p_sibling;
			else {
				for (q = q->p_child; q; q = q->p_sibling)
					if (q->p_sibling == p)
						break;
				ASSERT(q && q->p_sibling == p);
				q->p_sibling = p->p_sibling;
			}
		}

		crfree(cp->p_cred);
		pid_exit(cp);
		if (pidp)
			*pidp = -1;
		return -1;
	case 1:
		/* Child resumes here */
		if ((u.u_procp->p_flag & SPROCTR) == 0) {
			/*
			 * /proc tracing flags have not been
			 * inherited; clear syscall flags.
			 */
			u.u_systrap = 0;
			premptyset(&u.u_entrymask);
			premptyset(&u.u_exitmask);
		}
		if (pidp)
			*pidp = cp->p_ppid;

		if (xsemfork())
			return -1;
		if (pp->p_sdp)
			*perror = xsdfork(cp, pp);

		return 1;
	}

	if (u.u_nshmseg)
		shmfork(pp, cp);

	cp->p_stat=SRUN;

	/*
	 * If we just created init process put it in the
	 * correct scheduling class.
	 */
	if (cond & NP_INIT) {
		if (getcid(initclass, &cp->p_cid) || cp->p_cid <= 0) {
			cmn_err(CE_PANIC,
	  "Illegal or unconfigured class (%s) specified for init process.\n\
Change INITCLASS configuration parameter.", initclass);
		}
                if (CL_ENTERCLASS(&class[cp->p_cid], NULL, cp, &cp->p_stat,
                    &cp->p_pri, &cp->p_flag, &cp->p_cred,
                    &cp->p_clproc, NULL, NULL)) {
			cmn_err(CE_PANIC,
	  "Init process cannot enter %s scheduling class.\n\
Verify that %s class is properly configured.", initclass, initclass);
		}
		cp->p_clfuncs = class[cp->p_cid].cl_funcs;
		/*
		 * We call CL_FORKRET with a NULL parent pointer
		 * in this special case because the parent may
		 * be in a different class from the function we are
		 * calling and its class specific data would be
		 * meaningless to the function.
		 */
		CL_FORKRET(cp, cp->p_clproc, NULL);
	} else {
		CL_FORKRET(cp, cp->p_clproc, pp->p_clproc);
	}

	if (pidp)
		*pidp = cp->p_pid;	/* parent returns pid of child */

	if (cp->p_exec)
		VN_HOLD(cp->p_exec);
	return 0;
}


/*
 * Create a duplicate copy of a process.
 */
STATIC int
procdup(cp, pp, isvfork, no_swap)
	proc_t	*cp;
	proc_t	*pp;
	register int	isvfork;
	int		no_swap;
{
	register user_t		*uservad;
	register int		*sp;
	struct	 tss386		*tp;
	caddr_t			addr;
	extern	 pte_t		kpd0[];
	extern	 		resume();

	extern void 		ev_exit();

	register ebx, edi, esi;		/* ensure registers are saved */

	/*
	 * We must save all the registers when we
	 * enter procdup if fork is to work correctly.
	 */
	ebx = edi = esi = 1;		/* very machine dependent */

	/*
	 * Duplicate address space of current process. For
	 * vfork we just share the address space structure.
	 */
	if (pp->p_as != NULL) {
                /*
                 * Duplicate address space of current process. For
                 * vfork we just share the address space structure.
                 */
                if (isvfork) {
                        cp->p_as = pp->p_as;
                } else {
                        cp->p_as = as_dup(pp->p_as);
                        if (cp->p_as == NULL) {
                                return -1;
                        }
                }
        }

	/*
	 * Tell the general events VFS that the process forked.
	 * If this fails, then we must fail the fork.
	 */
	if (ev_config() && ev_fork(pp, cp) != 0) {
		if (isvfork == 0 && cp->p_as != NULL)
			as_free(cp->p_as);
		cp->p_as = NULL;
		return -1;
	}

	/* LINTED */
	if ((cp->p_segu = (struct seguser *)segu_get(cp, no_swap)) == NULL) {
		ev_exit(cp, 0);
		if (isvfork == 0 && cp->p_as != NULL)
			as_free(cp->p_as);
		cp->p_as = NULL;
		return -1;
	}
	uservad = PTOU(cp);
	cp->p_ldt = (caddr_t)uservad + (pp->p_ldt - (caddr_t)PTOU(pp));

	/*
	 * Assumption that nothing past this point fails.
	 * Otherwise, segu_release() would have to handle 
	 * being called on a process that was not ONPROC.
	 */

	/*
	 * Setup child u-block.
	 */
	setuctxt(cp, uservad);

#ifdef ASYNCIO
	/*
	 * Children don't inherit RAIO locked memory
	 */
	uservad->u_raioaddr = 0;
	uservad->u_raiosize = 0;
#endif /* ASYNC IO */

	/*
	 * Set up values for child to return to "newproc".
	 */
 	tp = (struct tss386 *)((char *)uservad + (uint)uservad->u_tss -
 				UVUBLK);

	/*
	 * Set up descriptor's for the child process.
	 */
 	setdscrbase(&uservad->u_tss_desc, (uint)tp);
 	/* u_sztss is constant, so we can inherit parent's limit
		setdscrlim(&uservad->u_tss_desc, tp->u_sztss - 1); */
	setdscrbase(&uservad->u_ldt_desc, cp->p_ldt);
	/* ldt will be same size as parent's, so we can inherit parent's limit
		setdscrlim(&uservad->u_ldt_desc,
			(uservad->u_ldtlimit + 1) * sizeof(struct seg_desc) - 1); */

	/*
	 * this is really machine dependent - 
	 * set the pc to return to our caller
	 */

 	/* Set up the child's regs in the tss so that the saved
 	 * register variables, eip, etc. get restored. The
 	 * function prolog saves the old frame pointer, allocates
 	 * space for locals and then saves the previous register
 	 * variables. This means that we have no way of accessing
 	 * the saved reg vars off the current frame pointer
 	 * since we don't know how many locals were allocated.
 	 * Hence the following - we know that the old
 	 * reg vars are currently at the top of the stack since
 	 * we are in the outermost block of the function. Cpreg
 	 * can now access them off its argument.
	 */

 	sp = (int *)(&cp - 1);		/* sp points to <ret addr> */
 	tp->t_link = 0;
 	tp->t_eip = (unsigned long) resume;
 	tp->t_eflags = 2;		/* no interrupts */
 	tp->t_eax = 1;			/* child */
 	tp->t_edx = (unsigned long) UTSSSEL;  /* task for child is UTSSSEL */
 	tp->t_esp = (unsigned long) sp;
 	tp->t_ebp = sp[-1];
 	tp->t_es = KDSSEL;
 	tp->t_cs = KCSSEL;
 	tp->t_ss = KDSSEL;
 	tp->t_ds = KDSSEL;
 	tp->t_fs = 0;
 	tp->t_gs = 0;
 	tp->t_ldt = LDTSEL;

 	cpreg(tp);
 
	/*
	 * Put the child on the run queue.
	 */
	cp->p_flag |= SULOAD;
	return 0;
}

/*
 * Save old register variables in tss. Much magic takes
 * place here. Cpreg knows that the reg vars are stored
 * immediately below the argument on the stack. Read the
 * long comment above.
 */
 
cpreg(tp)
struct tss386 *tp;
{
	tp->t_ebx = (unsigned long) *(&tp + 1);
 	tp->t_esi = (unsigned long) *(&tp + 2);
 	tp->t_edi = (unsigned long) *(&tp + 3);
}
 
/*
 * Setup context of child process.
 */

STATIC void
setuctxt(p, up)
	proc_t *p;	/* child proc pointer */
	register user_t *up;	/* child u-block pointer */
{
	register struct ufchunk *pufp, *cufp, *tufp;

 	ASSERT((MINUSIZE <= p->p_usize) && (p->p_usize <= MAXUSIZE));

	/* Copy u-block.  XXX - The amount to copy is machine dependent */

	bcopy((caddr_t)&u, (caddr_t)up, ctob(p->p_usize));
	up->u_procp = p;
	pufp = u.u_flist.uf_next;
	cufp = &up->u_flist;
	cufp->uf_next = NULL;
	while (pufp) {
		tufp = (struct ufchunk *)kmem_alloc(sizeof(struct ufchunk), KM_SLEEP);
		*tufp = *pufp;
		tufp->uf_next = NULL;
		cufp->uf_next = tufp;
		cufp = tufp;
		pufp = pufp->uf_next;
	}
}

/*
 * Release virtual memory.
 * Called by exit and getxfile (via execve).
 */

void
relvm(p)
	register proc_t *p;	/* process exiting or exec'ing */
{
	if ((p->p_flag & SVFORK) == 0) {
		if (p->p_as != NULL) {
			as_free(p->p_as);
			p->p_as = NULL;
		}
	} else {
		p->p_flag &= ~SVFORK;	/* no longer a vforked process */
		p->p_as = NULL;		/* no longer using parent's adr space */
		wakeprocs((caddr_t)p, PRMPT);	/* wake up parent */
		while ((p->p_flag & SVFDONE) == 0) {	/* wait for parent */
			(void) sleep((caddr_t)p, PZERO - 1);
		}
		p->p_flag &= ~SVFDONE;	/* continue on to die or exec */
	}
}

/*
 * Wait for child to exec or exit.
 * Called by parent of vfork'ed process.
 */

void
vfwait(pid)
	pid_t pid;
{
	register proc_t *pp = u.u_procp;
	register proc_t *cp = prfind(pid);

	ASSERT(cp != NULL && cp->p_parent == pp);

	/*
	 * Wait for child to exec or exit.
	 */
	while (cp->p_flag & SVFORK)
		(void) sleep((caddr_t)cp, PZERO-1);

	/*
	 * Copy back sizes to parent; child may have grown.
	 * We hope that this is the only info outside the
	 * "as" struct that needs to be shared like this!
	 */
	if (pp->p_brkbase == cp->p_brkbase)
		pp->p_brksize = cp->p_brksize;
	if (pp->p_stkbase == cp->p_stkbase)
		pp->p_stksize = cp->p_stksize;

	/*
	 * Wake up child, send it on its way.
	 */
	cp->p_flag |= SVFDONE;
	wakeprocs((caddr_t)cp, PRMPT);
}
