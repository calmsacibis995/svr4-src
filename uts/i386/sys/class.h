/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ifndef _SYS_CLASS_H
#define _SYS_CLASS_H

#ident	"@(#)head.sys:sys/class.h	1.14.3.1"


/*
 * NOTE: Developers making use of the scheduler class switch mechanism
 * to develop scheduling class modules should be aware that the
 * architecture is not frozen and the kernel interface for scheduling
 * class modules may change in future releases of System V.  Support
 * for the current interface is not guaranteed and class modules
 * developed to this interface may require changes in order to work
 * with future releases of the system.
 */


extern int	nclass;		/* number of configured scheduling classes */
extern char	*initclass;	/* class of init process */

typedef struct class {
	char	*cl_name;	/* class name */
	void	(*cl_init)();	/* class specific initialization function */
	struct classfuncs *cl_funcs;	/* pointer to classfuncs structure */
} class_t;

extern struct class	class[];	/* the class table */

typedef struct classfuncs {
	int		(*cl_admin)();
	int		(*cl_enterclass)();
	void		(*cl_exitclass)();
	int		(*cl_fork)();
	void		(*cl_forkret)();
	int		(*cl_getclinfo)();
	void		(*cl_getglobpri)();
	void		(*cl_parmsget)();
	int		(*cl_parmsin)();
	int		(*cl_parmsout)();
	int		(*cl_parmsset)();
	void		(*cl_preempt)();
	int		(*cl_proccmp)();
	void		(*cl_setrun)();
	void		(*cl_sleep)();
	void		(*cl_stop)();
	void		(*cl_swapin)();
	void 		(*cl_swapout)();
	void		(*cl_tick)();
	void		(*cl_trapret)();	/* Don't move without changing */
						/*  .set in ml/ttrap.s */
	void		(*cl_wakeup)();
	int		(*cl_filler[11])();
} classfuncs_t;


#define	CL_ADMIN(clp, uaddr, reqpcid, reqpcredp) \
(*(clp)->cl_funcs->cl_admin)(uaddr, reqpcid, reqpcredp)

#define	CL_ENTERCLASS(clp, clparmsp, pp, pstatp, pprip, pflagp, pcredpp, clprocpp, reqpcid, reqpcredp) \
(*(clp)->cl_funcs->cl_enterclass)\
  (clparmsp, pp, pstatp, pprip, pflagp, pcredpp, clprocpp, reqpcid, reqpcredp)

#define	CL_EXITCLASS(pp, clprocp) (*(pp)->p_clfuncs->cl_exitclass)(clprocp)

#define CL_FORK(pp, pclprocp, cprocp, cpstatp, cpprip, cpflagp, cpcredpp, clprocpp) \
(*(pp)->p_clfuncs->cl_fork)\
  (pclprocp, cprocp, cpstatp, cpprip, cpflagp, cpcredpp, clprocpp)

#define	CL_FORKRET(cp, cclprocp, pclprocp) \
(*(cp)->p_clfuncs->cl_forkret)(cclprocp, pclprocp)

#define	CL_GETCLINFO(clp, clinfop, reqpcid, reqpcredp) \
(*(clp)->cl_funcs->cl_getclinfo)(clinfop, reqpcid, reqpcredp)

#define CL_GETGLOBPRI(clp, clparmsp, globprip) \
(*(clp)->cl_funcs->cl_getglobpri)(clparmsp, globprip)

#define	CL_PARMSGET(pp, clprocp, clparmsp) \
    (*(pp)->p_clfuncs->cl_parmsget)(clprocp, clparmsp)

#define CL_PARMSIN(clp, clparmsp, rqpcid, rqpcredp, tgpcid, tgpcredp, tgpclpp) \
(*(clp)->cl_funcs->cl_parmsin)\
(clparmsp, rqpcid, rqpcredp, tgpcid, tgpcredp, tgpclpp)

#define CL_PARMSOUT(clp, clparmsp, reqpcid, reqpcredp, targpcredp) \
(*(clp)->cl_funcs->cl_parmsout)(clparmsp, reqpcid, reqpcredp, targpcredp)

#define	CL_PARMSSET(pp, clparmsp, clprocp, reqpcid, reqpcredp) \
    (*(pp)->p_clfuncs->cl_parmsset)(clparmsp, clprocp, reqpcid, reqpcredp)

#define CL_PREEMPT(pp, clprocp) (*(pp)->p_clfuncs->cl_preempt)(clprocp)

#define CL_PROCCMP(pp, clproc1p, clproc2p) \
    (*(pp)->p_clfuncs->cl_proccmp)(clproc1p, clproc2p)

#define CL_SETRUN(pp, clprocp) (*(pp)->p_clfuncs->cl_setrun)(clprocp)

#define CL_SLEEP(pp, clprocp, chan, disp) \
    (*(pp)->p_clfuncs->cl_sleep)(clprocp, chan, disp)

#define CL_STOP(pp, clprocp, why, what) \
    (*(pp)->p_clfuncs->cl_stop)(clprocp, why, what)

#define CL_SWAPIN(clp, fmem, procpp, runflagp) \
    (*(clp)->cl_funcs->cl_swapin)(fmem, procpp, runflagp)

#define CL_SWAPOUT(clp, fmem, justloaded, procpp, unloadokp) \
    (*(clp)->cl_funcs->cl_swapout)(freemem, justloaded, procpp, unloadokp)

#define CL_TICK(pp, clprocp) (*(pp)->p_clfuncs->cl_tick)(clprocp)

#define CL_TRAPRET(pp, clprocp) (*(pp)->p_clfuncs->cl_trapret)(clprocp)

#define CL_WAKEUP(pp, clprocp, preemptflg) \
    (*(pp)->p_clfuncs->cl_wakeup) (clprocp, preemptflg)

#endif	/* _SYS_CLASS_H */
