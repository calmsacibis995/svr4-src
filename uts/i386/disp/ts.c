/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-dsp:ts.c	1.3.1.2"

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
#include "sys/priocntl.h"
#include "sys/class.h"
#include "sys/disp.h"
#include "sys/procset.h"
#include "sys/debug.h"
#include "sys/ts.h"
#include "sys/tspriocntl.h"
#include "sys/evecb.h"
#include "sys/hrtcntl.h"
#include "sys/kmem.h"
#include "sys/systm.h"
#include "sys/inline.h"
#include "sys/errno.h"

/*
 * Class specific code for the time-sharing class
 */


/*
 * Extern declarations for variables defined in the ts master file
 */
extern tsdpent_t  ts_dptbl[];	/* time-sharing disp parameter table */
extern int	ts_kmdpris[];	/* array of global pris used by ts procs when */
				/*  sleeping or running in kernel after sleep */
extern short	ts_maxupri;	/* max time-sharing user priority */
extern short	ts_maxumdpri;	/* maximum user mode ts priority */
extern short	ts_maxkmdpri;	/* maximum kernel mode ts priority */


#define	TSPMEMSZ	 2048	/* request size for tsproc memory allocation */

#define	pstat		(*tspp->ts_pstatp)
#define	ppri		(*tspp->ts_pprip)
#define	pflag		(*tspp->ts_pflagp)
#define	tsumdpri	(tspp->ts_umdpri)
#define	tsmedumdpri	(ts_maxumdpri >> 1)

#define	TS_NEWUMDPRI(tspp)	\
{ \
if (((tspp)->ts_umdpri = (tspp)->ts_cpupri + (tspp)->ts_upri) > ts_maxumdpri) \
	(tspp)->ts_umdpri = ts_maxumdpri; \
else if ((tspp)->ts_umdpri < 0) \
	(tspp)->ts_umdpri = 0; \
}


void		ts_donice(), ts_init();
STATIC int	ts_admin(), ts_enterclass(), ts_fork(), ts_getclinfo();
STATIC int	ts_nosys(), ts_parmsin(), ts_parmsout(), ts_parmsset();
STATIC int	ts_proccmp();
STATIC void	ts_exitclass(), ts_forkret(), ts_getglobpri();
STATIC void	ts_nullsys(), ts_parmsget(), ts_preempt();
STATIC void	ts_setrun(), ts_sleep(), ts_swapin(), ts_swapout();
STATIC void	ts_tick(), ts_trapret(), ts_update(), ts_wakeup();


STATIC id_t	ts_cid;		/* time-sharing class ID */
STATIC int	ts_maxglobpri;	/* maximum global priority used by ts class */
STATIC tsproc_t	ts_plisthead;	/* dummy tsproc at head of tsproc list */
STATIC caddr_t	ts_pmembase;	/* base addr of memory allocated for tsprocs */


STATIC struct classfuncs ts_classfuncs = {
	ts_admin,
	ts_enterclass,
	ts_exitclass,
	ts_fork,
	ts_forkret,
	ts_getclinfo,
	ts_getglobpri,
	ts_parmsget,
	ts_parmsin,
	ts_parmsout,
	ts_parmsset,
	ts_preempt,
	ts_proccmp,
	ts_setrun,
	ts_sleep,
	ts_nullsys,
	ts_swapin,
	ts_swapout,
	ts_tick,
	ts_trapret,
	ts_wakeup,
	ts_nosys,
	ts_nosys,
	ts_nosys,
	ts_nosys,
	ts_nosys,
	ts_nosys,
	ts_nosys,
	ts_nosys,
	ts_nosys,
	ts_nosys,
	ts_nosys
};


/*
 * Time sharing class initialization.  Called by dispinit() at boot time.
 * We can ignore the clparmsz argument since we know that the smallest
 * possible parameter buffer is big enough for us.
 */
/* ARGSUSED */
void
ts_init(cid, clparmsz, clfuncspp, maxglobprip)
id_t		cid;
int		clparmsz;
classfuncs_t	**clfuncspp;
int		*maxglobprip;
{
	ts_maxglobpri = max(ts_kmdpris[ts_maxkmdpri],
	    ts_dptbl[ts_maxumdpri].ts_globpri);

	ts_cid = cid;		/* Record our class ID */

	/*
	 * Initialize the tsproc list.
	 */
	ts_plisthead.ts_next = ts_plisthead.ts_prev = &ts_plisthead;

	/*
	 * We're required to return a pointer to our classfuncs
	 * structure and the highest global priority value we use.
	 */
	*clfuncspp = &ts_classfuncs;
	*maxglobprip = ts_maxglobpri;
}


/*
 * Get or reset the ts_dptbl values per the user's request.
 */
/* ARGSUSED */
STATIC int
ts_admin(uaddr, reqpcid, reqpcredp)
caddr_t	uaddr;
id_t	reqpcid;
cred_t	*reqpcredp;
{
	tsadmin_t		tsadmin;
	register tsdpent_t	*tmpdpp;
	register int		userdpsz;
	register int		i;
	register int		tsdpsz;
	int			oldlvl;

	if (copyin(uaddr, (caddr_t)&tsadmin, sizeof(tsadmin_t)))
		return(EFAULT);

	tsdpsz = (ts_maxumdpri + 1) * sizeof(tsdpent_t);

	switch(tsadmin.ts_cmd) {

	case TS_GETDPSIZE:

		tsadmin.ts_ndpents = ts_maxumdpri + 1;
		if (copyout((caddr_t)&tsadmin, uaddr, sizeof(tsadmin_t)))
			return(EFAULT);
		break;

	case TS_GETDPTBL:

		userdpsz = MIN(tsadmin.ts_ndpents * sizeof(tsdpent_t), tsdpsz);
		if (copyout((caddr_t)ts_dptbl,
		    (caddr_t)tsadmin.ts_dpents, userdpsz))
			return(EFAULT);

		tsadmin.ts_ndpents = userdpsz / sizeof(tsdpent_t);
		if (copyout((caddr_t)&tsadmin, uaddr, sizeof(tsadmin_t)))
			return(EFAULT);

		break;

	case TS_SETDPTBL:

		/*
		 * We require that the requesting process have super user
		 * priveleges.  We also require that the table supplied by
		 * the user exactly match the current ts_dptbl in size.
		 */
		if (!suser(reqpcredp))
			return(EPERM);
		if (tsadmin.ts_ndpents * sizeof(tsdpent_t) != tsdpsz)
			return(EINVAL);

		/*
		 * We read the user supplied table into a temporary buffer
		 * where it is validated before being copied over the
		 * ts_dptbl.
		 */
		tmpdpp = (tsdpent_t *)kmem_alloc(tsdpsz, KM_SLEEP);
		ASSERT(tmpdpp != NULL);
		if (copyin((caddr_t)tsadmin.ts_dpents, (caddr_t)tmpdpp,
		    tsdpsz)) {
			kmem_free(tmpdpp, tsdpsz);
			return(EFAULT);
		}
		for (i = 0; i < tsadmin.ts_ndpents; i++) {

			/*
			 * Validate the user supplied values.  All we are doing
			 * here is verifying that the values are within their
			 * allowable ranges and will not panic the system.  We
			 * make no attempt to ensure that the resulting
			 * configuration makes sense or results in reasonable
			 * performance.
			 */
			if (tmpdpp[i].ts_quantum <= 0) {
				kmem_free(tmpdpp, tsdpsz);
				return(EINVAL);
			}
			if (tmpdpp[i].ts_tqexp > ts_maxumdpri ||
			    tmpdpp[i].ts_tqexp < 0) {
				kmem_free(tmpdpp, tsdpsz);
				return(EINVAL);
			}
			if (tmpdpp[i].ts_slpret > ts_maxumdpri ||
			    tmpdpp[i].ts_slpret < 0) {
				kmem_free(tmpdpp, tsdpsz);
				return(EINVAL);
			}
			if (tmpdpp[i].ts_maxwait < 0) {
				kmem_free(tmpdpp, tsdpsz);
				return(EINVAL);
			}
			if (tmpdpp[i].ts_lwait > ts_maxumdpri ||
			    tmpdpp[i].ts_lwait < 0) {
				kmem_free(tmpdpp, tsdpsz);
				return(EINVAL);
			}
		}

		/*
		 * Copy the user supplied values over the current ts_dptbl
		 * values.  The ts_globpri member is read-only so we don't
		 * overwrite it.
		 */
		oldlvl = splhi();
		for (i = 0; i < tsadmin.ts_ndpents; i++) {
			ts_dptbl[i].ts_quantum = tmpdpp[i].ts_quantum;
			ts_dptbl[i].ts_tqexp = tmpdpp[i].ts_tqexp;
			ts_dptbl[i].ts_slpret = tmpdpp[i].ts_slpret;
			ts_dptbl[i].ts_maxwait = tmpdpp[i].ts_maxwait;
			ts_dptbl[i].ts_lwait = tmpdpp[i].ts_lwait;
		}
		splx(oldlvl);

		kmem_free(tmpdpp, tsdpsz);
		break;

	default:
		return(EINVAL);
	}
	return(0);
}


/*
 * Allocate a time-sharing class specific proc structure and
 * initialize it with the parameters supplied. Also move process
 * to specified time-sharing priority.
 */
/* ARGSUSED */
STATIC int
ts_enterclass(tsparmsp, pp, pstatp, pprip, pflagp, pcredpp, tsprocpp,
							reqpcid, reqpcredp)
tsparms_t	*tsparmsp;
proc_t		*pp;
char		*pstatp;
int		*pprip;
uint		*pflagp;
cred_t		**pcredpp;
tsproc_t	**tsprocpp;
id_t		reqpcid;
cred_t		*reqpcredp;
{
	register tsproc_t	*tspp;
	register short		reqtsuprilim;
	register short		reqtsupri;
	register int 		oldlvl;
	register boolean_t	wasonq;
	static int		tspexists = 0;	/* set on first occurence of */
						/*   a time-sharing process */


	if ((tspp = (tsproc_t *)kmem_fast_alloc(&ts_pmembase, sizeof(tsproc_t),
	    TSPMEMSZ / sizeof(tsproc_t), KM_NOSLEEP)) == NULL)
		return(ENOMEM);


	/*
	 * Initialize the tsproc structure.
	 */
	if (tsparmsp == NULL) {
		/*
		 * Use default values.
		 */
		tspp->ts_uprilim = tspp->ts_upri = 0;
		tspp->ts_nice = 20;
		tspp->ts_umdpri = tspp->ts_cpupri = tsmedumdpri;
	} else {
		/*
		 * Use supplied values.
		 */
		if (tsparmsp->ts_uprilim == TS_NOCHANGE)
				reqtsuprilim = 0;
		else
			reqtsuprilim = tsparmsp->ts_uprilim;

		/*
		 * In order to set an initial upri limit greater than
		 * zero the requesting process must be super-user.
		 * This may have been checked previously but if our
		 * caller passed us a credential structure we assume
		 * it hasn't and we check it here.
		 */
		if (reqpcredp != NULL && reqtsuprilim > 0 &&
		    !suser(reqpcredp)) {
			kmem_fast_free(&ts_pmembase, (caddr_t)tspp);
			return(EPERM);
		}

		if (tsparmsp->ts_upri == TS_NOCHANGE) {
			reqtsupri = reqtsuprilim;
		} else {
			/*
			 * Set the user priority to the requested value
			 * or the upri limit, whichever is lower.
			 */
			reqtsupri = tsparmsp->ts_upri;
			if (reqtsupri > reqtsuprilim)
				reqtsupri = reqtsuprilim;
		}

		tspp->ts_uprilim = reqtsuprilim;
		tspp->ts_upri = reqtsupri;
		tspp->ts_nice = 20 - (20 * reqtsupri) / ts_maxupri;
		tspp->ts_cpupri = tsmedumdpri;
		TS_NEWUMDPRI(tspp);
	}

	tspp->ts_dispwait = 0;
	tspp->ts_flags = 0;
	tspp->ts_procp = pp;
	tspp->ts_pstatp = pstatp;
	tspp->ts_pprip = pprip;
	tspp->ts_pflagp = pflagp;

	/*
	 * Link new structure into tsproc list.
	 */
	tspp->ts_next = ts_plisthead.ts_next;
	tspp->ts_prev = &ts_plisthead;
	ts_plisthead.ts_next->ts_prev = tspp;
	ts_plisthead.ts_next = tspp;

	/*
	 * Reset priority. Process goes to a "user mode" priority
	 * here regardless of whether or not it has slept since
	 * entering the kernel.
	 */
	oldlvl = splhi();
	if (tspp->ts_procp == curproc) {
		if ((curpri = *tspp->ts_pprip =
		    ts_dptbl[tsumdpri].ts_globpri) > maxrunpri)
			tspp->ts_timeleft = ts_dptbl[tsumdpri].ts_quantum;
		else {
			tspp->ts_flags |= TSBACKQ;
			runrun++;
		}
	} else {
		wasonq = dispdeq(pp);
		*tspp->ts_pprip = ts_dptbl[tsumdpri].ts_globpri;
		if (wasonq == B_TRUE) {
			if (*tspp->ts_pprip > curpri)
				runrun++;
			tspp->ts_timeleft = ts_dptbl[tsumdpri].ts_quantum;
			setbackdq(pp);
		} else
			tspp->ts_flags |= TSBACKQ;
	}
	*tsprocpp = tspp;
	splx(oldlvl);

	/*
	 * If this is the first time-sharing process to occur since
	 * boot we set up the initial call to ts_update() here.
	 */
	if (tspexists == 0) {
		(void)timeout(ts_update, 0, HZ);
		tspexists++;
	}
	return(0);
}


/*
 * Free tsproc structure of process.
 */
STATIC void
ts_exitclass(tsprocp)
tsproc_t	*tsprocp;
{
	tsprocp->ts_prev->ts_next = tsprocp->ts_next;
	tsprocp->ts_next->ts_prev = tsprocp->ts_prev;

	kmem_fast_free(&ts_pmembase, (caddr_t)tsprocp);
}


/*
 * Allocate and initialize time-sharing class specific
 * proc structure for child.
 */
/* ARGSUSED */
STATIC int
ts_fork(ptspp, cprocp, cpstatp, cpprip, cpflagp, cpcredpp, tsprocpp)
register tsproc_t	*ptspp;
proc_t			*cprocp;
char			*cpstatp;
int			*cpprip;
uint			*cpflagp;
cred_t			**cpcredpp;
tsproc_t		**tsprocpp;
{
	register tsproc_t	*ctspp; /* ptr to child's tsproc structure */


	ctspp = (tsproc_t *)kmem_fast_alloc(&ts_pmembase, sizeof(tsproc_t),
	    TSPMEMSZ / sizeof(tsproc_t), KM_SLEEP);
	ASSERT(ctspp != NULL);


	/*
	 * Initialize child's tsproc structure.
	 */
	ctspp->ts_timeleft = ts_dptbl[ptspp->ts_umdpri].ts_quantum;
	ctspp->ts_umdpri = ptspp->ts_umdpri;
	ctspp->ts_cpupri = ptspp->ts_cpupri;
	ctspp->ts_uprilim = ptspp->ts_uprilim;
	ctspp->ts_upri = ptspp->ts_upri;
	ctspp->ts_nice = ptspp->ts_nice;
	ctspp->ts_dispwait = 0;
	ctspp->ts_flags = ptspp->ts_flags & ~TSBACKQ;
	ctspp->ts_procp = cprocp;
	ctspp->ts_pstatp = cpstatp;
	ctspp->ts_pprip = cpprip;
	ctspp->ts_pflagp = cpflagp;

	/*
	 * Link structure into tsproc list.
	 */
	ctspp->ts_next = ts_plisthead.ts_next;
	ctspp->ts_prev = &ts_plisthead;
	ts_plisthead.ts_next->ts_prev = ctspp;
	ts_plisthead.ts_next = ctspp;
	
	*tsprocpp = ctspp;
	return(0);
}


/*
 * Child is placed at back of dispatcher queue and parent gives
 * up processor so that the child runs first after the fork.
 * This allows the child immediately execing to break the multiple
 * use of copy on write pages with no disk home. The parent will
 * get to steal them back rather than uselessly copying them.
 */
STATIC void
ts_forkret(ctspp, ptspp)
register tsproc_t	*ctspp;
register tsproc_t	*ptspp;
{
	setbackdq(ctspp->ts_procp);

	if (ptspp != NULL) {
		ptspp->ts_flags |= (TSBACKQ|TSFORK);
		runrun++;
	}
}


/*
 * Get information about the time-sharing class into the buffer
 * pointed to by tsinfop. The maximum configured user priority
 * is the only information we supply.  We ignore the class and
 * credential arguments because anyone can have this information.
 */
/* ARGSUSED */
STATIC int
ts_getclinfo(tsinfop, reqpcid, reqpcredp)
tsinfo_t	*tsinfop;
id_t		reqpcid;
cred_t		*reqpcredp;
{
	tsinfop->ts_maxupri = ts_maxupri;
	return(0);
}


/*
 * Return the global scheduling priority that would be assigned
 * to a process entering the time-sharing class with the ts_upri
 * value specified in the tsparms buffer.
 */
STATIC void
ts_getglobpri(tsparmsp, globprip)
tsparms_t	*tsparmsp;
int		*globprip;
{
	register int	tspri;

	tspri = tsmedumdpri + tsparmsp->ts_upri;
	if (tspri > ts_maxumdpri)
		tspri = ts_maxumdpri;
	else if (tspri < 0)
		tspri = 0;
	*globprip = ts_dptbl[tspri].ts_globpri;
}


STATIC int
ts_nosys()
{
	return(ENOSYS);
}


STATIC void
ts_nullsys()
{
}


/*
 * Get the time-sharing parameters of the process pointed to by
 * tsprocp into the buffer pointed to by tsparmsp.
 */
STATIC void
ts_parmsget(tsprocp, tsparmsp)
tsproc_t	*tsprocp;
tsparms_t	*tsparmsp;
{
	tsparmsp->ts_uprilim = tsprocp->ts_uprilim;
	tsparmsp->ts_upri = tsprocp->ts_upri;
}


/*
 * Check the validity of the time-sharing parameters in the buffer
 * pointed to by tsparmsp. If our caller passes us a non-NULL
 * reqpcredp pointer we also verify that the requesting process
 * (whose credentials are pointed to by reqpcredp) has the necessary
 * permissions to set these parameters for the target process.
 */
/* ARGSUSED */
STATIC int
ts_parmsin(tsparmsp, reqpcid, reqpcredp, targpcid, targpcredp, tspp)
register tsparms_t	*tsparmsp;
id_t			reqpcid;
cred_t			*reqpcredp;
id_t			targpcid;
cred_t			*targpcredp;
tsproc_t		*tspp;
{
	/*
	 * Check validity of parameters.
	 */
	if ((tsparmsp->ts_uprilim > ts_maxupri ||
	    tsparmsp->ts_uprilim < -ts_maxupri) &&
	    tsparmsp->ts_uprilim != TS_NOCHANGE)
		return(EINVAL);

	if ((tsparmsp->ts_upri > ts_maxupri || tsparmsp->ts_upri < -ts_maxupri) &&
	    tsparmsp->ts_upri != TS_NOCHANGE)
		return(EINVAL);

	if (reqpcredp == NULL || tsparmsp->ts_uprilim == TS_NOCHANGE)
		return(0);

	/* 
	 * Our caller passed us non-NULL credential pointers so
	 * we are being asked to check permissions as well as
	 * the validity of the parameters.  The basic rules are
	 * that the calling process must be super-user in order
	 * to raise the target process' upri limit above its
	 * current value.  If the target process is not currently
	 * time-sharing, the calling process must be super-user in
	 * order to set a upri limit greater than zero.
	 */
	if (targpcid == ts_cid) {
		if (tsparmsp->ts_uprilim > tspp->ts_uprilim &&
		    !suser(reqpcredp))
			return(EPERM);
	} else {
		if (tsparmsp->ts_uprilim > 0 && !suser(reqpcredp))
			return(EPERM);
	}

	return(0);
}


/*
 * Nothing to do here but return success.
 */
STATIC int
ts_parmsout()
{
	return(0);
}


/*
 * Set the scheduling parameters of the process pointed to by tsprocp
 * to those specified in the buffer pointed to by tsparmsp.
 */
/* ARGSUSED */
STATIC int
ts_parmsset(tsparmsp, tspp, reqpcid, reqpcredp)
register tsparms_t	*tsparmsp;
register tsproc_t	*tspp;
id_t			reqpcid;
cred_t			*reqpcredp;
{
	register int		oldlvl;
	boolean_t		wasonq;
	register char		nice;
	register short		reqtsuprilim;
	register short		reqtsupri;

	if (tsparmsp->ts_uprilim == TS_NOCHANGE)
			reqtsuprilim = tspp->ts_uprilim;
	else
		reqtsuprilim = tsparmsp->ts_uprilim;

	if (tsparmsp->ts_upri == TS_NOCHANGE)
		reqtsupri = tspp->ts_upri;
	else
		reqtsupri = tsparmsp->ts_upri;

	/*
	 * Make sure the user priority doesn't exceed the upri limit.
	 */
	if (reqtsupri > reqtsuprilim)
		reqtsupri = reqtsuprilim;

	/*
	 * Basic permissions enforced by generic kernel code
	 * for all classes require that a process attempting
	 * to change the scheduling parameters of a target
	 * process be super-user or have a real or effective
	 * UID matching that of the target process. We are not
	 * called unless these basic permission checks have
	 * already passed. The time-sharing class requires in
	 * addition that the calling process be super-user if it
	 * is attempting to raise the upri limit above its current
	 * value This may have been checked previously but if our
	 * caller passed us a non-NULL credential pointer we assume
	 * it hasn't and we check it here.
	 */
	if (reqpcredp != NULL) {
		if (reqtsuprilim > tspp->ts_uprilim && !suser(reqpcredp))
			return(EPERM);
	}

	oldlvl = splhi();
	tspp->ts_uprilim = reqtsuprilim;
	tspp->ts_upri = reqtsupri;
	TS_NEWUMDPRI(tspp);

	/*
	 * Set ts_nice to the nice value corresponding to the user
	 * priority we are setting.
	 */
	nice = 20 - (tsparmsp->ts_upri * 20) / ts_maxupri;
	if (nice == 40)
		nice = 39;
	tspp->ts_nice = nice;

	if ((tspp->ts_flags & TSKPRI) != 0) {
		splx(oldlvl);
		return(0);
	}

	tspp->ts_dispwait = 0;
	if (tspp->ts_procp == curproc) {
		if ((curpri = *tspp->ts_pprip =
		    ts_dptbl[tsumdpri].ts_globpri) > maxrunpri)
			tspp->ts_timeleft = ts_dptbl[tsumdpri].ts_quantum;
		else {
			tspp->ts_flags |= TSBACKQ;
			runrun++;
		}
	} else {
		wasonq = dispdeq(tspp->ts_procp);
		*tspp->ts_pprip = ts_dptbl[tsumdpri].ts_globpri;
		if (wasonq == B_TRUE) {
			if (*tspp->ts_pprip > curpri)
				runrun++;
			tspp->ts_timeleft = ts_dptbl[tsumdpri].ts_quantum;
			setbackdq(tspp->ts_procp);
		} else
			tspp->ts_flags |= TSBACKQ;
	}
	splx(oldlvl);
	return(0);
}


/*
 * Arrange for process to be placed in appropriate location
 * on dispatcher queue.  Runs at splhi() since the clock
 * interrupt can cause TSBACKQ to be set.
 */
STATIC void
ts_preempt(tspp)
tsproc_t	*tspp;
{
	register int	oldlvl;

	oldlvl = splhi();

	switch (tspp->ts_flags & (TSBACKQ|TSKPRI|TSFORK)) {
		case TSBACKQ:
			tspp->ts_timeleft = ts_dptbl[tsumdpri].ts_quantum;
			tspp->ts_dispwait = 0;
			tspp->ts_flags &= ~TSBACKQ;
			setbackdq(tspp->ts_procp);
			break;
		case (TSBACKQ|TSKPRI):
			tspp->ts_flags &= ~TSBACKQ;
			setbackdq(tspp->ts_procp);
			break;
		case (TSBACKQ|TSFORK):
			tspp->ts_dispwait = 0;
			tspp->ts_flags &= ~(TSBACKQ|TSFORK);
			setbackdq(tspp->ts_procp);
			break;
		default:
			setfrontdq(tspp->ts_procp);
	}

	splx(oldlvl);
}


/*
 * ts_proccmp() is part of the implementation of the PC_GETPARMS
 * command of the priocntl system call. When the user specifies
 * multiple processes to priocntl PC_GETPARMS the criteria
 * for selecting a process from the set is class specific. The
 * criteria used by the time-sharing class is the upri value
 * of the process. ts_proccmp() simply compares two processes based
 * on their upri values.  All the ugly work of looping through the 
 * processes in the set is done by higher level (class independent)
 * functions.
 */
STATIC int
ts_proccmp(tsproc1p, tsproc2p)
tsproc_t	*tsproc1p;
tsproc_t	*tsproc2p;
{
	return(tsproc1p->ts_upri - tsproc2p->ts_upri);
}


STATIC void
ts_setrun(tspp)
tsproc_t	*tspp;
{
	if ((tspp->ts_flags & TSKPRI) == 0) {
		tspp->ts_timeleft = ts_dptbl[tsumdpri].ts_quantum;
		tspp->ts_dispwait = 0;
	}
	tspp->ts_flags &= ~(TSBACKQ|TSFORK);
	setbackdq(tspp->ts_procp);
}


/*
 * Prepare process for sleep. We reset the process priority so it will
 * run at the requested priority (as specified by the disp argument)
 * when it wakes up.
 */
/* ARGSUSED */
STATIC void
ts_sleep(tsprocp, chan, disp)
register tsproc_t	*tsprocp;
caddr_t			chan;
int			disp;
{
	tsprocp->ts_flags |= TSKPRI;
	*tsprocp->ts_pprip = ts_kmdpris[ts_maxkmdpri - (disp & PMASK)];
}


/*
 * Nominate a process for sched to "swap in".  Choose
 * the highest priority runnable process which is unloaded.
 */
/* ARGSUSED */
STATIC void
ts_swapin(fm, procpp, runflagp)
int	fm;
proc_t	**procpp;
int	*runflagp;
{
	register tsproc_t	*tspp;
	register tsproc_t	*retpp;
	register int		maxpri;
	register int		oldlvl;

	maxpri = -1;
	oldlvl = splhi();
	*runflagp = 0;
	for (tspp = ts_plisthead.ts_next; tspp != &ts_plisthead;
	    tspp = tspp->ts_next) {
		if (pflag & SUSWAP)
			continue;
		if (pstat == SRUN) {
			if ((pflag & SLOAD) == 0) {
				if (ppri > maxpri) {
					retpp = tspp;
					maxpri = ppri;
				}
			} else {
				*runflagp = 1;
			}
		}
	}
	splx(oldlvl);
	if (maxpri == -1)
		*procpp = NULL;
	else
		*procpp = retpp->ts_procp;
}


/*
 * Nominate a process for sched to swap out. Nominate the lowest
 * priority sleeping or stopped process, or if none, nominate
 * the lowest priority runnable process.
 */
/* ARGSUSED */
STATIC void
ts_swapout(fm, jl, procpp, unloadokp)
int		fm;
proc_t		*jl;
proc_t		**procpp;
boolean_t	*unloadokp;
{
	register tsproc_t	*tspp;
	register tsproc_t	*retpp;
	register int		minspri;
	register int		minrpri;
	register int		oldlvl;

	retpp = NULL;
	minspri = minrpri = ts_maxglobpri + 1;
	oldlvl = splhi();
	for (tspp = ts_plisthead.ts_next; tspp != &ts_plisthead;
	    tspp = tspp->ts_next) {
		if (pstat == SZOMB)
			continue;
		if ((pflag & (SLOAD|SSYS|SLOCK|SUSWAP|SPROCIO|SSWLOCKS))
		    != SLOAD)
			continue;
		if (tspp->ts_procp == jl)
			continue;
		if ((pstat == SSLEEP || pstat == SSTOP) && ppri < minspri) {
			retpp = tspp;
			minspri = ppri;
		} else {
			if (retpp == NULL && pstat == SRUN && ppri < minrpri) {
				retpp = tspp;
				minrpri = ppri;
			}
		}
	}
	splx(oldlvl);
	if (retpp == NULL) {
		*procpp = NULL;
	} else {
		*procpp = retpp->ts_procp;
		*unloadokp = B_TRUE;
	}
}


/*
 * Check for time slice expiration.  If time slice has expired
 * move proc to priority specified in tsdptbl for time slice expiration
 * and set runrun to cause preemption.
 */

STATIC void
ts_tick(tspp)
register tsproc_t	*tspp;
{
	register boolean_t	wasonq;

	if ((tspp->ts_flags & TSKPRI) != 0)
		/*
		 * No time slicing of procs at kernel mode priorities.
		 */
		return;

	if (--tspp->ts_timeleft == 0) {
		wasonq = dispdeq(tspp->ts_procp);
		tspp->ts_cpupri = ts_dptbl[tspp->ts_cpupri].ts_tqexp;
		TS_NEWUMDPRI(tspp);
		curpri = *tspp->ts_pprip = ts_dptbl[tsumdpri].ts_globpri;
		tspp->ts_dispwait = 0;
		if (wasonq == B_TRUE) {
			tspp->ts_timeleft = ts_dptbl[tsumdpri].ts_quantum;
			setbackdq(tspp->ts_procp);
		} else
			tspp->ts_flags |= TSBACKQ;
		runrun++;
	}
}


/*
 * If process is currently at a kernel mode priority (has slept)
 * we assign it the appropriate user mode priority and time quantum
 * here.  If we are lowering the process' priority below that of
 * other runnable processes we will normally set runrun here to
 * cause preemption.  We don't do this, however, if a non-preemptive
 * wakeup has occurred since we were switched in because doing so
 * would defeat the non-preemptive wakeup mechanism.
 */
STATIC void
ts_trapret(tspp)
tsproc_t	*tspp;
{
	if ((tspp->ts_flags & TSKPRI) == 0)
		return;

	tspp->ts_cpupri = ts_dptbl[tspp->ts_cpupri].ts_slpret;
	TS_NEWUMDPRI(tspp);
	tspp->ts_flags &= ~TSKPRI;
	curpri = *tspp->ts_pprip = ts_dptbl[tsumdpri].ts_globpri;
	tspp->ts_timeleft = ts_dptbl[tsumdpri].ts_quantum;
	tspp->ts_dispwait = 0;
	if (npwakecnt == 0 && curpri < maxrunpri)
		runrun++;
}


/*
 * Update the ts_dispwait values of all time sharing processes that
 * are currently runnable at a user mode priority and bump the priority
 * if ts_dispwait exceeds ts_maxwait.  Called once per second via
 * timeout which we reset here.
 */
STATIC void
ts_update()
{
	register tsproc_t	*tspp;
	register int		oldlvl;
	register boolean_t	wasonq;

	oldlvl = splhi();
	for (tspp = ts_plisthead.ts_next; tspp != &ts_plisthead;
	    tspp = tspp->ts_next) {
		if ((tspp->ts_flags & TSKPRI) != 0)
			continue;
		if ((pstat != SRUN && pstat != SONPROC) || (pflag & SPROCIO))
			continue;
		tspp->ts_dispwait++;
		if (tspp->ts_dispwait <= ts_dptbl[tsumdpri].ts_maxwait)
			continue;
		tspp->ts_cpupri = ts_dptbl[tspp->ts_cpupri].ts_lwait;
		TS_NEWUMDPRI(tspp);
		tspp->ts_dispwait = 0;
		wasonq = dispdeq(tspp->ts_procp);
		if (tspp->ts_procp == curproc)
			curpri = *tspp->ts_pprip =
			    ts_dptbl[tsumdpri].ts_globpri;
		else
			*tspp->ts_pprip = ts_dptbl[tsumdpri].ts_globpri;
		if (wasonq == B_TRUE) {
			tspp->ts_timeleft = ts_dptbl[tsumdpri].ts_quantum;
			setbackdq(tspp->ts_procp);
		} else
			tspp->ts_flags |= TSBACKQ;
	}
	runrun++;
	splx(oldlvl);
	(void)timeout(ts_update, 0, HZ);
}


/*
 * Processes waking up go to the back of their queue.  We don't
 * need to assign a time quantum here because process is still
 * at a kernel mode priority and the time slicing is not done
 * for processes running in the kernel after sleeping.  The proper
 * time quantum will be assigned by ts_trapret before the process
 * returns to user mode.
 * Note that we ignore the preemption flag (we permit non-preemptive
 * wakeups).
 */
/* ARGSUSED */
STATIC void
ts_wakeup(tsprocp, preemptflg)
tsproc_t	*tsprocp;
int		preemptflg;
{
	tsprocp->ts_flags &= ~(TSBACKQ|TSFORK);
	setbackdq(tsprocp->ts_procp);
}


/*
 * Increment the nice value of the specified process by incr and
 * return the new value in *retvalp.
 */
void
ts_donice(tspp, incr, retvalp)
tsproc_t	*tspp;
int		incr;
int		*retvalp;
{
	int		newnice;
	tsparms_t	tsparms;

	/*
	 * Specifying a nice increment greater than the upper limit of
	 * 2 * NZERO - 1 will result in the process's nice value being
	 * set to the upper limit.  We check for this before computing
	 * the new value because otherwise we could get overflow 
	 * if a super-user specified some ridiculous increment.
	 */
	if (incr > 2 * NZERO - 1)
		incr = 2 * NZERO - 1;

	newnice = tspp->ts_nice + incr;
	if (newnice >= 2 * NZERO)
		newnice = 2 * NZERO - 1;
	else if (newnice < 0)
		newnice = 0;

	tsparms.ts_uprilim = tsparms.ts_upri =
	    -((newnice - NZERO) * ts_maxupri) / NZERO;

	/*
	 * Reset the uprilim and upri values of the process.
	 */
	(void)ts_parmsset(&tsparms, tspp, (id_t)0, (cred_t *)NULL);

	/*
	 * Although ts_parmsset already reset ts_nice it may
	 * not have been set to precisely the value calculated above
	 * because ts_parmsset determines the nice value from the
	 * user priority and we may have truncated during the integer
	 * conversion from nice value to user priority and back.
	 * We reset ts_nice to the value we calculated above.
	 */
	tspp->ts_nice = (char)newnice;

	if (retvalp)
		*retvalp = newnice - NZERO;
}
