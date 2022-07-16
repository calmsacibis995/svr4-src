/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:clock.c	1.3.2.1"

#include "sys/param.h"
#include "sys/types.h"
#include "sys/tuneable.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/callo.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/evecb.h"
#include "sys/hrtcntl.h"
#include "sys/conf.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/cmn_err.h"
#include "sys/map.h"
#include "sys/swap.h"
#include "sys/inline.h"
#include "sys/disp.h"
#include "sys/class.h"
#include "sys/seg.h"
#include "sys/pit.h"
#include "sys/fs/rf_acct.h"
#include "sys/time.h"
#include "sys/debug.h"

#include "vm/anon.h"
#include "vm/rm.h"

#ifdef  VPIX
#include "sys/v86.h"
#endif

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	alarm clock signals
 *	jab the scheduler
 */

#ifdef AT386
extern int sanity_clk;		/* tunable SANITYCLK */
extern int eisa_bus;		/* set in main for presence of eisa bus */
#endif

#define	PRF_ON	01
unsigned	prfstat;
extern int	fsflush();
extern struct buf *bclnlist;
extern int 	desfree;
extern int	tickdelta;
extern long	timedelta;
extern int	doresettodr;
extern int	idleswtch;	/* flag set while idle in pswtch() */

extern int (*io_poll[])();	/* driver entry points to poll every tick */

time_t	time;		/* time in seconds since 1970 */
			/* is here only for compatibility */
timestruc_t hrestime;	/* time in seconds and nsec since since 1970 */

clock_t lbolt;		/* time in HZ since last boot */

#ifdef  VPIX
extern int v86timer;    /* Current v86 timer value */
extern int vpixenable;	/* Is VP/ix enabled or disabled */
#endif

int	one_sec = 1;
int	fsflushcnt;	/* counter for t_fsflushr */
proc_t	*curproc;

#define BASEPRI (oldipl)        /* true if servicing another interrupt  */

int dotimein;                   /* flag that triggers call to timein()  */
#define timepoke() (dotimein++) /* set dotimein flag                    */

int	calllimit	= -1;	/* index of last valid entry in table */

/*
 * Kludge for SVID-compliance is preferable to allocating the
 * structure in generic code.  rf_init points this at a
 * counter.
 */
time_t	*rfsi_servep;

#ifdef DEBUG
int	catchmenowcnt;		/* counter for debuging interrupt */
int	catchmestart = 60;	/* counter for debuging interrupt */
int 	idlecntdown;
int 	idlemsg;
#endif


int
clock(pc, cs, flags, oldipl)
caddr_t pc;
int cs;
int flags;
int oldipl;
{
	extern	void	clkreld();
	register struct proc *pp;
	register int	retval, i;
	register rlim_t rlim_cur;
	static rqlen, sqlen;
	extern caddr_t waitloc;
	extern int (*io_poll[])();
#ifdef AT386
	int sanity_ctl	 = SANITY_CTL;		/* EISA santity ctl word */
	int sanity_ctr0	 = SANITY_CTR0;		/* EISA santity timer */
	int sanity_mode = PIT_C0|PIT_ENDSIGMODE|PIT_READMODE ;
	unsigned int sanitynum = SANITY_NUM;	/* interval for sanitytimer */
#endif
	int s;
	char byte;

	retval = 0;

	/*
	 * If panic, clock should be stopped.
	 */
	ASSERT(panicstr == NULL);

	/*
	 * clock time auxiliary processing
	 */
	for (i=0; io_poll[i]; i++)
		(*io_poll[i])(oldipl);


	/*
	 * Service timeout() requests if any are due at this time.
	 * This code is simpler than the original array-based callout
	 * table since we are using absolute times in the table.
	 * No need to decrement relative times; merely see if the first
	 * (earliest) entry is due to be called.
	 */

	if ((calllimit >= 0) && (callout[0].c_time <= lbolt))
		timepoke();

	if (prfstat & PRF_ON)
		prfintr((u_int)pc, cs);
	pp = u.u_procp;
	if ((flags & PS_VM) || USERMODE(cs)) {
		sysinfo.cpu[CPU_USER]++;
		pp->p_utime++;
		if (u.u_prof.pr_scale & ~1)
			retval = 1;
		/*
		 * Enforce CPU rlimit.
		 */
		rlim_cur = u.u_rlimit[RLIMIT_CPU].rlim_cur;
		if ((rlim_cur != RLIM_INFINITY) &&
		    ((pp->p_utime/HZ) + (pp->p_stime/HZ) > rlim_cur))
			psignal(pp, SIGXCPU);
	} else {
		if (pc == waitloc) {
			if (syswait.iowait+syswait.swap+syswait.physio) {
				sysinfo.cpu[CPU_WAIT]++;
				if (syswait.iowait)
					sysinfo.wait[W_IO]++;
				if (syswait.swap)
					sysinfo.wait[W_SWAP]++;
				if (syswait.physio)
					sysinfo.wait[W_PIO]++;
			} else {
				sysinfo.cpu[CPU_IDLE]++;
			}
		} else {
			sysinfo.cpu[CPU_KERNEL]++;
			pp->p_stime++;
			if (rfsi_servep && RF_SERVER())
				(*rfsi_servep)++;
			/*
			 * Enforce CPU rlimit.
			 */
			rlim_cur = u.u_rlimit[RLIMIT_CPU].rlim_cur;
			if ((rlim_cur != RLIM_INFINITY) &&
			    ((pp->p_utime/HZ) + (pp->p_stime/HZ) > rlim_cur))
				psignal(pp, SIGXCPU);
		}
	}
	
	/*
	 * Update memory usage for the currently running process.
	 */
	if (pp->p_stat == SONPROC) {
		u.u_mem = rm_asrss(pp->p_as);

		/* Call the class specific function to do the once-per-tick
		 * processing for the current process.  If we are in the process
		 * of being switched out we may not be on a dispatcher queue.
		 * The class specific code is responsible for worrying about this.
		 */
		CL_TICK(pp, pp->p_clproc);
	}
	if (idleswtch == 0 && pp->p_cpu < 80)
		pp->p_cpu++;


#ifdef VPIX
	if (pp->p_v86 && pp->p_v86->vp_pri_state == V86_PRI_LO &&
	    pp->p_cpu < 80)
		pp->p_cpu++;
#endif

	lbolt++;	/* time in ticks */
#ifdef AT386
	/* If EISA machine and sanity tunable on, reprime sanity timer*/
	if (eisa_bus && sanity_clk) {
		s = splhi();
		outb(sanity_ctl, sanity_mode);
		byte = sanitynum;
		outb(sanity_ctr0, byte);
		byte = sanitynum>>8;
		outb(sanity_ctr0, byte);
		splx(s);
	}
#endif

#ifdef  VPIX
	if (vpixenable && (--v86timer <= 0))   /* Count ticks for v86 process */
		v86timerint();          /* Timer interrupts for V86 tasks */
#endif

	/*
	 * "double" long arithmetic for minfo.freemem.
	 */
	if (!BASEPRI) {
		unsigned long ofrmem;

		ofrmem = minfo.freemem[0];
		minfo.freemem[0] += freemem;
		if (minfo.freemem[0] < ofrmem)
			minfo.freemem[1]++;
	}

	/*
	 * Increment the time-of-day
         */
	if (timedelta == 0) {
		BUMPTIME(&hrestime, TICK, one_sec);
	} else {
		/*
		 * Drift clock if necessary.
		 */
		register long delta;
			
		if (timedelta < 0) {
			/*
			 * Want more ticks per second, because
			 * we want the clock to advance more
			 * slowly.
			 */
			delta = (MICROSEC/HZ) - tickdelta/2;
			timedelta += tickdelta/2;
		} else {
			/*
			 * Speed up clock: fewer ticks
			 * per second.
			 */
			delta = (MICROSEC/HZ) + tickdelta;
			timedelta -= tickdelta;
		}	
		/*
		 * Convert from msecs to nsecs
 		 */
		delta *= (NANOSEC/MICROSEC);
		BUMPTIME(&hrestime, delta, one_sec);
		if (-tickdelta < timedelta && timedelta < tickdelta) {
			/*
			 * We're close enough.
			 */
			timedelta = 0;
			if (doresettodr) {
				doresettodr = 0;
				wtodc();
			}
		}
	}
	if (one_sec) {

		time++;

		if (BASEPRI)
			return retval;

#ifdef DEBUG
            	if (idlemsg && --idlecntdown == 0)
                        cmn_err(CE_WARN, "System is idle\n");
#endif

		minfo.freeswap = anoninfo.ani_free;

		rqlen = 0;
		sqlen = 0;
		for (pp = practive; pp != NULL; pp = pp->p_next) {
			if (pp->p_clktim)
				if (--pp->p_clktim == 0)
					psignal(pp, SIGALRM);
			pp->p_cpu >>= 1;
			if (pp->p_stat == SRUN || pp->p_stat == SONPROC)
				if (pp->p_flag & SLOAD)
					rqlen++;
				else
					sqlen++;
		}

		if (rqlen) {
			sysinfo.runque += rqlen;
			sysinfo.runocc++;
		}
		if (sqlen) {
			sysinfo.swpque += sqlen;
			sysinfo.swpocc++;
		}


#ifdef DEBUG
		/*
                 * call this routine at regular intervals
                 * to allow debugging.
                 */
                if (--catchmenowcnt <= 0) {
                        catchmenowcnt = catchmestart;
                        catchmenow();
		}
#endif

		/*
		 * Wake up fsflush to write out DELWRI
		 * buffers, dirty pages and other cached
		 * administrative data, e.g. inodes.
		 */
		if (--fsflushcnt <= 0) {
			fsflushcnt = tune.t_fsflushr;
			wakeprocs((caddr_t)fsflush, PRMPT);
		} 
		/*
		* XXX
		* All VFSs should have a VFS_CLOCK operation called from
		* here.
		*/

		/*
		* rf_clock() routine basically does rfs related accounting
		* so just passing variables pc and cs is enough for the
		* purpose
		*/
		rf_clock(pc, cs); 
		vmmeter();
		if (runin != 0) {
			runin = 0;
			setrun(proc_sched);
		}
                if (((freemem <= tune.t_gpgslo) || sqlen) && runout != 0) {
                        runout = 0;
			setrun(proc_sched);
		}
		if (bclnlist != NULL || freemem < desfree) {
			wakeprocs((caddr_t)proc_pageout, PRMPT);
		}
		one_sec = 0;

	}
	return retval;
}

/*
 * Timeout(), untimeout(), timein(), heap_up(), heap_down():
 *
 * These routines manage the callout table as a heap.  The interfaces
 * and table structure are identical to the standard array-based version;
 * the routines impose the heap structure internally to improve 
 * the overhead of using timein(), timeout(), and untimeout() when the
 * table has more than 2 or 3 entries.  
 */

int	timeid		= 0;	/* unique sequence number for entry id */

int
timeout(fun, arg, tim)
	void (*fun)();
	caddr_t arg;
	long tim;
/*
 * Timeout() is called to arrange that fun(arg) be called in tim/HZ seconds.
 * An entry is added to the callout heap structure.  The time in each structure
 * entry is the absolute time at which the function should be called (compare
 * with the relative timing scheme used in the standard array-based version).
 *
 * The panic is there because there is nothing intelligent to be done if
 * an entry won't fit.
 */
{
	register struct	callo	*p1;	/* pointer to entry we are adding */
	register int	j;		/* index to entry we are adding */
	int	t;			/* absolute time fun should be called */
	int	id;			/* id of the entry added */
	int	s;			/* temp variable for spl() */

	t = lbolt + tim;		/* absolute time in the future */

	s = spl7();

	if ((j = calllimit + 1) == v.v_call)
		cmn_err(CE_PANIC,"Timeout table overflow");

	/*
	 * We add the new entry into the next empty slot in the
	 * array representation of the heap.  Heap_up() will
	 * restore the heap by moving the new entry up until
	 * it lies in a valid position.
	 */

	calllimit = j;
	j = heap_up(t, j);

	/*
	 * j is the index of the new entry in the correct
	 * (legal heap) position.  Fill in the particulars
	 * of the request.
	 */

	p1		= &callout[j];
	p1->c_time	= t;
	p1->c_func	= fun;
	p1->c_arg	= arg;
	p1->c_id	= id = ++timeid;

	splx(s);

	/*
	 * Return the id, suitable for a call to untimeout().
	 */

	return id;
}

int
ttimeout(fun, arg, tim)
	void (*fun)();
	caddr_t  arg;
{
	int	s;
	int	id;

	s = spl7();

	if (calllimit + 1 == v.v_call) {
		splx(s);
		return -1;
	}
	else {
		id = timeout(fun, arg, (long)tim);
		splx(s);
		return id;
	}
}

/*
 * untimeout(id) is called to remove an entry in the callout
 * table that was originally placed there by a call to timeout().
 * id is the unique identifier returned by the timeout() call.
 */
int
untimeout(id)
	int id;
{
	register struct	callo	*p1;	/* pointer to entry with proper id */
	register struct	callo	*pend;	/* pointer to last valid table entry */
	register int	f;		/* index to entry with proper id */
	int	s;			/* temp variable for spl() */
	int	t;			/* temp variable for time for reheap */
	int	j;			/* index for last element in reheap */ 

	s = splhi();
	
	/*
	 * Linear search through table looking for an entry
	 * with an id that matches the id requested for removal.
	 */

	f = -1;
	pend = &callout[calllimit];
	for (p1 = &callout[0]; p1 <= pend; p1++) {
		++f;
		if (p1->c_id == id)
			goto found;
	}
	f = -1;
	goto badid;

	/*
	 * We have the entry at f; delete it, move the last entry
	 * of the table into this location, and reheap.
	 */

found:
	if (f == calllimit--)	/* last entry in table; no reheap necessary */
		goto done;

	if (calllimit >= 0) {
		t = pend->c_time;
		j = (f-1) >> 1;	/* j is the parent of f */

		if (f > 0 && callout[j].c_time > t)
			j = heap_up(t, f);
		else
			j = heap_down(t, f);

		callout[j] = *pend;
	}
badid:
done:
	splx(s);

	/*
	 * Failure returns -1
	 */

	return f;
}

/*
 * timein() is at interrupt priority level zero when the 
 * dotimein flag has been set by timepoke() macro in clock()
 */
int
timein()
{
	register struct	callo	*p0;	/* pointer to first entry in table */
	register struct	callo	*plast;	/* pointer to last entry in table */
	struct	callo	svcall;		/* pointer to current entry */
	int	t;			/* time of current entry */
	int	j;			/* index of current entry */
	int	s;			/* temp variable for spl() */
	int	tmpaddr;

	s = splhi();

	p0 = &callout[0];
	while ((p0->c_time <= lbolt) && (calllimit >= 0)) {

		svcall = *p0;

		/*
		 * timein() deletes the first entry in the table.
		 * Move the last entry up, and reheap.
		 */

		plast = &callout[calllimit--];
		if (calllimit >= 0) {
			t = plast->c_time;
			j = heap_down(t,0);
			callout[j] = *plast;
		}

		(svcall.c_func)(svcall.c_arg);
	}
	splx(s);
	return 0;
}

/*
 * heap_up and heap_down are internal support functions that
 * maintain the heap structure of the callout table.
 * heap_up will take an illegal entry and percolate it up until
 * it lies in a legal position; heap_down will percolate an
 * illegal entry down until it falls in a legal position.
 */

STATIC int
heap_up(t, j)
	int t, j;
{
	register int	k;		/* index of parent of entry j */
	register struct	callo *p1;	/* pointer to parent of entry j */

	while (j-- > 0) {

		k = j >> 1;
		p1 = &callout[k];

		if (p1->c_time > t) {
			callout[++j] = *p1;
			j = k;
		} else
			break;
	}

	return 1+j;
}

STATIC int
heap_down(t, j)
	int t, j;
{
	register int	k;		/* index of child to exchange */
	register struct callo	*pk;	/* pointer to child to exchange */

	for (;;) {

	  	if ((k = 1 + (j << 1)) > calllimit)	/* left child ? */
			break;

		pk = &callout[k];

		if ((k < calllimit)			/* right child? */
		    && ((pk + 1)->c_time < pk->c_time)) {
			pk++;
			k++;
		}

		if (pk->c_time >= t)
			break;

		callout[j] = *pk;
		j = k;
	}
	return j;
}

#define	PDELAY	(PZERO-1)

void
delay(ticks)
	long ticks;
{
	int s;

	if (ticks <= 0)
		return;
	s = splhi();
	(void)timeout((void(*)())wakeup, (caddr_t)u.u_procp+1, ticks);
	sleep((caddr_t)u.u_procp+1, PDELAY);
	splx(s);
}


/*
 * clockintr() is required for configuring the clock interrupt priority.
 * It should never be called.
 */
clockintr()
{
	cmn_err(CE_PANIC,
	      "clockintr() entered; probably bad clock intpri configured\n");
}

/*
 * SunOS function to generate monotonically increasing time values.
 */
void
uniqtime(tv)
	register struct timeval *tv;
{
 	static struct timeval last;

	if (last.tv_sec != (long)hrestime.tv_sec) {
		last.tv_sec = (long)hrestime.tv_sec;
		last.tv_usec = (long)0;
	} else {
		last.tv_usec++;
	}
	*tv = last;
}
