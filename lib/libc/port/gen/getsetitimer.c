/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getsetitimer.c	1.2"

#ifdef __STDC__
	#pragma weak getitimer = _getitimer
	#pragma weak setitimer = _setitimer
#endif
#include "synonyms.h"
#include <stddef.h>
#include <sys/types.h>
#include <memory.h>
#include <sys/evecb.h>
#include <sys/hrtcntl.h>
#include <sys/time.h>
#include <sys/param.h>
#include <errno.h>

#define bzero(c, len)		(void) memset((c), '\0', (len))

#define signal_to_clk(sig)	(((sig) == SIGALRM) ? ITIMER_REAL : \
				  (((sig) == SIGVTALRM) ? ITIMER_VIRTUAL : \
				   ITIMER_PROF)) 

#define clk_to_signal(clk)	(((clk) == ITIMER_REAL) ? SIGALRM : \
				  (((clk) == ITIMER_VIRTUAL) ? SIGVTALRM: \
				   SIGPROF)) 
extern int _hrtnewres();

static void timeval_to_hrtime(), hrtime_to_timeval();
static int stuff_cmd_clk(), itimerfix();

struct bsd_sig_info {
	int interval_flag;		/* interval was specified */
	struct timeval interval_val;	/* value of the interval */
};

static struct bsd_sig_info bsd_sig_info[3];	/* info for each signal type */

/*
 * Find out how much time remains on a timer.
 */
getitimer(which, value)
	int which;
	struct itimerval *value;
{
	hrtcmd_t cmd;
	int retval;

	bzero(&cmd, sizeof cmd);
	if (stuff_cmd_clk(&cmd, which))
		return (-1);
	
	bzero(value, sizeof (struct itimerval));
	cmd.hrtc_cmd = HRT_BSD_PEND;
	retval = hrtalarm(&cmd, 1);

	if (retval != 1) {
		if (cmd.hrtc_error != EDOM) {
			errno = EINVAL;
			return(-1);
		}
	}

	hrtime_to_timeval(&value->it_value, &(cmd.hrtc_int));
	if (bsd_sig_info[which].interval_flag)
		value->it_interval = bsd_sig_info[which].interval_val;
	return (0);
}

setitimer(which, val, oval)
	int which;				/* which timer */
	struct itimerval *val, *oval;		/* new, old timer value(s) */
{
	register int want_interval;		/* t if interval specified */
	register struct bsd_sig_info *bsip;	/* info for signal specified */
	hrtcmd_t cmd[2];			/* command to run */
	hrtcmd_t *cmdp;
	int retval;

#define REG_CMD	 cmd[0]				/* non-repeating timer cmd */
#define INT_CMD	 cmd[1]				/* interval timer cmd */

	/*
	 * Validate arguments.
	 */
	if (which > 2 || which < 0 || val == 0 ||
	    itimerfix(&(val->it_value)) || itimerfix(&(val->it_interval))) {
		errno = EINVAL;
		return (-1);
	}

	if (oval) {
		/*
		 * Need to return old value.
		 * If can't, return - error code will have been
		 * put in errno by getitimer()
		 */
		if (getitimer(which, oval))
			return (-1);
	}

	want_interval = timerisset(&(val->it_interval));
	bsip = &(bsd_sig_info[which]);
	bsip->interval_val = val->it_interval;

	bzero(&REG_CMD, sizeof (struct hrtcmd));
	if (stuff_cmd_clk(&REG_CMD, which))
		return (-1);
	
	if (want_interval && timerisset(&(val->it_value))) {

		bzero(&INT_CMD, sizeof (struct hrtcmd));
		if (stuff_cmd_clk(&INT_CMD, which))
			return (-1);

		INT_CMD.hrtc_cmd = HRT_BSD_REP;

		/*
		 * Fill in repeating alarm period and
		 * intial delay.
		 */
		timeval_to_hrtime(&val->it_interval,
				   &(INT_CMD.hrtc_int)); 
		timeval_to_hrtime(&val->it_value,
				   &(INT_CMD.hrtc_tod)); 
		/*
		 * Round up to the resolution of
		 * the system clock.
                 */
		_hrtnewres(&(INT_CMD.hrtc_int), HZ, HRT_RNDUP);
		_hrtnewres(&(INT_CMD.hrtc_int), MICROSEC, HRT_RNDUP);
		_hrtnewres(&(INT_CMD.hrtc_tod), HZ, HRT_RNDUP);
		_hrtnewres(&(INT_CMD.hrtc_tod), MICROSEC, HRT_RNDUP);
	}

	if ( ! timerisset(&val->it_value) ||
		( bsip->interval_flag && ! want_interval) ) {
	
		/*
		 * A single or repeative timer currently set we will be
		 * disabled if it_value is zero.
		 * A repeative timer currently set we will be
		 * be disabled if it_interval is zero.
		 * In the case of repeative timer we will enable
		 * a single timer if it_value is non-zero.
		 */
		bsip->interval_flag = 0;
		REG_CMD.hrtc_cmd = HRT_BSD_CANCEL;
		retval = hrtalarm(&REG_CMD, 1);
		if (retval != 1) {
			errno = EINVAL;
			return (-1);
		}

		if (! timerisset(&val->it_value))
			/*
			 * If no value specified, don't 
			 * need to start any new timers.
			 */
			return(0);
	}

	/*
	 * Run timer command(s).
	 */

	bsip->interval_flag = want_interval;
	if (want_interval)
		/*
		 * Fire a single alarm at it_value and then
	         * repeative alarms at it_interval
		 * Only one hrtalarm command is needed.
		 * The intervals have been rounded up.
		 */
		cmdp = &INT_CMD;
	else {
		/*
		 * Fire a single alarm.
		 */
		REG_CMD.hrtc_cmd = HRT_BSD;
		timeval_to_hrtime(&val->it_value, &(REG_CMD.hrtc_int));
		_hrtnewres(&(REG_CMD.hrtc_int), HZ, HRT_RNDUP);
		_hrtnewres(&(REG_CMD.hrtc_int), MICROSEC, HRT_RNDUP);
		cmdp = &REG_CMD;
	}

	retval = hrtalarm(cmdp, 1);
	
	if (retval != 1) {
		errno = EINVAL;
		return (-1);
	}
	
	return (0);
}

/*
 * Convert a timeval to a hrtime.
 */
static void
timeval_to_hrtime(tvp, hrtp)
	register struct timeval *tvp;
	register hrtime_t *hrtp;
{
	hrtp->hrt_secs = (ulong) (tvp->tv_sec);
	hrtp->hrt_rem = tvp->tv_usec;
	hrtp->hrt_res = MICROSEC;
}
	

/*
 * Convert a hrtime to a timeval.  Assumes that hrtime has proper
 * (MICROSEC) resolution. 
 */
static void
hrtime_to_timeval(tvp, hrtp)
	register struct timeval *tvp;
	register hrtime_t *hrtp;
{
	tvp->tv_sec = (long) (hrtp->hrt_secs);
	tvp->tv_usec = hrtp->hrt_rem;
}

/*
 * Fill in the hrtc_clk field of hrtcmd_t based on "which" clock we are using.
 * If "which" is invalid,  set errno appropriately and return -1,
 * else return 0.
 */
static int
stuff_cmd_clk(cmd, which)
	register hrtcmd_t *cmd;
	register int which;
{
	switch (which) {
	case ITIMER_REAL:
		cmd->hrtc_clk = CLK_STD;
		break;

	case ITIMER_VIRTUAL:
		cmd->hrtc_clk = CLK_USERVIRT;
		break;

	case ITIMER_PROF:
		cmd->hrtc_clk = CLK_PROCVIRT;
		break;

	default:
		errno = EINVAL;
		return (-1);
	}
	return (0);
}
	
/* from SunOS kern_time.c */
/*
 * Check that a proposed value to load into the .it_value or
 * .it_interval part of an interval timer is acceptable, and
 * fix it to have at least minimal value (i.e. if it is less
 * than the resolution of the clock, round it up.)
 */
static int
itimerfix(tv)
	register struct timeval *tv;
{
	if (tv->tv_sec < 0 || tv->tv_sec > 100000000 ||
	    tv->tv_usec < 0 || tv->tv_usec >= 1000000)
		return (EINVAL);
	return (0);
}

