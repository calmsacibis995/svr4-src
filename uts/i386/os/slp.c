/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:slp.c	1.3"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/map.h"
#include "sys/vfs.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/disp.h"
#include "sys/class.h"
#include "sys/fault.h"
#include "sys/syscall.h"
#include "sys/ucontext.h"
#include "sys/prsystm.h"
#include "sys/priocntl.h"
#include "sys/evecb.h"
#include "sys/hrtcntl.h"
#include "sys/events.h"
#include "sys/evsys.h"

/*
 * sleep-wakeup hashing:  Each entry in sleepq[] points
 * to the front and back of a linked list of sleeping processes.
 * Processes going to sleep go to the back of the appropriate
 * sleep queue and wakeprocs wakes them from front to back (so the
 * first process to go to sleep on a given channel will be the first
 * to run after a wakeup on that channel).
 * NSLEEPQ must be a power of 2.  Sqhash(x) is used to index into
 * sleepq[] based on the sleep channel.
 */

#define NSLEEPQ		64
#define sqhash(X)	(&sleepq[((int)X >> 3) & (NSLEEPQ - 1)])

struct sleepq {
	proc_t	*sq_first;
	proc_t	*sq_last;
} sleepq[NSLEEPQ];

#ifdef KPERF
/*
 * This is used to hold the addr of each process being awaken up 
 * for that chan so they will be written to the buffer at the end 
 * of wakeprocs call.
 */

caddr_t	wpp[1024];
#endif /* KPERF */

void	unsleep(), wakeprocs(), setrun();

/*
 * Give up the processor till a wakeup occurs
 * on chan. The disp argument determines whether
 * the sleep can be interrupted by a signal. If
 * disp & PMASK <= PZERO the SNWAKE bit in p_flag
 * is set and a signal cannot disturb the sleep;
 * if disp & PMASK > PZERO signals will be processed.
 * Callers of this routine must be prepared for
 * premature return, and check that the reason for
 * sleeping has gone away.
 */

int
sleep(chan, disp)
	caddr_t		chan;
	register int	disp;
{
	register proc_t		*pp = u.u_procp;
	register struct sleepq	*sqp;
	register int		s;
	int			why;

#ifdef KPERF
	if (kpftraceflg) {
		/* asm( "MOVW 8(%ap),Kpc "); */
		asm ( "movl 4(%ebp), Kpc "); /* move program counter into Kpc */
		kperf_write(KPT_SLEEP, Kpc, curproc);
	}
#endif /* KPERF */

	s = splhi();
	if (panicstr) {
		(void) spl0();
		splx(s);
		return 0;
	}

	/*
	 * Put process on sleep queue.
	 */
	ASSERT(chan != 0);

	pp->p_stat = SSLEEP;
	pp->p_wchan = chan;
	sqp = sqhash(chan);
	if (sqp->sq_first == NULL) {
		ASSERT(sqp->sq_last == NULL);
		sqp->sq_first = sqp->sq_last = pp;
	} else {
		sqp->sq_last->p_link = pp;
		sqp->sq_last = pp;
	}
	pp->p_link = NULL;
	if ((disp & PMASK) > PZERO) {
		pp->p_flag &= ~(SNWAKE|SNOSTOP);

		/*
		 * If it's not safe to stop here the caller will have set
		 * PNOSTOP in "disp"; in this case make sure that we won't
		 * stop when issig() is called.
		 */
		if (disp & PNOSTOP) {
			pp->p_flag |= SNOSTOP;
			why = JUSTLOOKING;
		} else
			why = FORREAL;
		pp->p_flag |= SASLEEP;
		u.u_sysabort = 0;
		if (ISSIG(pp, why) || u.u_sysabort || EV_ISTRAP(pp)) {
			/*
			 * Signal pending or debugger aborted the syscall.
			 * Take process off sleep queue if still there.
			 */
			u.u_sysabort = 0;
			if (pp->p_stat == SSLEEP) {
				unsleep(pp);
				pp->p_stat = SONPROC;
			}
			goto psig;
		}

		/*
		 * We may have stopped in issig(); make sure we're still
		 * on the sleep queue.
		 */
		if (pp->p_wchan == 0)
			goto out;
		ASSERT(pp->p_stat == SSLEEP);
		if (runin != 0) {
			runin = 0;
			wakeprocs((caddr_t)&runin, PRMPT);
		}
		pp->p_flag &= ~SASLEEP;

		/* 
		 * Call class specific function to do whatever it deems
		 * appropriate before we give up the processor.
		 */
		CL_SLEEP(pp, pp->p_clproc, chan, disp);

		/*
		 * A debugger may set u_sysabort while we are asleep.
		 * If it is set when we wake up, abort the syscall.
		 * We may also stop in issig(); check on return as well.
		 */
		swtch();
		pp->p_flag |= SASLEEP;
		if (u.u_sysabort || ISSIG(pp, why) || u.u_sysabort
		  || EV_ISTRAP(pp)) {
			u.u_sysabort = 0;
			goto psig;
		}
	} else {
		pp->p_flag |= SNWAKE;

		CL_SLEEP(pp, pp->p_clproc, chan, disp);

		swtch();
	}
out:
	pp->p_flag &= ~(SNWAKE|SNOSTOP|SASLEEP);
	if (pp->p_trace)	/* hook for /proc deadlock prevention */
		prawake(pp);
	splx(s);
	return 0;

	/*
	 * If priority was low (>PZERO) and there has been a signal,
	 * then if PCATCH is set return 1, otherwise do a non-local
	 * jump to the qsav location.
	 */
psig:
	pp->p_flag &= ~(SNOSTOP|SASLEEP);
	if (pp->p_trace)	/* hook for /proc deadlock prevention */
		prawake(pp);
	(void) splx(s);
	if (disp & PCATCH)
		return 1;

	longjmp(&u.u_qsav);

	/* NOTREACHED */
}


/*
 * Remove a process from its sleep queue.
 */
void
unsleep(pp)
	register proc_t	*pp;
{
	register proc_t		*rp;
	register proc_t		*prp;
	register struct sleepq	*sqp;
	register int		s;

	s = splhi();
	if (pp->p_wchan) {
		sqp = sqhash(pp->p_wchan);
		rp = sqp->sq_first;
		prp = NULL;
		ASSERT(sqp->sq_last == NULL || sqp->sq_last->p_link == NULL);
		while (rp != pp) {
			prp = rp;
			rp = rp->p_link;
		}
		if (prp == NULL) {
			if ((sqp->sq_first = rp->p_link) == NULL)
				sqp->sq_last = NULL;
		} else {
			if ((prp->p_link = rp->p_link) == NULL)
				sqp->sq_last = prp;
		}
		pp->p_wchan = 0;
	}
	splx(s);
}


/*
 * Wake up all processes sleeping on chan.  Note that a process on a
 * sleep queue may be either in state SSLEEP or in state SSTOP; if it
 * was stopped we remove it from its sleep queue but we don't set it
 * running here.  The preemptflg indicates whether the caller is
 * requesting a preemptive or non-preemptive wakeup.  Preemptive
 * wakeups are the norm.  Non-preemptive wakeups should only be used
 * in situations where is is critical to limit the context switching
 * between cooperating procsses (e.g.readers and writers on a pipe).
 * Also, non-preemptive wakups may not be supported by all scheduling
 * classes (the class may force the preemption regardless of the value
 * of preemptflg).
 */
void
wakeprocs(chan, preemptflg)
	register caddr_t	chan;
	int			preemptflg;
{
	register proc_t		*pp;
	register proc_t		*rp;
	register proc_t		**plinkp;
	register struct sleepq	*sqp;
	int			runsched = 0;
	register int		s;

#ifdef KPERF
	int			wppcount = 0;
	int			index;
	if (kpftraceflg) {
		/* asm( "MOVW 8(%ap),Kpc "); */
		asm( "movl 4(%ebp), Kpc ");
	}
#endif /* KPERF */

	s = splhi();
	sqp = sqhash(chan);
	rp = NULL;
	for (plinkp = &sqp->sq_first; (pp = *plinkp) != NULL; ) {
		ASSERT(pp->p_stat == SSLEEP || pp->p_stat == SSTOP);
		if (pp->p_wchan == chan) {
#ifdef KPERF
			if (kpftraceflg && outbuf == 0) {
				if(wppcount < 1024) {
					wpp[wppcount]= (caddr_t )pp;
					wppcount++;
				}
			}
#endif /* KPERF */
			/* 
			 * Take off sleep queue, put on dispatcher queue
			 * if not SSTOP.
			 */
			pp->p_wchan = 0;
			*plinkp = pp->p_link;
			if (pp->p_stat == SSLEEP) {
				pp->p_stat = SRUN;
				if (preemptflg == NOPRMPT)
					npwakecnt++;

				/*
				 * Call class specific wakeup function.
				 * Although we pass the preemptflg the
				 * class specific code we call here may
				 * choose to ignore requests for
				 * non-preemptive wakeups in accordance
				 * with its particular scheduling policy.
				 */
				CL_WAKEUP(pp, pp->p_clproc, preemptflg);

				/*
				 * Make arrangements for swapin or
				 * preemption if necessary.
				 */
				if ((pp->p_flag & SLOAD) == 0) {
					/*
					 * We must not call setrun here!
					 */
					if (runout > 0)
						runsched = 1;
				} else if (pp->p_pri > curpri &&
				    preemptflg != NOPRMPT)
					runrun = 1;
			}
		} else {
			rp = pp;
			plinkp = &pp->p_link;
		}
	}
	sqp->sq_last = rp;
	if (runsched) {
		runout = 0;
		setrun(proc_sched);
	}
#ifdef KPERF
	if (kpftraceflg && outbuf == 0) {
		/*
		 * write a record for each process awaken up.
		 */
		for (index=0;index < wppcount; index++) {
			kperf_write(KPT_WAKEUP, Kpc, wpp[index]);
		}
	}
#endif /* KPERF */
	
	splx(s);
}


/*
 * Set the process running; arrange for it to be swapped in if necessary.
 */
void
setrun(pp)
	register proc_t	*pp;
{
	register int s;

	s = splhi();
	if (pp->p_stat == SSLEEP || pp->p_stat == SSTOP) {

		/*
		 * Take off sleep queue.
		 */
		unsleep(pp);
	} else if (pp->p_stat == SRUN) {

		/*
		 * Already on dispatcher queue.
		 */
		splx(s);
		return;
	}

	ASSERT(pp->p_stat != SONPROC);

	if (pp->p_stat == SSTOP) {

		/*
		 * Both ptrace() or the sending of SIGCONT (SXSTART) and
		 * /proc (SPSTART) must have requested that the process be run.
		 * Just calling setrun() is not sufficient to set a stopped
		 * process running.  SXSTART is always set if the process is
		 * not ptrace()d or stopped by a jobcontrol stop signal.
		 * SPSTART is always set if /proc is not controlling it.
		 * The process won't be stopped unless one of ptrace() or
		 * receipt of a jobcontrol stop signal or /proc did it.
		 * 
		 * These flags must be set before calling setrun(p).
		 * They can't be passed as arguments because the streams
		 * code calls setrun() indirectly and the mechanism for
		 * doing so admits only one argument.
		 */
		if ((pp->p_flag & (SXSTART|SPSTART)) != (SXSTART|SPSTART)) {
			/*
			 * Wake up ptrace()d process's parent if /proc issued
			 * the run request.  It may be sleeping in ptrace().
			 */
			if ((pp->p_flag & SPSTART)
			  && pp->p_whystop == PR_SIGNALLED)
				wakeprocs((caddr_t)pp->p_parent, PRMPT);
			splx(s);
			return;
		}
		/*
		 * Process is no longer stopped.
		 * Make sure wait(2) will not see it until it stops again.
		 */
		pp->p_wcode = 0;
		pp->p_wdata = 0;
	}

	ASSERT(pp->p_wchan == 0);
	pp->p_stat = SRUN;
	pp->p_whystop = 0;
	pp->p_whatstop = 0;

	/*
	 * Let the class put the process on the dispatcher queue.
	 */
	CL_SETRUN(pp, pp->p_clproc);

	/*
	 * Make arrangements for swapin or preemption if necessary.
	 */
	if ((pp->p_flag & SLOAD) == 0) {
		if (runout > 0) {
			runout = 0;
			setrun(proc_sched);
		}
	} else if (pp->p_pri > curpri)
		runrun = 1;

	splx(s);
}


void
wakeup(chan)
register caddr_t	chan;
{
	wakeprocs(chan, PRMPT);
}
