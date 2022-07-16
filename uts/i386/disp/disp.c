/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-dsp:disp.c	1.4.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/seg.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/proc.h"
#include "sys/procset.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/priocntl.h"
#include "sys/disp.h"
#include "sys/class.h"
#include "sys/bitmap.h"
#include "sys/kmem.h"
#include "sys/regset.h"

extern void	runqueues();
extern char	qrunflag;
#ifdef i386
extern int	do386b1;	/* enable B1 stepping workarounds */
#endif

#ifdef DEBUG
int idlecntdown = 60;
#endif

int		runrun;		/* scheduling flag - set to cause preemption */
int		kprunrun;	/* set to preempt at next krnl prmption point */
int		npwakecnt;	/* count of npwakeups since last pswtch() */
proc_t		*curproc;	/* currently running process */
int		curpri;		/* priority of current process */
int		maxrunpri;	/* priority of highest priority active queue */
int		idleswtch;	/* flag set while idle in pswtch() */

STATIC ulong	*dqactmap;	/* bitmap to keep track of active disp queues */
STATIC dispq_t	*dispq;		/* ptr to array of disp queues indexed by pri */
STATIC int	srunprocs;	/* total number of loaded, runnable procs */


#ifdef KPERF
asm int 
geteip()
{
	leal 0(%esp), %eax
}
#endif /* KPERF */



/*
 * Scheduler Initialization
 */
void
dispinit()
{
	register id_t	cid;
	register int	maxglobpri;
	int		cl_maxglobpri;

	maxglobpri = -1;

	/*
	 * Call the class specific initialization functions. We pass the size
	 * of a class specific parameter buffer to each of the initialization
	 * functions to try to catch problems with backward compatibility of
	 * class modules.  For example a new class module running on an old
	 * system which didn't provide sufficiently large parameter buffers
	 * would be bad news.  Class initialization modules can check for
	 * this and take action if they detect a problem.
	 */
	for (cid = 0; cid < nclass; cid++) {
		(*class[cid].cl_init)(cid, PC_CLPARMSZ, &class[cid].cl_funcs,
		    &cl_maxglobpri);
		if (cl_maxglobpri > maxglobpri)
			maxglobpri = cl_maxglobpri;
	}

	v.v_nglobpris = maxglobpri + 1;

	/*
	 * Allocate memory for the dispatcher queue headers
	 * and the active queue bitmap.
	 */
	if ((dispq = (dispq_t *)kmem_zalloc(v.v_nglobpris * sizeof(dispq_t),
	    KM_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,
		    "Can't allocate memory for dispatcher queues.");

	if ((dqactmap = (ulong *)kmem_zalloc(((v.v_nglobpris / BT_NBIPUL) + 1) *
	    sizeof(long), KM_NOSLEEP)) == NULL)
		cmn_err(CE_PANIC,
		    "Can't allocate memory for dispq active map.");

	srunprocs = 0;
	maxrunpri = -1;
}


/*
 * Preempt the currently running process in favor of the highest
 * priority process.  The class of the current process controls
 * where it goes on the dispatcher queues.
 */
void
preempt()
{
#ifdef i386
	if (do386b1 && u.u_ar0) {
		/* Workaround for 80386 B1 stepping Errata 17.
		   This bug can cause a process to hang if a floating-
		   point instruction crosses a page boundary and the
		   second page is not present. */

		u_int	cs = u.u_ar0[CS] & 0xFFFF;
		u_int	lip;	/* linear address equivalent of CS:IP */

		if (u.u_escbug) {
			/* If u.u_escbug is set, we must have slept or
			   hit another preemption point before returning
			   to the user.  Just go back and try again. */
			goto escbug;
		}

		/* Convert CS:IP to a linear address */
		lip = u.u_ar0[EIP];
		if (u.u_ar0[EFL] & PS_VM)	/* virtual 8086 mode */
			lip = (cs << 4) + (lip & 0xFFFF);
		else if (!USERMODE(cs))		/* kernel mode */
			lip = 0;	/* do nothing */
		else if (cs != USER_CS) {	/* 286 user CS */
			char	*dp;	/* ptr to LDT code descriptor */
			dp = (char *)
				(((struct dscr *)(u.u_procp->p_ldt))
				       + seltoi(cs));
			lip = ((dp[7] << 24) |
				(*(int *)&dp[2] & 0x00FFFFFF))
				      + (lip & 0xFFFF);
		}
		/* See if the linear address equivalent of CS:IP is in
		 * the last 8 bytes of its page, the last byte is an
		 * ESC opcode, and all bytes between CS:IP and the ESC
		 * are valid prefixes.
		 */
		if ((lip & 0xFFF) >= 0xFF8 &&
		    (fubyte((char *)(lip | 0xFFF)) & 0xF8) == 0xD8) {
			u_int	op;

			while ((lip & 0xFFF) < 0xFFF) {
				op = fubyte((char *)lip);
				if (op == -1)
					break;
				if ((op & 0xE7) != 0x26 &&
				    (op < 0x64 || op > 0x67))
					break;
				lip++;
			}
			if ((lip & 0xFFF) == 0xFFF) {
				/* All conditions met; we're probably
				   hung due to Errata 17.  Fault the
				   next page in, and let the proc run
				   for one more instruction. */
				u.u_escbug = (char *)(lip + 1);
escbug:
				if (fubyte(u.u_escbug) == -1)
					u.u_escbug = NULL;
				else {
					u.u_ar0[EFL] |= PS_T;
					runrun = kprunrun = npwakecnt = 0;
					return;
				}
			}
		}
	}
#endif /* i386 */

	CL_PREEMPT(u.u_procp, u.u_procp->p_clproc);
	swtch();
}

void
pswtch()
{
	register proc_t		*pp;
	register proc_t		*rp;
	register dispq_t	*dq;
	register int		maxrunword;
	int	i;

	extern int slice_size;
#ifdef VPIX
	extern int v86_slice_size;
	extern char v86_slice_start;
	extern int num_v86procs;
#endif

	sysinfo.pswitch++;
	old_curproc = pp = curproc;

	switch (pp->p_stat) {
	case SZOMB:

		/*
		 * Save the current process in oldproc.
		 * The ublock, its page tables and directory
		 * will be freed in the context of the 
		 * process, when we return from pswitch.
		 */
		oldproc = pp;
		/*
		 *  386 specific: Cannot free the proc structure now. We
		 *  		  need it for segu_release() to get the
		 *		  right amount of pages allocated for the
		 *		  ublock.
		 *
		 * segu_release(pp);
		 * if (pp->p_parent->p_flag & SNOWAIT)
		 *	freeproc(pp);
		 *
		 */
		break;

	case SONPROC:
		ASSERT(pp->p_wchan == 0);

		pp->p_stat = SRUN;
		break;
	}

	/*
	 * Find the highest priority loaded, runnable process.
	 */
	splhi();
	while (maxrunpri == -1) {
		if (qrunflag) {
			runqueues();
			continue; /* might have made someone runnable */
		}
		curpri = 0;
		curproc = proc_sched;
#ifdef KPERF
		if (kpftraceflg) {
			/* asm(" MOVAW 0(%pc),Kpc "); */
			Kpc = geteip();
			kperf_write(KPT_IDLE, Kpc, curproc);
		}
#endif	/* KPERF */
		idleswtch++;
		idle();
		splhi();
		idleswtch = 0;
	}
	runrun = kprunrun = npwakecnt = 0;
	dq = &dispq[maxrunpri];
	pp = dq->dq_first;
	ASSERT(pp != NULL && pp->p_stat == SRUN);
	while ((pp->p_flag & (SLOAD|SPROCIO)) != SLOAD
	  && (pp->p_flag & SSYS) == 0) {
		rp = pp;
		pp = pp->p_link;
		ASSERT(pp != NULL && pp->p_stat == SRUN);
	}

	/*
	 * Found it so remove it from queue.
	 */
	if (pp == dq->dq_first) {

		/*
		 * We are dequeuing the first proc on the list.
		 * Check for creating a null list.
		 */
		if ((dq->dq_first = pp->p_link) == NULL)
			dq->dq_last = NULL;
	} else {

		/*
		 * We are not dequeuing the first proc on the list.
		 * Check for dequeuing the last one.
		 */
		if ((rp->p_link = pp->p_link) == NULL)
			dq->dq_last = rp;
	}
	srunprocs--;
	if (--dq->dq_sruncnt == 0) {
		maxrunword = maxrunpri >> BT_ULSHIFT;
		dqactmap[maxrunword] &= ~BT_BIW(maxrunpri);
		if (srunprocs == 0)
			maxrunpri = -1;
		else
			bt_gethighbit(dqactmap, maxrunword, &maxrunpri);
	}

	pp->p_stat = SONPROC;	/* process pp will be running */
#ifdef  VPIX
	if (pp->p_v86) {         /* Small slice for dual mode process */
		v86_slice_start++;
		}
#endif
	curpri = pp->p_pri;
	curproc = pp;
#ifdef DEBUG
	if (pp != proc_bdflush)		/* skip bdflush */
		idlecntdown = 60;
#endif
	
	/*
	 * Switch context only if really process switching.
	 */
	if (pp != old_curproc) {
		/*
		 * Kernel stack is expected NOT to overflow into
		 * the system-wide non-sharable page when
		 * context switch. If this situation is detected,
		 * undetermined system behavior will occur. Panic
		 * the system now.
		 */
		asm("cmpl	$0xE0000000, %esp");
		asm("jb		panic_stk");

		/* XENIX Support */
		/*
		 * Save XENIX shared data context.
		 */
		if ((BADVISE_XSDSWTCH) && (old_curproc->p_sdp != NULL))
			xsdswtch(0);
		/* End XENIX Support */

		/*
		 * Map tss of new process by JTSSEL
		 */
		mapnewtss(pp);
	}

#ifdef KPERF
	if (kpftraceflg) {
		/* asm(" MOVAW 0(%pc),Kpc "); */
		Kpc = geteip();
		kperf_write(KPT_PSWTCH, Kpc, curproc);
	}
#endif	/* KPERF */

	(void) spl0();
}

/*
 * Panic: process switch is occuring - but the current context
 * to be saved is larger than the 1st page in the ublock allocated
 * for the kernel stack and the floating point stuff. The context
 * overflows into the non-sharable system-wide page, we will be
 * saving/retrieving incomplete contexts... Panic the system now.
 */
pswtch_panic()
{
asm("panic_stk:");
	cmn_err(CE_PANIC, "pswtch: kernel stack overflows during cntxswtch\n");
}


/*
 * Put the specified process on the back of the dispatcher
 * queue corresponding to its current priority.
 */
void
setbackdq(pp)
register proc_t	*pp;
{
	register dispq_t	*dq;
#ifdef DEBUG
	register proc_t		*rp;
#endif
	register int		ppri;
	register int		oldlvl;

	ASSERT(pp->p_stat == SRUN || pp->p_stat == SONPROC);

	oldlvl = splhi();


#ifdef DEBUG
	for (dq = &dispq[0] ; dq < &dispq[v.v_nglobpris] ; dq++) {
		ASSERT(dq->dq_last == NULL || dq->dq_last->p_link == NULL);
		for (rp = dq->dq_first; rp; rp = rp->p_link) {
			if (pp == rp)
				cmn_err(CE_PANIC, "setbackdq - proc on q.");
		}
	}
#endif

	ppri = pp->p_pri;
	dq = &dispq[ppri];
	if (dq->dq_last == NULL) {
		ASSERT(dq->dq_first == NULL);
		pp->p_link = NULL;
		dq->dq_first = dq->dq_last = pp;
	} else {
		ASSERT(dq->dq_first != NULL);
		pp->p_link = NULL;
		dq->dq_last->p_link = pp;
		dq->dq_last = pp;
	}

	if ((pp->p_flag & (SLOAD|SPROCIO)) == SLOAD || (pp->p_flag & SSYS)) {
		srunprocs++;
		if (++dq->dq_sruncnt == 1) {
			BT_SET(dqactmap, ppri);
			if (ppri > maxrunpri)
				maxrunpri = ppri;
		}
	}
	splx(oldlvl);
}


/*
 * Put the specified process on the front of the dispatcher
 * queue corresponding to its current priority.
 */
void
setfrontdq(pp)
register proc_t	*pp;
{
	register dispq_t	*dq;
#ifdef DEBUG
	register proc_t		*rp;
#endif
	register int		ppri;
	register int		oldlvl;

	ASSERT(pp->p_stat == SRUN || pp->p_stat == SONPROC);

	oldlvl = splhi();

#ifdef DEBUG
	for (dq = &dispq[0] ; dq < &dispq[v.v_nglobpris] ; dq++) {
		ASSERT(dq->dq_last == NULL || dq->dq_last->p_link == NULL);
		for (rp = dq->dq_first; rp; rp = rp->p_link) {
			if (pp == rp)
				cmn_err(CE_PANIC, "setfrontdq - proc on q.");
		}
	}
#endif

	ppri = pp->p_pri;
	dq = &dispq[ppri];
	if (dq->dq_first == NULL) {
		ASSERT(dq->dq_last == NULL);
		pp->p_link = NULL;
		dq->dq_first = dq->dq_last = pp;
	} else {
		ASSERT(dq->dq_last != NULL);
		pp->p_link = dq->dq_first;
		dq->dq_first = pp;
	}

	if ((pp->p_flag & (SLOAD|SPROCIO)) == SLOAD || (pp->p_flag & SSYS)) {
		srunprocs++;
		if (++dq->dq_sruncnt == 1) {
			BT_SET(dqactmap, ppri);
			if (ppri > maxrunpri)
				maxrunpri = ppri;
		}
	}
	splx(oldlvl);
}


/*
 * Remove a process from the dispatcher queue if it is on it.
 * It is not an error if it is not found but we return whether
 * or not it was found in case the caller wants to check.
 */
boolean_t
dispdeq(pp)
register proc_t		*pp;
{
	register dispq_t	*dq;
	register proc_t		*rp;
	register proc_t		*prp;
	register int		ppri;
	int			oldlvl;

	oldlvl = splhi();
	ppri = pp->p_pri;
	dq = &dispq[ppri];
	rp = dq->dq_first;
	prp = NULL;

	ASSERT(dq->dq_last == NULL  ||  dq->dq_last->p_link == NULL);

	while (rp != pp && rp != NULL) {
		prp = rp;
		rp = prp->p_link;
	}
	if (rp == NULL) {

#ifdef DEBUG
		for(dq = &dispq[0] ; dq < &dispq[v.v_nglobpris]; dq++) {
			ASSERT(dq->dq_last == NULL
			  || dq->dq_last->p_link == NULL);
			for(rp = dq->dq_first; rp; rp = rp->p_link) {
				if (pp == rp)
					cmn_err(CE_PANIC,
					"dispdeq - proc %x on wrong q %x.",
						pp, dq);
			}
		}
#endif
		splx(oldlvl);
		return(B_FALSE);
	}

	/*
	 * Found it so remove it from queue.
	 */
	ASSERT(dq - dispq == rp->p_pri);

	if (prp == NULL) {

		/*
		 * We are dequeueing the first proc on the list.
		 * Check for creating a null list.
		 */
		if ((dq->dq_first = rp->p_link) == NULL)
			dq->dq_last = NULL;
	} else {

		/*
		 * We are not dequeueing the first proc on the list.
		 * Check for dequeueing the last one.
		 */
		if ((prp->p_link = rp->p_link) == NULL)
			dq->dq_last = prp;
	}

	if ((rp->p_flag & (SLOAD|SPROCIO)) == SLOAD || (rp->p_flag & SSYS)) {
		srunprocs--;
		if (--dq->dq_sruncnt == 0) {
			dqactmap[ppri >> BT_ULSHIFT] &= ~BT_BIW(ppri);
			if (srunprocs == 0)
				maxrunpri = -1;
			else if (ppri == maxrunpri)
				bt_gethighbit(dqactmap, maxrunpri >> BT_ULSHIFT,
				    &maxrunpri);
		}
	}

	splx(oldlvl);
	return(B_TRUE);
}

/*
 * dq_sruninc and dq_srundec are public functions for
 * incrementing/decrementing the sruncnts when a process on
 * a dispatcher queue is made schedulable/unschedulable by
 * resetting the SLOAD or SPROCIO flags.
 * The caller MUST set splhi() such that the operation which changes
 * the flag, the operation that checks the status of the process to
 * determine if it's on a disp queue AND the call to this function
 * are one atomic operation with respect to interrupts.
 * We don't set splhi() only because we trust that the caller has.
 */
void
dq_sruninc(pri)
int	pri;
{
	register dispq_t	*dq;

	srunprocs++;
	dq = &dispq[pri];
	if (++dq->dq_sruncnt == 1) {
		BT_SET(dqactmap, pri);
		if (pri > maxrunpri)
			maxrunpri = pri;
	}
}


/*
 * See comment on calling conventions above.
 */
void
dq_srundec(pri)
int	pri;
{
	register dispq_t	*dq;

	srunprocs--;
	dq = &dispq[pri];
	if (--dq->dq_sruncnt == 0) {
		dqactmap[pri >> BT_ULSHIFT] &= ~BT_BIW(pri);
		if (srunprocs == 0)
			maxrunpri = -1;
		else if (pri == maxrunpri)
			bt_gethighbit(dqactmap, maxrunpri >> BT_ULSHIFT,
			    &maxrunpri);
	}
}

/*
 * Get class ID given class name.
 */
int
getcid(clname, cidp)
char	*clname;
id_t	*cidp;
{
	register class_t	*clp;

	for (clp = &class[0]; clp < &class[nclass]; clp++) {
		if (strcmp(clp->cl_name, clname) == 0) {
			*cidp = clp - &class[0];
			return(0);
		}
	}
	return(EINVAL);
}


/*
 * Get the global scheduling priority associated with a set of
 * scheduling parameters.  The global priority is returned
 * in *globprip.  As you can see the class specific code does
 * the work.  This function simply provides a class independent
 * interface.
 */
void
getglobpri(parmsp, globprip)
pcparms_t	*parmsp;
int		*globprip;
{
	CL_GETGLOBPRI(&class[parmsp->pc_cid], parmsp->pc_clparms, globprip);
}


/*
 * Get the scheduling parameters of the process pointed to by
 * pp into the buffer pointed to by parmsp.
 */
void
parmsget(pp, parmsp)
proc_t		*pp;
pcparms_t	*parmsp;
{
	parmsp->pc_cid = pp->p_cid;
	CL_PARMSGET(pp, pp->p_clproc, parmsp->pc_clparms);
}


/*
 * Check the validity of the scheduling parameters in the buffer
 * pointed to by parmsp. If our caller passes us non-NULL process
 * pointers we are also being asked to verify that the requesting
 * process (pointed to by reqpp) has the necessary permissions to
 * impose these parameters on the target process (pointed to by
 * targpp).
 * We check validity before permissions because we assume the user
 * is more interested in finding out about invalid parms than a
 * permissions problem.
 * Note that the format of the parameters may be changed by class
 * specific code which we call.
 */
int
parmsin(parmsp, reqpp, targpp)
register pcparms_t	*parmsp;
register proc_t		*reqpp;
register proc_t		*targpp;
{
	register int		error;
	id_t			reqpcid;
	register cred_t		*reqpcredp;
	id_t			targpcid;
	register cred_t		*targpcredp;
	register caddr_t	targpclpp;

	if (parmsp->pc_cid >= nclass || parmsp->pc_cid < 1)
		return(EINVAL);

	if (reqpp != NULL && targpp != NULL) {
		reqpcid = reqpp->p_cid;
		reqpcredp = reqpp->p_cred;
		targpcid = targpp->p_cid;
		targpcredp = targpp->p_cred;
		targpclpp = targpp->p_clproc;
	} else {
		reqpcredp = targpcredp = NULL;
		targpclpp = NULL;
	}

	/*
	 * Call the class specific routine to validate class
	 * specific parameters.  Note that the data pointed to
	 * by targpclpp is only meaningful to the class specific
	 * function if the target process belongs to the class of
	 * the function.
	 */
	error = CL_PARMSIN(&class[parmsp->pc_cid], parmsp->pc_clparms,
		reqpcid, reqpcredp, targpcid, targpcredp, targpclpp);
	if (error)
		return(error);

	if (reqpcredp != NULL)
		/*
		 * Check the basic permissions required for all classes.
		 */
		if (!hasprocperm(targpcredp, reqpcredp))
			return(EPERM);
	return(0);
}

	
/*
 * Call the class specific code to do the required processing
 * and permissions checks before the scheduling parameters
 * are copied out to the user.
 * Note that the format of the parameters may be changed by the
 * class specific code.
 */
int
parmsout(parmsp, reqpp, targpp)
register pcparms_t	*parmsp;
register proc_t		*reqpp;
register proc_t		*targpp;
{
	register int	error;
	id_t		reqpcid;
	register cred_t	*reqpcredp;
	register cred_t	*targpcredp;

	reqpcid = reqpp->p_cid;
	reqpcredp = reqpp->p_cred;
	targpcredp = targpp->p_cred;

	error = CL_PARMSOUT(&class[parmsp->pc_cid], parmsp->pc_clparms,
		reqpcid, reqpcredp, targpcredp);

	return(error);
}


/*
 * Set the scheduling parameters of the process pointed to by
 * targpp to those specified in the pcparms structure pointed
 * to by parmsp.  If reqpp is non-NULL it points to the process
 * that initiated the request for the parameter change and indicates
 * that our caller wants us to verify that the requesting process
 * has the appropriate permissions.
 */
int
parmsset(parmsp, reqpp, targpp)
register pcparms_t	*parmsp;
register proc_t		*reqpp;
register proc_t		*targpp;
{
	caddr_t			clprocp;
	register int		error;
	register id_t		reqpcid;
	register cred_t		*reqpcredp;
	int			oldlvl;

	if (reqpp != NULL) {
		reqpcid = reqpp->p_cid;
		reqpcredp = reqpp->p_cred;

		/*
		 * Check basic permissions.
		 */
		if (!hasprocperm(targpp->p_cred, reqpcredp))
			return(EPERM);
	} else {
		reqpcredp = NULL;
	}
	
	if (parmsp->pc_cid != targpp->p_cid) {

		/*
		 * Target process must change to new class.
		 */
		error = CL_ENTERCLASS(&class[parmsp->pc_cid],
		    parmsp->pc_clparms, targpp, &targpp->p_stat, &targpp->p_pri,
		    &targpp->p_flag, &targpp->p_cred, &clprocp, reqpcid,
		    reqpcredp);
		if (error)
			return(error);
		else {

			/*
			 * Change to new class successful so release resources
			 * for old class and complete change to new one.
			 */
			oldlvl = splhi();
			CL_EXITCLASS(targpp, targpp->p_clproc);
			targpp->p_cid = parmsp->pc_cid;
			targpp->p_clfuncs = class[parmsp->pc_cid].cl_funcs;
			targpp->p_clproc = clprocp;
			splx(oldlvl);
		}
	} else {

		/*
		 * Not changing class
		 */
		error = CL_PARMSSET(targpp, parmsp->pc_clparms,
		    targpp->p_clproc, reqpcid, reqpcredp);
		if (error)
			return(error);
	}
	return(0);
}
