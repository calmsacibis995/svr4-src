/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_DISP_H
#define _SYS_DISP_H

#ident	"@(#)head.sys:sys/disp.h	1.13.3.1"

#ifndef _SYS_PRIOCNTL_H
#include <sys/priocntl.h>
#endif


/*
 * The following is the format of a dispatcher queue entry.
 */
typedef struct dispq {
	struct proc	*dq_first;	/* first proc on queue or NULL */
	struct proc	*dq_last;	/* last proc on queue or NULL */
	int		dq_sruncnt;	/* no. of loaded, runnable procs on queue */
} dispq_t;


#ifdef _KERNEL

/*
 * Global scheduling variables.
 */
extern int	runrun;		/* preemption flag */
extern int	kprunrun;	/* kernel preemption flag */
extern int	npwakecnt;	/* count of non-preemptive wakeups */
extern struct proc *curproc;	/* currently running process */
extern int	curpri;		/* priority of current process */
extern int	maxrunpri;	/* priority of highest priority active queue */


/*
 * Public scheduling functions.
 */
#if defined(__STDC__)

extern boolean_t	dispdeq(proc_t *pp);
extern int	getcid(char *clname, id_t *cidp);
extern int	parmsin(pcparms_t *parmsp, proc_t *reqpp, proc_t *targpp);
extern int	parmsout(pcparms_t *parmsp, proc_t *reqpp, proc_t *targpp);
extern int	parmsset(pcparms_t *parmsp, proc_t *reqpp, proc_t *targpp);
extern void	dispinit();
extern void	getglobpri(pcparms_t *parmsp, int *globprip);
extern void	parmsget(proc_t *pp, pcparms_t *parmsp);
extern void	preempt();
extern void	setbackdq(proc_t *pp);
extern void	setfrontdq(proc_t *pp);
#ifndef KPERF
extern void	swtch();
#endif /* KPERF */
extern void	dq_sruninc(int pri);
extern void	dq_srundec(int pri);

#else

extern boolean_t	dispdeq();
extern int	getcid(), parmsin(), parmsout(), parmsset();
extern void	dispinit(), getglobpri(), parmsget(), preempt();
extern void	setbackdq(), setfrontdq(), swtch(), dq_sruninc(), dq_srundec();

#endif	/* __STDC__ */


#ifdef	KPERF
#define PREEMPT() \
{ \
	if (kprunrun != 0) { \
		preempt(); \
	} \
	else if (kpftraceflg) { \
		asm("  movl	%eip, Kpc"); \
		kperf_write(KPT_PREEMPT,Kpc,curproc); \
	} \
}

#else	/* !KPERF */

#define	PREEMPT()	if (kprunrun != 0){ \
				preempt(); \
			}

#endif	/* KPERF */

#endif	/* _KERNEL */

#endif	/* _SYS_DISP_H */
