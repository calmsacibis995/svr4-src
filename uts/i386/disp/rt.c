/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-dsp:rt.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/cred.h"
#include "sys/proc.h"
#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/evecb.h"
#include "sys/hrtcntl.h"
#include "sys/priocntl.h"
#include "sys/class.h"
#include "sys/disp.h"
#include "sys/procset.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/rt.h"
#include "sys/rtpriocntl.h"
#include "sys/kmem.h"
#include "sys/systm.h"
#include "sys/inline.h"
#include "sys/errno.h"


/*
 * Class specific code for the real-time class
 */

/*
 * Extern declarations for variables defined in the rt master file
 */
extern rtdpent_t rt_dptbl[];	/* real-time dispatcher parameter table */
extern short	rt_maxpri;	/* maximum real-time priority */


#define	RTPMEMSZ	1024	/* request size for rtproc memory allocation */


void		rt_init();
STATIC int	rt_admin(), rt_enterclass(), rt_fork(), rt_getclinfo();
STATIC int	rt_nosys(), rt_parmsin(), rt_parmsout(), rt_parmsset();
STATIC int	rt_proccmp();
STATIC void	rt_exitclass(), rt_forkret(), rt_getglobpri(), rt_nullsys();
STATIC void	rt_parmsget(), rt_preempt(), rt_setrun(), rt_sleep();
STATIC void	rt_stop(), rt_swapin(), rt_swapout(), rt_tick(), rt_wakeup();


STATIC id_t	rt_cid;		/* real-time class ID */
STATIC rtproc_t	rt_plisthead;	/* dummy rtproc at head of rtproc list */
STATIC caddr_t	rt_pmembase;	/* base addr of memory allocated for rtprocs */


STATIC struct classfuncs rt_classfuncs = {
	rt_admin,
	rt_enterclass,
	rt_exitclass,
	rt_fork,
	rt_forkret,
	rt_getclinfo,
	rt_getglobpri,
	rt_parmsget,
	rt_parmsin,
	rt_parmsout,
	rt_parmsset,
	rt_preempt,
	rt_proccmp,
	rt_setrun,
	rt_sleep,
	rt_stop,
	rt_swapin,
	rt_swapout,
	rt_tick,
	rt_nullsys,
	rt_wakeup,
	rt_nosys,
	rt_nosys,
	rt_nosys,
	rt_nosys,
	rt_nosys,
	rt_nosys,
	rt_nosys,
	rt_nosys,
	rt_nosys,
	rt_nosys,
	rt_nosys
};


/*
 * Real-time class initialization. Called by dispinit() at boot time.
 * We can ignore the clparmsz argument since we know that the smallest
 * possible parameter buffer is big enough for us.
 */
/* ARGSUSED */
void
rt_init(cid, clparmsz, clfuncspp, maxglobprip)
id_t		cid;
int		clparmsz;
classfuncs_t	**clfuncspp;
int		*maxglobprip;
{
	rt_cid = cid;	/* Record our class ID */

	/*
	 * Initialize the rtproc list.
	 */
	rt_plisthead.rt_next = rt_plisthead.rt_prev = &rt_plisthead;

	/*
	 * We're required to return a pointer to our classfuncs
	 * structure and the highest global priority value we use.
	 */
	*clfuncspp = &rt_classfuncs;
	*maxglobprip = rt_dptbl[rt_maxpri].rt_globpri;
}


/*
 * Get or reset the rt_dptbl values per the user's request.
 */
/* ARGSUSED */
STATIC int
rt_admin(uaddr, reqpcid, reqpcredp)
caddr_t	uaddr;
id_t	reqpcid;
cred_t	*reqpcredp;
{
	rtadmin_t		rtadmin;
	register rtdpent_t	*tmpdpp;
	register int		userdpsz;
	register int		i;
	register int		rtdpsz;
	int			oldlvl;

	if (copyin(uaddr, (caddr_t)&rtadmin, sizeof(rtadmin_t)))
		return(EFAULT);

	rtdpsz = (rt_maxpri + 1) * sizeof(rtdpent_t);

	switch(rtadmin.rt_cmd) {

	case RT_GETDPSIZE:

		rtadmin.rt_ndpents = rt_maxpri + 1;
		if (copyout((caddr_t)&rtadmin, uaddr, sizeof(rtadmin_t)))
			return(EFAULT);
		break;

	case RT_GETDPTBL:

		userdpsz = MIN(rtadmin.rt_ndpents * sizeof(rtdpent_t), rtdpsz);
		if (copyout((caddr_t)rt_dptbl,
		    (caddr_t)rtadmin.rt_dpents, userdpsz))
			return(EFAULT);

		rtadmin.rt_ndpents = userdpsz / sizeof(rtdpent_t);
		if (copyout((caddr_t)&rtadmin, uaddr, sizeof(rtadmin_t)))
			return(EFAULT);

		break;

	case RT_SETDPTBL:

		/*
		 * We require that the requesting process have super user
		 * priveleges.  We also require that the table supplied by
		 * the user exactly match the current rt_dptbl in size.
		 */
		if (!suser(reqpcredp))
			return(EPERM);
		if (rtadmin.rt_ndpents * sizeof(rtdpent_t) != rtdpsz)
			return(EINVAL);

		/*
		 * We read the user supplied table into a temporary buffer
		 * where the time quantum values are validated before
		 * being copied to the rt_dptbl.
		 */
		tmpdpp = (rtdpent_t *)kmem_alloc(rtdpsz, KM_SLEEP);
		ASSERT(tmpdpp != NULL);
		if (copyin((caddr_t)rtadmin.rt_dpents,
		    (caddr_t)tmpdpp, rtdpsz)) {
			kmem_free(tmpdpp, rtdpsz);
			return(EFAULT);
		}
		for (i = 0; i < rtadmin.rt_ndpents; i++) {

			/*
			 * Validate the user supplied time quantum values.
			 */
			if (tmpdpp[i].rt_quantum <= 0 &&
			    tmpdpp[i].rt_quantum != RT_TQINF) {
				kmem_free(tmpdpp, rtdpsz);
				return(EINVAL);
			}
		}

		/*
		 * Copy the user supplied values over the current rt_dptbl
		 * values.  The rt_globpri member is read-only so we don't
		 * overwrite it.
		 */
		oldlvl = splhi();
		for (i = 0; i < rtadmin.rt_ndpents; i++)
			rt_dptbl[i].rt_quantum = tmpdpp[i].rt_quantum;
		splx(oldlvl);

		kmem_free(tmpdpp, rtdpsz);
		break;

	default:
		return(EINVAL);
	}
	return(0);
}

	
/*
 * Allocate a real-time class specific proc structure and
 * initialize it with the parameters supplied. Also move process
 * to specified real-time priority.
 */
/* ARGSUSED */
STATIC int
rt_enterclass(rtkparmsp, pp, pstatp, pprip, pflagp, pcredpp, rtprocpp,
							reqpcid, reqpcredp)
register rtkparms_t	*rtkparmsp;
proc_t			*pp;
char			*pstatp;
int			*pprip;
uint			*pflagp;
cred_t			**pcredpp;
rtproc_t		**rtprocpp;
id_t			reqpcid;
cred_t			*reqpcredp;
{
	register rtproc_t	*rtpp;
	register int 		oldlvl;
	register uint		oldflag;
	register boolean_t	wasonq;

	/*
	 * For a process to enter the real-time class the process
	 * which initiates the request must be super-user.
	 * This may have been checked previously but if our
	 * caller passed us a credential structure we assume it
	 * hasn't and we check it here.
	 */
	if (reqpcredp != NULL && !suser(reqpcredp))
		return(EPERM);


	/*
	 * If the process' u-block is not currently in core and/or the
	 * process is unloaded we must bring the u-block in here and
	 * mark the process loaded.  Normally this is the job of the 
	 * swapper process but we must make sure the process is loaded
	 * before we can return successfully in order to guarantee
	 * sufficiently fast response for this real time process.
	 */
	if ((*pflagp & SLOAD) == 0) {
		if (memlow())
			return(ENOMEM);
		if (*pflagp & SUSWAP)
			return(EAGAIN);
		ub_lock(pp);
		if (swapinub(pp) == 0) {
			ub_rele(pp);
			return(ENOMEM);
		}
		ub_rele(pp);
		oldlvl = splhi();
		oldflag = *pflagp;
		*pflagp |= SLOAD;
		if (*pstatp == SRUN && (oldflag & (SLOAD|SPROCIO)) == 0)
			dq_sruninc(*pprip);
		splx(oldlvl);
	}

	/*
	 * Allocate an rtproc structure
	 */
	if ((rtpp = (rtproc_t *)kmem_fast_alloc(&rt_pmembase, sizeof(rtproc_t),
	    RTPMEMSZ / sizeof(rtproc_t), KM_NOSLEEP)) == NULL)
		return(ENOMEM);

	/*
	 * Initialize the rtproc structure
	 */
	if (rtkparmsp == NULL) {
		/*
		 * Use default values
		 */
		rtpp->rt_pri = 0;
		rtpp->rt_pquantum = rt_dptbl[0].rt_quantum;
	} else {
		/*
		 * Use supplied values
		 */
		if (rtkparmsp->rt_pri == RT_NOCHANGE) {
			rtpp->rt_pri = 0;
		} else {
			rtpp->rt_pri = rtkparmsp->rt_pri;
		}
		if (rtkparmsp->rt_tqntm == RT_TQINF)
			rtpp->rt_pquantum = RT_TQINF;
		else if (rtkparmsp->rt_tqntm == RT_TQDEF ||
			 rtkparmsp->rt_tqntm == RT_NOCHANGE)
			rtpp->rt_pquantum = rt_dptbl[rtpp->rt_pri].rt_quantum;
		else
			rtpp->rt_pquantum = rtkparmsp->rt_tqntm;
	}
	rtpp->rt_flags = 0;
	rtpp->rt_procp = pp;
	rtpp->rt_pstatp = pstatp;
	rtpp->rt_pprip = pprip;
	rtpp->rt_pflagp = pflagp;

	/*
	 * Link new structure into rtproc list
	 */
	rtpp->rt_next = rt_plisthead.rt_next;
	rtpp->rt_prev = &rt_plisthead;
	rt_plisthead.rt_next->rt_prev = rtpp;
	rt_plisthead.rt_next = rtpp;

	/*
	 * Reset process priority
	 */
	oldlvl = splhi();
	if (rtpp->rt_procp == curproc) {
		if ((curpri = *rtpp->rt_pprip =
		    rt_dptbl[rtpp->rt_pri].rt_globpri) > maxrunpri)
			rtpp->rt_timeleft = rtpp->rt_pquantum;
		else {
			rtpp->rt_flags |= RTBACKQ;
			runrun++;
			kprunrun++;
		}
	} else {
		wasonq = dispdeq(pp);
		*rtpp->rt_pprip = rt_dptbl[rtpp->rt_pri].rt_globpri;
		if (wasonq == B_TRUE) {
			if (*rtpp->rt_pprip > curpri) {
				runrun++;
				kprunrun++;
			}
			rtpp->rt_timeleft = rtpp->rt_pquantum;
			setbackdq(pp);
		} else
			rtpp->rt_flags |= RTBACKQ;
	}
	*rtprocpp = rtpp;
	splx(oldlvl);
	return(0);
}
	

/* 
 * Free rtproc structure of process.
 */
STATIC void
rt_exitclass(rtprocp)
rtproc_t	*rtprocp;
{
	rtprocp->rt_prev->rt_next = rtprocp->rt_next;
	rtprocp->rt_next->rt_prev = rtprocp->rt_prev;

	kmem_fast_free(&rt_pmembase, (caddr_t)rtprocp);
}


/*
 * Allocate and initialize real-time class specific
 * proc structure for child.
 */
/* ARGSUSED */
STATIC int
rt_fork(prtpp, cprocp, cpstatp, cpprip, cpflagp, cpcredpp, rtprocpp)
register rtproc_t	*prtpp;
proc_t			*cprocp;
char			*cpstatp;
int			*cpprip;
uint			*cpflagp;
cred_t			**cpcredpp;
rtproc_t		**rtprocpp;
{
	register rtproc_t	*crtpp; /* ptr to child's rtproc structure */


	crtpp = (rtproc_t *)kmem_fast_alloc(&rt_pmembase, sizeof(rtproc_t),
	    RTPMEMSZ / sizeof(rtproc_t), KM_SLEEP);
	ASSERT(crtpp != NULL);

	/*
	 * Initialize child's rtproc structure
	 */
	crtpp->rt_timeleft = crtpp->rt_pquantum = prtpp->rt_pquantum;
	crtpp->rt_pri = prtpp->rt_pri;
	crtpp->rt_flags = prtpp->rt_flags & ~RTBACKQ;
	crtpp->rt_procp = cprocp;
	crtpp->rt_pstatp = cpstatp;
	crtpp->rt_pprip = cpprip;
	crtpp->rt_pflagp = cpflagp;

	/*
	 * Link structure into rtproc list
	 */
	crtpp->rt_next = rt_plisthead.rt_next;
	crtpp->rt_prev = &rt_plisthead;
	rt_plisthead.rt_next->rt_prev = crtpp;
	rt_plisthead.rt_next = crtpp;
	
	*rtprocpp = crtpp;
	return(0);
}


/*
 * The child goes to the back of its dispatcher queue while the
 * parent continues to run after a real time process forks.
 */
/* ARGSUSED */
STATIC void
rt_forkret(crtpp, prtpp)
register rtproc_t	*crtpp;
register rtproc_t	*prtpp;
{
	setbackdq(crtpp->rt_procp);
}


/*
 * Get information about the real-time class into the buffer
 * pointed to by rtinfop.  The maximum configured real-time
 * priority is the only information we supply.  We ignore the
 * class and credential arguments because anyone can have this
 * information.
 */
/* ARGSUSED */
STATIC int
rt_getclinfo(rtinfop, reqpcid, reqpcredp)
rtinfo_t	*rtinfop;
id_t		reqpcid;
cred_t		*reqpcredp;
{
	rtinfop->rt_maxpri = rt_maxpri;
	return(0);
}


/*
 * Return the global scheduling priority corresponding to the
 * rt_pri value in the rtkparms buffer.
 */
STATIC void
rt_getglobpri(rtkparmsp, globprip)
rtkparms_t	*rtkparmsp;
int		*globprip;
{
	*globprip = rt_dptbl[rtkparmsp->rt_pri].rt_globpri;
}


STATIC int
rt_nosys()
{
	return(ENOSYS);
}


STATIC void
rt_nullsys()
{
}


/*
 * Get the real-time scheduling parameters of the process pointed to by
 * rtprocp into the buffer pointed to by rtkparmsp.
 */
STATIC void
rt_parmsget(rtprocp, rtkparmsp)
rtproc_t	*rtprocp;
rtkparms_t	*rtkparmsp;
{
	rtkparmsp->rt_pri = rtprocp->rt_pri;
	rtkparmsp->rt_tqntm = rtprocp->rt_pquantum;
}



/*
 * Check the validity of the real-time parameters in the buffer
 * pointed to by rtprmsp.  If our caller passes us a non-NULL
 * reqpcredp pointer we also verify that the requesting process
 * (whose class and credentials are indicated by reqpcid and reqpcredp)
 * has the necessary permissions to set these parameters for a
 * target process with class targpcid. We also convert the
 * rtparms buffer from the user supplied format to our internal
 * format (i.e. time quantum expressed in ticks).
 */
/* ARGSUSED */
STATIC int
rt_parmsin(rtprmsp, reqpcid, reqpcredp, targpcid, targpcredp, rtpp)
register rtparms_t	*rtprmsp;
id_t			reqpcid;
cred_t			*reqpcredp;
id_t			targpcid;
cred_t			*targpcredp;
rtproc_t		*rtpp;
{
	hrtime_t		hrtqntm;
	register int		error;


	/*
	 * First check the validity of parameters and convert
	 * the buffer to kernel format.
	 */
	if ((rtprmsp->rt_pri < 0 || rtprmsp->rt_pri > rt_maxpri) &&
	    rtprmsp->rt_pri != RT_NOCHANGE)
		return(EINVAL);

	if ((rtprmsp->rt_tqsecs == 0 && rtprmsp->rt_tqnsecs == 0) ||
	    rtprmsp->rt_tqnsecs >= 1000000000)
		return(EINVAL);
	
	if (rtprmsp->rt_tqnsecs >= 0) {
		hrtqntm.hrt_secs = rtprmsp->rt_tqsecs;
		hrtqntm.hrt_rem = rtprmsp->rt_tqnsecs;
		hrtqntm.hrt_res = NANOSEC;
		error = hrt_tohz(&hrtqntm, HRT_RNDUP);
		if (error)
			return(error);
		((rtkparms_t *)rtprmsp)->rt_tqntm = hrtqntm.hrt_rem;
	} else {
		if (rtprmsp->rt_tqnsecs != RT_NOCHANGE &&
		    rtprmsp->rt_tqnsecs != RT_TQINF &&
		    rtprmsp->rt_tqnsecs != RT_TQDEF)
			return(EINVAL);
		((rtkparms_t *)rtprmsp)->rt_tqntm = rtprmsp->rt_tqnsecs;
	}

	/*
	 * If our caller passed us non-NULL cred pointers
	 * we are being asked to check permissions as well
	 * as the validity of the parameters. In order to
	 * set any parameters the real-time class requires
	 * that the requesting process be real-time or
	 * super-user.  If the target process is currently in
	 * a class other than real-time the requesting process
	 * must be super-user.
	 */
	if (reqpcredp != NULL) {
		if (targpcid == rt_cid) {
			if (reqpcid != rt_cid && !suser(reqpcredp))
				return(EPERM);
		} else {  /* target process is not real-time */
			if (!suser(reqpcredp))
				return(EPERM);
		}
	}

	return(0);
}

/*
 * Do required processing on the real-time parameter buffer
 * before it is copied out to the user. We ignore the class
 * and credential arguments passed by our caller because we
 * don't require any special permissions to read real-time
 * scheduling parameters.  All we have to do is convert the
 * buffer from kernel to user format (i.e. convert time quantum
 * from ticks to seconds-nanoseconds).
 */
/* ARGSUSED */
STATIC int
rt_parmsout(rtkprmsp, reqpcid, reqpcredp, targpcredp)
register rtkparms_t	*rtkprmsp;
id_t			reqpcid;
cred_t			*reqpcredp;
cred_t			*targpcredp;
{
	hrtime_t		hrtqntm;
	register int		error;

	if (rtkprmsp->rt_tqntm < 0) {
		/*
		 * Quantum field set to special value (e.g. RT_TQINF)
		 */
		((rtparms_t *)rtkprmsp)->rt_tqnsecs = rtkprmsp->rt_tqntm;
		((rtparms_t *)rtkprmsp)->rt_tqsecs = 0;
	} else {
		/* Convert quantum from ticks to seconds-nanoseconds */
		hrtqntm.hrt_secs = 0;
		hrtqntm.hrt_rem = rtkprmsp->rt_tqntm;
		hrtqntm.hrt_res = HZ;
		error = hrt_newres(&hrtqntm, NANOSEC, HRT_TRUNC);
		if (error)
			/*
			 * We should never get an error here because a
			 * long should always hold a second full of
			 * nanoseconds.  But if we do, the best we can
			 * do is to return it.
			 */
			return(error);

		((rtparms_t *)rtkprmsp)->rt_tqsecs = hrtqntm.hrt_secs;
		((rtparms_t *)rtkprmsp)->rt_tqnsecs = hrtqntm.hrt_rem;
	}

	return(0);
}


/*
 * Set the scheduling parameters of the process pointed to by rtprocp
 * to those specified in the buffer pointed to by rtkprmsp.
 * Note that the parameters are expected to be in kernel format
 * (i.e. time quantm expressed in ticks).  Real time parameters copied
 * in from the user should be processed by rt_parmsin() before they are
 * passed to this function.
 */
STATIC int
rt_parmsset(rtkprmsp, rtpp, reqpcid, reqpcredp)
register rtkparms_t	*rtkprmsp;
register rtproc_t	*rtpp;
id_t			reqpcid;
cred_t			*reqpcredp;
{
	register int		oldlvl;
	register boolean_t	wasonq;

	/* 
	 * Basic permissions enforced by generic kernel code
	 * for all classes require that a process attempting
	 * to change the scheduling parameters of a target process
	 * be super-user or have a real or effective UID
	 * matching that of the target process. We are not
	 * called unless these basic permission checks have
	 * already passed. The real-time class requires in addition
	 * that the requesting process be real-time unless it is super-user.
	 * This may also have been checked previously but if our caller
	 * passes us a credential structure we assume it hasn't and
	 * we check it here.
	 */
	if (reqpcredp != NULL && reqpcid != rt_cid && !suser(reqpcredp))
		return(EPERM);

	oldlvl = splhi();
	if (rtkprmsp->rt_pri != RT_NOCHANGE) {
		rtpp->rt_pri = rtkprmsp->rt_pri;
		if (rtpp->rt_procp == curproc) {
			if ((curpri = *rtpp->rt_pprip =
			    rt_dptbl[rtpp->rt_pri].rt_globpri) <= maxrunpri) {
				rtpp->rt_flags |= RTBACKQ;
				runrun++;
				kprunrun++;
			}
		} else {
			wasonq = dispdeq(rtpp->rt_procp);
			*rtpp->rt_pprip = rt_dptbl[rtpp->rt_pri].rt_globpri;
			if (wasonq == B_TRUE) {
				if (*rtpp->rt_pprip > curpri) {
					runrun++;
					kprunrun++;
				}
				setbackdq(rtpp->rt_procp);
			} else
				rtpp->rt_flags |= RTBACKQ;
		}
	}
	if (rtkprmsp->rt_tqntm == RT_TQINF)
		rtpp->rt_pquantum = RT_TQINF;
	else if (rtkprmsp->rt_tqntm == RT_TQDEF)
		rtpp->rt_timeleft = rtpp->rt_pquantum =
		    rt_dptbl[rtpp->rt_pri].rt_quantum;
	else if (rtkprmsp->rt_tqntm != RT_NOCHANGE)
		rtpp->rt_timeleft = rtpp->rt_pquantum = rtkprmsp->rt_tqntm;
	splx(oldlvl);
	return(0);
}


/*
 * Arrange for process to be placed in appropriate location
 * on dispatcher queue.  Runs at splhi() since the clock
 * interrupt can cause RTBACKQ to be set.
 */
STATIC void
rt_preempt(rtpp)
rtproc_t	*rtpp;
{
	register int	oldlvl;

	oldlvl = splhi();

	if (rtpp->rt_procp == curproc)
		rtpp->rt_flags |= RTRAN;
	if ((rtpp->rt_flags & RTBACKQ) != 0) {
		rtpp->rt_timeleft = rtpp->rt_pquantum;
		rtpp->rt_flags &= ~RTBACKQ;
		setbackdq(rtpp->rt_procp);
	} else
		setfrontdq(rtpp->rt_procp);

	splx(oldlvl);
}


/*
 * rt_proccmp() is part of the implementation of the PC_GETPARMS
 * command of the priocntl system call. When the user specifies
 * multiple processes to priocntl PC_GETPARMS the criteria
 * for selecting a process from the set is class specific. The
 * criteria used by real-time is the real-time priority value
 * of the process. rt_proccmp() simply compares two processes based
 * on real-time priority.  All the ugly work of looping through the 
 * processes in the set is done by higher level (class independent)
 * functions.
 */
STATIC int
rt_proccmp(rtproc1p, rtproc2p)
rtproc_t	*rtproc1p;
rtproc_t	*rtproc2p;
{
	return(rtproc1p->rt_pri - rtproc2p->rt_pri);
}


STATIC void
rt_setrun(rtpp)
rtproc_t	*rtpp;
{
	rtpp->rt_timeleft = rtpp->rt_pquantum;
	rtpp->rt_flags &= ~RTBACKQ;
	setbackdq(rtpp->rt_procp);
	if (*rtpp->rt_pprip > curpri) {
		runrun++;
		kprunrun++;
	}
}


/* ARGSUSED */
STATIC void
rt_sleep(rtprocp, chan, disp)
rtproc_t	*rtprocp;
caddr_t		chan;
int		disp;
{
	rtprocp->rt_flags |= RTRAN;
}


/* ARGSUSED */
STATIC void
rt_stop(rtprocp, why, what)
rtproc_t	*rtprocp;
int		why;
int		what;
{
	if (rtprocp->rt_procp == curproc)
		rtprocp->rt_flags |= RTRAN;
}


/*
 * Real-time processes can't be unloaded and their u-blocks are locked
 * in core so we never need to "swap them in".  We are however, required
 * to return (in *runflagp)  an indication of whether there are currently
 * any loaded, runnable processes in this class.
 */
/* ARGSUSED */
STATIC void
rt_swapin(fm, procpp, runflagp)
int		fm;
struct proc	**procpp;
int		*runflagp;
{
	register rtproc_t	*rtpp;

	*runflagp = 0;
	for (rtpp = rt_plisthead.rt_next; rtpp != &rt_plisthead;
	    rtpp = rtpp->rt_next) {
		if (*rtpp->rt_pstatp == SRUN) {
			*runflagp = 1;
			break;
		}
	}
	*procpp = NULL;
}


/*
 * Nominate a process for sched() to "swap out".  We set *unloadokp to
 * B_FALSE so sched() will try to swap out the pages of the process
 * we nominate but won't swap out the u-block or mark process unloaded.
 * We nominate the lowest priority process which:
 *	(a) isn't locked in
 *	(b) has run since the last time it was nominated for swapout
 */
/* ARGSUSED */
STATIC void
rt_swapout(fm, jl, procpp, unloadokp)
int		fm;
proc_t	 	*jl;
proc_t		**procpp;
boolean_t	*unloadokp;
{
	register minpri;
	register rtproc_t	*rtpp;
	register rtproc_t	*minpripp;

	minpri = rt_maxpri + 1;
	for (rtpp = rt_plisthead.rt_next; rtpp != &rt_plisthead;
	    rtpp = rtpp->rt_next) {
		if (*rtpp->rt_pstatp == SZOMB || *rtpp->rt_pstatp == SIDL)
			continue;
		if ((rtpp->rt_flags & RTRAN) == 0)
			continue;
		if (*rtpp->rt_pflagp & (SSYS|SLOCK|SPROCIO|SSWLOCKS))
			continue;
		ASSERT(*rtpp->rt_pflagp & SULOAD);
		if (rtpp->rt_pri < minpri) {
			minpripp = rtpp;
			minpri = rtpp->rt_pri;
		}
	}
	if (minpri == rt_maxpri + 1) {
		*procpp = NULL;
	} else {
		minpripp->rt_flags &= ~RTRAN;
		*procpp = minpripp->rt_procp;
		*unloadokp = B_FALSE;
	}
}


/*
 * Check for time slice expiration (unless process has infinite time 
 * slice).  If time slice has expired arrange for process to be preempted
 * and placed on back of queue.
 */
STATIC void
rt_tick(rtprocp)
register rtproc_t	*rtprocp;
{
	if (rtprocp->rt_pquantum != RT_TQINF && --rtprocp->rt_timeleft == 0) {
		rtprocp->rt_flags |= RTBACKQ;
		runrun++;
		kprunrun++;
	}
}


/*
 * Place the process waking up on the dispatcher queue.  Also if the
 * process waking up is higher priority than the current process and
 * the caller is requesting a non-preemptive wakeup we set runrun to
 * force the preemption.  The real time class does not honor requests
 * for non-preemptive wakeups.
 */
/* ARGSUSED */
STATIC void
rt_wakeup(rtprocp, preemptflg)
rtproc_t	*rtprocp;
int		preemptflg;
{
	rtprocp->rt_timeleft = rtprocp->rt_pquantum;
	rtprocp->rt_flags &= ~RTBACKQ;
	setbackdq(rtprocp->rt_procp);
	if (*rtprocp->rt_pprip > curpri) {
		runrun++;
		kprunrun++;
	}
}
