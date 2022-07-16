/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:hrtimers.c	1.3.1.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/cred.h"
#include "sys/callo.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/inline.h"
#include "sys/debug.h"
#include "sys/vnode.h"
#include "sys/evecb.h"
#include "sys/hrtcntl.h"
#include "sys/dl.h"
#include "sys/errno.h"
#include "sys/priocntl.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/procset.h"
#include "sys/events.h"
#include "sys/evsys.h"
#include "sys/hrtsys.h"
#include "sys/time.h"
#include "sys/cmn_err.h"

#ifdef KPERF
#include "sys/disp.h"
#endif /* KPERF */

/*	This file contains the code that manages the hardware clocks and 
**	timers.  We must provide UNIX with a HZ resolution clock and give 
**	the user an interface to the timers through system calls.
*/

timer_t		hrt_active;	/* List of active high-resolution	*/
				/* timer structures.			*/
timer_t		hrt_avail;	/* List of available high-resolution	*/
				/* timer structures.			*/
timer_t		hrt_null;	/* Null structure for initializing.	*/

timer_t		it_avail;	/* List of available interval		*/
				/* timer structures.			*/

/* moved to hrtcntl.h */
/* scaling factor used to save fractional remainder */
/*
#define SCALE		1000000	
*/

dl_t scalel = { SCALE,0 };
dl_t tick;

extern uint	timer_resolution;	/* Resolution of hardware	*/
					/* timer.  Interrupts at	*/
					/* timer_resolution times per	*/
					/* second.			*/
extern	caddr_t	waitloc;

uint		ticks_til_clock;	/* Number of times the hardware	*/
					/* clock has to interrupt	*/
					/* before calling the UNIX	*/
					/* clock routine.		*/
uint		unix_tick;		/* Time remaining until the	*/
					/* next call to the UNIX clock	*/
					/* routine.			*/

ulong		 hr_lbolt;		/* High resolution lighting	*/
					/* bolt.			*/
/*
 * Argument vectors for the various flavors of hrtsys().
 */

#define	HRTCNTL		0
#define	HRTALARM	1
#define HRTSLEEP	2
#define	HRTCANCEL	3

struct 	hrtsysa {
	int	opcode;
};

struct	hrtcntla {
	int		opcode;
	int		cmd;
	int		clk;
	interval_t	*intp;
	hrtime_t	*hrtp;
};

struct	hrtalarma {
	int	opcode;
	hrtcmd_t	*cmdp;
	int		cmds;
};

struct	hrtsleepa {
	int	opcode;
	hrtcmd_t	*cmdp;
};

struct	hrtcancela {
	int	opcode;
	long	*eidp;
	int	eids;
};

clock_int(pc,cs,flags, oldipl)
caddr_t	pc;
int	cs;
int	flags;
int	oldipl;
{
	register timer_t	*hrp, *nhrp;
	register proc_t		*prp;
	register int		type;   /* type of virtual clock */
					/* 0 - CLK_USERVIRT	 */
					/* 1 - CLK_PROCVIRT	 */

#ifdef	KPERF
	/*
	** hr_lbolt is incremented at the start of this routine to allow
	**	correct time-stamping of Kernel PERFormance records.
	*/

	++hr_lbolt;

	if (kpftraceflg)
		kperf_write(KPT_INTR, clock_int, curproc);
#endif	/* KPERF */
	/*	If panic, stop clock
	*/

	if(panicstr){
		clkreld();
		return(0);
	}
	
	/*	Process the high-resolution timer list.
	*/

	if(hrt_active.hrt_next != &hrt_active){

		/*	There is an active high-resolution timer.
		**	Decrement it and see if it fires.
		*/

		hrp = hrt_active.hrt_next;
		if(--hrp->hrt_time == 0){

			/*	The timer fired.  If this was some
			**	kind of an alarm, then post the event.
			**	If it was a sleep, then do the wakeup.
			*/

			do{
				/*	Remove the entry from the list.
				*/

				hrt_dequeue(hrp);

				if(hrp->hrt_cmd == HRT_INTSLP){
					wakeup((caddr_t)hrp);
				} else {
					hrp->hrt_fn(hrp, 0, 0);
				}

				if(hrp->hrt_int){
				/* Repeative alarm */
					hrp->hrt_crem += hrp->hrt_rem;
					hrp->hrt_time = hrp->hrt_int;
					if(hrp->hrt_crem >= tick.dl_lop){
						hrp->hrt_time += 1;
						hrp->hrt_crem -= tick.dl_lop;
					};
					hrt_timeout(hrp, 0);
				} else {
					hrt_free(hrp, &hrt_avail);
				}

				hrp = hrt_active.hrt_next;
			} while(hrp != &hrt_active  &&  hrp->hrt_time == 0);
		}
	}

	/*
         *	We are using u_uservirt to implement the clock 
	 *	measuring the user process virtual time and
	 * 	u_procvirt for the process' virtual
	 *	time clock.
 	 *	Charge the time out based on the mode the cpu is in.
	 *	We assume that the current state has been around at
	 *	least one tick.
	 */
	if (USERMODE(cs)) {
		/*
		 * Update process virtual time and
		 * user virtual time.
		 */
		u.u_uservirt++;
		u.u_procvirt++;
		type = 0;
	} else {
		if (pc == waitloc) {
			type = 2;
		} else {
			/* Update process virtual time */
			u.u_procvirt++;
			type = 1;
		}
	}

	/* 	Process the interval-timers list.
	*/

	prp = u.u_procp;

	while (type < 2) {

		hrp = prp->p_italarm[type];

		if (hrp != NULL) {

			/*	There is an active interval timer.
			**	Decrement it and see if it fires.
			*/

			if (--hrp->hrt_time == 0) {

				/*	The timer fired. Post an event. */

				do {
					nhrp = hrp->hrt_next;

					/*	Remove the entry from the list.
					*/
					itimer_dequeue(hrp, type);

					hrp->hrt_fn(hrp, 0, 0);

					if(hrp->hrt_int){
					/* Repeative alarm */
						hrp->hrt_crem += hrp->hrt_rem;
						hrp->hrt_time = hrp->hrt_int;
						if(hrp->hrt_crem >= tick.dl_lop){
							hrp->hrt_time += 1;
							hrp->hrt_crem -= tick.dl_lop;
						};
						itimer_timeout(hrp, type, 0, 0);
					} else {
						hrt_free(hrp, &it_avail);
					}

					hrp = nhrp;
				} while (hrp != NULL && hrp->hrt_time == 0);
			}	
		}
		type++;
	}
	
#ifndef	KPERF
	/*
	** See comment at the start of this routine....
	*/

	++hr_lbolt;
#endif	/* !KPERF */

	if (--unix_tick == 0) {
		unix_tick = ticks_til_clock;
		return(clock(pc,cs,flags,oldipl));	/* Give Unix a tick */
	}

	return(0);	/* no profiling to do now */
}

/* 
 * Hrtcntl (time control) system call.
 */


int
hrtcntl(uap, rvp)
	register struct hrtcntla *uap;
	rval_t	*rvp;
{
	register int	error = 0;
	register ulong	start_time;
	register ulong	stop_time;
	ulong		new_res;
	int		clock;
	interval_t	it_buf;	
	hrtime_t	temptofd;

	switch(uap->cmd) {

		case	HRT_GETRES:	/* Get the resolution of a clock */

			if ((error = hrt_checkclock(uap->clk)))
				break;
			rvp->r_val1 = timer_resolution;
			break;

		case	HRT_TOFD:	/* Get the time of day 		 */

			if (uap->clk != CLK_STD) {
				error = EINVAL;
				break;
			}

			if (copyin((caddr_t)uap->hrtp,
			    (caddr_t)&temptofd, sizeof(hrtime_t))) {
				error = EFAULT;
				break;
			}

			if ((error = hrt_checkres(temptofd.hrt_res))) {
				break;
			}

			hrt_gettofd(&temptofd);

			if (copyout((caddr_t)&temptofd,
			    (caddr_t)uap->hrtp, sizeof(hrtime_t))) {
				error = EFAULT;
			}

			break;

		case	HRT_STARTIT:	/* Start timing an activity      */	
			
			clock = uap->clk;

			if (clock == CLK_STD)
				it_buf.i_word1 = hr_lbolt;
			else if (clock == CLK_PROCVIRT)
				it_buf.i_word1 = u.u_procvirt;
			else if (clock == CLK_USERVIRT)
				it_buf.i_word1 = u.u_uservirt;
			else {
				error = EINVAL;
				break;
			}
			it_buf.i_clock = clock;
			if (copyout((caddr_t)&it_buf,
			    (caddr_t)uap->intp, sizeof(interval_t)))
				error = EFAULT;
			break;
		case	HRT_GETIT:	/* Get value of interval timer	  */

			/*
			 * 	Record stop time in case we page fault
			 *	and get delayed.
			 */

			if (copyin((caddr_t)uap->intp,
			    (caddr_t)&it_buf, sizeof(interval_t))) { 
				error = EFAULT;
				break;
			}
			clock = it_buf.i_clock;
			if (clock == CLK_STD)
				stop_time = hr_lbolt;
			else if (clock == CLK_PROCVIRT)
				stop_time = u.u_procvirt;
			else if (clock == CLK_USERVIRT)
				stop_time = u.u_uservirt;
			else {
				error = EINVAL;
				break;
			}

			if (copyin((caddr_t)uap->hrtp,
			    (caddr_t)&temptofd, sizeof(hrtime_t))) {
				error = EFAULT;
				break;
			}

			if ((error = hrt_checkres(temptofd.hrt_res))) {
				break;
			}

			start_time = it_buf.i_word1;

			new_res = temptofd.hrt_res;
			temptofd.hrt_secs = 0;
			temptofd.hrt_rem  = stop_time - start_time;
			temptofd.hrt_res  = timer_resolution;
			error = hrt_newres(&temptofd, new_res,
						HRT_TRUNC);

			if (error)
				break;

			if (copyout((caddr_t)&temptofd,
			    (caddr_t)uap->hrtp, sizeof(hrtime_t))) {
				error = EFAULT;
			}

			break;
		default:
			error = EINVAL;
			break;
	}
	return error;
}			

/*
 * Hrtalarm (start one or more alarms) system call.
 */

int
hrtalarm(uap, rvp)
	register struct hrtalarma *uap;
	rval_t	*rvp;
{
	register hrtcmd_t	*cp;
	register ulong		base_lbolt;
	register int		oldpri;
	hrtcmd_t		*hrcmdp;
	hrtime_t		*htpnd_p;
	int			error = 0;
	int			cnt;
	int			cmd;
	uint			alarm_cnt;
	int			alarm_type;
	hrtcmd_t		timecmd;
	hrtime_t		delay_ht;

	/*	Get a consistent point in time from which to base
	**	all of the alarms in the list.  Make sure we don't
	**	get an interrupt during the sampling.
	*/

	oldpri		= splhi();
	base_lbolt	= hr_lbolt;
	splx(oldpri);

	/*
	 * Return EINVAL for negative and zero counts.
	 */

	if (uap->cmds <= 0)
		return(EINVAL);

	cp = &timecmd;
	hrcmdp = uap->cmdp;
	alarm_cnt = 0;

	/*	Loop through and process each command.
	*/

	for (cnt = 0; cnt < uap->cmds; cnt++, hrcmdp++) {

		if(copyin((caddr_t)hrcmdp,
		    (caddr_t)cp, sizeof(hrtcmd_t))) {
			error = EFAULT;
			return error;
		}

		/*	See if events are configured.
		*/

		cmd = cp->hrtc_cmd;
		
		if ( (cmd == HRT_ALARM || cmd == HRT_RALARM ||
			cmd == HRT_TODALARM || cmd == HRT_INT_RPT ||
			     cmd == HRT_TOD_RPT || cmd == HRT_PENDING) &&
					! ev_config() )
			return(ENOPKG);

		/*
 	         * If we try to post a Berkley Timer remove
		 * previous timers.
		 */

		if (cmd == HRT_BSD || cmd == HRT_RBSD ||
			cmd == HRT_BSD_REP)
			(void)hrt_bsd_cancel(cp->hrtc_clk);

		/*	See what kind of command we have.
		*/
				 
		switch(cp->hrtc_cmd){
			case HRT_ALARM  :
			case HRT_RALARM :
			case HRT_BSD:
			case HRT_RBSD:
				if (error = hrt_checkclock(cp->hrtc_clk))
					break;
				
				error = hrt_alarm(cp->hrtc_cmd, cp->hrtc_clk,
						&cp->hrtc_int, NULL,
					  	&cp->hrtc_ecb, base_lbolt);
				break;

			case HRT_TODALARM:
				if (cp->hrtc_clk != CLK_STD) {
					error = EINVAL;
					break;
				}

				error = hrt_todalarm(&cp->hrtc_tod,
						NULL, &cp->hrtc_ecb);

				break;

			case HRT_INT_RPT:
			case HRT_BSD_REP:
				if (error = hrt_checkclock(cp->hrtc_clk))
					break;

				if (cp->hrtc_cmd == HRT_INT_RPT)
					cmd = HRT_ALARM;
				else
					cmd = HRT_BSD;

				error = hrt_alarm(cmd, cp->hrtc_clk, 
						&cp->hrtc_tod, NULL,
					  	&cp->hrtc_ecb, base_lbolt);

				if (error)
					break;

				if (cp->hrtc_cmd == HRT_INT_RPT)
					cmd = HRT_RALARM;
				else
					cmd = HRT_RBSD;

				error = hrt_alarm(cmd, cp->hrtc_clk,
						&cp->hrtc_int, &cp->hrtc_tod,
					  	&cp->hrtc_ecb, base_lbolt);
				break;

			case HRT_TOD_RPT:
				if (cp->hrtc_clk != CLK_STD) {
					error = EINVAL;
					break;
				}

				error = hrt_todalarm(&cp->hrtc_tod,
						&delay_ht, &cp->hrtc_ecb);

				if (error)
					break;

				error = hrt_alarm(HRT_RALARM, CLK_STD,
						&cp->hrtc_int, &delay_ht,
						&cp->hrtc_ecb, base_lbolt);
				break;

			case HRT_PENDING:
			case HRT_BSD_PEND:
				if (error = hrt_checkclock(cp->hrtc_clk))
					break;

				alarm_type = 0;

				error = hrt_match_tid(cp->hrtc_cmd,
						cp->hrtc_clk, &alarm_type,
				           	   &delay_ht, &cp->hrtc_ecb);
				if (error)
					break;

				if (alarm_type == HRT_TODALARM)
					htpnd_p = &hrcmdp->hrtc_tod;
				else
					htpnd_p = &hrcmdp->hrtc_int;

				if (copyout((caddr_t)&delay_ht,
				    (caddr_t)htpnd_p,  sizeof(hrtime_t))) {
					error = EFAULT;
				}
				
				break;

			case HRT_BSD_CANCEL:
				if (error = hrt_checkclock(cp->hrtc_clk))
					break;

				error = hrt_bsd_cancel(cp->hrtc_clk);

				break;

			default :
				error = EINVAL;
				break;
		}
		if (error) {
			cp->hrtc_flags |= HRTF_ERROR;
			cp->hrtc_error = error;
		} else {
			cp->hrtc_flags |= HRTF_DONE;
			cp->hrtc_error = 0;
			alarm_cnt++;
		}
		if (copyout((caddr_t)&cp->hrtc_flags,
		    (caddr_t)&hrcmdp->hrtc_flags,
		    sizeof(cp->hrtc_flags) + sizeof(cp->hrtc_error))) {
			error = EFAULT;
			return error;
		}
	}
	rvp->r_val1 = alarm_cnt;
	return(0);
}

		
/*
 * Hrtsleep (suspend the execution of a process) system call.
 */

/* ARGSUSED */
int
hrtsleep(uap, rvp)
	struct hrtsleepa *uap;
	rval_t	*rvp;
{
	register hrtcmd_t  *cmdp;
	register int	error = 0;
	hrtcmd_t	timecmd;

	if (! ev_config())
		return(ENOPKG);

	cmdp = &timecmd;

	if (copyin((caddr_t)uap->cmdp, (caddr_t)cmdp, sizeof(hrtcmd_t)))
		return(EFAULT);

	/*
	** Only the real-time clock can be
	** used with the command.
	*/

	if (cmdp->hrtc_clk != CLK_STD)
		return(EINVAL);

	switch (cmdp->hrtc_cmd) {

		case	HRT_INTSLP:

				error = hrt_intsleep(&cmdp->hrtc_int, 
						&cmdp->hrtc_ecb);

				break;
		case	HRT_TODSLP:

				error = hrt_todsleep(&cmdp->hrtc_tod,
						 &cmdp->hrtc_ecb);

				break;
		default:
				error = EINVAL;
				break;
	}

	if (error && error != EINTR)
		return(error);

	if (cmdp->hrtc_cmd == HRT_TODSLP)
		return(error);

	if (copyout((caddr_t)&cmdp->hrtc_int,
	    (caddr_t)&uap->cmdp->hrtc_int, sizeof(hrtime_t)))
		return(EFAULT);

	return(error);
}

/*
 * Hrtcancel (cancel one or more outastanding alarms) system call.
 */


int
hrtcancel(uap, rvp)
	register struct hrtcancela *uap;
	rval_t	*rvp;
{
	register long	*tidp;
	register int	tidc;
	register int	cancel_cnt;
	int		error = 0;
	
	if (! ev_config())
		return(ENOPKG);

	tidp = uap->eidp;
	tidc = uap->eids;

	if (tidp == NULL)
		cancel_cnt = hrt_cancel_proc();
	else
		cancel_cnt = hrt_cancel_tid(tidp, tidc, &error);

	if (error)
		return(error);
	else {
		rvp->r_val1 = cancel_cnt;
		return(0);
	}
}

/*	Do the HRT_TODALARM function.
*/


int
hrt_todalarm(utdp, delayp, ev_block)
register hrtime_t	*utdp;
register hrtime_t	*delayp;
ecb_t			*ev_block;
{
	register int		oldpri;
	register ulong		base_lbolt;
	register ulong		rem;
	hrtime_t		remhrt;
	long			interval;
	int			error = 0;

	/*	Get a reliable time base for computing the interval.
	**	Don't let a clock tick happen during this step.
	*/

	oldpri = splhi();
	interval = utdp->hrt_secs - hrestime.tv_sec;
	base_lbolt	  = hr_lbolt;
	rem		  = base_lbolt % timer_resolution;
	splx(oldpri);

	/*
 	 * 	Check for errors
	 */
	if (interval < 0) {
		return(hrt_past_time(HRT_TODALARM, utdp->hrt_res, ev_block));
	}

	utdp->hrt_secs = interval;
	remhrt.hrt_secs = 0;
	remhrt.hrt_rem  = rem;
	remhrt.hrt_res  = timer_resolution;

	if ((error = hrt_newres(&remhrt, utdp->hrt_res, HRT_RND)))
		return(error);

	if ((interval = hrt_convert(utdp)) == -1)
		return(ERANGE);

	interval -= remhrt.hrt_rem;

	if (interval <= 0) {
		return(hrt_past_time(HRT_TODALARM, utdp->hrt_res, ev_block));
	}

	utdp->hrt_secs = interval / utdp->hrt_res;
	utdp->hrt_rem  = interval % utdp->hrt_res;

	/*	Initialize the initial delay */

	if (delayp != NULL) {
		delayp->hrt_secs = utdp->hrt_secs;
		delayp->hrt_rem  = utdp->hrt_rem;
		delayp->hrt_res  = utdp->hrt_res;
	}

	/*	Now set the timer.
	*/

	return(hrt_alarm(HRT_TODALARM, CLK_STD, utdp,
			delayp, ev_block, base_lbolt));
}

/*	Do the HRT_ALARM, HRT_RALARM, or HRT_TODALARM function.
*/


int
hrt_alarm(cmd, clock, hrtp, delayp, ev_block, base_lbolt)
register	int	cmd;
register	int	clock;
register	hrtime_t	*hrtp;
register	hrtime_t	*delayp;
ecb_t			*ev_block;
ulong			base_lbolt;
{
	register timer_t	*hrp;
	register int		rounding;
	timer_t			*free_list;
	ulong			numerator;
	ulong			kinterval;
	ulong			fudge;
	long			interval;
	long			user_delay;
	ulong			res;
	int			error = 0;
	dl_t			user_int;
	dl_t			user_res;
	dl_t			sys_int;
	dl_t			real_int;

	/*	Check that the requested resolution is
	**	legal.
	*/

	if (error = hrt_checkres(hrtp->hrt_res)) {
		return error;
	}

	if (hrtp->hrt_rem < 0)
		return ERANGE;

	if (hrtp->hrt_rem >= hrtp->hrt_res) {
		hrtp->hrt_secs += hrtp->hrt_rem / hrtp->hrt_res;
		hrtp->hrt_rem = hrtp->hrt_rem % hrtp->hrt_res;
	}

	interval = hrtp->hrt_rem;
	res = hrtp->hrt_res;

	/*	Allocate a hrtime structure.  Fail with
	**	EAGAIN if none available.
	*/

	if (clock == CLK_STD)
		free_list = &hrt_avail;
	else
		free_list = &it_avail;

	hrp = hrt_alloc(free_list);
	if(hrp == NULL){
		error = EAGAIN;
		return error;
	}

	/*	Initialize the hrtime structure.  If this
	**	fails, an error code has already been set.
	*/

	if(error = hrt_setup(hrp, cmd, clock, hrtp->hrt_res, ev_block)) {
		hrt_free(hrp, free_list);
		return error;
	}


	/*	Determine which way to round depending on whether
	**	we are doing a single alarm or a repeating alarm.
	*/

	if(cmd == HRT_RALARM)
		rounding = HRT_TRUNC;
	else
		rounding = HRT_RND;
	
	/*	Convert the interval from the base the user specified
	**	to our internal base using the rounding just determined.
	**	Check for erroneous specification of interval or
	**	resolution.  This alarm may be part of request for multiple
	**	alarms in which case some time may have elapsed since the start
	**	of the call.  Try to correct for this by subtracting off
	**	of the specified interval the time which has expired
	**	since the start of the call.
	*/

	if ((error = hrt_newres(hrtp, (ulong)timer_resolution, rounding))) {
		hrt_free(hrp, free_list);
		return(error);
	}
	fudge = hr_lbolt - base_lbolt;
	hrp->hrt_time = hrt_convert(hrtp);
	if(hrp->hrt_time == -1) {
		hrt_free(hrp, free_list);
		return(ERANGE);
	}

	if ( (cmd == HRT_ALARM || cmd == HRT_BSD) && (hrp->hrt_time == 0
				 || hrp->hrt_time <= fudge) ) {

		/* For small intervals fire the alarm immediately */

		error = hrp->hrt_fn(hrp, 0, 0);
		hrt_free(hrp, free_list);
		return(error);
	}

	if ( (cmd == HRT_RALARM) && (hrp->hrt_time == 0
				 || hrp->hrt_time <= fudge) ) {

		/* For small intervals return an error */

		hrt_free(hrp, free_list);
		return(ERANGE);
	}


	/*	If we are doing a repeating alarm, we must get the
	**	remainder from the base conversion calculation.
	**	We use this to alter the interval from alarm to
	**	alarm if necessary in order to not accumulate
	**	drift.  Of course, if the remainder is zero, then
	**	all alarms will be exact.  This will happen if the
	**	interval specified is a multiple of our internal
	**	resolution.
	*/

	if(cmd == HRT_RALARM || cmd == HRT_RBSD){

		/*	Calculate the remainder from the above
		**	conversion of the interval to our resolution.
		**	So that it will fit nicely in a 32 bit long,
		**	we really calculate and store the fractional
		**	remainder as if it were multiplied by SCALE
		**	(1 million).  This effectively gives us 6
		**	digits of accuracy to the right of the decimal
		**	point.
		**
		**	We first do the calculation in normal long
		**	arithmetic.  If this works O.K., we use the
		**	result.  However, if we get overflow, then
		**	we use the double precision integer package
		**	created especially for this purpose.
		*/

		hrp->hrt_int = hrp->hrt_time;

		if (delayp) {
			if ((error = hrt_newres(delayp, (ulong)timer_resolution,
						rounding))) {
				return(error);
			}
			user_delay = hrt_convert(delayp);
			if (user_delay == -1) {
				return(ERANGE);
			}
			hrp->hrt_time += user_delay;
		}
		if (interval != 0 && hrtp->hrt_rem != 0) {
			numerator = interval * SCALE;
			kinterval = hrtp->hrt_rem * (SCALE / timer_resolution);

			if(numerator / interval == SCALE  &&
		   		kinterval / hrtp->hrt_rem ==
						 (SCALE / timer_resolution)) {
				hrp->hrt_rem = numerator / res - kinterval;
			} else {
			
				/*	Must use double-precision routines.
				*/

				user_int.dl_hop = 0;
				user_int.dl_lop = interval;
				user_res.dl_hop = 0;
				user_res.dl_lop = res;
				sys_int.dl_hop  = 0;
				sys_int.dl_lop  = hrtp->hrt_rem;

				real_int = ldivide(lmul(user_int, scalel), user_res);
				hrp->hrt_rem =
			     	lsub(real_int, lmul(sys_int, tick)).dl_lop + 1;
			}
		}	
	}

	/*	Now queue the timer request into the
	**	list where it belongs.
	*/

	if (clock == CLK_STD)
		hrt_timeout(hrp, base_lbolt);
	else {
		if (clock == CLK_USERVIRT)
			itimer_timeout(hrp, 0, base_lbolt, clock);
		else
			itimer_timeout(hrp, 1, base_lbolt, clock); 
	}

	return 0;
}

/*	Do the HRT_TODSLEEP function.
*/


int
hrt_todsleep(utdp, ev_block)
register hrtime_t	*utdp;
register ecb_t		*ev_block;
{
	register int		oldpri;
	register long		rem;
	register ulong		base_lbolt;
	long			interval;
	hrtime_t		remhrt;
	int			error;

	/*	Get a reliable time base for computing the interval.
	**	Don't let a clock tick happen during this step.
	*/

	oldpri		   = splhi();
	interval	   = utdp->hrt_secs - hrestime.tv_sec;
	base_lbolt	   = hr_lbolt;
	rem		   = base_lbolt % timer_resolution;
	splx(oldpri);

	/*	Check for errors.  Then convert the whole thing to
	**	the requested resolution.
	*/

	if(interval < 0)
		return(EINVAL);

	utdp->hrt_secs  = interval;
	remhrt.hrt_secs = 0;
	remhrt.hrt_rem  = rem;
	remhrt.hrt_res  = timer_resolution; 

	if((error = hrt_newres(&remhrt, utdp->hrt_res, HRT_RND)))
		return(error);	

	if ((interval = hrt_convert(utdp)) == -1)
		return(ERANGE);

	interval -= remhrt.hrt_rem;

	if(interval <= 0)
		return(EINVAL);

	utdp->hrt_secs = interval / utdp->hrt_res;
	utdp->hrt_rem  = interval % utdp->hrt_res;

	/*	Now set the timer.
	*/

	return(hrt_intsleep(utdp, ev_block));
}

/*	Do the HRT_INTSLEEP function.
*/


hrt_intsleep(hrtp, ev_block)
register hrtime_t	*hrtp;
register ecb_t		*ev_block;
{
	register timer_t	*hrp;
	register ulong		stop_time;
	register ulong		res;
	int			error = 0;

	res = hrtp->hrt_res;

	/*	Check that the resolution is legal.
        **
         */
	if (error = hrt_checkres(res))
		return(error);


	/*	Allocate a hrtime structure.  Fail with
	**	EAGAIN if none available.
	*/

	hrp = hrt_alloc(&hrt_avail);
	if(hrp == NULL)
		return(EAGAIN);

	/*	Initialize the hrtime structure.  
	*/

	if(error = hrt_setup(hrp, HRT_INTSLP, CLK_STD, res, ev_block)){
		hrt_free(hrp, &hrt_avail);
		return error;
	}

	/*	Convert to internal resolution.
	*/

	if ((error = hrt_newres(hrtp, (ulong)timer_resolution, HRT_RND))) {
		hrt_free(hrp, &hrt_avail);
		return(error);
	}

	if ((hrp->hrt_time  = hrt_convert(hrtp)) == -1) {
		hrt_free(hrp, &hrt_avail);
		return(ERANGE);
	}

	if (hrp->hrt_time == 0) {
		hrt_free(hrp, &hrt_avail);
		return(0);
	}

	/*
	**	Insert the timer structure on the timeout
	**	list where it belongs.
	*/

	hrt_timeout(hrp, 0);

	/*	Now go to sleep waiting for the timeout
	**	to occur.  We sleep at an interruptible
	**	priority.  The following code probably 
	**	looks a little funny since we do a PCATCH
	**	and then don't test the result.  The problem
	**	is that the result doesn't tell us what we
	**	want to know.  Consider the following case.
	**
	**	Suppose the user does a "signal(SIGINT, SIG_IGN)"
	**	and then does a "timeevent(T_SLEEP, ...)".  Then
	**	he hits <break>.  The code in os/sig.c/psignal
	**	will do a setrun on the sleeping process.  The
	**	code in os/slp.c/sleep will call an ISSIG which
	**	will call os/sig.c/issig.  This last routine will
	**	discover that the signal is being ignored and turn
	**	it off.  Then issig will return 0 to sleep.  In
	**	turn, sleep will return 0 to us just as though a
	**	wakeup were done.  However, a wakeup wasn't done
	**	and the timer_t entry for the sleep is still on
	**	the queue.  That is why we must always do the
	**	hrt_unsleep.
	*/


	if (sleep((caddr_t)hrp, (PZERO + 1) | PCATCH)) {
		error = EINTR;
	 }
	stop_time = hrt_unsleep(u.u_procp);
	if (stop_time != 0) {
		hrtp->hrt_secs = 0;
		hrtp->hrt_rem  = stop_time;
		hrtp->hrt_res  = timer_resolution;
	}
	(void)hrt_newres(hrtp, res, HRT_TRUNC);

	return(error);
}

/*	See if a process has a high-resolution sleep pending.
**	If so, clear it and return the time interval (from now)
**	when it would have fired.
*/

int
hrt_unsleep(pp)
register proc_t	*pp;
{
	register timer_t	*hrp;
	register ulong		time;
	register int		oldpri;

	/*	Search the list for the first entry which satisfies
	**	the request.
	*/

	oldpri = splhi();
	hrp = hrt_active.hrt_next;
	time = 0;

	while(hrp != &hrt_active){
		time += hrp->hrt_time;
		if(hrp->hrt_proc == pp && (hrp->hrt_cmd == HRT_INTSLP))
			break;
		hrp = hrp->hrt_next;
	}

	/*	If we didn't find one, that's alright.
	*/

	if(hrp == &hrt_active){
		splx(oldpri);
		return(0);
	}

	/*	We found an entry.  Fix up the time of the next entry so
	**	that it fires at the same time that it would have.  Then
	**	take the entry we want off of the list.
	*/

	hrp->hrt_next->hrt_time += hrp->hrt_time;
	hrt_dequeue(hrp);
	hrt_free(hrp, &hrt_avail);

	splx(oldpri);
	return(time);
}

/*	Set a high-resolution timer.
*/

void
hrt_timeout(hrp, base_lbolt)
register timer_t	*hrp;
register ulong		base_lbolt;
{
	register timer_t	*nhrp;
	register int		oldpri;
	register ulong		fudge;

	/*	Find where the new entry belongs in the time ordered
	**	list.  Fix up the time as we go.
	*/

	oldpri = splhi();
	if (base_lbolt) {
		fudge = hr_lbolt - base_lbolt;
		if (fudge >= hrp->hrt_time)
			hrp->hrt_time = 1;
		else
			hrp->hrt_time -= fudge;
	}
	nhrp = hrt_active.hrt_next;
	while(nhrp != &hrt_active  &&  nhrp->hrt_time <= hrp->hrt_time){
		hrp->hrt_time -= nhrp->hrt_time;
		nhrp = nhrp->hrt_next;
	}

	nhrp->hrt_time -= hrp->hrt_time;
	hrt_enqueue(hrp, nhrp);
	splx(oldpri);
}

/*	Set an interval timer. 
*/

void
itimer_timeout(hrp, type, base_lbolt, clock)
register timer_t	*hrp;
register		type;
register ulong		base_lbolt;
int			clock;
{
	register timer_t	*nhrp;
	register proc_t		*pp;
	register ulong		fudge;
	register int		oldpri;

	pp = u.u_procp;

	/*	Find where the new entry belongs in the time ordered
	**	list.  Fix up the time as we go.
	*/

	oldpri = splhi();
	if (base_lbolt && clock == CLK_PROCVIRT) {
		fudge = hr_lbolt - base_lbolt;
		if (fudge >= hrp->hrt_time)
			hrp->hrt_time = 1;
		else
			hrp->hrt_time -= fudge;
	}

	nhrp = pp->p_italarm[type];

	if (nhrp == NULL) {
		pp->p_italarm[type] = hrp;
		hrp->hrt_prev = hrp;
		hrp->hrt_next = NULL;
		splx(oldpri);
		return;
	}	

	while(nhrp->hrt_time <= hrp->hrt_time){
		hrp->hrt_time -= nhrp->hrt_time;
		if(nhrp->hrt_next == NULL) {
			nhrp->hrt_next = hrp;
			hrp->hrt_prev = nhrp;
			hrp->hrt_next = NULL;
			splx(oldpri);
			return;
		}
		nhrp = nhrp->hrt_next;
	}

	nhrp->hrt_time -= hrp->hrt_time;
	itimer_enqueue(hrp, nhrp, type);
	splx(oldpri);
}

/*	Do the hrtcancel function when a set to timer id's has been
**	specified.
*/


int
hrt_cancel_tid(tidp, tidc, errp)
register long	*tidp;
register int	tidc;
register int	*errp;
{
	register long		tid;
	register timer_t	*hrp;
	register timer_t	*nhrp;
	int			cnt;
	int			oldpri;
	int			time;
	int			type;
	proc_t			*pp;

	ASSERT(tidp != NULL);

	if (tidc <= 0)
		return(EINVAL);

	oldpri = splhi();

	/*	Loop through all of the timer id's specified.
	*/

	pp  = u.u_procp;
	cnt = 0;

	while(tidc-- > 0){

		/*	Get the text tid specified.  If the addres is
		**	bad, fail the hrtcancel system call.
		*/

		tid = fuword((int *)tidp++);
		if(tid == -1) {
			*errp = EFAULT;
			return(cnt);
		}

		/*	Ignore tids of zero.
		*/

		if(tid == 0)
			continue;
		
		/*	Loop through all of the active timer requests
		**	looking for those which have a matching tid.
		*/

		hrp = nhrp = hrt_active.hrt_next;
		time = 0;

		for( ; hrp != &hrt_active ; hrp = nhrp){
			nhrp = hrp->hrt_next;
			time += hrp->hrt_time;
			if(hrp->hrt_ecb.ecb_eid == tid  &&  hrp->hrt_proc == pp){

				/*	Before we take the entry off of
				**	the list, add its time to the
				**	following entry so that it's time
				**	will stay correct.  Then fix up
				**	the time field of the entry we
				**	are deleteing to be the time from
				**	now when it would have fired.  
				**	This is so that the event generated
				**	will have the correct time in it.
				*/

				cnt++;
				nhrp->hrt_time += hrp->hrt_time;
				hrp->hrt_time   = time;
				hrt_dequeue(hrp);
				(void)hrt_cancelreq(hrp);
				hrt_free(hrp, &hrt_avail);
			}
		}

		type = 0;

		while(type < 2) {

			hrp = pp->p_italarm[type];
			time = 0;

			for(; hrp !=NULL; hrp = nhrp) {
				nhrp = hrp->hrt_next;
				time += hrp->hrt_time;
				if(hrp->hrt_ecb.ecb_eid == tid
						&&  hrp->hrt_proc == pp){

				/*	Before we take the entry off of
				**	the list, add its time to the
				**	following entry so that it's time
				**	will stay correct.  Then fix up
				**	the time field of the entry we
				**	are deleteing to be the time from
				**	now when it would have fired.  
				**	This is so that the event generated
				**	will have the correct time in it.
				*/

					cnt++;
					if (nhrp != NULL)
						nhrp->hrt_time += hrp->hrt_time;
					hrp->hrt_time   = time;
					itimer_dequeue(hrp, type);
					(void)hrt_cancelreq(hrp);
					hrt_free(hrp, &it_avail);
				}
			}
		type++;
		}
	}

	splx(oldpri);
	return(cnt);
}

/*	Do the hrtcancel function when all timers for the calling
**	process have been requested.
*/


int
hrt_cancel_proc()
{
	register timer_t	*hrp;
	register timer_t	*nhrp;
	register proc_t		*pp;
	register int		cnt;
	register int		oldpri;
	long			time;
	int			type;

	pp	= u.u_procp;
	cnt	= 0;
	type	= 0;
	oldpri	= splhi();

	while (type < 2) {

		time	= 0;
		hrp = nhrp = pp->p_italarm[type];

		for(; hrp != NULL; hrp = nhrp) {
			nhrp = hrp->hrt_next;
			time += hrp->hrt_time;
			cnt++;
			if (nhrp != NULL)
				nhrp->hrt_time += hrp->hrt_time;
			hrp->hrt_time    = time;
			itimer_dequeue(hrp, type);
			(void)hrt_cancelreq(hrp);
			hrt_free(hrp, &it_avail);
		}
		pp->p_italarm[type] = NULL;
		type++;
	}

	/*	Loop through all of the active timer requests looking
	**	for those which were specified by the calling process.
	*/

	time = 0;
	hrp	= hrt_active.hrt_next;
	for( ; hrp != &hrt_active ; hrp = nhrp){
		nhrp = hrp->hrt_next;
		time += hrp->hrt_time;
		if(hrp->hrt_proc == pp){

			/*	Before we take the entry off of
			**	the list, add its time to the
			**	following entry so that it's time
			**	will stay correct.  Then fix up
			**	the time field of the entry we
			**	are deleteing to be the time from
			**	now when it would have fired.  
			**	This is so that the event generated
			**	will have the correct time in it.
			*/

			cnt++;
			nhrp->hrt_time += hrp->hrt_time;
			hrp->hrt_time   = time;
			hrt_dequeue(hrp);
			(void)hrt_cancelreq(hrp);
			hrt_free(hrp, &hrt_avail);
		}
	}
	splx(oldpri);
	return(cnt);
}

/*
 * Cancel BSD timers 
 */ 

int
hrt_bsd_cancel(clock)
int	clock;
{
	register timer_t	*hrp;
	register timer_t	*nhrp;
	register proc_t		*pp = u.u_procp; 
	int			oldpri;
	int			time;
	int			type;

	oldpri = splhi();

	if (clock == CLK_STD) {

		hrp = nhrp = hrt_active.hrt_next;
		time = 0;

		for( ; hrp != &hrt_active ; hrp = nhrp){
			nhrp = hrp->hrt_next;
			time += hrp->hrt_time;
			if ( (hrp->hrt_cmd == HRT_BSD || 
				hrp->hrt_cmd == HRT_RBSD) &&
				  hrp->hrt_proc == pp ) {
				nhrp->hrt_time += hrp->hrt_time;
				hrp->hrt_time   = time;
				hrt_dequeue(hrp);
				(void)hrt_cancelreq(hrp);
				hrt_free(hrp, &hrt_avail);
			}
		}
		splx(oldpri);
		return(0);
	}
	else if (clock == CLK_USERVIRT)
		type = 0;
	else
		type = 1;

	hrp = pp->p_italarm[type];
	time = 0;

	for(; hrp !=NULL; hrp = nhrp) {
		nhrp = hrp->hrt_next;
		time += hrp->hrt_time;
		if ( (hrp->hrt_cmd == HRT_BSD || 
			hrp->hrt_cmd == HRT_RBSD) &&
			  hrp->hrt_proc == pp ) {
			if (nhrp != NULL)
				nhrp->hrt_time += hrp->hrt_time;
			hrp->hrt_time   = time;
			itimer_dequeue(hrp, type);
			(void)hrt_cancelreq(hrp);
			hrt_free(hrp, &it_avail);
		}
	}
	splx(oldpri);
	return(0);
}

/*
** Do the HRT_PENDING command when the event identifier is specified.
**
 */

int
hrt_match_tid(cmd, clock, alarm_typep, htp, ev_block)
int	cmd;
int	clock;
int	*alarm_typep;
register hrtime_t   *htp;
ecb_t	*ev_block;
{
	long		tid;
	int		qd;
	register timer_t	*hrp;
	register timer_t	*nhrp;
	long			time_to_fire;
	int			oldpri;
	int			type;
	ulong			user_res;
	proc_t			*pp;

	pp = u.u_procp;
	if (cmd == HRT_PENDING) {
		tid = ev_block->ecb_eid;
		qd  = ev_block->ecb_eqd;
	}
	if (clock == CLK_USERVIRT)
		type = 0;
	else if (clock == CLK_PROCVIRT)
		type = 1;
	else
		type = 2;
	htp->hrt_secs = 0;
	htp->hrt_rem  = 0;
	time_to_fire = 0;

	if (tid == 0)
		return(EDOM);

	oldpri = splhi();

	if (clock == CLK_STD) {
		hrp = nhrp = hrt_active.hrt_next;

		for ( ; hrp != &hrt_active ; hrp = nhrp) {
			nhrp = hrp->hrt_next;
			time_to_fire += hrp->hrt_time;
			if (hrp->hrt_cmd == HRT_INTSLP)
				continue;
			if ( (cmd == HRT_BSD_PEND &&
			       (hrp->hrt_cmd == HRT_BSD ||
 				  hrp->hrt_cmd == HRT_RBSD) &&
					 hrp->hrt_proc == pp) ||
				(cmd == HRT_PENDING &&
			            tid == hrp->hrt_ecb.ecb_eid && 
				         qd == hrp->hrt_ecb.ecb_eqd &&
						   hrp->hrt_proc == pp) ) {
				*alarm_typep = hrp->hrt_cmd;
				user_res = hrp->hrt_res;
				htp->hrt_res  = timer_resolution;
				if (hrp->hrt_cmd != HRT_TODALARM) {
					htp->hrt_secs = 0;
					htp->hrt_rem  = time_to_fire;
				} /* if then */
				else { 
					htp->hrt_secs = hrestime.tv_sec;
					htp->hrt_rem  = hr_lbolt % 
							  timer_resolution +
					 		  time_to_fire;
				} /* else */
				splx(oldpri);
				(void)hrt_newres(htp, user_res, HRT_RND);
				return(0);
			} /* end if */
		} /* end for */
	} /* end if (clock = CLK_STD) */
	/*
	**	Search the process lists for pending alarms.
	**
	 */
	if (type != 2) {

		hrp = pp->p_italarm[type];

		for(; hrp != NULL; hrp = nhrp) {
			nhrp = hrp->hrt_next;
			time_to_fire += hrp->hrt_time;	
			if ( (cmd == HRT_BSD_PEND &&
			       (hrp->hrt_cmd == HRT_BSD ||
 				  hrp->hrt_cmd == HRT_RBSD) &&
					 hrp->hrt_proc == pp) ||
				(cmd == HRT_PENDING &&
			            tid == hrp->hrt_ecb.ecb_eid && 
				         qd == hrp->hrt_ecb.ecb_eqd &&
						   hrp->hrt_proc == pp) ) {
				*alarm_typep = hrp->hrt_cmd;
				user_res = hrp->hrt_res;
				htp->hrt_res  = timer_resolution;
				htp->hrt_secs = 0;
				htp->hrt_rem  = time_to_fire;

				splx(oldpri);
				(void)hrt_newres(htp, user_res, HRT_RND);
				return(0);
			} /* end if */
		} /* end for */

	} /* end while */

	splx(oldpri);
	return(EDOM);
}


/*	Generate the appropriate event or wakeup when a timer
**	request is cancelled.
*/

int
hrt_cancelreq(hrp)
register timer_t	*hrp;
{
	register long	secs;
	register long	rem;

	/*	A cancelled request must be for an alarm.  Since a
	**	process can only cancel its own requests and it can't
	**	cancel a request while it is asleep, the sleep
	**	request cannot be cancelled.
	*/

	ASSERT(hrp->hrt_cmd != HRT_INTSLP);

	/*	For an alarm, generate an event with the
	**	remaining time to the alarm.
	*/

	if (hrp->hrt_ecb.ecb_flags & ECBF_POSTCAN) {
		secs = hrp->hrt_time / timer_resolution;
		rem  = hrp->hrt_time % timer_resolution;
		return(hrp->hrt_fn(hrp, secs, rem));
	}
	return(0);
}

/*
 * If the ECBF_LATEEER flag is set return EINVAL.
 * otherwise, post an event
 */


int
hrt_past_time(cmd, res, ev_block)
register int	cmd;
register ulong	res;
register ecb_t	*ev_block;
{
	register timer_t	*hrp;
	register vnode_t	*vp;
	timer_t			hrtime;
	int			error;

	hrp = &hrtime;

	if (ev_block->ecb_flags & ECBF_LATEERR)
		return	EINVAL;

	error = hrt_setup(hrp, cmd, CLK_STD, res, ev_block);
	if (error)
		return error;

	error = hrp->hrt_fn(hrp, 0, 0);

	vp = hrp->hrt_vp;
	if (vp != NULL) {
		VN_RELE(vp);
	}

	return(error);
}


/*	Post the event for a timer.
*/


int
hrt_postevent(hrp, seconds, rem)
register timer_t	*hrp;
register long		seconds;
register long		rem;
{
	register evkev_t	*kvp;
	register evd_hrt_t	*tdp;
	register int		error;
	hrtime_t		hrt;
	void			*rvp;


	/*	Allocate a kernel event structure and the data
	**	structure for timer data.
	*/

	error = ev_mem_alloc(EV_MT_KEV, 1, EV_NOWAIT, &rvp);
	if (error)
		return error;
	kvp = (evkev_t *)rvp;


	if (seconds + rem != 0) {
		error = ev_mem_alloc(EV_MT_DATA, sizeof(evd_hrt_t),
				EV_NOWAIT, &rvp);
		if (error) {
			ev_mem_free(EV_MT_KEV, kvp, 1);
			return error;
		}
		tdp = (evd_hrt_t *)rvp;

	/*	If this is a HRT_TODALARM, then we must convert the
	**	relative time to absolute. 
	*/

			if(hrp->hrt_cmd == HRT_TODALARM) {
				seconds	+= hrestime.tv_sec;
				rem	+= hr_lbolt % timer_resolution;
			}
			/*	Set up the data structure.
			*/
			hrt.hrt_secs = seconds;
			hrt.hrt_rem  = rem;
			hrt.hrt_res  = timer_resolution;
			error	= hrt_newres(&hrt, hrp->hrt_res, HRT_RND);
			if (error)
				return(error);

			tdp->hrte_time.hrt_secs	= hrt.hrt_secs;
			tdp->hrte_time.hrt_rem	= hrt.hrt_rem;
			tdp->hrte_time.hrt_res  = hrp->hrt_res;
	}

	/*	Set up the event.
	*/

	kvp->kev_ev.ev_eid	= hrp->hrt_ecb.ecb_eid;
	kvp->kev_ev.ev_hostid	= P_MYHOSTID;
	kvp->kev_ev.ev_type	= ET_TIMER;
	kvp->kev_ev.ev_pri	= hrp->hrt_ecb.ecb_evpri;
	kvp->kev_ev.ev_pid	= hrp->hrt_proc->p_pid;
	kvp->kev_ev.ev_uid	= hrp->hrt_proc->p_uid;
	if (seconds + rem != 0) {
		kvp->kev_ev.ev_flags    = EF_CANCEL;
		kvp->kev_ev.ev_datasize	= sizeof(evd_hrt_t);
		kvp->kev_ev.ev_data	= (char *)tdp;
	}
	else {
		kvp->kev_ev.ev_flags	= EF_DONE;
		kvp->kev_ev.ev_datasize	= 0;
		kvp->kev_ev.ev_data	= (char *)0;
	}

	/*	Post a kernel event on the queue.  If it fails, free up
	**	the space we have allocated.  It can fail because the
	**	queue is full.
	*/

	error = ev_kev_post(kvp, hrp->hrt_vp, EV_NOWAIT);
	if (error) {
		ev_kev_free(kvp);
		return error;
	}

	/*	See if someone needs to be awakened for the event
	**	we just posted.
	*/

	ev_checkq(hrp->hrt_vp);

	return 0;
}

/*
 * This routine will be used to implement BSD timers.
 */

/* ARGSUSED */
int
hrt_sndsignal(hrp, arg1, arg2)
register timer_t	*hrp;
int			arg1, arg2;
{
	if (hrp->hrt_clk == CLK_STD)
		psignal(hrp->hrt_proc, SIGALRM);
	else if (hrp->hrt_clk == CLK_USERVIRT)
		psignal(hrp->hrt_proc, SIGVTALRM);
	else if (hrp->hrt_clk == CLK_PROCVIRT)
		psignal(hrp->hrt_proc, SIGPROF);
	return(0);
}


/*	Initialize an hrtime structure for a user request.
*/


int
hrt_setup(hrp, cmd, clk, res, ev_block)
register timer_t	*hrp;
register int		cmd;
register int		clk;
register ulong		res;
register ecb_t		*ev_block;
{
	register vnode_t	*vp;
	int			error;
	vnode_t			*rvp;

	/*	If this is one of the functions which post an event,
	**	then convert the event queue identifier to an inode
	**	pointer.
	*/

	if(cmd != HRT_INTSLP && cmd != HRT_BSD && cmd != HRT_RBSD){
		error = ev_eqdtovp(ev_block->ecb_eqd, FWRITE, &rvp, NULL);
		if(error)
			return(error);
		vp = rvp;
		VN_HOLD(vp);
	} else {
		vp = NULL;
	}

	hrp->hrt_proc	= u.u_procp;
	hrp->hrt_clk	= clk;
	hrp->hrt_res	= res;
	hrp->hrt_cmd	= (ushort)cmd;
	hrp->hrt_vp	= vp;
	if (cmd == HRT_BSD || cmd == HRT_RBSD) {
		hrp->hrt_fn = hrt_sndsignal;
		return 0;
	}
	else
		hrp->hrt_fn = hrt_postevent;
	hrp->hrt_ecb.ecb_eqd = ev_block->ecb_eqd;
	hrp->hrt_ecb.ecb_eid = ev_block->ecb_eid;
	hrp->hrt_ecb.ecb_evpri = ev_block->ecb_evpri;
	hrp->hrt_ecb.ecb_flags = ev_block->ecb_flags;

	return 0;
}

/*
**	Return 0 if "res" is a legal resolution. Otherwise,
**	return an error code, ERANGE.
 */


int
hrt_checkres(res)
ulong	res;
{
	if (res <= 0 || res > NANOSEC)
		return ERANGE;
	else
		return 0;
}

/*
**	Return 0 if "clock" is a valid clock. Otherwise,
**	return an error code, EINVAL.
 */

int
hrt_checkclock(clock)
register	clock;
{
	if (clock != CLK_STD && clock != CLK_USERVIRT &&
			clock != CLK_PROCVIRT)
		return EINVAL;
	else
		return 0;
}

/*
**	Convert the high-resolution time to HZ and return it
**	in htp->hrt_rem. Return EDOM if the value of
**	htp->hrt_sec plus htp->hrt_rem convert to more HZ
**	than will fit in the htp->hrt_rem. Otherwise, return
**	zero.
**	Round the conversion according to "rnd" which is either
**	HRT_RND or HRT_TRUNC.
 */

int
hrt_tohz(htp, rnd)
hrtime_t	*htp;
int		rnd;
{
	long	result;
	int	error;

	if ((error = hrt_newres(htp, HZ, rnd)))
		return(error);

	if ((result = hrt_convert(htp)) == -1)
		return(EDOM);

	htp->hrt_secs = 0;
	htp->hrt_rem  = result;
	return(0);
}

/*	Set the current time of day in a specified resolution into
**	a hrtime_t structure.
*/

void
hrt_gettofd(td)
register hrtime_t	*td;
{
	register int	oldpri;
	register ulong	new_res;

	oldpri = splhi();	/* Make sure we get a	*/
				/* consistent time	*/
				/* sample.		*/
	/* save time here  */
	/* since we might page fault */
	/* or loose time in the      */
	/* conversion routines	     */

	td->hrt_secs = hrestime.tv_sec;
	td->hrt_rem  = hrestime.tv_nsec / TICK;
	splx(oldpri);

	new_res = td->hrt_res;
	td->hrt_res = timer_resolution;

	if(new_res != td->hrt_res){
		hrt_newres(td, new_res, HRT_TRUNC); 
	}
}

/*	Convert "htp" from resolution "htp->hrt_res" resolution
**	to new resolution. Round using "rnd" which is either
**	HRT_RND or HRT_TRUNC. Change "htp->hrt_res" to be
**	"new_res".
**
**	Calculate: (interval * new_res) / htp->hrt_res  rounding off as
**		specified by rnd.
**
**	Note:	All args are assumed to be positive.  If
**	the last divide results in something bigger than
**	a long, then ERANGE is returned instead,
**	othewise, zero is returned.
*/

int
hrt_newres(htp, new_res, rnd)
register hrtime_t	*htp;
register ulong		new_res;
register int		rnd;
{
	register long  interval;
	dl_t		dint;
	dl_t		dto_res;
	dl_t		drem;
	dl_t		dfrom_res;
	dl_t		prod;
	dl_t		quot;
	register long	numerator;
	register long	result;
	ulong		modulus;
	ulong		twomodulus;
	long		temp;
	int error;

	if (error = hrt_checkres(new_res))
		return(error);

	if (htp->hrt_rem >= htp->hrt_res) {
		htp->hrt_secs += htp->hrt_rem / htp->hrt_res;
		htp->hrt_rem = htp->hrt_rem % htp->hrt_res;
	}

	interval = htp->hrt_rem;
	if (interval == 0) {
		htp->hrt_res = new_res;
		return(0);
	}

	/*	Try to do the calculations in single precision first
	**	(for speed).  If they overflow, use double precision.
	**	What we want to compute is:
	**
	**		(interval * new_res) / hrt->hrt_res
	*/

	numerator = interval * new_res;

	if (numerator / new_res  ==  interval) {
			
		/*	The above multiply didn't give overflow since
		**	the division got back the original number.  Go
		**	ahead and compute the result.
		*/
	
		result = numerator / htp->hrt_res;
	
		/*	For HRT_RND, compute the value of:
		**
		**		(interval * new_res) % htp->hrt_res
		**
		**	If it is greater than half of the htp->hrt_res,
		**	then rounding increases the result by 1.
		**
		**	For HRT_RNDUP, we increase the result by 1 if:
		**
		**		result * htp->hrt_res != numerator
		**
		**	because this tells us we truncated when calculating
		**	result above.
		**
		**	We also check for overflow when incrementing result
		**	although this is extremely rare.
		*/
	
		if (rnd == HRT_RND) {
			modulus = numerator - result * htp->hrt_res;
			if ((twomodulus = 2 * modulus) / 2 == modulus) {

				/*
				 * No overflow (if we overflow in calculation
				 * of twomodulus we fall through and use
				 * double precision).
				 */
				if (twomodulus >= htp->hrt_res) {
					temp = result + 1;
					if (temp - 1 == result)
						result++;
					else
						return(ERANGE);
				}
				htp->hrt_res = new_res;
				htp->hrt_rem = result;
				return(0);
			}
		} else if (rnd == HRT_RNDUP) {
			if (result * htp->hrt_res != numerator) {
				temp = result + 1;
				if (temp - 1 == result)
					result++;
				else
					return(ERANGE);
			}
			htp->hrt_res = new_res;
			htp->hrt_rem = result;
			return(0);
		} else {	/* rnd == HRT_TRUNC */
			htp->hrt_res = new_res;
			htp->hrt_rem = result;
			return(0);
		}
	}
	
	/*	We would get overflow doing the calculation is
	**	single precision so do it the slow but careful way.
	**
	**	Compute the interval times the resolution we are
	**	going to.
	*/

	dint.dl_hop	= 0;
	dint.dl_lop	= interval;
	dto_res.dl_hop	= 0;
	dto_res.dl_lop	= new_res;
	prod		= lmul(dint, dto_res);

	/*	For HRT_RND the result will be equal to:
	**
	**		((interval * new_res) + htp->hrt_res / 2) / htp->hrt_res
	**
	**	and for HRT_RNDUP we use:
	**
	**		((interval * new_res) + htp->hrt_res - 1) / htp->hrt_res
	**
	** 	This is a different but equivalent way of rounding.
	*/

	if (rnd == HRT_RND) {
		drem.dl_hop = 0;
		drem.dl_lop = htp->hrt_res / 2;
		prod	    = ladd(prod, drem);
	} else if (rnd == HRT_RNDUP) {
		drem.dl_hop = 0;
		drem.dl_lop = htp->hrt_res - 1;
		prod	    = ladd(prod, drem);
	}

	dfrom_res.dl_hop = 0;
	dfrom_res.dl_lop = htp->hrt_res;
	quot		 = ldivide(prod, dfrom_res);

	/*	If the quotient won't fit in a long, then we have
	**	overflow.  Otherwise, return the result.
	*/

	if (quot.dl_hop != 0) {
		return(ERANGE);
	} else {
		htp->hrt_res = new_res;
		htp->hrt_rem = quot.dl_lop;
		return(0);
	}
}


/*
**	Convert "htp" to htp->hrt_res. Return the result.
**/

long
hrt_convert(htp)
register hrtime_t	*htp;
{
	register long	sum;
	register long	product;

	product = htp->hrt_secs * htp->hrt_res;

	if (product / htp->hrt_res == htp->hrt_secs) {
		sum = product + htp->hrt_rem;
		if (sum - htp->hrt_rem == product) {
			return(sum);
		}
	}
	return(-1);
}



/*	Initialize the hrtimes array.  Called from startup.
*/

void
hrtinit()
{
	register int	cnt;

	for(cnt = 0 ; cnt < hrtimes_size ; cnt++){
		hrtimes[cnt].hrt_next = &hrtimes[cnt + 1];
		hrtimes[cnt].hrt_prev = &hrtimes[cnt - 1];
	}

	hrtimes[0].hrt_prev = &hrt_avail;
	hrtimes[cnt - 1].hrt_next = &hrt_avail;
	hrt_avail.hrt_next = &hrtimes[0];
	hrt_avail.hrt_prev = &hrtimes[cnt - 1];

	hrt_active.hrt_next = &hrt_active;
	hrt_active.hrt_prev = &hrt_active;
}

/*	Initialize the itimes array.  Called from startup.
*/

void
itinit()
{
	register int	cnt;

	for(cnt = 0 ; cnt < itimes_size ; cnt++){
		itimes[cnt].hrt_next = &itimes[cnt + 1];
		itimes[cnt].hrt_prev = &itimes[cnt - 1];
	}

	itimes[0].hrt_prev = &it_avail;
	itimes[cnt - 1].hrt_next = &it_avail;
	it_avail.hrt_next = &itimes[0];
	it_avail.hrt_prev = &itimes[cnt - 1];

}

/*	Allocate a timer_t structure.
*/

timer_t	*
hrt_alloc(free_list)
register timer_t	*free_list;
{
	register timer_t	*hrp;
	register int		oldpri;

	oldpri = splhi();
	hrp = free_list->hrt_next;

	if(hrp == free_list){
		splx(oldpri);
		return(NULL);
	}

	hrp->hrt_next->hrt_prev = free_list;
	free_list->hrt_next = hrp->hrt_next;
	splx(oldpri);
	*hrp = hrt_null;
	return(hrp);
}

/*	Free an timer_t structure.
*/

void
hrt_free(hrp, free_list)
register timer_t	*hrp;
register timer_t	*free_list;
{
	register vnode_t	*vp;
	register int		oldpri;

	vp = hrp->hrt_vp;
	if(vp != NULL){
		VN_RELE(vp);
	}
	oldpri = splhi();
	hrp->hrt_next = free_list->hrt_next;
	hrp->hrt_prev = free_list;
	free_list->hrt_next->hrt_prev = hrp;
	free_list->hrt_next = hrp;
	splx(oldpri);
}

/*	Enqueue a new hrtime structure (hrp) in front of an
**	existing one (ohrp) on the active list.
*/

void
hrt_enqueue(hrp, nhrp)
register timer_t	*hrp;
register timer_t	*nhrp;
{
	register int	oldpri;

	oldpri = splhi();
	hrp->hrt_next		 = nhrp;
	hrp->hrt_prev		 = nhrp->hrt_prev;
	nhrp->hrt_prev->hrt_next = hrp;
	nhrp->hrt_prev		 = hrp;
	splx(oldpri);
}

/*	Enqueue a new hrtime structure (hrp) in front of an
**	existing one (ohrp) on the appropriate list.
*/

void
itimer_enqueue(hrp, nhrp, type)
register timer_t	*hrp;
register timer_t	*nhrp;
register		type;
{
	register int	oldpri;
	register proc_t *pp = u.u_procp;	

	oldpri = splhi();

	if (nhrp == nhrp->hrt_prev) {
		pp->p_italarm[type] = hrp;
		hrp->hrt_next = nhrp;
		hrp->hrt_prev = hrp;
		nhrp->hrt_prev = hrp;
		splx(oldpri);
		return;
	}

	hrp->hrt_next		 = nhrp;
	hrp->hrt_prev		 = nhrp->hrt_prev;
	nhrp->hrt_prev->hrt_next = hrp;
	nhrp->hrt_prev		 = hrp;
	splx(oldpri);
}
/*	Unlink an timer_t structure from the doubly linked list
**	it is on.
*/

void
hrt_dequeue(hrp)
register timer_t	*hrp;
{
	register int	oldpri;

	oldpri = splhi();
	hrp->hrt_prev->hrt_next = hrp->hrt_next;
	hrp->hrt_next->hrt_prev = hrp->hrt_prev;
	splx(oldpri);
}

/*
**	Unlink an timer_t structure from the doubly linked list
**	it is on. The list starts from the proc table.
*/

void
itimer_dequeue(hrp, type)
register timer_t	*hrp;
register int		type;
{
	register int	oldpri;
	register proc_t	*pp;

	pp = u.u_procp;

	oldpri = splhi();

	/* hrp the first in the list */

	if (hrp == hrp->hrt_prev) {
		pp->p_italarm[type] = hrp->hrt_next;
		if (hrp->hrt_next != NULL) {
			hrp->hrt_next->hrt_prev = hrp->hrt_next;
		}
	splx(oldpri);
	return;
	}	

	/* hrp the last entry in the list */

	if (hrp->hrt_next == NULL) {
		hrp->hrt_prev->hrt_next = NULL;
		splx(oldpri);
		return;
	}

	hrp->hrt_prev->hrt_next = hrp->hrt_next;
	hrp->hrt_next->hrt_prev = hrp->hrt_prev;

	splx(oldpri);

}

/*
 * System entry point for hrtcntl, hrtalarm. hrtsleep and hrtcancel
 * system calls.
 */

int
hrtsys(uap, rvp)
	register struct	hrtsysa *uap;
	rval_t *rvp;
{
	register int	error;

	switch (uap->opcode) {
	case	HRTCNTL:
		error = hrtcntl((struct hrtcntla *)uap, rvp);
		break;
	case	HRTALARM:
		error = hrtalarm((struct hrtalarma *)uap, rvp);
		break;
	case	HRTSLEEP:
		error = hrtsleep((struct hrtsleepa *)uap, rvp);
		break;
	case	HRTCANCEL:
		error = hrtcancel((struct hrtcancela *)uap, rvp);
		break;
	default:
		error = EINVAL;
		break;
	}

	return error;
}
