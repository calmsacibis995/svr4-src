/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:sched.c	1.3"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/fp.h"
#include "sys/tuneable.h"
#include "sys/debug.h"
#include "sys/inline.h"
#include "sys/cmn_err.h"
#include "sys/buf.h"
#include "sys/disp.h"
#include "sys/class.h"

/*
 * The scheduler sleeps on runout when there is no one to swap in.
 * It sleeps on runin when it could not find space to swap someone
 * in or after swapping someone in.
 */

char	runout;
char	runin;

/*
 * Memory scheduler.
 */
sched()
{
	register proc_t		*pp; 
	register class_t	*clp;
	register int		maxpri;
	register int		minpri;
	register int		i;
	int			runnable;
	proc_t			*justloaded;
	boolean_t		unloadok;

loop:
	/*
	 * Find a process to activate. 
	 * Call the class specific swapin() functions to nominate
	 * one process from each class to bring in.  Select nominee
	 * with highest priority.
	 */

	(void) spl0();
	cleanup();
	(void) splhi();
	pp = NULL;

	maxpri = -1;
	runnable = 0;
	for (clp = &class[1]; clp < &class[nclass]; clp++) {
		proc_t *rp = NULL;
		int runflag = 0;

 		CL_SWAPIN(clp, freemem, &rp, &runflag);
		if (rp != NULL && rp->p_pri > maxpri) {
			pp = rp;
			maxpri = rp->p_pri;
		}
		if (runflag != 0)
			runnable++;
	}

	/*
	 * We only accept the nominated process if it is unloaded
	 * and its u-block is not currently being swapped.
	 * If the nominated process fails these criteria or there
	 * is no nominated process we wait.
	 */
	if (maxpri == -1 || (pp->p_flag & (SLOAD|SUSWAP))) {
		if (freemem <= tune.t_gpgslo)
			goto unload;
		runout++;
		sleep((caddr_t)&runout, PSWP);
		goto loop;
	}

	/*
	 * See if there is memory for that process; if so, let it go
	 */
	justloaded = NULL;

	if (freemem > tune.t_gpgslo || runnable == 0) {
		/* 
		 * We know this won't sleep because we've already
		 * checked that the lock (SUSWAP) is not set.
		 */
		ub_lock(pp);
		(void)spl0();
		i = swapinub(pp);
		(void) splhi();
		ub_rele(pp);
		if (i == 0)
			goto unload;
 
		vminfo.v_swpin++;
		vminfo.v_pswpin += pp->p_usize;
		pp->p_flag |= SLOAD;
		if (pp->p_stat == SRUN && (pp->p_flag & SPROCIO) == 0)
			dq_sruninc(pp->p_pri);

		if (freemem > tune.t_gpgslo)
			goto loop;
		justloaded = pp;
	} 

	/*
	 * Look for a process to swap out.
	 * Call the class specific swapout() functions to nominate
	 * one process from each class to swap out.  Select nominee
	 * with lowest priority.
	 */

unload:
 
	minpri = v.v_nglobpris;
	for (clp = &class[1]; clp < &class[nclass]; clp++) {
		proc_t *rp = NULL;
		boolean_t ulok = B_TRUE;

		CL_SWAPOUT(clp, freemem, justloaded, &rp, &ulok);
		if (rp != NULL && rp->p_pri < minpri) {
			pp = rp;
			minpri = rp->p_pri;
			unloadok = ulok;
		}
	}

	/*
	 * If we have a valid nominee try to swap it out.  If CL_SWAPOUT set
	 * unloadok to B_FALSE we will swap pages but won't swap out
	 * the u-block or make process "unrunnable".
	 * If we can't swap process out, or no class nominated a
	 * process, wait a bit and try again.
	 */
	if (minpri != v.v_nglobpris &&
	    (pp->p_flag & (SLOAD|SSYS|SLOCK|SUSWAP|SPROCIO|SSWLOCKS)) == SLOAD) {
		if (unloadok == B_TRUE) {
			if (pp->p_stat == SRUN)
				dq_srundec(pp->p_pri);
			pp->p_flag &= ~SLOAD;
		}
		ub_lock(pp);
		(void) spl0();
		i = swapout(pp, unloadok);
		ub_rele(pp);
		if (i != 0) {

			/*
			 * Process successfully swapped out.
			 */
			goto loop;
		} else if (unloadok == B_TRUE) {
			(void) splhi();
			pp->p_flag |= SLOAD;
			if (pp->p_stat == SRUN && (pp->p_flag & SPROCIO) == 0)
				dq_sruninc(pp->p_pri);
		}
	}

	/*
	 * Delay for 1 second and look again later.
	 */
	runin++;
	sleep((caddr_t)&runin, PSWP);
	goto loop;
}

/*
 * Swap out process p.
 */
int
swapout(p, ubswapok)
	register proc_t	*p;
	boolean_t	ubswapok;
{
	register int rtn;

        as_swapout(p->p_as);
	vminfo.v_swpout++;
	vminfo.v_pswpout += p->p_swrss;

	if (ubswapok == B_FALSE)
		return 1;
 
	if (p->p_flag & SPROCIO)
		return 0;

	/*
	 * if the process we are swapping owns the floating
	 * point unit, save its state
	 */
	if (fp_proc == p) {
		fpsave();
	}
 
	rtn = swapoutub(p);

	if (rtn)
		vminfo.v_pswpout += p->p_usize;

	return rtn;
}
