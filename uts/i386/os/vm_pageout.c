/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)kern-os:vm_pageout.c	1.3.1.2"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/uio.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/tuneable.h"
#include "sys/inline.h"
#include "sys/mman.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/vm.h"
#include "sys/vmparam.h"
#include "vm/trace.h"
#include "sys/cmn_err.h"
#include "sys/sysinfo.h"
#include "sys/debug.h"

#include "sys/immu.h"
#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/page.h"
#include "vm/pvn.h"
#ifdef WEITEK
#include "sys/debug.h"
#include "sys/weitek.h"
#endif

#if defined(__STDC__)
STATIC int checkpage (page_t *, int);
extern void	bp_mapout(struct buf *);
#else
STATIC int checkpage ();;
extern void	bp_mapout();
#endif

extern void	cleanup();

/*
 * The following parameters control operation of the page replacement
 * algorithm.  They are initialized to 0, and then computed at boot time
 * based on the size of the system.  If they are patched non-zero in
 * a loaded vmunix they are left alone and may thus be changed per system
 * using adb on the loaded system.
 */

STATIC int	slowscan = 0;
STATIC int	fastscan = 0;
STATIC int	handspread = 0;
STATIC int	loopfraction = 2;

int		maxpgio = 0;	/* max paging i/o per sec before start swaps */
int		lotsfree = 0;	/* max free before clock freezes */
int		minfree = 0;	/* minimum free pages before swapping begins */
int		desfree = 0;	/* # of pages to try to keep free via daemon */

int		freemem;	/* remaining blocks of free memory */
int		deficit;	/* estimate of needs of new swapped in procs */
int		nscan;		/* number of scans in last second */
int		desscan;	/* desired pages scanned per second */

extern	int minpagefree;

/*
 * The size of the clock loop.
 */
#define	LOOPPAGES	(epages - pages)

/* insure non-zero */
#define	nz(x)	((x) != 0 ? (x) : 1)

/*
 * Set up the paging constants for the clock algorithm.
 * Called after the system is initialized and the amount of memory
 * and number of paging devices is known.
 *
 * Threshold constants are defined in ../machine/vmparam.h.
 */
void
setupclock()
{
	int i;

	/*
	 * Set up thresholds for paging:
	 */
	/*
	 * Lotsfree is threshold where paging daemon turns on.
	 */
	if (lotsfree == 0) {
		lotsfree = LOTSFREE / MMU_PAGESIZE;
		if (lotsfree > LOOPPAGES / LOTSFREEFRACT)
			lotsfree = LOOPPAGES / LOTSFREEFRACT;
	}
	/*
	 * Desfree is amount of memory desired free.
	 * If less than this for extended period, do swapping.
	 */
	if (desfree == 0) {
		desfree = DESFREE / MMU_PAGESIZE;
		if (desfree > LOOPPAGES / DESFREEFRACT)
			desfree = LOOPPAGES / DESFREEFRACT;
	}
	/*
	 * Minfree is minimal amount of free memory which is tolerable.
	 */
	if (minfree == 0) {
		minfree = MINFREE / MMU_PAGESIZE;
		if (minfree > desfree / MINFREEFRACT)
			minfree = desfree / MINFREEFRACT;
	}
	if ( minpagefree > minfree) {
		i = minpagefree - minfree;
		minfree += i;
		desfree += i;
		lotsfree += i;
		tune.t_gpgslo += i;
	}
	if (minpagefree > tune.t_gpgslo)
		tune.t_gpgslo = minpagefree;
	/*
	 * Maxpgio thresholds how much paging is acceptable.
	 * This figures that 2/3 busy on an arm is all that is
	 * tolerable for paging.  We assume one operation per disk rev.
	 */
	if (maxpgio == 0)
		maxpgio = (DISKRPM * 2) / 3;
	/*
	 * Handspread is distance (in bytes) between front and back
	 * pageout daemon hands.  It must be < the amount of pageable
	 * memory.
	 */
	if (handspread == 0)
		handspread = HANDSPREAD;
	if (handspread >= LOOPPAGES * PAGESIZE)
		handspread = (LOOPPAGES - 1) * PAGESIZE;

	/*
	 * Clock to scan using max of ~~10% of processor time for sampling,
	 *  this estimated to allow maximum of 200 samples per second.
	 * This yields a ``fastscan'' of roughly (with CLSIZE == 2):
	 *	<=1m	2m	3m	4m	8m
	 * 	5s	10s	15s	20s	40s
	 * The *scan variables are in units of pages scanned per second.
	 */
	if (fastscan == 0)
		fastscan = 200;
	if (fastscan > LOOPPAGES / loopfraction)
		fastscan = LOOPPAGES / loopfraction;

	/*
	 * Set slow scan time to 1/2 the fast scan time.
	 */
	if (slowscan == 0)
		slowscan = fastscan / 2;

	/*
	 * Make sure that back hand follows front hand by at least
	 * 1/RATETOSCHEDPAGING seconds.  Without this test, it is possible
	 * for the back hand to look at a page during the same wakeup of
	 * the pageout daemon in which the front hand cleared its ref bit.
	 */
	if (handspread < fastscan * PAGESIZE)
		handspread = fastscan * PAGESIZE;
}

/*
 * Pageout scheduling.
 *
 * Schedpaging controls the rate at which the page out daemon runs by
 * setting the global variables nscan and desscan RATETOSCHEDPAGING
 * times a second.  Nscan records the number of pages pageout has examined
 * in its current pass; schedpaging resets this value to zero each time
 * it runs.  Desscan records the number of pages pageout should examine
 * in its next pass; schedpaging sets this value based on the amount of
 * currently available memory.
 */

#define	RATETOSCHEDPAGING	4		/* HZ that is */

/*
 * Schedule rate for paging.
 * Rate is linear interpolation between
 * slowscan with lotsfree and fastscan when out of memory.
 */

void
schedpaging()
{
	register int vavail;

	nscan = 0;
	vavail = freemem - deficit;
	if (vavail < 0)
		vavail = 0;
	if (vavail > lotsfree)
		vavail = lotsfree;
	desscan = (slowscan * vavail + fastscan * (lotsfree - vavail)) /
			nz(lotsfree) / RATETOSCHEDPAGING;
	if (freemem < lotsfree) {
		trace1(TR_PAGEOUT_CALL, 3);
		wakeprocs((caddr_t)proc_pageout, PRMPT);
	}
	timeout(schedpaging, (caddr_t)0, HZ / RATETOSCHEDPAGING);
}

STATIC int	pushes;
struct buf *bclnlist;
int pageout_asleep = 0;

#define	FRONT	1
#define	BACK	2

int dopageout = 1;	/* must be non-zero to turn page stealing on */

/*
 * The page out daemon, which runs as process 2.
 *
 * As long as there are at least lotsfree pages,
 * this process is not run.  When the number of free
 * pages stays in the range desfree to lotsfree,
 * this daemon runs through the pages in the loop
 * at a rate determined in schedpaging().  Pageout manages
 * two hands on the clock.  The front hand moves through
 * memory, clearing the reference bit,
 * and stealing pages from procs that are over maxrss.
 * The back hand travels a distance behind the front hand,
 * freeing the pages that have not been referenced in the time
 * since the front hand passed.  If modified, they are pushed to
 * swap before being freed.
 *
 * For now, this thing is in very rough form.
 */
pageout()
{
	register struct page *fronthand, *backhand;
	register int count;

	/*
	 * Set the two clock hands to be separated by a reasonable amount,
	 * but no more than 360 degrees apart.
	 *
	 * N.B.: assumes pages are all contiguous.
	 */
	backhand = pages;
	fronthand = pages + handspread / PAGESIZE;
	if (fronthand >= epages)
		fronthand = epages - 1;

loop:
	/*
	 * XXX:	Add trace points to the loop below.
	 */

	/*
	 * Before sleeping, look to see if there are any swap I/O headers
	 * in the ``cleaned'' list that correspond to dirty
	 * pages that have been pushed asynchronously. If so,
	 * empty the list by calling cleanup().
	 *
	 * N.B.: We guarantee never to block while the cleaned list is nonempty.
	 */
	(void) spl6();
	if (bclnlist != NULL) {
		(void) spl0();
		cleanup();
		goto loop;
	}
	pageout_asleep = 1;
	(void) sleep((caddr_t)proc_pageout, PSWP+1);
	pageout_asleep = 0;
	(void) spl0();
	if (!dopageout) goto loop;
	count = 0;
	pushes = 0;

	trace5(TR_PAGEOUT, 0, nscan, desscan, freemem, lotsfree);
	while (nscan < desscan && freemem < lotsfree) {
		register int rvfront, rvback;

		/*
		 * If checkpage manages to add a page to the free list,
		 * we give ourselves another couple of trips around the loop.
		 */
		if ((rvfront = checkpage(fronthand, FRONT)) == 1)
			count = 0;
		if ((rvback = checkpage(backhand, BACK)) == 1)
			count = 0;
		cnt.v_scan++;
		vminfo.v_scan++;
		/*
		 * Don't include ineligible pages in the number scanned.
		 */
		if (rvfront != -1 || rvback != -1)
			nscan++;
		if (++backhand >= epages) {
			trace2(TR_PAGEOUT_WRAP, freemem, BACK);
			backhand = pages;
		}
		if (++fronthand >= epages) {
			trace2(TR_PAGEOUT_WRAP, freemem, FRONT);
			fronthand = pages;
			cnt.v_rev++;
			if (count > 2) {
				/*
				 * Extremely unlikely, but we went around
				 * the loop twice and didn't get anywhere.
				 * Don't cycle, stop till the next clock tick.
				 */
				goto loop;
			}
			count++;
		}
	}
	trace5(TR_PAGEOUT, 1, nscan, desscan, freemem, lotsfree);
	goto loop;
}


/*
 * An iteration of the clock pointer (hand) around the loop.
 * Look at the page at hand.  If it is locked (e.g., for physical i/o),
 * system (u., page table) or free, then leave it alone.  Otherwise,
 * if we are running the front hand, turn off the page's reference bit.
 * If the proc is over maxrss, we take it.  If running the back hand,
 * check whether the page has been reclaimed.  If not, free the page,
 * pushing it to disk first if necessary.
 *
 * Return values:
 *	-1 if the page was locked and therefore not a candidate at all,
 *	 0 if not freed, or
 *	 1 if we freed it.
 */
STATIC int
checkpage (pp, whichhand)
	register struct page *pp;
	int whichhand;
{

	/*
	 * Reject pages that it doesn't make sense to free.
	 */
	if (pp->p_lock || pp->p_free || pp->p_intrans || 
	    pp->p_lckcnt > 0 || pp->p_cowcnt > 0 || pp->p_keepcnt > 0)
		return (-1);

#ifdef WEITEK
	ASSERT((weitek_pt < 0) || (pp != page_numtopp(weitek_pt)));
#endif

	/*
	 * XXX - Where do we simulate reference bits for
	 * stupid machines (like the vax) that don't have them?
	 *
	 * hat_pagesync will turn off ref and mod bits loaded
	 * into the hardware.
	 */
	hat_pagesync(pp);

	/*
	 * If page is referenced; make unreferenced but reclaimable.
	 * If this page is not referenced, then it must be reclaimable
	 * and we can add it to the free list.
	 */
	if (pp->p_ref) {
		trace4(TR_PG_POREF, pp, pp->p_vnode, pp->p_offset, whichhand);
		if (whichhand == BACK) {
			/*
			 * Somebody referenced the page since the front
			 * hand went by, so it's not a candidate for
			 * freeing up.
			 */
			return (0);
		}
		pp->p_ref = 0;
		/*
		 * Checking of rss or madvise flags needed here...
		 *
		 * If not "well-behaved", fall through into the code
		 * for not referenced.
		 */
		return (0);
	}

	/*
	 * If the page is currently dirty, we have to arrange
	 * to have it cleaned before it can be freed.
	 *
	 * An unkept modified page with no vnode shouldn't really
	 * happen; but if it does, the page is fair game to be taken.
	 */
	if (pp->p_mod && pp->p_vnode) {
		/*
		 * Limit pushes to avoid saturating pageout devices.
		 */
		if (pushes > maxpgio / RATETOSCHEDPAGING) {
			trace2(TR_PAGEOUT_MAXPGIO, freemem, nscan);
			return (0);
		}

		/*
		 * Test for process being swapped out or about to exit?
		 * [Can't get back to process(es) using the page.]
		 */

		/*
		 * The call to VOP_PUTPAGE can fail if we are
		 * currently out of free pageio buf headers or we have
		 * some other condition that requires us to allocate
		 * more memory.  Since we don't want to deadlock in
		 * these case, we cannot risk calling kmem_alloc and
		 * sleeping waiting for memory.  Rather we just hope
		 * that this call will start the IO, even though in
		 * some cases it might not.  However, we only count
		 * successful pushes.  If we fail, try calling cleanup
		 * to free up some buffer headers to prevent us from
		 * looping here.
		 */
		if (VOP_PUTPAGE(pp->p_vnode, pp->p_offset,
		    PAGESIZE, B_ASYNC | B_FREE,
		    (struct cred *)0) == 0) {
			trace5(TR_PG_POCLEAN, pp, pp->p_vnode, pp->p_offset,
				whichhand, freemem);
			pushes++;
			/*
			 * Credit now, even though the page isn't
			 * yet freed.
			 */
			return (1);
		} else {
			cleanup();
			return (0);
		}
	}

	/*
	 * Now we lock the page, unload all the
	 * translations, and put the page back
	 * on the free list.
	 */
	trace5(TR_PG_POFREE, pp, pp->p_vnode, pp->p_offset,
		whichhand, freemem);
	page_lock(pp);
	hat_pageunload(pp);
	page_free(pp, 0);
	cnt.v_dfree++;
	vminfo.v_dfree++;

	return (1);		/* freed a page! */
}

/*
 * Add a buffer to the ``cleaned'' list.  Called from biodone().
 */
void
swdone(bp)
	register struct buf *bp;
{
	int s;

	s = spl6();
	bp->av_forw = bclnlist;
	bclnlist = bp;
	(void) splx(s);

	/*
	 * Rather than wake up the pageout daemon, look for
	 * a sleeper on one of the pages associated with the
	 * buffer and wake it up, with the expectation that
	 * it will clean the list.
	 */
	{
		register struct page *pp = bp->b_pages;

		if (pp) {
			do {
				if (pp->p_want) {
					wakeprocs((caddr_t)pp, PRMPT);
					pp->p_want = 0;
					break;
				}
				pp = pp->p_next;
			} while (pp != bp->b_pages);
		}
	}
/* this is for deadlock prevention with interim specfs code.
 * When specfs stops using pbuf headers, something else may be needed.
 */
	if ((bp->b_flags & B_PHYS)
	    && pfreecnt == 0 && (pfreelist.b_flags & B_WANTED))
		wakeprocs((caddr_t)&pfreelist, PRMPT);
}

/*
 * Process the list of ``cleaned'' pages
 * or asynchronous I/O requests that need
 * synchronous cleanup.
 */
void
cleanup()
{
	register struct buf *bp;
	register int s;

	for (;;) {
		s = spl6();
		if ((bp = bclnlist) == NULL) {
			(void) splx(s);
			break;
		}
		bclnlist = bp->av_forw;
		(void) splx(s);

		if ((bp->b_flags & B_ASYNC) == 0)
			panic("cleanup: not async");
		if (bp->b_flags & B_PAGEIO)
			pvn_done(bp);
		else {
			/* bp was B_ASYNC and B_REMAPPED (and not B_PAGEIO) */
			if ((bp->b_flags & B_REMAPPED) == 0)
				panic("cleanup: not remapped");
			bp_mapout(bp);
			brelse(bp);
		}
	}
}
