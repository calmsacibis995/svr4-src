/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-dsp:priocntl.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/cred.h"
#include "sys/proc.h"
#include "sys/procset.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/priocntl.h"
#include "sys/disp.h"
#include "sys/class.h"
#include "sys/ts.h"
#include "sys/tspriocntl.h"


/*
 * Structure used to pass arguments to the priocntlsys() function.
 */
struct priocntlargs {
	int		pc_version;
	procset_t	*psp;
	int		cmd;
	caddr_t		arg;
};

/*
 * Structure used to pass arguments to the proccmp() function.
 * The arguments must be passed in a structure because proccmp()
 * is called indirectly through the dotoprocs() function which
 * will only pass through a single one word argument.
 */
struct pcmpargs {
	id_t	*pcmp_cidp;
	int	*pcmp_cntp;
	proc_t	**pcmp_retprocpp;
};

/*
 * Structure used to pass arguments to the setparms() function
 * which is called indirectly through dotoprocs().
 */
struct stprmargs {
	struct pcparms	*stp_parmsp;	/* pointer to parameters */
	int		stp_error;	/* some errors returned here */
};


STATIC int	proccmp(), setparms();


/*
 * The priocntl system call.
 */
int
priocntlsys(uap, rvp)
register struct priocntlargs	*uap;
rval_t				*rvp;
{
	pcinfo_t		pcinfo;
	pcparms_t		pcparms;
	pcadmin_t		pcadmin;
	procset_t		procset;
	struct stprmargs	stprmargs;
	struct pcmpargs		pcmpargs;
	int			count;
	proc_t			*retprocp;
	register proc_t		*initpp;
	register int		clnullflag;
	register int		error = 0;

	/*
	 * First just check the version number. Right now there
	 * is only one version we know about and support.  If we
	 * get some other version number from the application it
	 * may be that the application was built with some future
	 * version and is trying to run on an old release of the
	 * system (that's us).  In any case if we don't recognize
	 * the version number all we can do is return error.
	 */
	if (uap->pc_version != PC_VERSION)
		return(EINVAL);


	switch(uap->cmd) {

	case PC_GETCID:

		/*
		 * If the arg pointer is NULL, the user just wants to know
		 * the number of classes. If non-NULL, the pointer should
		 * point to a valid user pcinfo buffer.
		 */
		if (uap->arg == NULL) {
			rvp->r_val1 = nclass;
			break;
		} else {
			if (copyin(uap->arg, (caddr_t)&pcinfo, sizeof(pcinfo)))
				return(EFAULT);
		}

		/*
		 * Get the class ID corresponding to user supplied name.
		 */
		error = getcid(pcinfo.pc_clname, &pcinfo.pc_cid);
		if (error)
			return(error);

		/*
		 * Can't get info about the sys class.
		 */
		if (pcinfo.pc_cid == 0)
			return(EINVAL);

		/*
		 * Get the class specific information.
		 */
		error = CL_GETCLINFO(&class[pcinfo.pc_cid], pcinfo.pc_clinfo,
			u.u_procp->p_cid, u.u_procp->p_cred);
		if (error)
			return(error);

		if (copyout((caddr_t)&pcinfo, uap->arg, sizeof(pcinfo)))
			return(EFAULT);

		rvp->r_val1 = nclass;

		break;

	case PC_GETCLINFO:

		/*
		 * If the arg pointer is NULL, the user just wants to know
		 * the number of classes. If non-NULL, the pointer should
		 * point to a valid user pcinfo buffer.
		 */
		if (uap->arg == NULL) {
			rvp->r_val1 = nclass;
			break;
		} else {
			if (copyin(uap->arg, (caddr_t)&pcinfo, sizeof(pcinfo)))
				return(EFAULT);
		}

		if (pcinfo.pc_cid >= nclass || pcinfo.pc_cid < 1)
			return(EINVAL);

		bcopy(class[pcinfo.pc_cid].cl_name, pcinfo.pc_clname, PC_CLNMSZ);

		/*
		 * Get the class specific information.
		 */
		error = CL_GETCLINFO(&class[pcinfo.pc_cid], pcinfo.pc_clinfo,
			u.u_procp->p_cid, u.u_procp->p_cred);
		if (error)
			return(error);

		if (copyout((caddr_t)&pcinfo, uap->arg, sizeof(pcinfo)))
			return(EFAULT);

		rvp->r_val1 = nclass;

		break;

	case PC_SETPARMS:
		if (copyin(uap->arg, (caddr_t)&pcparms, sizeof(pcparms)))
			return(EFAULT);

		/*
		 * First check the validity of the parameters
		 * we got from the user.  We don't do any permissions
		 * checking here because it's done on a per process
		 * basis by parmsset().
		 */
		error = parmsin(&pcparms, NULL, NULL);
		if (error)
			return(error);

		/*
		 * Get the procset from the user.
		 */
		if (copyin((caddr_t)uap->psp, (caddr_t)&procset,
		    sizeof(procset)))
			return(EFAULT);

		/*
		 * For performance we do a quick check here to catch common
		 * cases where the current process is the only one in the
		 * set.  In such cases we can call parmsset() directly,
		 * avoiding the relatively lengthy path through dotoprocs().
		 */
		if (cur_inset_only(&procset) == B_TRUE) {
			error = parmsset(&pcparms, u.u_procp, u.u_procp);
			if (error == 0) {
				/*
				 * parmsset() succeeded in changing the
				 * parameters so notify the events
				 * mechanism in case it is interested.
				 */
				ev_newpri(u.u_procp);
			}
		} else {
			stprmargs.stp_parmsp = &pcparms;
			stprmargs.stp_error = 0;

			/*
			 * The dotoprocs() call below will cause setparms()
			 * to be called for each process in the specified
			 * procset. setparms() will in turn call parmsset()
			 * (which does the real work).
			 */
			error = dotoprocs(&procset, setparms,
			    (char *)&stprmargs);
	
			/*
			 * If setparms() encounters a permissions error for
			 * one or more of the processes it returns EPERM in
			 * stp_error so dotoprocs() will continue through
			 * the process set.  If dotoprocs() returned an error
			 * above, it was more serious than permissions and
			 * dotoprocs quit when the error was encountered.
			 * We return the more serious error if there was one,
			 * otherwise we return EPERM if we got that back.
			 */
			if (error == 0 && stprmargs.stp_error != 0)
				error = stprmargs.stp_error;
		}
		break;

	case PC_GETPARMS:
		if (copyin(uap->arg, (caddr_t)&pcparms, sizeof(pcparms)))
			return(EFAULT);

		if (pcparms.pc_cid >= nclass ||
		    (pcparms.pc_cid < 1 && pcparms.pc_cid != PC_CLNULL))
			return(EINVAL);

		if (copyin((caddr_t)uap->psp, (caddr_t)&procset,
		    sizeof(procset)))
			return(EFAULT);

		/*
		 * Check to see if the current process is the only one
		 * in the set. If not we must go through the whole set
		 * to select a process.
		 */
		if (cur_inset_only(&procset) == B_TRUE)
			if (pcparms.pc_cid != PC_CLNULL &&
			    pcparms.pc_cid != u.u_procp->p_cid)

				/*
				 * Specified process not in specified class.
				 */
				return(ESRCH);
			else
				retprocp = u.u_procp;
		else {

			/*
			 * Select the process (from the set) whose
			 * parameters we are going to return.  First we
			 * set up some locations for return values, then
			 * we call proccmp() indirectly through dotoprocs().
			 * proccmp() will call a class specific routine which
			 * actually does the selection.  To understand how
			 * this works take a careful look at the code below,
			 * the dotoprocs() function, the proccmp() function,
			 * and the class specific cl_proccmp() functions.
			 */
			if (pcparms.pc_cid == PC_CLNULL)
				clnullflag = 1;
			else
				clnullflag = 0;
			count = 0;
			retprocp = NULL;
			pcmpargs.pcmp_cidp = &pcparms.pc_cid;
			pcmpargs.pcmp_cntp = &count;
			pcmpargs.pcmp_retprocpp = &retprocp;
	
			error = dotoprocs(&procset, proccmp, (char *)&pcmpargs);
			if (error)
				return(error);
			/*
			 * dotoprocs() ignores the init process if it is
			 * in the set, unless it was the only process found.
			 * Since we are getting parameters here rather than
			 * setting them, we want to make sure init is not
			 * excluded if it is in the set.
			 */
			initpp = prfind(P_INITPID);
			ASSERT(initpp != NULL);
			if (procinset(initpp, &procset) && retprocp != initpp)
				(void)proccmp(initpp, &pcmpargs);
	
			/*
			 * If dotoprocs returned success it found at least
			 * one process in the set.  If proccmp() failed to
			 * select a process it is because the user specified
			 * a class and none of the processes in the set
			 * belonged to that class.
			 */
			if (retprocp == NULL) {
				ASSERT(clnullflag == 0);
				return(ESRCH);
			}

			/*
			 * User can only use PC_CLNULL with one process in set.
			 */
			if (clnullflag && count > 1)
				return(EINVAL);
		}

		/*
		 * We've selected a process so now get the parameters.
		 */
		parmsget(retprocp, &pcparms);
	
		/*
		 * Prepare to return parameters to the user
		 */
		error = parmsout(&pcparms, u.u_procp, retprocp);
		if (error)
			return(error);

		if (copyout((caddr_t)&pcparms, uap->arg, sizeof(pcparms)))
			return(EFAULT);

		/*
		 * And finally, return the pid of the selected process.
		 */
		rvp->r_val1 = retprocp->p_pid;

		break;

	case PC_ADMIN:
		if (copyin(uap->arg, (caddr_t)&pcadmin, sizeof(pcadmin_t)))
			return(EFAULT);

		if (pcadmin.pc_cid >= nclass || pcadmin.pc_cid < 1)
			return(EINVAL);

		/*
		 * Have the class do whatever the user is requesting.
		 */
		error = CL_ADMIN(&class[pcadmin.pc_cid], pcadmin.pc_cladmin,
		    u.u_procp->p_cid, u.u_procp->p_cred);
		break;

	default:
		error = EINVAL;
		break;
	}
	return(error);
}


struct niceargs {
	int	niceness;
};

/*
 * We support the nice system call for compatibility although
 * the priocntl system call supports a superset of nice's functionality.
 * We support nice only for time sharing processes.  It will fail
 * if called by a process from another class.
 * The code below is an aberration.  Class independent kernel code
 * (such as this) should never require specific knowledge of any
 * particular class.  An alternative would be to add a CL_NICE
 * function call to the class switch and require all classes other than
 * time sharing to implement a class specific nice function which simply
 * returns EINVAL.  This would be even uglier than the approach we have
 * chosen below.
 */

extern int	donice();

int
nice(uap, rvp)
register struct niceargs	*uap;
rval_t				*rvp;
{
	int error, retval;

	if (error = donice(u.u_procp, u.u_cred, uap->niceness, &retval))
		return(error);

	rvp->r_val1 = retval;
	return(0);
}


extern void	ts_donice();

int
donice(pp, cr, incr, retvalp)
proc_t	*pp;
cred_t	*cr;
int	incr;
int	*retvalp;
{

	/*
	 * Check that the class is time-sharing.
	 */
	if (strcmp(class[pp->p_cid].cl_name, "TS") != 0)
		/*
		 * Process from some class other than time-sharing.
		 */
		return(EINVAL);

#ifdef VPIX
	/* Do the permission check on the user's credentials ONLY
	** if this is NOT a vpix (v86) process, since the v86 kernel
	** code will attempt to adjust the user's nice value even if
	** the user isn't super user. */
	if (!pp->p_v86)
#endif
		if ((incr < 0 || incr > 2 * NZERO) && !suser(cr))
			return(EPERM);

	/*
	 * Call the time-sharing class to take care of
	 * all the time-sharing specific stuff.
	 */
	ts_donice(pp->p_clproc, incr, retvalp);

	return(0);
}


/*
 * The proccmp() function is part of the implementation of the
 * PC_GETPARMS command of the priocntl system call.  This function
 * works with the system call code and with the class specific
 * cl_proccmp() function to select one process from a specified
 * procset based on class specific criteria. proccmp() is called
 * indirectly from the priocntl code through the dotoprocs function.
 * Basic strategy is dotoprocs() calls us once for each process in
 * the set.  We in turn call the class specific function to compare
 * the current process from dotoprocs to the "best" (according to
 * the class criteria) found so far.  We keep the "best" process
 * in *pcmp_retprocpp.
 */
STATIC int
proccmp(pp, argp)
proc_t		*pp;
struct pcmpargs	*argp;
{
	register caddr_t	clproc1p;
	register caddr_t	clproc2p;

	(*argp->pcmp_cntp)++;	/* Increment count of procs in the set */
	if (*argp->pcmp_cidp == PC_CLNULL)
		*argp->pcmp_cidp = pp->p_cid;
	else if (pp->p_cid != *argp->pcmp_cidp)

		/*
		 * Process is in set but not in class.
		 */
		return(0);

	if (*argp->pcmp_retprocpp == NULL) {

		/*
		 * First time through for this set.
		 */
		*argp->pcmp_retprocpp = pp;
		return(0);
	} else {
		clproc2p = (*argp->pcmp_retprocpp)->p_clproc;
	}

	clproc1p = pp->p_clproc;
	if (CL_PROCCMP(pp, clproc1p, clproc2p) > 0)
		*argp->pcmp_retprocpp = pp;
	return(0);
}


/*
 * The setparms() function is called indirectly by priocntlsys()
 * through the dotoprocs() function).  setparms() acts as an
 * intermediary between dotoprocs() and the parmsset() function,
 * calling parmsset() for each process in the set and handling
 * the error returns on their way back up to dotoprocs().
 */
STATIC int
setparms(targpp, stprmp)
register proc_t			*targpp;
register struct stprmargs	*stprmp;
{
	register int	error;

	error = parmsset(stprmp->stp_parmsp, u.u_procp, targpp);
	if (error) {
		if (error == EPERM) {
			stprmp->stp_error = EPERM;
			return(0);
		} else {
			return(error);
		}
	} else {
		
		/*
		 * parmsset() succeded in changing the parameters so notify
		 * the events mechanism in case it is interested.
		 */
		ev_newpri(targpp);
		return(0);
	}
}
