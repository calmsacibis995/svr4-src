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

#ident  "@(#)kern-vm:vm_page.c	1.3.1.5"

/*
 * VM - physical page management.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/immu.h"		/* XXX - needed for outofmem() */
#include "sys/proc.h"		/* XXX - needed for outofmem() */
#include "sys/vm.h"
#include "vm/trace.h"
#include "sys/swap.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/tuneable.h"
#include "sys/sysmacros.h"
#include "sys/sysinfo.h"
#include "sys/inline.h"
#include "sys/disp.h"		/* XXX - needed for PREEMPT() */

#include "vm/hat.h"
#include "vm/anon.h"
#include "vm/page.h"
#include "vm/seg.h"
#include "vm/pvn.h"
#include "vm/mp.h"
#ifdef AT386	/* 16MB support */
#include "sys/dmaable.h"
#endif	/* 16MB support */

#include "sys/user.h"		/* XXX - needed for getcpages() */

#if defined(__STDC__)
STATIC void page_add(page_t **, page_t *);
STATIC int page_addmem(int);
STATIC int page_delmem(int);
STATIC void page_print(page_t *);
STATIC void page_unfree(page_t *);
STATIC void page_reuse(page_t *);
#else
STATIC void page_add();
STATIC int page_addmem();
STATIC int page_delmem();
STATIC void page_print();
STATIC void page_unfree();
STATIC void page_reuse();
#endif

STATIC int nopageage = 1;

STATIC u_int max_page_get;	/* max page_get request size in pages */

#ifdef AT386	/* 16MB DMA support */
STATIC u_int dma_max_page_get;	/* DMA max page_get request size in pages */
STATIC u_int dma_freemem;	/* DMAable freemem - subset of freemem */
#endif	/* 16MB DMA support */

STATIC u_int freemem_wait;	/* someone (local) waiting for freemem */

u_int ext_freemem_wait;		/* someone external waiting for freemem */

extern int minpagefree;		/* minimum memory in reserve */

u_int pages_pp_locked = 0;	/* physical pages actually locked */
u_int pages_pp_claimed = 0;	/* physical pages reserved */
u_int pages_pp_kernel = 0;	/* physical page locks by kernel */

extern void	cleanup();
void	call_debug();

/* XXX where should this be defined ? */
struct buf *bclnlist;
int availrmem;
int availsmem;

stop_on_funny()
{}

#ifdef DEBUG
#define PAGE_DEBUG 1
#endif

#ifdef PAGE_DEBUG
int do_checks = 0;
int do_check_vp = 1;
int do_check_free = 1;
int do_check_list = 1;
int do_check_pp = 1;

STATIC void page_vp_check();
STATIC void page_free_check();
STATIC void page_list_check();
STATIC void page_pp_check();

#define	CHECK(vp)	if (do_checks && do_check_vp) page_vp_check(vp)
#define	CHECKFREE()	if (do_checks && do_check_free) page_free_check()
#define	CHECKLIST(pp)	if (do_checks && do_check_list) page_list_check(pp)
#define	CHECKPP(pp)	if (do_checks && do_check_pp) page_pp_check(pp)

#else /* PAGE_DEBUG */

#define	CHECK(vp)
#define	CHECKFREE()
#define	CHECKLIST(pp)
#define	CHECKPP(pp)

#endif /* PAGE_DEBUG */


/*
 * Set to non-zero to avoid reclaiming pages which are
 * busy being paged back until the IO and completed.
 */
int nopagereclaim = 0;

/*
 * The logical page free list is maintained as two physical lists.
 * The free list contains those pages that should be reused first.
 * The cache list contains those pages that should remain unused as
 * long as possible so that they might be reclaimed.
 */
STATIC page_t *page_freelist;		/* free list of pages */
STATIC page_t *page_cachelist;		/* cache list of free pages */
STATIC int page_freelist_size;		/* size of free list */
STATIC int page_cachelist_size;		/* size of cache list */

#ifdef AT386	/* 16MB DMA support */
STATIC page_t *dma_page_freelist;		/* DMA free list of pages */
STATIC page_t *dma_page_cachelist;		/* DMA cache list of free pages */
STATIC int dma_page_freelist_size;		/* size of DMA free list */
STATIC int dma_page_cachelist_size;		/* size of DMA cache list */
#endif	/* 16MB DMA support */

mon_t	page_mplock;			/* lock for manipulating page links */

STATIC	mon_t	page_freelock;		/* lock for manipulating free list */

page_t *pages;				/* array of all page structures */
page_t *epages;				/* end of all pages */
u_int	pages_base;			/* page # for pages[0] */
u_int	pages_end;			/* page # for pages[max] */

#ifdef	sun386
page_t *epages2;			/* end of all pages */
u_int	pages_base2;			/* page # for discontiguous phys mem */
u_int	pages_end2;			/* page # for end */

#define OK_PAGE_GET 0x50		/* min ok value for MAX_PAGE_GET */
#define KEEP_FREE 0x20			/* number of pages to keep free */

#endif /* sun386 */


STATIC	mon_t page_locklock;	/* mutex on locking variables */

#if 0
STATIC	u_int pages_pp_factor = 10;/* divisor for unlocked percentage */
#endif

#define	PAGE_LOCK_MAXIMUM \
	((1 << (sizeof (((page_t *)0)->p_lckcnt) * NBBY)) - 1)

STATIC struct pagecnt {
	int	pc_free_cache;		/* free's into cache list */
	int	pc_free_dontneed;	/* free's with dontneed */
	int	pc_free_pageout;	/* free's from pageout */
	int	pc_free_free;		/* free's into free list */
	int	pc_get_cache;		/* get's from cache list */
	int	pc_get_free;		/* get's from free list */
	int	pc_reclaim;		/* reclaim's */
	int	pc_abortfree;		/* abort's of free pages */
	int	pc_find_hit;		/* find's that find page */
	int	pc_find_miss;		/* find's that don't find page */
#define	PC_HASH_CNT	(2*PAGE_HASHAVELEN)
	int	pc_find_hashlen[PC_HASH_CNT+1];
} pagecnt;

/*
 * Initialize the physical page structures.
 * Since we cannot call the dynamic memory allocator yet,
 * we have startup() allocate memory for the page
 * structs and the hash tables for us.
 */
#ifdef	sun386
void
page_init(pp, num, base, num2, base2)
	register page_t *pp;
	u_int num, base;
	u_int num2, base2;
#else
#ifdef i386
void
page_init(pap)
	register struct pageac *pap;
#else
void
page_init(pp, num, base)
	register page_t *pp;
	u_int num, base;
#endif
#endif
{
#ifdef i386
	register page_t		*pp;
	register struct pageac	*preva, *cura;

	/*
	 *	Handles discontiguous memory segments.
	 */
	pap->endpfn = pap->firstpfn + pap->num * (PAGESIZE/MMU_PAGESIZE);
	pap->endpp = pap->firstpp + pap->num;

	/*
	 *	The address of the 1st and Last page structures for
	 *	discontiguous memory segments.
	 */

	if ((pages == (page_t *) NULL) || (pap->firstpp < pages)) {
		pages = pap->firstpp;
		pages_base = pap->firstpfn;
	}

	if ((epages == (page_t *) NULL) || (pap->endpp > epages)) {
		epages = pap->endpp;
		pages_end = pap->endpfn;
	}

	ASSERT((pages <= epages) && (pages_base <= pages_end));

	/*
	 *	Add this memory segment to link list of other memory segments.
	 *	Memory segments get added in list according to number of pages
	 *	in memory segments - large segments get added to top of list.
	 *	This is done with the hope that when scanning for a particular
	 *	page structure/page frame number - the scan will be more efficient.
	 */

	for (preva = (struct pageac *) &pageahead, cura = pageahead;
		cura != (struct pageac *) NULL;
		preva = cura, cura = cura->panext) {
			if (cura->num < pap->num)
				break;
	}

	/*
	 *	And insert into the list.
	 */
	pap->panext = cura;
	preva->panext = pap;
	pagepoolsize += pap->num;
#else
	/*
	 * Store away info in globals.  In the future, we will want to
	 * redo this stuff so that we can have multiple chunks.
	 */
	pages = pp;
	epages = &pp[num];
	pages_base = base;
	pages_end = base + num;
#endif

#ifdef	sun386
	epages2 = &pp[num+num2];
	pages_base2 = base2;
	pages_end2 = base2 + num2;
#endif
	/*
	 * Arbitrarily limit the max page_get request
	 * to 1/2 of the page structs we have.
	 *
	 * If this value is < OK_PAGE_GET, then we set max_page_get to
	 * num - KEEP_FREE.  If this number is less 1/2 of memory,
	 * use 1/2 of mem.  If it's greater than OK_PAGE_GET, use OK_PAGE_GET.
	 *
	 * All of this is just an attempt to run even if very little memory
	 * is available.  There are no guarantees!  The system will probably
	 * die later with insufficient memory even though we get by here.
	 */
#ifdef	sun386
	max_page_get = (num + num2) >> 1;

	if (max_page_get < OK_PAGE_GET && num - KEEP_FREE > max_page_get) {
		max_page_get = num - KEEP_FREE;
		if (max_page_get > OK_PAGE_GET)
			max_page_get = OK_PAGE_GET;
	}
#else
#ifdef i386
	max_page_get = pagepoolsize >> 1;
#else
	max_page_get = num >> 1;
#endif
#endif

	/*
	 * The physical space for the pages array
	 * representing ram pages have already been
	 * allocated.  Here we mark all the pages as
	 * locked.  Later calls to page_free() will
	 * make the pages available.
	 */
#ifdef	sun386
	for (; pp < epages2; pp++)
		pp->p_lock = 1;
#else
#ifdef i386
	for (pp = pap->firstpp; pp < pap->endpp; pp++)
		pp->p_lock = 1;
#else
	for (; pp < epages; pp++)
		pp->p_lock = 1;
#endif
#endif
	/*
	 * Determine the number of pages that can be pplocked.  This
	 * is the number of page frames less the maximum that can be
	 * taken for buffers less another percentage.  The percentage should
	 * be a tunable parameter, and in SVR4 should be one of the "tune"
	 * structures.
	 */
/*
	pages_pp_maximum = 
#ifdef	sun386
	    num + num2;
#else	sun386
	    num;
#endif	sun386
	pages_pp_maximum = num / 10;
*/
	if (pages_pp_maximum <= (tune.t_minarmem + 20) ||
#ifdef i386
	    pages_pp_maximum > pagepoolsize)
		pages_pp_maximum = pagepoolsize / 10;
#else
	    pages_pp_maximum > num)
		pages_pp_maximum = num / 10;
#endif

#if 0
	pages_pp_maximum -= (btop(nbuf * MAXBSIZE) + 1) +
	    (pages_pp_maximum / pages_pp_factor);
#endif

	/*
	 * Verify that the hashing stuff has been initialized by machdep.c
	 */
	if (page_hash == NULL || page_hashsz == 0)
		cmn_err(CE_PANIC, "page_init");

#ifdef lint
	page_print(pp);
#endif /* lint */
}

#ifdef AT386	/* 16MB DMA support */
/*
 *	Should be done before the free page pool list is created so as to separate
 *	free page pool list into dma/nondmaable pools.
 */
dma_page_init()
{
	register struct pageac	*pap;
	register struct pageac	*dma_pap = (struct pageac *) NULL;
	u_int			dma_pfn = (u_int) 0;

	if (DMA_CHECK_ENABLED) {	/* DMA check turned on? */

		dma_check_on = 1;
		dmaable_sleep = 0;

		/*
		 *	Which is the last DMAABLE page frame number?
		 */
		for (pap = pageahead; pap; pap = pap->panext)
			if ((pap->firstpfn <= (LAST_DMAABLE_PFN -1)) &&
				(pap->firstpfn >= dma_pfn))
				dma_pfn = (dma_pap = pap)->firstpfn;


		if (dma_pap->endpfn >= LAST_DMAABLE_PFN)
			dma_limit_pfn = LAST_DMAABLE_PFN - 1;
		else	dma_limit_pfn = dma_pap->endpfn - 1;

		dma_max_page_get = 0;
		for (pap = pageahead; pap; pap = pap->panext)
			if (pap->firstpfn <= dma_limit_pfn) {
				if ((pap->endpfn - 1) > dma_limit_pfn)
					dma_max_page_get +=
						(dma_limit_pfn - pap->firstpfn);
				else	dma_max_page_get += pap->num;
			}
		dma_max_page_get >= 1;

		ASSERT(dma_limit_pfn);

		dma_limit_pp = page_numtopp(dma_limit_pfn);

		if (dma_limit_pp == (page_t *) NULL)
			cmn_err(CE_PANIC,"dma_page_init: dma_limit_pp");

		ASSERT(page_pptonum(dma_limit_pp) == dma_limit_pfn);
		ASSERT(page_numtopp(dma_limit_pfn) ==  dma_limit_pp);
	}
	else {
		dma_check_on = 0;
		dma_limit_pp = (page_t *) NULL;
		dma_limit_pfn = (u_int) 0;
		dmaable_pages = dmaable_free = 0;
	}
}
#endif	/* 16MB DMA support */

/*
 * Use cv_wait() to wait for the given page.  It is assumed that
 * the page_mplock is already locked upon entry and this lock
 * will continue to be held upon return.
 *
 * NOTE:  This routine must be called at splvm() and the caller must
 *	  re-verify the page identity.
 */
void
page_cv_wait(pp)
	register page_t *pp;
{
	register int s;

	/*
	 * Protect against someone clearing the
	 * want bit before we get to sleep.
	 */
	s = splvm();
	if (bclnlist == NULL) {
		/* page may be done except for cleanup if
		 * bclnlist != NULL.
		 * during startup, in particular, this
		 * would cause deadlock.
		 * Later, it causes an unnecessary delay
		 * unless that case is handled.
		 */
		pp->p_want = 1;
		cv_wait(&page_mplock, (char *)pp);
	}
	(void) splx(s);

	/*
	 * We may have been awakened from swdone,
	 * in which case we must clean up the i/o
	 * list before being able to use the page.
	 */
	mon_exit(&page_mplock);
	if (bclnlist != NULL) {
		s = spl0();
		cleanup();
		(void) splx(s);
	}
	mon_enter(&page_mplock);
}

/*
 * Reclaim the given page from the free list to vp list.
 */
void
page_reclaim(pp)
	register page_t *pp;
{
	register int s;
	register struct anon *ap;

	ASSERT(pp >= pages && pp < epages);
	s = splvm();
	mon_enter(&page_freelock);

	if (pp->p_free) {
#ifdef	TRACE
		register int age = pp->p_age;

		ap = NULL;
#endif	/* TRACE */
		page_unfree(pp);
		pagecnt.pc_reclaim++;
		if (pp->p_vnode) {
			cnt.v_pgrec++;
			cnt.v_pgfrec++;
			vminfo.v_pgrec++;

			if (ap = swap_anon(pp->p_vnode, pp->p_offset)) {
				if (ap->un.an_page == NULL && ap->an_refcnt > 0)
					ap->un.an_page = pp;
				cnt.v_xsfrec++;
				vminfo.v_xsfrec++;
			} else {
				cnt.v_xifrec++;
				vminfo.v_xifrec++;
			}
			CHECK(pp->p_vnode);
		}

		trace6(TR_PG_RECLAIM, pp, pp->p_vnode, pp->p_offset,
			ap, age, freemem);
	}

	mon_exit(&page_freelock);
	(void) splx(s);
}

/*
 * Search the hash list for a page with the specified <vp, off> and
 * then reclaim it if found on the free list.
 */
page_t *
page_find(vp, off)
	register struct vnode *vp;
	register u_int off;
{
	register page_t *pp;
	register int len = 0;
	register int s;

	s = splvm();
	mon_enter(&page_mplock);
	for (pp = page_hash[PAGE_HASHFUNC(vp, off)]; pp; pp = pp->p_hash, len++)
		if (pp->p_vnode == vp && pp->p_offset == off && pp->p_gone == 0)
			break;
	if (pp != NULL) {
		pagecnt.pc_find_hit++;
		if (pp->p_free)
			page_reclaim(pp);
	} else
		pagecnt.pc_find_miss++;
	if (len > PC_HASH_CNT)
		len = PC_HASH_CNT;
	pagecnt.pc_find_hashlen[len]++;
	mon_exit(&page_mplock);
	(void) splx(s);
	return (pp);
}

/*
 * Quick page lookup to merely find if a named page exists
 * somewhere w/o having to worry about which list it is on.
 */
page_t *
page_exists(vp, off)
	register struct vnode *vp;
	register u_int off;
{
	register page_t *pp;
	register int len = 0;
	register int s;

	s = splvm();
	mon_enter(&page_mplock);
	for (pp = page_hash[PAGE_HASHFUNC(vp, off)]; pp; pp = pp->p_hash, len++)
		if (pp->p_vnode == vp && pp->p_offset == off && pp->p_gone == 0)
			break;
	if (pp)
		pagecnt.pc_find_hit++;
	else
		pagecnt.pc_find_miss++;
	if (len > PC_HASH_CNT)
		len = PC_HASH_CNT;
	pagecnt.pc_find_hashlen[len]++;
	mon_exit(&page_mplock);
	(void) splx(s);
	return (pp);
}

/*
 * Find a page representing the specified <vp, offset>.
 * If we find the page but it is intransit coming in,
 * we wait for the IO to complete and then reclaim the
 * page if it was found on the free list.
 */
page_t *
page_lookup(vp, off)
	struct vnode *vp;
	u_int off;
{
	register page_t *pp;
	register int s;

again:
	pp = page_find(vp, off);
	if (pp != NULL) {
		ASSERT(pp >= pages && pp < epages);
		/*
		 * Try calling cleanup here to reap the
		 * async buffers queued up for processing.
		 */
		if (pp->p_intrans && pp->p_pagein && bclnlist) {
			cleanup();
		}

		s = splvm();
		mon_enter(&page_mplock);
		while (pp->p_lock && pp->p_intrans && pp->p_vnode == vp &&
		    pp->p_offset == off && !pp->p_gone &&
		    (pp->p_pagein || nopagereclaim)) {
			cnt.v_intrans++;
			page_cv_wait(pp);
		}

		/*
		 * If we still have the right page and it is now
		 * on the free list, get it back via page_reclaim.
		 * Note that when a page is on the free list, it
		 * maybe ripped away at interrupt level.  After
		 * we reclaim the page, it cannot not be taken away
		 * from us at interrupt level anymore.
		 */
		if (pp->p_vnode == vp && pp->p_offset == off && !pp->p_gone) {
			if (pp->p_free)
				page_reclaim(pp);
		} else {
			mon_exit(&page_mplock);
			(void) splx(s);
			goto again;
		}
		mon_exit(&page_mplock);
		(void) splx(s);
	}
	return (pp);
}

/*
 * Enter page ``pp'' in the hash chains and
 * vnode page list as referring to <vp, offset>.
 */
int
page_enter(pp, vp, offset)
	page_t *pp;
	struct vnode *vp;
	u_int offset;
{
	register int v;

	mon_enter(&page_mplock);

	if (page_exists(vp, offset) != NULL) {
		/* already entered? */
		v = -1;
	} else {
		page_hashin(pp, vp, offset, 1);
		CHECK(vp);

		v = 0;
	}

	mon_exit(&page_mplock);

	trace4(TR_PG_ENTER, pp, vp, offset, v);

	return (v);
}

/*
 * page_abort will cause a page to lose its
 * identity and to go (back) to the free list.
 */
void
page_abort(pp)
	register page_t *pp;
{

	ASSERT(pp >= pages && pp < epages);
	ASSERT(pp->p_free == 0);

	/* Set the `gone' bit */
	if (pp->p_vnode != NULL)
		pp->p_gone = 1;

	if (pp->p_keepcnt != 0) {
		/*
		 * We cannot do anything with the page now.
		 * page_free() will be called later when
		 * the keep count goes back to zero.
		 */
		trace4(TR_PG_ABORT, pp, pp->p_vnode, pp->p_offset, 1);
		return;
	}
	if (pp->p_intrans) {
		/*
		 * Since the page is already `gone', we can
		 * just let pvn_done() worry about freeing
		 * this page later when the IO finishes.
		 */
		trace4(TR_PG_ABORT, pp, pp->p_vnode, pp->p_offset, 2);
		return;
	}

	/*
	 * Page is set to go away -- kill any logical locking.
	 */
	if (pp->p_lckcnt > 0) {
		mon_enter(&page_locklock);
		pages_pp_locked--;
		availrmem++;
		pp->p_lckcnt = 0;
		mon_exit(&page_locklock);
	}
	if (pp->p_cowcnt > 0) {
		mon_enter(&page_locklock);
		pages_pp_locked -= pp->p_cowcnt;
		availrmem += pp->p_cowcnt;
		pp->p_cowcnt = 0;
		mon_exit(&page_locklock);
	}

	if (pp->p_mapping) {
		/*
		 * Should be ok to just unload now
		 */
		hat_pageunload(pp);
	}

	pp->p_ref = pp->p_mod = 0;
	trace4(TR_PG_ABORT, pp, pp->p_vnode, pp->p_offset, 0);

	/*
	 * Let page_free() do the rest of the work
	 */
	page_free(pp, 0);
}

/*
 * Put page on the "free" list.  The free list is really two circular lists
 * with page_freelist and page_cachelist pointers into the middle of the lists.
 */
void
page_free(pp, dontneed)
	register page_t *pp;
	int dontneed;
{
	register struct vnode *vp;
	struct anon *ap;
	register int s;
#ifdef AT386	/* 16MB DMA support */
	int dma_pp;
#endif	/* 16MB DMA support */

	ASSERT(pp >= pages && pp < epages);
	ASSERT(pp->p_free == 0);
	ASSERT(pp->p_uown == NULL);

	vp = pp->p_vnode;
	CHECK(vp);

	/*
	 * If we are a swap page, get rid of corresponding
	 * page hint pointer in the anon vector (since it is
	 * easy to do right now) so that we have to find this
	 * page via a page_lookup to force a reclaim.
	 */
	if (ap = swap_anon(pp->p_vnode, pp->p_offset)) {
		if (ap->an_refcnt > 0)
			ap->un.an_page = NULL;
	}

	if (pp->p_gone) {
		if (pp->p_intrans || pp->p_keepcnt != 0) {
			/*
			 * This page will be freed later from pvn_done
			 * (intrans) or the segment unlock routine.
			 * For now, the page will continue to exist,
			 * but with the "gone" bit on.
			 */
			trace6(TR_PG_FREE, pp, vp, pp->p_offset,
				dontneed, freemem, 0);
			return;
		}
		if (vp)
			page_hashout(pp);
		vp = NULL;
	}
	ASSERT(pp->p_intrans == 0);

	if (pp->p_keepcnt != 0 || pp->p_mapping != NULL ||
	    pp->p_lckcnt != 0 || pp->p_cowcnt != 0)
		cmn_err(CE_PANIC, "page_free");

	s = splvm();
	mon_enter(&page_freelock);

	/*
	 * Unlock the page before inserting it on the free list.
	 */
	page_unlock(pp);

	/*
	 * Now we add the page to the head of the free list.
	 * But if this page is associated with a paged vnode
	 * then we adjust the head forward so that the page is
	 * effectively at the end of the list.
	 */
	freemem++;
	pp->p_free = 1;
	pp->p_ref = pp->p_mod = 0;
#ifdef AT386	/* 16MB DMA support */
	dma_pp = (dma_check_on && DMA_PP(pp));
#endif	/* 16MB DMA support */
	if (vp == NULL) {
		/* page has no identity, put it on the front of the free list */
		pp->p_age = 1;
#ifdef AT386	/* 16MB DMA support */
		if (dma_pp) {
			dma_freemem++;
			dma_page_freelist_size++;
			page_add(&dma_page_freelist, pp);
		}
		else {
#endif	/* 16MB DMA support */
		page_freelist_size++;
		page_add(&page_freelist, pp);
#ifdef AT386	/* 16MB DMA support */
		}
#endif	/* 16MB DMA support */
		pagecnt.pc_free_free++;
		trace6(TR_PG_FREE, pp, vp, pp->p_offset, dontneed, freemem, 1);
	} else {
#ifdef AT386	/* 16MB DMA support */
		if (dma_pp) {
			dma_freemem++;
			dma_page_cachelist_size++;
			page_add(&dma_page_cachelist, pp);
		}
		else {
#endif	/* 16MB DMA support */
		page_cachelist_size++;
		page_add(&page_cachelist, pp);
#ifdef AT386	/* 16MB DMA support */
		}
#endif	/* 16MB DMA support */
		if (!dontneed || nopageage) {
			/* move it to the tail of the list */
#ifdef AT386	/* 16MB DMA support */
			if (dma_pp)
				dma_page_cachelist = dma_page_cachelist->p_next;
			else
#endif	/* 16MB DMA support */
			page_cachelist = page_cachelist->p_next;
			pagecnt.pc_free_cache++;
			trace6(TR_PG_FREE, pp, vp, pp->p_offset,
				dontneed, freemem, 2);
		} else {
			pagecnt.pc_free_dontneed++;
			trace6(TR_PG_FREE, pp, vp, pp->p_offset,
				dontneed, freemem, 3);
		}
	}

	mon_exit(&page_freelock);

	CHECK(vp);
	CHECKFREE();

	if (freemem_wait) {
		freemem_wait = 0;
		wakeprocs((caddr_t)&freemem, PRMPT);
	}
	if (ext_freemem_wait) {
		ext_freemem_wait = 0;
		wakeprocs((caddr_t)&ext_freemem_wait, PRMPT);
	}
	(void) splx(s);
}

STATIC int free_pages = 1;

void
free_vp_pages(vp, off, len)
	register struct vnode *vp;
	register u_int off;
	u_int len;
{
	extern int swap_in_range();
	register page_t *pp, *epp;
	register u_int eoff;
	int s;

	eoff = off + len;

	if (free_pages == 0)
		return;
	if (swap_in_range(vp, off, len))
		return;
	CHECK(vp);
	/* free_vp_page may take some time so PREEMPT() */
	PREEMPT();
	s = splvm();
	if ((pp = epp = vp->v_pages) != 0) {
		do {
			/* ASSERT(pp->p_vnode == vp);
			 * Not true because of dummy marker pages.
			 */
			if (pp->p_vnode != vp)
				continue;
			if (pp->p_offset < off || pp->p_offset >= eoff)
				continue;
			ASSERT(!pp->p_intrans || pp->p_keepcnt);
			if (pp->p_mod ) /* XXX somebody needs to handle these */
				continue;
			if (pp->p_keepcnt || pp->p_mapping || pp->p_free ||
				pp->p_lckcnt || pp->p_cowcnt)
				continue;
			ASSERT(pp >= pages && pp < epages);
			ASSERT(!pp->p_gone);	/* p_keepcnt would be up */
			mon_enter(&page_freelock);
			page_unlock(pp);
			freemem++;
			ASSERT(pp->p_free == 0);
			ASSERT(pp->p_intrans == 0);
			ASSERT(pp->p_keepcnt == 0);
			pp->p_free = 1;
			pp->p_ref = 0;
#ifdef AT386	/* 16MB DMA support */
			if (dma_check_on && DMA_PP(pp)) {
				dma_freemem++;
				dma_page_cachelist_size++;
				page_add(&dma_page_cachelist, pp);
				dma_page_cachelist = dma_page_cachelist->p_next;
			}
			else {
#endif	/* 16MB DMA support */
			page_cachelist_size++;
			page_add(&page_cachelist, pp);
			page_cachelist = page_cachelist->p_next;
#ifdef AT386	/* 16MB DMA support */
			}
#endif	/* 16MB DMA support */
			pagecnt.pc_free_cache++;
			trace6(TR_PG_FREE, pp, vp, pp->p_offset,
				dontneed, freemem, 3);
			mon_exit(&page_freelock);
			CHECK(vp);
			CHECKFREE();
			page_unlock(pp);
		} while ((pp = pp->p_vpnext) != epp);
#ifdef AT386
		if (freemem_wait && (page_cachelist != NULL ||
			(dma_check_on && dma_page_cachelist != NULL)))
#else
		if (freemem_wait && page_cachelist != NULL)
#endif
		{
			freemem_wait = 0;
			wakeprocs((caddr_t)&freemem, PRMPT);
		}
#ifdef AT386
		if (ext_freemem_wait && (page_cachelist != NULL ||
			(dma_check_on && dma_page_cachelist != NULL)))
#else
		if (ext_freemem_wait && page_cachelist != NULL)
#endif
		{
			ext_freemem_wait = 0;
			wakeprocs((caddr_t)&ext_freemem_wait, PRMPT);
		}
		splx(s);
	}
}

/*
 * Remove the page from the free list.
 * The caller is responsible for calling this
 * routine at splvm().
 */
STATIC void
page_unfree(pp)
	register page_t *pp;
{
#ifdef AT386	/* 16MB DMA support */
	register int	dmacheck = (dma_check_on && DMA_PP(pp));
#endif	/* 16MB DMA support */

	ASSERT(pp >= pages && pp < epages);
	ASSERT(pp->p_intrans == 0);
	ASSERT(pp->p_free != 0);

	if (pp->p_age) {
#ifdef AT386	/* 16MB DMA support */
		if (dmacheck) {
			page_sub(&dma_page_freelist, pp);
			dma_page_freelist_size--;
			dma_freemem--;
		} else {
#endif	/* 16MB DMA support */
		page_sub(&page_freelist, pp);
		page_freelist_size--;
#ifdef AT386	/* 16MB DMA support */
		}
#endif	/* 16MB DMA support */
	} else {
#ifdef AT386	/* 16MB DMA support */
		if (dmacheck) {
			page_sub(&dma_page_cachelist, pp);
			dma_page_cachelist_size--;
			dma_freemem--;
		} else {
#endif	/* 16MB DMA support */
		page_sub(&page_cachelist, pp);
		page_cachelist_size--;
#ifdef AT386	/* 16MB DMA support */
		}
#endif	/* 16MB DMA support */
	}
	pp->p_free = pp->p_age = 0;
	freemem--;
}

/*
 * Allocate enough pages for bytes of data.
 * Return a doubly linked, circular list of pages.
 * Must spl around entire routine to prevent races from
 * pages being allocated at interrupt level.
 */
page_t *
page_get(bytes, flags)
	u_int bytes;
	u_int flags;
{
	register page_t *pp;
	page_t *plist = NULL;
	register int npages;
	register int physcontig;
#ifdef AT386	/* 16MB DMA support */
	register int dmacheck;
#endif	/* 16MB DMA support */
	register int reqfree;
	int s;

	npages = btopr(bytes);
	/*
	 * Try to see whether request is too large to *ever* be
	 * satisfied, in order to prevent deadlock.  We arbitrarily
	 * decide to limit maximum size requests to max_page_get.
	 */
#ifdef AT386	/* 16MB DMA support */
	if ((dmacheck = (dma_check_on && (flags & P_DMA)))) {
		if (npages >= dma_max_page_get) {
			trace4(TR_PAGE_GET, bytes, flags, dma_freemem, 1);
			return (plist);
		}
	}
	else
#endif	/* 16MB DMA support */
	if (npages >= max_page_get) {
		trace4(TR_PAGE_GET, bytes, flags, freemem, 1);
		return (plist);
	}

	physcontig = ((flags & P_PHYSCONTIG) && (npages > 1));

	/*
	 * Never subject sched to the resource limit.
	 * We use curproc instead of u.u_procp because segu_get() is called
	 * at startup before "u." structure is mapped in.
	 * Note that although curproc is not necessarily valid at interrupt
	 * level (since pswtch() can be interrupted), the only time we should
	 * get here at interrupt level is for kmem_alloc(), in which case,
	 * P_NORESOURCELIM will already be set.
	 */
	if (curproc == proc_sched)
		flags |= P_NORESOURCELIM;

	reqfree = (flags & P_NORESOURCELIM) ? npages : npages + minpagefree;

	/*
	 * If possible, wait until there are enough
	 * free pages to satisfy our entire request.
	 *
	 * XXX:	Before waiting, we try to arrange to get more pages by
	 *	processing the i/o completion list and prodding the
	 *	pageout daemon.  However, there's nothing to guarantee
	 *	that these actions will provide enough pages to satisfy
	 *	the request.  In particular, the pageout daemon stops
	 *	running when freemem > lotsfree, so if npages > lotsfree
	 *	there's nothing going on that will bring freemem up to
	 *	a value large enough to satisfy the request.
	 */
	s = splvm();
#ifdef AT386	/* 16MB DMA support */
	while ((dmacheck ? dma_freemem : freemem) < reqfree)
#else
	while (freemem < reqfree)
#endif	/* 16MB DMA support */
	{

try_again:
		if (!(flags & P_CANWAIT)) {
			trace4(TR_PAGE_GET, bytes, flags, freemem, 2);
			(void) splx(s);
			return (plist);
		}
		/*
		 * Given that we can wait, call cleanup directly to give
		 * it a chance to add pages to the free list.  This strategy
		 * avoids the cost of context switching to the pageout
		 * daemon unless it's really necessary.
		 */
		if (bclnlist != NULL) {
			(void) splx(s);
			cleanup();
			s = splvm();
			continue;
		}
		/*
		 * There's nothing immediate waiting to become available.
		 * Turn the pageout daemon loose to find something.
		 */
		trace1(TR_PAGEOUT_CALL, 0);
		outofmem();
		freemem_wait++;
		trace4(TR_PAGE_GET_SLEEP, bytes, flags, freemem, 0);
		(void) sleep((caddr_t)&freemem, PSWP+2);
		trace4(TR_PAGE_GET_SLEEP, bytes, flags, freemem, 1);
	}

	mon_enter(&page_freelock);

	if (physcontig) {
		register int numpages;
#ifdef i386
		register struct pageac	*pap;

		/*
		 *	On the 80386 we handle discontiguous chunks of physical
		 *	memory sections.
		 *	N.B. Note that "epages" and hence "endpp" does not
		 *	     really exist.
		 */
		for (pap = pageahead; pap; pap = pap->panext) {
			for (pp = pap->firstpp; pp <= pap->endpp - npages; ++pp) {
				if (!pp->p_free)
					continue;
#ifdef AT386	/* 16MB DMA support */
				if (dmacheck && !DMA_PP(pp + npages - 1))
					break;
#endif	/* 16MB DMA support */
				numpages = npages;
				do {
#ifdef AT386	/* 16MB DMA support */
					ASSERT(!dmacheck || DMA_PP(pp));
#endif	/* 16MB DMA support */
					++pp;
					if (--numpages == 0)
						goto found_pages;
				} while (pp->p_free);
			}
		}
#else

		for(pp = pages; pp <= epages; ++pp) {
			if (pp->p_free) {
				numpages = npages;
				for(++pp; --numpages > 0 && pp <= epages; pp++)
					if (!pp->p_free)
						break;
				if (numpages == 0)
					goto found_pages;
			}
		}
#endif

		mon_exit(&page_freelock);
		goto try_again;
	}

found_pages:
	freemem -= npages;
#ifdef AT386	/* 16MB DMA support */
	if (dmacheck)
		dma_freemem -= npages;
#endif	/* 16MB DMA support */

	trace4(TR_PAGE_GET, bytes, flags, freemem, 0);
	/*
	 * If satisfying this request has left us with too little
	 * memory, start the wheels turning to get some back. The
	 * first clause of the test prevents waking up the pageout
	 * daemon in situations where it would decide that there's
	 * nothing to do.  (However, it also keeps bclnlist from
	 * being processed when it otherwise would.)
	 *
	 * XXX: Check against lotsfree rather than desfree?
	 */
	if (nscan < desscan && freemem < desfree) {
		trace1(TR_PAGEOUT_CALL, 1);
		outofmem();
	}

	/*
	 * Pull the pages off the free list and build the return list.
	 */
	while (npages--) {

		if (physcontig) {
			--pp;
#ifdef AT386	/* 16MB DMA support */
			if (dmacheck && ! DMA_PP(pp))
				cmn_err(CE_PANIC,"physcontig: dma: pp %x\n",pp);
#endif	/* 16MB DMA support */
		}
#ifdef AT386	/* 16MB DMA support */
		else if (dmacheck) {
			if ((pp = dma_page_freelist) == NULL) {
				pp = dma_page_cachelist;
				if (pp == NULL)
					cmn_err(CE_PANIC,"page_get: dma_freemem error");
				ASSERT(pp->p_age == 0);
			}
#ifdef DEBUG
			else {
				ASSERT(pp->p_age != 0);
				ASSERT(pp->p_vnode == NULL);
			}
#endif
		}
#endif	/* 16MB DMA support */
		else if ((pp = page_freelist) == NULL) {
			pp = page_cachelist;
			if (pp == NULL)
#ifdef AT386	/* 16MB DMA support */
			{
				if (dma_check_on) {
					dma_freemem--;
					if ((pp = dma_page_freelist) == NULL) {
						pp = dma_page_cachelist;
						if (pp == NULL)
							cmn_err(CE_PANIC, "page_get: freemem error");
						ASSERT(DMA_PP(pp));
						ASSERT(pp->p_age == 0);
					}
#ifdef DEBUG
					else {
						ASSERT(DMA_PP(pp));
						ASSERT(pp->p_age != 0);
						ASSERT(pp->p_vnode == NULL);
					}
#endif
				} else cmn_err(CE_PANIC,"page_get: freemem error\n");
			}
#ifdef DEBUG
			else {
				if (dma_check_on && DMA_PP(pp))
					cmn_err(CE_PANIC,"page_get: page_cachelist has DMA pp %x\n", pp);
				ASSERT(pp->p_age == 0);
			}
#endif

#else	/* else not 16MB case */
				cmn_err(CE_PANIC, "page_get: freemem error");
			ASSERT(pp->p_age == 0);
#endif	/* 16MB DMA support */
		}
#ifdef DEBUG
		else {
#ifdef AT386	/* 16MB DMA support */
			if (dma_check_on && DMA_PP(pp))
				cmn_err(CE_PANIC,"page_get: page_freelist has DMA pp %x\n", pp);
#endif	/* 16MB DMA support */
			ASSERT(pp->p_age != 0);
			ASSERT(pp->p_vnode == NULL);
		}
#endif

#ifdef AT386	/* 16MB DMA support */
		if (dmacheck) {
			if (! DMA_PP(pp)) {
				if (pp->p_age)
					cmn_err(CE_PANIC,"dma_freelist %x pp %x\n",
							dma_page_freelist, pp);
				else
					cmn_err(CE_PANIC,"dma_cachelist %x pp %x\n",
							dma_page_cachelist, pp);
			}
		}
#endif	/* 16MB DMA support */

		page_reuse(pp);
		page_add(&plist, pp);
	}
	mon_exit(&page_freelock);
	CHECKFREE();
	(void) splx(s);
	return (plist);
}


STATIC void
page_reuse(pp)
	register page_t	*pp;
{
	register u_int	i;
#ifdef AT386	/* 16MB DMA support */
	register int	dmacheck = (dma_check_on && DMA_PP(pp));
#endif	/* 16MB DMA support */

	if (pp->p_age) {
		trace5(TR_PG_ALLOC, pp, pp->p_vnode, pp->p_offset,
			0, 0);
		pagecnt.pc_get_free++;
#ifdef AT386	/* 16MB DMA support */
		if (dmacheck) {
			page_sub(&dma_page_freelist, pp);
			dma_page_freelist_size--;
		} else {
#endif	/* 16MB DMA support */
		page_sub(&page_freelist, pp);
		page_freelist_size--;
#ifdef AT386	/* 16MB DMA support */
		}
#endif	/* 16MB DMA support */
	} else {
		trace5(TR_PG_ALLOC, pp, pp->p_vnode, pp->p_offset,
			pp->p_age, 1);
		pagecnt.pc_get_cache++;
#ifdef AT386	/* 16MB DMA support */
		if (dmacheck) {
			page_sub(&dma_page_cachelist, pp);
			dma_page_cachelist_size--;
		} else {
#endif	/* 16MB DMA support */
		page_sub(&page_cachelist, pp);
		page_cachelist_size--;
#ifdef AT386	/* 16MB DMA support */
		}
#endif	/* 16MB DMA support */
		if (pp->p_vnode) {
			/* destroy old vnode association */
			CHECK(pp->p_vnode);
			page_hashout(pp);
		}
	}

	ASSERT(pp->p_mapping == NULL);
	ASSERT(pp->p_free);
	ASSERT(pp->p_intrans == 0);
	ASSERT(pp->p_keepcnt == 0);

	/*
	 * Initialize the p_dblist[] fields.
	 */
	for (i=0; i<(PAGESIZE/NBPSCTR); i++)
		pp->p_dblist[i] = -1;

	pp->p_free = pp->p_mod = pp->p_nc = pp->p_age = 0;
	pp->p_lock = pp->p_intrans = pp->p_pagein = 0;
	pp->p_ref = 1;		/* protect against immediate pageout */
	pp->p_keepcnt = 1;	/* mark the page as `kept' */
}

/* #ifdef i386 */

/*
 * Special version of page_get() which enforces specified alignment constraints
 * on the physical address.
 * The first page is guaranteed to have (addr & align_mask) == align_val.
 * The remaining pages will be physically contigous.
 */

#define ALIGNED_PP(pp)	((page_pptonum(pp) & align_mask) == align_val)

page_t *
page_get_aligned(bytes, align_mask, align_val, flags)
	u_int bytes;
	u_int align_mask, align_val;
	u_int flags;
{
	register page_t 	*pp;
	register int		npages, numpages;
	register struct pageac	*pap;
	page_t			*plist = NULL;
	int			s;
#ifdef AT386	/* 16MB DMA support */
	register int	dmacheck = (dma_check_on && (flags & P_DMA));
#endif	/* 16MB DMA support */
	register int		reqfree;

	ASSERT(PAGOFF(align_mask) == 0 && PAGOFF(align_val) == 0);
	align_mask = PAGNUM(align_mask);
	align_val = PAGNUM(align_val);

	npages = btopr(bytes);

	/*
	 * Try to see whether request is too large to *ever* be
	 * satisfied, in order to prevent deadlock.  We arbitrarily
	 * decide to limit maximum size requests to max_page_get.
	 */
#ifdef AT386	/* 16MB DMA support */
	if ((dmacheck = (dma_check_on && (flags & P_DMA)))) {
		if (npages >= dma_max_page_get) {
			trace4(TR_PAGE_GET, bytes, flags, dma_freemem, 1);
			return (plist);
		}
	}
	else
#endif	/* 16MB DMA support */
	if (npages >= max_page_get) {
		trace4(TR_PAGE_GET, bytes, flags, freemem, 1);
		return (plist);
	}

	/*
	 * Never subject sched to the resource limit.
	 * We use curproc instead of u.u_procp because segu_get() is called
	 * at startup before "u." structure is mapped in.
	 * Note that although curproc is not necessarily valid at interrupt
	 * level (since pswtch() can be interrupted), the only time we should
	 * get here at interrupt level is for kmem_alloc(), in which case,
	 * P_NORESOURCELIM will already be set.
	 */
	if (curproc == proc_sched)
		flags |= P_NORESOURCELIM;

	reqfree = (flags & P_NORESOURCELIM) ? npages : npages + minpagefree;

	/*
	 * If possible, wait until there are enough
	 * free pages to satisfy our entire request.
	 *
	 * XXX:	Before waiting, we try to arrange to get more pages by
	 *	processing the i/o completion list and prodding the
	 *	pageout daemon.  However, there's nothing to guarantee
	 *	that these actions will provide enough pages to satisfy
	 *	the request.  In particular, the pageout daemon stops
	 *	running when freemem > lotsfree, so if npages > lotsfree
	 *	there's nothing going on that will bring freemem up to
	 *	a value large enough to satisfy the request.
	 */
	s = splvm();

#ifdef AT386	/* 16MB DMA support */
	while ((dmacheck ? dma_freemem : freemem) < reqfree)
#else
	while (freemem < reqfree)
#endif	/* 16MB DMA support */
	{

try_again:
		if (!(flags & P_CANWAIT)) {
			trace4(TR_PAGE_GET, bytes, flags, freemem, 2);
			(void) splx(s);
			return(plist);
		}
		/*
		 * Given that we can wait, call cleanup directly to give
		 * it a chance to add pages to the free list.  This strategy
		 * avoids the cost of context switching to the pageout
		 * daemon unless it's really necessary.
		 */
		if (bclnlist != NULL) {
			(void) splx(s);
			cleanup();
			s = splvm();
			continue;
		}
		/*
		 * There's nothing immediate waiting to become available.
		 * Turn the pageout daemon loose to find something.
		 */
		trace1(TR_PAGEOUT_CALL, 0);
		outofmem();
		freemem_wait++;
		trace3(TR_PAGE_GET_SLEEP, flags, freemem, 0);
		(void) sleep((caddr_t)&freemem, PSWP+2);
		trace3(TR_PAGE_GET_SLEEP, flags, freemem, 1);
	}

	/*
	 * Scan pages looking for a free page.  If found, check to see
	 * if the page is DMA-able (if required); if not, skip it.
	 * Also, check for proper alignment (according to the arguments).
	 *
	 * Keep going until we find a contiguous chunk of pages that
	 * meet the constraints.
	 */

	for (pap = pageahead; pap; pap = pap->panext) {
		for (pp = pap->firstpp; pp <= pap->endpp - npages; ++pp) {
			if (!pp->p_free)
				continue;
#ifdef AT386	/* 16MB DMA support */
			if (dmacheck && !DMA_PP(pp + npages - 1))
				break;
#endif	/* 16MB DMA support */
			numpages = npages;
			do {
				if (!ALIGNED_PP(pp))
					break;
#ifdef AT386	/* 16MB DMA support */
				ASSERT(!dmacheck || DMA_PP(pp));
#endif	/* 16MB DMA support */
				++pp;
				if (--numpages == 0)
					goto found_pages;
			} while (pp->p_free);
		}
	}

	mon_exit(&page_freelock);
	goto try_again;

found_pages:
	freemem -= npages;
#ifdef AT386	/* 16MB DMA support */
	if (dmacheck)
		dma_freemem -= npages;
#endif	/* 16MB DMA support */

	trace4(TR_PAGE_GET, bytes, flags, freemem, 0);

	/*
	 * If satisfying this request has left us with too little
	 * memory, start the wheels turning to get some back. The
	 * first clause of the test prevents waking up the pageout
	 * daemon in situations where it would decide that there's
	 * nothing to do.  (However, it also keeps bclnlist from
	 * being processed when it otherwise would.)
	 *
	 * XXX: Check against lotsfree rather than desfree?
	 */
	if (nscan < desscan && freemem < desfree) {
		trace1(TR_PAGEOUT_CALL, 1);
		outofmem();
	}

	/*
	 * Pull the pages off the free list and build the return list.
	 */
	while (npages--) {
		page_reuse(--pp);
		page_add(&plist, pp);
	}

	mon_exit(&page_freelock);
	CHECKFREE();
	(void) splx(s);
	return pp;
}

/*
 *	386 specific: Gets physically contiguous pages.
 *		      Maintained for backwards compatibility.
 */

pte_t *
getcpages(npgs, nosleep)
	int npgs;		/* in clicks */
	int nosleep;		/* wait for pages iff nosleep == 0 */
{
	register page_t	*pp;

	if (npgs <= 0) {
#ifdef DEBUG
		cmn_err(CE_NOTE,"getcpages: request for %d pages\n",npgs);
#endif
		return(NULL);
	}

	if ((availrmem - npgs) < tune.t_minarmem) {
		nomemmsg("getcpages", npgs, 1, 0);
		return(NULL);
	}
	else	availrmem -= npgs;

	/*
	 *	Release 3.2 feature maintained, e.g., first try out without
	 *	sleeping for the pages, etc.
	 */

	if ((pp = page_get(ctob(npgs), P_PHYSCONTIG | P_DMA)) != (page_t *) NULL)
		goto found_contig;

	if (u.u_procp->p_pid == 0) {
		availrmem += npgs;
		nomemmsg("getcpages", npgs, 1, 0);
		return(NULL);
	}
	if (npgs > 1 && nosleep) {
		availrmem += npgs;
		nomemmsg("getcpages", npgs, 1, 0);
		return(NULL);
	}
	cmn_err(CE_NOTE,"!getcpages - waiting for %d contiguous pages",npgs);

	/*
	 *	Now try sleeping for the pages - assured to get it !!!
	 */
	pp = page_get(ctob(npgs), P_CANWAIT | P_PHYSCONTIG | P_DMA);

found_contig:
	if ((pp == (page_t *) NULL) || (pp < pages) || (pp + (npgs - 1) >= epages))
		cmn_err(CE_PANIC,"getcpages: Invalid pp = %x npgs = %d\n",pp, npgs);
#ifdef DEBUG
	{
		register int	i;
		register page_t	*check_pp, *next_pp;

		for (check_pp = next_pp = pp, i = 0;
			i < (npgs - 1);
			i++, next_pp = next_pp->p_next, check_pp++)
			if (check_pp != next_pp)
				cmn_err(CE_PANIC,"getcpages: not contiguous pages: npgs %d pageno %d pp %x checkpp %x nextpp %x\n",
					npgs, i, pp, check_pp, next_pp);
		bzero((caddr_t)phystokv(ctob(page_pptonum(pp))), ctob(npgs));
	}
#endif

	/*
	 *	Usually these pages are used for DMA - and their keep count is
	 *	already bumped up - so they will NOT be stolen/swapped.
	 *	Since we are returning a kernel virtual address, the virtual map
	 *	for the vtop translation is also thus never lost.
	 *	These pages are locked down in memory - until freepage() is invoked.
	 */

	pages_pp_kernel += npgs;	/* bump up pages kept for kernel */

	return((pte_t *) phystokv(ctob(page_pptonum(pp))));
}

/*
 *	Frees a single page - pages allocated by getcpages().
 */
freepage(pfn)
	register u_int pfn;
{
	register page_t	*pp;

	pp = page_numtopp(pfn);			/* Must succeed */
	ASSERT(pp >= pages && pp < epages);	/* and hence this too */

	availrmem++;
	pages_pp_kernel--;

	page_rele(pp);				/* and then page_abort() */
}

/* #endif i386 */

#ifdef DEBUG
/*
 * XXX - need to fix up all this page rot!
 */

/*
 * Release a keep count on the page and handle aborting the page if the
 * page is no longer held by anyone and the page has lost its identity.
 */
void
page_rele(pp)
	page_t *pp;
{

	mon_enter(&page_mplock);

	ASSERT(pp >= pages && pp < epages);
	ASSERT(pp->p_free == 0);

	if (pp->p_keepcnt == 0)			/* sanity check */
		cmn_err(CE_PANIC, "page_rele");
	if (--pp->p_keepcnt == 0) {
		ASSERT(pp->p_intrans == 0);
		while (pp->p_want) {
			pp->p_want = 0;
			cv_broadcast(&page_mplock, (char *)pp);
		}
		ASSERT(pp->p_intrans == 0);
	}

	mon_exit(&page_mplock);

	if (pp->p_keepcnt == 0 && (pp->p_gone || pp->p_vnode == NULL))
		page_abort(pp);			/* yuck */
}

/*
 * Lock a page.
 */
void
page_lock(pp)
	page_t *pp;
{

	ASSERT(pp >= pages && pp < epages);
	ASSERT(pp->p_free == 0);
	mon_enter(&page_mplock);
	while (pp->p_lock)
		page_cv_wait(pp);
	pp->p_lock = 1;
	mon_exit(&page_mplock);
}

/*
 * Unlock a page.
 */
void
page_unlock(pp)
	page_t *pp;
{

	ASSERT(pp >= pages && pp < epages);
	ASSERT(!pp->p_intrans);

	mon_enter(&page_mplock);
	pp->p_lock = 0;
	while (pp->p_want) {
		pp->p_want = 0;
		cv_broadcast(&page_mplock, (char *)pp);
	}
	mon_exit(&page_mplock);
}
#endif

/*
 * Add page ``pp'' to the hash/vp chains for <vp, offset>.
 */
void
page_hashin(pp, vp, offset, lock)
	register page_t *pp;
	register struct vnode *vp;
	u_int offset, lock;
{
	register page_t **hpp;
	register int s;

	ASSERT(pp >= pages && pp < epages);
	ASSERT(pp->p_uown == NULL);

	pp->p_vnode = vp;
	pp->p_offset = offset;
	pp->p_lock = lock;

	/*
	 * Raise priority to splvm() since the hash list
	 * can be manipulated at interrupt level.
	 */
	s = splvm();

	hpp = &page_hash[PAGE_HASHFUNC(vp, offset)];
	pp->p_hash = *hpp;
	*hpp = pp;

	/* XXX this should never happen */
	if (vp == (struct vnode *)NULL) {
		(void) splx(s);
		return;			/* no real vnode */
	}

	/*
	 * Add the page to the end of the v_pages linked list.
	 * This is part of making pvn_vplist_dirty() et al.
	 * process all new pages added while it was sleeping.
	 */
	if (vp->v_pages == NULL) {
		vp->v_pages = pp->p_vpnext = pp->p_vpprev = pp;
	} else {
		pp->p_vpnext = vp->v_pages;
		pp->p_vpprev = vp->v_pages->p_vpprev;
		vp->v_pages->p_vpprev = pp;
		pp->p_vpprev->p_vpnext = pp;
	}
	CHECKPP(pp);
	(void) splx(s);
}

/*
 * Remove page ``pp'' from the hash and vp chains and remove vp association.
 * If v_pages points to this page, move backwards.  This insures that
 * pvn_vplist_dirty() sees all new pages.
 */
void
page_hashout(pp)
	register page_t *pp;
{
	register page_t **hpp, *hp;
	register struct vnode *vp;
	register int s;

	ASSERT(pp >= pages && pp < epages);

	/*
	 * Raise priority to splvm() since the hash list
	 * can be manipulated at interrupt level.
	 */
	s = splvm();
	CHECKPP(pp);
	vp = pp->p_vnode;
	hpp = &page_hash[PAGE_HASHFUNC(vp, pp->p_offset)];
	for (;;) {
		hp = *hpp;
		if (hp == pp)
			break;
		if (hp == NULL)
			cmn_err(CE_PANIC, "page_hashout");
		hpp = &hp->p_hash;
	}
	*hpp = pp->p_hash;

	pp->p_hash = NULL;
	pp->p_vnode = NULL;
	pp->p_offset = 0;
	pp->p_gone = 0;

	/*
	 * Remove this page from the linked list of pages
	 * using p_vpnext/p_vpprev pointers for the list.
	 */
	CHECKPP(pp);
	if (vp->v_pages == pp)
		vp->v_pages = pp->p_vpnext;		/* go to next page */

	if (vp->v_pages == pp)
		vp->v_pages = NULL;			/* page list is gone */
	else {
		pp->p_vpprev->p_vpnext = pp->p_vpnext;
		pp->p_vpnext->p_vpprev = pp->p_vpprev;
	}
	pp->p_vpprev = pp->p_vpnext = pp;	/* make pp a list of one */
	(void) splx(s);
}

/*
 * Add the page to the front of the linked list of pages
 * using p_next/p_prev pointers for the list.
 * The caller is responsible for protecting the list pointers.
 */
STATIC void
page_add(ppp, pp)
	register page_t **ppp, *pp;
{

	ASSERT(pp >= pages && pp < epages);
	if (*ppp == NULL) {
		pp->p_next = pp->p_prev = pp;
	} else {
		pp->p_next = *ppp;
		pp->p_prev = (*ppp)->p_prev;
		(*ppp)->p_prev = pp;
		pp->p_prev->p_next = pp;
	}
	*ppp = pp;
	CHECKPP(pp);
}

/*
 * Remove this page from the linked list of pages
 * using p_next/p_prev pointers for the list.
 * The caller is responsible for protecting the list pointers.
 */
void
page_sub(ppp, pp)
	register page_t **ppp, *pp;
{

	ASSERT(pp >= pages && pp < epages);
	CHECKPP(pp);
	if (*ppp == NULL || pp == NULL)
		cmn_err(CE_PANIC, "page_sub");

	if (*ppp == pp)
		*ppp = pp->p_next;		/* go to next page */

	if (*ppp == pp)
		*ppp = NULL;			/* page list is gone */
	else {
		pp->p_prev->p_next = pp->p_next;
		pp->p_next->p_prev = pp->p_prev;
	}
	pp->p_prev = pp->p_next = pp;		/* make pp a list of one */
}

/*
 * Add this page to the list of pages, sorted by offset.
 * Assumes that the list given by *ppp is already sorted.
 * The caller is responsible for protecting the list pointers.
 */
void
page_sortadd(ppp, pp)
	register page_t **ppp, *pp;
{
	register page_t *p1;
	register u_int off;

	ASSERT(pp >= pages && pp < epages);
	CHECKLIST(*ppp);
	CHECKPP(pp);
	if (*ppp == NULL) {
		pp->p_next = pp->p_prev = pp;
		*ppp = pp;
	} else {
		/*
		 * Figure out where to add the page to keep list sorted
		 */
		p1 = *ppp;
		if (pp->p_vnode != p1->p_vnode && p1->p_vnode != NULL &&
		    pp->p_vnode != NULL)
			cmn_err(CE_PANIC, "page_sortadd: bad vp");

		off = pp->p_offset;
		if (off < p1->p_prev->p_offset) {
			do {
				if (off == p1->p_offset)
					cmn_err(CE_PANIC,
						"page_sortadd: same offset");
				if (off < p1->p_offset)
					break;
				p1 = p1->p_next;
			} while (p1 != *ppp);
		}

		/* link in pp before p1 */
		pp->p_next = p1;
		pp->p_prev = p1->p_prev;
		p1->p_prev = pp;
		pp->p_prev->p_next = pp;

		if (off < (*ppp)->p_offset)
			*ppp = pp;		/* pp is at front */
	}
	CHECKLIST(*ppp);
}

/*
 * Wait for page if kept and then reclaim the page if it is free.
 * Caller needs to verify page contents after calling this routine.
 *
 * NOTE:  The caller must ensure that the page is not on
 *	  the free list before calling this routine.
 */
void
page_wait(pp)
	register page_t *pp;
{
	register struct vnode *vp;
	register u_int offset;
	register int s;

	ASSERT(pp >= pages && pp < epages);
	ASSERT(pp->p_free == 0);
	CHECKPP(pp);
	vp = pp->p_vnode;
	offset = pp->p_offset;

	/*
	 * Reap any pages in the to be cleaned list.
	 * This might cause the page that we might
	 * have to wait for to become available.
	 */
	if (bclnlist != NULL) {
		cleanup();
		/*
		 * The page could have been freed by cleanup, so
		 * verify the identity after raising priority since
		 * it may be ripped away at interrupt level.
		 */
		s = splvm();
		if (pp->p_vnode != vp || pp->p_offset != offset) {
			(void) splx(s);
			return;
		}
	} else
		s = splvm();

	mon_enter(&page_mplock);
	while (pp->p_keepcnt != 0) {
		page_cv_wait(pp);
		/*
		 * Verify the identity of the page since it
		 * could have changed while we were sleeping.
		 */
		if (pp->p_vnode != vp || pp->p_offset != offset)
			break;
	}

	/*
	 * If the page is now on the free list and still has
	 * its original identity, get it back.  If the page
	 * has lost its old identity, the caller of page_wait
	 * is responsible for verifying the page contents.
	 */
	if (pp->p_vnode == vp && pp->p_offset == offset && pp->p_free) {
		page_reclaim(pp);
	}

	mon_exit(&page_mplock);
	(void) splx(s);
	CHECKPP(pp);
}

/*
 * Lock a physical page into memory "long term".  Used to support "lock
 * in memory" functions.  Accepts the page to be locked, and a cow variable
 * to indicate whether a the lock will travel to the new page during
 * a potential copy-on-write).  
 */

/* ARGSUSED */
int
page_pp_lock(pp, cow, kernel)
	page_t *pp;			/* page to be locked */
	int cow;			/* cow lock */
	int kernel;			/* must succeed -- ignore checking */
{
	int r = 0;			/* result -- assume failure */

	ASSERT((short)pp->p_lckcnt >= 0);
	ASSERT((short)pp->p_cowcnt >= 0);
	page_lock(pp);
	mon_enter(&page_locklock);

	if (cow) {
		if ((availrmem - 1 ) >= pages_pp_maximum) {
		 	--availrmem;
			pages_pp_locked++;
			if (pp->p_cowcnt < (u_short) PAGE_LOCK_MAXIMUM)
				if (++pp->p_cowcnt == PAGE_LOCK_MAXIMUM)
					cmn_err(CE_WARN,
					"Page frame 0x%x locked permanently\n",
						page_pptonum(pp));
			r = 1;
		} 
	} else {
		if (pp->p_lckcnt) {
			if (pp->p_lckcnt < (u_short) PAGE_LOCK_MAXIMUM)
				if (++pp->p_lckcnt == PAGE_LOCK_MAXIMUM)
					cmn_err(CE_WARN,
					"Page frame 0x%x locked permanently\n",
						page_pptonum(pp));
			r = 1;
		} else {
			if (kernel) {
				/* availrmem accounting done by caller */
				pages_pp_kernel++;
				++pp->p_lckcnt;
				r = 1;
			} else if ((availrmem - 1) >= pages_pp_maximum) {
				pages_pp_locked++;
				--availrmem;
				++pp->p_lckcnt;
				r = 1;
			}
		}
	}
	mon_exit(&page_locklock);
	page_unlock(pp);
	ASSERT((short)pp->p_lckcnt >= 0);
	ASSERT((short)pp->p_cowcnt >= 0);
	return (r);
}

/*
 * Decommit a lock on a physical page frame.  Account for cow locks if
 * appropriate.
 */

/* ARGSUSED */

void
page_pp_unlock(pp, cow, kernel)
	page_t *pp;			/* page to be unlocked */
	int cow;			/* expect cow lock */
	int kernel;			/* this was a kernel lock */
{

	ASSERT((short)pp->p_lckcnt >= 0);
	ASSERT((short)pp->p_cowcnt >= 0);
	page_lock(pp);
	mon_enter(&page_locklock);

	if (cow) {
		ASSERT(pp->p_cowcnt > 0);
		pp->p_cowcnt--;
		pages_pp_locked--;
		availrmem++;
	} else {
		ASSERT(pp->p_lckcnt > 0);
		if (--pp->p_lckcnt == 0) {
			if (kernel) {
				pages_pp_kernel--;
			} else {
				pages_pp_locked--;
				availrmem++;
			}
		}
	}
	mon_exit(&page_locklock);
	page_unlock(pp);
	ASSERT((short)pp->p_lckcnt >= 0);
	ASSERT((short)pp->p_cowcnt >= 0);
}

/*
 * Transfer a cow lock to a real lock on a physical page.  Used after a 
 * copy-on-write of a locked page has occurred.  
 */
void
page_pp_useclaim(opp, npp)
	page_t *opp;		/* original page frame losing lock */
	page_t *npp;		/* new page frame gaining lock */
{
	ASSERT((short)opp->p_lckcnt >= 0);
	ASSERT((short)opp->p_cowcnt >= 1);
	ASSERT((short)npp->p_lckcnt >= 0);
	ASSERT((short)npp->p_cowcnt >= 0);

	page_lock(npp);
	page_lock(opp);
	mon_enter(&page_locklock);
	opp->p_cowcnt--;
	npp->p_cowcnt++;
	mon_exit(&page_locklock);
	page_unlock(opp);
	page_unlock(npp);
}

/*
 * Simple claim adjust functions -- used to support to support changes in
 * claims due to changes in access permissions.  Used by segvn_setprot().
 */
int
page_addclaim(pp)
	page_t *pp;
{
	int r = 1;			/* result */

	ASSERT((short)pp->p_lckcnt >= 0);
	ASSERT((short)pp->p_cowcnt >= 0);
	mon_enter(&page_locklock);
	ASSERT(pp->p_lckcnt > 0);
	if (--pp->p_lckcnt == 0) {
		pp->p_cowcnt++;
	} else {
		if ((availrmem - 1 ) >= pages_pp_maximum) {
		 	--availrmem;
			pages_pp_locked++;
			if (pp->p_cowcnt < (u_short) PAGE_LOCK_MAXIMUM)
				if (++pp->p_cowcnt == PAGE_LOCK_MAXIMUM)
					cmn_err(CE_WARN,
					"Page frame 0x%x locked permanently\n",
						page_pptonum(pp));
		} else {
			pp->p_lckcnt++;
			r = 0;
		}
	}
	mon_exit(&page_locklock);
	ASSERT((short)pp->p_lckcnt >= 0);
	ASSERT((short)pp->p_cowcnt >= 0);
	return (r);
}

void
page_subclaim(pp)
	page_t *pp;
{

	ASSERT((short)pp->p_lckcnt >= 0);
	ASSERT((short)pp->p_cowcnt >= 0);
	mon_enter(&page_locklock);

	ASSERT(pp->p_cowcnt > 0);
	pp->p_cowcnt--;
	if (pp->p_lckcnt) {
		availrmem++;
		pages_pp_locked--;
	}
	pp->p_lckcnt++;
	mon_exit(&page_locklock);
	ASSERT((short)pp->p_lckcnt >= 0);
	ASSERT((short)pp->p_cowcnt >= 0);
}

/*
 * Mark a page as read-only when the page
 * contains holes as a result of file size
 * change (truncate up or lseek and write).
 */
u_int
page_rdonly(pp)
	page_t *pp;
{
	return (hat_rdonly(pp));
}

/*
 * Simple functions to transform a page pointer to a
 * physical page number and vice versa.  For now assume
 * pages refers to the page number pages_base and pages
 * increase from there with one page structure per
 * PAGESIZE / MMU_PAGESIZE physical page frames.
 */

#ifdef	sun386
u_int
page_pptonum(pp)
	page_t *pp;
{
	if (pp > epages2)
		cmn_err(CE_PANIC, "page_pptonum");

	if (pp >= epages)
		return ((u_int)((pp - epages) * (PAGESIZE/MMU_PAGESIZE)) +
		    pages_base2);

	return ((u_int)((pp - pages) * (PAGESIZE/MMU_PAGESIZE)) + pages_base);
}

page_t *
page_numtopp(pfn)
	register u_int pfn;
{
	if (pfn >= pages_base2 && pfn < pages_end2)
		return (&epages[(pfn - pages_base2)/(PAGESIZE/MMU_PAGESIZE)]);

	if (pfn < pages_base || pfn >= pages_end)
		return ((page_t *)NULL);
	else
		return (&pages[(pfn - pages_base) / (PAGESIZE/MMU_PAGESIZE)]);
}

#else

#if	defined(DEBUG) || defined(i386)

u_int
page_pptonum(pp)
	page_t *pp;
{
#ifdef i386
	register struct pageac	*pap;

	ASSERT(pp >= pages && pp < epages);
	for (pap = pageahead; pap; pap = pap->panext) {
		if (pp >= pap->firstpp && pp < pap->endpp)
			return(pap->firstpfn + (pp - pap->firstpp)
						* (PAGESIZE/MMU_PAGESIZE));
	}
	cmn_err(CE_PANIC,"page_pptonum: pp %x not in page pool\n",pp);
#else
	ASSERT(pp >= pages && pp < epages);
	return ((u_int)((pp - pages) * (PAGESIZE/MMU_PAGESIZE)) + pages_base);
#endif
}

page_t *
page_numtopp(pfn)
	register u_int pfn;
{
#ifdef i386
	register struct pageac	*pap;

	for (pap = pageahead; pap; pap = pap->panext) {
		if (pfn >= pap->firstpfn && pfn < pap->endpfn)
			return(&pap->firstpp[(pfn - pap->firstpfn)
						/ (PAGESIZE/MMU_PAGESIZE)]);
	}
/*
#ifdef DEBUG
	cmn_err(CE_WARN,"page_numtopp: pfn %x does NOT exist\n",pfn);
#endif
*/
	return((page_t *) NULL);
#else
	if (pfn < pages_base || pfn >= pages_end)
		return ((page_t *)NULL);
	else
		return (&pages[(pfn - pages_base) / (PAGESIZE/MMU_PAGESIZE)]);
#endif
}
#endif	/*DEBUG || i386*/
#endif	/*sun386*/

/*
 * This routine is like page_numtopp, but will only return page structs
 * for pages which are ok for loading into hardware using the page struct.
 */
page_t *
page_numtookpp(pfn)
	register u_int pfn;
{
	register page_t *pp;

#ifdef	sun386
	if (pfn >= pages_base2 && pfn < pages_end2) {
		pp = &epages[(pfn - pages_base2) / (PAGESIZE/MMU_PAGESIZE)];
		if (pp->p_free || pp->p_gone)
			return ((page_t *)NULL);
		return (pp);
	}
#endif

#ifdef i386
	register struct pageac	*pap;

	for (pap = pageahead; pap; pap = pap->panext) {
		if (pfn >= pap->firstpfn && pfn < pap->endpfn) {
			pp = &pap->firstpp[(pfn - pap->firstpfn)
						/ (PAGESIZE/MMU_PAGESIZE)];
			break;
		}
	}
	if (pap == (struct pageac *) NULL) {
/*
#ifdef DEBUG
		cmn_err(CE_WARN,"page_numtookpp: pfn %x does not exist\n",pfn);
#endif
*/
		return((page_t *) NULL);
	}
	if (pp->p_free || pp->p_gone)
		return((page_t *) NULL);
	return(pp);
#else
	if (pfn < pages_base || pfn >= pages_end)
		return ((page_t *)NULL);
	pp = &pages[(pfn - pages_base) / (PAGESIZE/MMU_PAGESIZE)];
	if (pp->p_free || pp->p_gone)
		return ((page_t *)NULL);
	return (pp);
#endif
}

/*
 * This routine is like page_numtopp, but will only return page structs
 * for pages which are ok for loading into hardware using the page struct.
 * If not for the things like the window system lock page where we
 * want to make sure that the kernel and the user are exactly cache
 * consistent, we could just always return a NULL pointer here since
 * anyone mapping physical memory isn't guaranteed all that much
 * on a virtual address cached machine anyways.  The important thing
 * here is not to return page structures for things that are possibly
 * currently loaded in DVMA space, while having the window system lock
 * page still work correctly.
 */
page_t *
page_numtouserpp(pfn)
	register u_int pfn;
{
	register page_t *pp;

#ifdef	sun386
	if (pfn >= pages_base2 && pfn < pages_end2) {
		pp = &epages[(pfn - pages_base2) / (PAGESIZE/MMU_PAGESIZE)];
		if (pp->p_free || pp->p_gone || pp->p_intrans || pp->p_lock ||
		    /* is this page possibly involved in indirect (raw) IO */
		    (pp->p_keepcnt > 0 && pp->p_vnode != NULL))
			return ((page_t *)NULL);
		return (pp);
	}
#endif

#ifdef i386
	register struct pageac	*pap;

	for (pap = pageahead; pap; pap = pap->panext) {
		if (pfn >= pap->firstpfn && pfn < pap->endpfn) {
			pp = &pap->firstpp[(pfn - pap->firstpfn)
						/ (PAGESIZE/MMU_PAGESIZE)];
			break;
		}
	}
	if (pap == (struct pageac *) NULL)
		return((page_t *) NULL);
	if (pp->p_free || pp->p_gone || pp->p_intrans || pp->p_lock ||
	    (pp->p_keepcnt > 0 && pp->p_vnode != NULL))
			return((page_t *) NULL);
	return(pp);
	
#else
	if (pfn < pages_base || pfn >= pages_end)
		return ((page_t *)NULL);
	pp = &pages[(pfn - pages_base) / (PAGESIZE/MMU_PAGESIZE)];
	if (pp->p_free || pp->p_gone || pp->p_intrans || pp->p_lock ||
	    /* is this page possibly involved in indirect (raw) IO? */
	    (pp->p_keepcnt > 0 && pp->p_vnode != NULL))
		return ((page_t *)NULL);
	return (pp);
#endif
}

/*
 * Debugging routine only!
 * XXX - places calling this should be debugging routines
 * or remove the test altogether or call cmn_err(CE_PANIC).
 */

extern int	call_demon();
STATIC int call_demon_flag = 1;

void
call_debug(mess)
	char *mess;
{
 	cmn_err(CE_WARN, mess);
	if (call_demon_flag)
		call_demon();
}

#ifdef PAGE_DEBUG
/*
 * Debugging routine only!
 */
STATIC void
page_vp_check(vp)
	register struct vnode *vp;
{
	register page_t *pp;
	int count = 0;
	int err = 0;
	extern char stext[], sdata[];	/* Start of kernel text and data */

	if (vp == NULL)
		return;

	if ((pp = vp->v_pages) == NULL) {
		/* random check to see if no pages on this vp exist */
		if ((pp = page_find(vp, 0)) != NULL) {
			cmn_err(CE_CONT, "page_vp_check: pp=%x on NULL vp list\n", vp);
			call_debug("page_vp_check");
		}
		return;
	}

	do {
		if (pp->p_vnode != vp
		 && ((char *)pp->p_vnode < stext
		 || (char *)pp->p_vnode >= sdata)) {
			cmn_err(CE_CONT, "pp=%x pp->p_vnode=%x, vp=%x\n",
			    pp, pp->p_vnode, vp);
			err++;
		}
		if (pp->p_vpnext->p_vpprev != pp) {
			cmn_err(CE_CONT, "pp=%x, p_vpnext=%x, p_vpnext->p_vpprev=%x\n",
			    pp, pp->p_vpnext, pp->p_vpnext->p_vpprev);
			err++;
		}
		if (++count > 10000) {
			cmn_err(CE_CONT, "vp loop\n");
			err++;
			break;
		}
		pp = pp->p_vpnext;
	} while (err == 0 && pp != vp->v_pages);

	if (err)
		call_debug("page_vp_check");
}

/*
 * Debugging routine only!
 */
STATIC void
page_free_check()
{
	int err = 0;
	int count = 0;
	register page_t *pp;
#ifdef AT386	/* 16MB DMA support */
	int dma_count = 0;
	int dma_err = 0;
#endif	/* 16MB DMA support */

	if (page_freelist != NULL) {
		pp = page_freelist;
		do {
			if (pp->p_free == 0 || pp->p_age == 0) {
				err++;
				cmn_err(CE_CONT, "page_free_check: pp = %x\n", pp);
			}
			count++;
			pp = pp->p_next;
		} while (pp != page_freelist);
	}
	if (page_cachelist != NULL) {
		pp = page_cachelist;
		do {
			if (pp->p_free == 0 || pp->p_age != 0) {
				err++;
				cmn_err(CE_CONT, "page_free_check: pp = %x\n", pp);
			}
			count++;
			pp = pp->p_next;
		} while (pp != page_cachelist);
	}
#ifdef AT386	/* 16MB DMA support */
	if (dma_check_on && dma_page_freelist != NULL) {
		pp = dma_page_freelist;
		do {
			if (pp->p_free == 0 || pp->p_age == 0) {
				err++;
				cmn_err(CE_CONT,"page_free_check: dma: pp %x\n", pp);
			}
			if (! DMA_PP(pp)) {
				cmn_err(CE_CONT,"page_free_check: dma: pp %x nondma\n", pp);
				dma_err++;
			}
			dma_count++;
			count++;
			pp = pp->p_next;
		} while (pp != dma_page_freelist);
	}
	if (dma_check_on &&dma_page_cachelist != NULL) {
		pp = dma_page_cachelist;
		do {
			if (pp->p_free == 0 || pp->p_age != 0) {
				err++;
				cmn_err(CE_CONT,"page_free_check: dma pp %x\n", pp);
			}
			if (! DMA_PP(pp)) {
				cmn_err(CE_CONT,"page_free_check: dma pp %x nondma\n",pp);
				dma_err++;
			}
			dma_count++;
			count++;
			pp = pp->p_next;
		} while (pp != dma_page_cachelist);
	}
#endif	/* 16MB DMA support */

	if (err || count != freemem) {
		cmn_err(CE_CONT, "page_free_check:  count = %x, freemem = %x\n",
		    count, freemem);
		call_debug("page_check_free");
	}

#ifdef AT386	/* 16MB DMA support */
	if (dma_err || dma_count != dma_freemem) {
		cmn_err(CE_CONT,"page_free_check: dma_count %x dma_freemem %x\n",
				dma_count, dma_freemem);
		cmn_err(CE_CONT,"page_free_check: dma_err %x\n", dma_err);
		call_debug("page_check_free");
	}
#endif	/* 16MB DMA support */
}

/*
 * Debugging routine only!
 * Verify that the list is properly sorted by offset on same vp
 */
void
page_list_check(plist)
	page_t *plist;
{
	register page_t *pp = plist;

	if (pp == NULL)
		return;
	while (pp->p_next != plist) {
		if (pp->p_next->p_offset <= pp->p_offset ||
		    pp->p_vnode != pp->p_next->p_vnode) {
			cmn_err(CE_CONT, "pp = %x <%x, %x> pp next = %x <%x, %x>\n",
			    pp, pp->p_vnode, pp->p_offset, pp->p_next,
			    pp->p_next->p_vnode, pp->p_next->p_offset);
			call_debug("page_list_check");
		}
		pp = pp->p_next;
	}
}

/*
 * Debugging routine only!
 * Verify that pp is actually on vp page list.
 */
void
page_pp_check(pp)
	register page_t *pp;
{
	register page_t *p1;
	register struct vnode *vp;

	if ((vp = pp->p_vnode) == (struct vnode *)NULL)
		return;

	if ((p1 = vp->v_pages) == (page_t *)NULL) {
		cmn_err(CE_CONT, "pp = %x, vp = %x\n", pp, vp);
		call_debug("NULL vp page list");
		return;
	}

	do {
		if (p1 == pp)
			return;
	} while ((p1 = p1->p_vpnext) != vp->v_pages);

	cmn_err(CE_CONT, "page %x not on vp %x page list\n", pp, vp);
	call_debug("vp page list");
}
#endif /* PAGE_DEBUG */

/*
 * The following are used by the sys3b S3BDELMEM and S3BADDMEM
 * functions.
 */

STATIC page_t *Delmem;	/* Linked list of deleted pages. */
STATIC int Delmem_cnt;	/* Count of number of deleted pages. */

STATIC int
page_delmem(count)
	register int count;
{
	register page_t	*pp;
		
	if (freemem < count
	  || availrmem - count < tune.t_minarmem 
	  || availsmem - count < tune.t_minasmem) {
		return EINVAL;
	}

	while (count > 0) {
		page_t **ppl;

		ppl = &Delmem;

		pp = page_get(PAGESIZE, P_NOSLEEP|P_NORESOURCELIM);
		if (pp == NULL) 
			return EINVAL;
		page_add(ppl, pp);

		count--;
		Delmem_cnt++;
		availrmem--;
		availsmem--;
	}

	return(0);
}

STATIC int
page_addmem(count)
	register int count;
{
	register page_t	*pp;

	while (count > 0) {
		page_t **ppl;

		pp = *(ppl = &Delmem);
		if (pp == NULL) 
			return EINVAL;
		page_sub(ppl, pp);
		page_rele(pp);

		count--;
		Delmem_cnt--;
		availrmem++;
		availsmem++;
	}

	return(0);
}

int
page_deladd(add, count, rvp)
	register int count;
	rval_t *rvp;
{
	register int error;

	if (add)
		error = page_addmem(count);
	else
		error = page_delmem(count);
	if (error == 0) {
		if (add)
			rvp->r_val1 = freemem;
		else
			rvp->r_val1 = Delmem_cnt;
	}
	return(error);
} 

/*
 * Debugging routine only!
 */

STATIC void
page_print(pp)
	register page_t *pp;
{
	register struct vnode *vp;
	extern char stext[], sdata[];	/* Start of kernel text and data */

        cmn_err(CE_CONT, "^mapping 0x%x nio %d keepcnt %d lck %d cow %d",
                pp->p_mapping, pp->p_nio, pp->p_keepcnt,
                pp->p_lckcnt, pp->p_cowcnt);
	cmn_err(CE_CONT, "^%s%s%s%s%s%s%s%s%s\n", 
		(pp->p_lock)    ? " LOCK"    : "" ,
		(pp->p_want)    ? " WANT"    : "" ,
		(pp->p_free)    ? " FREE"    : "" ,
		(pp->p_intrans) ? " INTRANS" : "" ,
		(pp->p_gone)    ? " GONE"    : "" ,
		(pp->p_mod)     ? " MOD"     : "" ,
		(pp->p_ref)     ? " REF"     : "" ,
		(pp->p_pagein)  ? " PAGEIN"  : "" ,
		(pp->p_age)	? " AGE"  : "" );
        cmn_err(CE_CONT, "^vnode 0x%x, offset 0x%x",
                pp->p_vnode, pp->p_offset);
	if (swap_anon(pp->p_vnode, pp->p_offset))
		cmn_err(CE_CONT, "^  (ANON)");
	else if ((vp = pp->p_vnode) != 0
		 && ((char *)vp < stext || (char *)vp >= sdata)) {
		cmn_err(CE_CONT, "^  v_flag 0x%x, v_count %d, v_type %d",
			vp->v_flag, vp->v_count, vp->v_type);
	}
	cmn_err(CE_CONT, "^\nnext 0x%x, prev 0x%x, vpnext 0x%x vpprev 0x%x\n",
	    pp->p_next, pp->p_prev, pp->p_vpnext, pp->p_vpprev);
}

void
phystopp(v)
{
	int pfn;
	page_t *pp;

	pfn = v >> PAGESHIFT;
	cmn_err(CE_CONT, "^pfn=0x%x, ", pfn);
	pp = page_numtopp(pfn);
	if (pp)
		cmn_err(CE_CONT, "^pp=0x%x\n", pp);
	else
		cmn_err(CE_CONT, "^pp=NULL\n");
}

void
pptophys(pp)
	page_t *pp;
{
	int pfn;
#ifdef i386
	register struct pageac	*pap;

	for (pap = pageahead; pap; pap = pap->panext) {
		if (pp >= pap->firstpp && pp < pap->endpp) {
			pfn = pap->firstpfn + (pp - pap->firstpp)
						* (PAGESIZE/MMU_PAGESIZE);
			break;
		}
	}
	if (pap == (struct pageac *) NULL) {
		cmn_err(CE_CONT, "^pfn= NULL\n");
		return;
	}
#else
	pfn = ((u_int)((pp - pages) * (PAGESIZE/MMU_PAGESIZE)) + pages_base);
#endif
	cmn_err(CE_CONT, "^pfn=0x%x, ", pfn);
	cmn_err(CE_CONT, "^phys=0x%x\n", ctob(pfn));
}

xpage_find(vp, off)
	register struct vnode *vp;
	register u_int off;
{
	register page_t *pp;
	register int len = 0;

	for (pp = page_hash[PAGE_HASHFUNC(vp, off)]; pp; pp = pp->p_hash, len++)
		if (pp->p_vnode == vp && pp->p_offset == off) {
			if (pp->p_gone == 0)
				return 0;
		}
	return 1;
}

void
findpage(vp, off)
	register struct vnode *vp;
	register u_int off;
{
	register page_t *pp;
	register int len = 0;
	register int found = 0;

	for (pp = page_hash[PAGE_HASHFUNC(vp, off)]; pp; pp = pp->p_hash, len++)
		if (pp->p_vnode == vp && pp->p_offset == off) {
			if (found++)
				cmn_err(CE_CONT, "^\t\t\t\t\t\t      ");
			cmn_err(CE_CONT, "^%x %s%s%s%s%s%s%s%s%s %d %d %d\n", 
				pp,
				(pp->p_lock)    ? "L"    : " " ,
				(pp->p_want)    ? "W"    : " " ,
				(pp->p_free)    ? "F"    : " " ,
				(pp->p_intrans) ? "I" : " " ,
				(pp->p_gone)    ? "G"    : " " ,
				(pp->p_mod)     ? "M"     : " " ,
				(pp->p_ref)     ? "R"     : " " ,
				(pp->p_pagein)  ? "P"  : " " ,
				(pp->p_age)	? "A"  : "" ,
				pp->p_keepcnt, pp->p_lckcnt, pp->p_cowcnt);
		}
	if (found == 0)
		cmn_err(CE_CONT, "^not found\n");
}


#if	defined(DEBUG) && defined(i386)
print_page_map_table()
{
	/*
	register struct pageac	*pap;

	cmn_err(CE_CONT,"\nPage Map Entries:\n");
	for (pap = pageahead; pap; pap = pap->panext)
		cmn_err(CE_CONT,"PG (%x , %x)      PP (%x , %x)\n",
				pap->firstpfn, pap->endpfn, pap->firstpp, pap->endpp);
	cmn_err(CE_CONT,"\n");
	*/
	return ;
}
#endif
