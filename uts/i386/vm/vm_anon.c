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

#ident  "@(#)kern-vm:vm_anon.c	1.3.1.5"

/*
 * VM - anonymous pages.
 *
 * This layer sits immediately above the vm_swap layer.  It manages
 * physical pages that have no permanent identity in the file system
 * name space, using the services of the vm_swap layer to allocate
 * backing storage for these pages.  Since these pages have no external
 * identity, they are discarded when the last reference is removed.
 *
 * An important function of this layer is to manage low-level sharing
 * of pages that are logically distinct but that happen to be
 * physically identical (e.g., the corresponding pages of the processes
 * resulting from a fork before one process or the other changes their
 * contents).  This pseudo-sharing is present only as an optimization
 * and is not to be confused with true sharing in which multiple
 * address spaces deliberately contain references to the same object;
 * such sharing is managed at a higher level.
 *
 * The key data structure here is the anon struct, which contains a
 * reference count for its associated physical page and a hint about
 * the identity of that page.  Anon structs typically live in arrays,
 * with an instance's position in its array determining where the
 * corresponding backing storage is allocated; however, the swap_xlate()
 * routine abstracts away this representation information so that the
 * rest of the anon layer need not know it.  (See the swap layer for
 * more details on anon struct layout.)
 *
 * In the future versions of the system, the association between an
 * anon struct and its position on backing store will change so that
 * we don't require backing store all anonymous pages in the system.
 * This is important for consideration for large memory systems.
 * We can also use this technique to delay binding physical locations
 * to anonymous pages until pageout/swapout time where we can make
 * smarter allocation decisions to improve anonymous klustering.
 *
 * Many of the routines defined here take a (struct anon **) argument,
 * which allows the code at this level to manage anon pages directly,
 * so that callers can regard anon structs as opaque objects and not be
 * concerned with assigning or inspecting their contents.
 *
 * Clients of this layer refer to anon pages indirectly.  That is, they
 * maintain arrays of pointers to anon structs rather than maintaining
 * anon structs themselves.  The (struct anon **) arguments mentioned
 * above are pointers to entries in these arrays.  It is these arrays
 * that capture the mapping between offsets within a given segment and
 * the corresponding anonymous backing storage address.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/mman.h"
#include "sys/time.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/vmmeter.h"
#include "sys/swap.h"
#include "sys/tuneable.h"
#include "sys/cmn_err.h"
#include "sys/sysmacros.h"
#include "sys/debug.h"

#include "sys/proc.h"		/* XXX - needed for PREEMPT() */
#include "sys/disp.h"		/* XXX - needed for PREEMPT() */

#include "vm/hat.h"
#include "vm/anon.h"
#include "vm/as.h"
#include "vm/page.h"
#include "vm/seg.h"
#include "vm/pvn.h"
#include "vm/rm.h"
#include "vm/mp.h"
#include "vm/trace.h"

extern int	availsmem;
extern void	pagecopy();
extern void	pagezero();

struct	anoninfo anoninfo;

STATIC mon_t anon_lock;
STATIC int npagesteal;

/*
 * Reserve anon space.
 * Return non-zero on success.
 */
int
anon_resv(size)
	u_int size;
{
	register int pages = btopr(size);

	if (availsmem - pages < tune.t_minasmem) {
		nomemmsg("anon_resv", pages, 0, 0);
		return 0;
	}

	if (anoninfo.ani_resv + pages > anoninfo.ani_max) {
		return 0;
	} 
	anoninfo.ani_resv += pages;
	availsmem -= pages;
	return (1);
}

/*
 * Give back an anon reservation.
 */
void
anon_unresv(size)
	u_int size;
{
	register u_int pages = btopr(size);

	availsmem += pages;
	anoninfo.ani_resv -= pages;
	if ((int)anoninfo.ani_resv < 0)
		cmn_err(CE_WARN, "anon: reservations below zero???\n");
}

/*
 * Allocate an anon slot.
 */
struct anon *
anon_alloc()
{
	register struct anon *ap;

	mon_enter(&anon_lock);
	ap = swap_alloc();
	if (ap != NULL) {
		anoninfo.ani_free--;
		ap->an_refcnt = 1;
		ap->un.an_page = NULL;
		ap->an_flag = ALOCKED;
#ifdef DEBUG
		ASSERT(ap->an_use == AN_NONE);
		ap->an_use = AN_DATA;
#endif
	}
	mon_exit(&anon_lock);
	return (ap);
}

#ifdef DEBUG
struct anon *
anon_upalloc()
{
	register struct anon *ap;

	mon_enter(&anon_lock);
	ap = swap_alloc();
	if (ap != NULL) {
		anoninfo.ani_free--;
		ap->an_refcnt = 1;
		ap->un.an_page = NULL;
		ap->an_flag = ALOCKED;
		ASSERT(ap->an_use == AN_NONE);
		ap->an_use = AN_UPAGE;
	}
	mon_exit(&anon_lock);
	return (ap);
}
#endif

/*
 * Decrement the reference count of an anon page.
 * If reference count goes to zero, free it and
 * its associated page (if any).
 */
STATIC void
anon_decref(ap)
	register struct anon *ap;
{
	register page_t *pp;
	struct vnode *vp;
	u_int off;

	ASSERT(ap->an_refcnt > 0);
#ifdef DEBUG
	ASSERT(ap->an_use == AN_DATA);
#endif
	if (ap->an_refcnt == 1) {
		/*
		 * If there is a page for this anon slot we will need to
		 * call page_abort to get rid of the vp association and
		 * put the page back on the free list as really free.
		 */
		swap_xlate(ap, &vp, &off);
		pp = page_find(vp, off);

		/*
		 * XXX - If we have a page, wait for it's keepcnt to become
		 * zero, re-verify it's identity before aborting it and
		 * freeing the swap slot. This ensures that any pending i/o
		 * always completes before the swap slot is freed.
		 */
		if (pp != NULL) {
			/*
			 * XXX - Don't know why this is commented out,
			 * but if it's ever enabled it'll break segu_release,
			 * since page_wait can sleep.
			 *
			if (pp->p_keepcnt != 0)
				page_wait(pp);
			if (pp->p_vnode == vp && pp->p_offset == off)
			*/
			if (pp->p_intrans == 0)
				page_abort(pp);
		}
#ifdef DEBUG
		ap->an_use = AN_NONE;
#endif
		ap->an_refcnt--;
		mon_enter(&anon_lock);
		swap_free(ap);
		anoninfo.ani_free++;
		mon_exit(&anon_lock);
	} else {
		ap->an_refcnt--;
	}

}

#ifdef DEBUG
void
anon_updecref(ap)
	register struct anon *ap;
{
	register page_t *pp;
	struct vnode *vp;
	u_int off;


	ASSERT(ap->an_use == AN_UPAGE);
	ASSERT(ap->an_refcnt == 1);
	if (ap->an_refcnt == 1) {
		/*
		 * If there is a page for this anon slot we will need to
		 * call page_abort to get rid of the vp association and
		 * put the page back on the free list as really free.
		 */
		swap_xlate(ap, &vp, &off);
		pp = page_find(vp, off);

		/*
		 * XXX - If we have a page, wait for it's keepcnt to become
		 * zero, re-verify it's identity before aborting it and
		 * freeing the swap slot. This ensures that any pending i/o
		 * always completes before the swap slot is freed.
		 */
		if (pp != NULL) {
			/*
			if (pp->p_keepcnt != 0)
				page_wait(pp);
			if (pp->p_vnode == vp && pp->p_offset == off)
			*/
				page_abort(pp);
		}
		ap->an_use = AN_NONE;
		ap->an_refcnt--;
		mon_enter(&anon_lock);
		swap_free(ap);
		anoninfo.ani_free++;
		mon_exit(&anon_lock);
	} else {
		ap->an_refcnt--;
	}
}
#endif

/*
 * Duplicate references to size bytes worth of anon pages.
 * Used when duplicating a segment that contains private anon pages.
 * This code assumes that procedure calling this one has already used
 * hat_chgprot() to disable write access to the range of addresses that
 * that *old actually refers to.
 */
void
anon_dup(old, new, size)
	register struct anon **old, **new;
	u_int size;
{
	register int i;

	i = btopr(size);
	while (i-- > 0) {
		if ((*new = *old) != NULL) {
			(*new)->an_refcnt++;
#ifdef DEBUG
			ASSERT((*new)->an_use == AN_DATA);
#endif
		}
		old++;
		new++;
	}
}

/*
 * Free a group of "size" anon pages, size in bytes.
 */
void
anon_free(app, size)
	register struct anon **app;
	u_int size;
{
	register int i;

	i = btopr(size);
	while (i-- > 0) {
		if (*app != NULL) {
			anon_decref(*app);
			*app = NULL;
		}
		app++;
		/* 
		 * This loop takes a while so we put in a preemption point
		 * here. We preempt only when the current process is not
		 * in zombie state. (This function is also called when a 
		 * process's state is set to zombie)
		 */
		if (curproc->p_stat != SZOMB)
                        PREEMPT();
	}
}

#ifdef DEBUG
void
anon_upfree(app, size)
	register struct anon **app;
	u_int size;
{
	register int i;

	i = btopr(size);
	while (i-- > 0) {
		if (*app != NULL) {
			anon_updecref(*app);
			*app = NULL;
		}
		app++;
	}
}
#endif

/*
 * Return the kept page(s) and protections back to the segment driver.
 */
int
anon_getpage(app, protp, pl, plsz, seg, addr, rw, cred)
	struct anon **app;
	u_int *protp;
	page_t *pl[];
	u_int plsz;
	struct seg *seg;
	addr_t addr;
	enum seg_rw rw;
	struct cred *cred;
{
	register page_t *pp, **ppp;
	register struct anon *ap = *app;
	struct vnode *vp;
	u_int off;
	int err;
	extern int nopagereclaim;
	register int s;

	ALOCK(ap);
	swap_xlate(ap, &vp, &off);
again:
	pp = ap->un.an_page;
	/*
	 * If the anon pointer has a page associated with it,
	 * see if it looks ok.  If page is being paged in,
	 * wait for it to finish as we must return a list of
	 * pages since this routine acts like the VOP_GETPAGE
	 * routine does.
	 */
	s = splvm();
	if (pp != NULL && pp->p_vnode == vp && pp->p_offset == off &&
	    !pp->p_gone) {
		if (pp->p_intrans && (pp->p_pagein || nopagereclaim)) {
			(void) splx(s);
			page_wait(pp);
			goto again;		/* try again */
		}
		if (pp->p_free)
			page_reclaim(pp);
		(void) splx(s);
		PAGE_HOLD(pp);
		if (ap->an_refcnt == 1)
			*protp = PROT_ALL;
		else
			*protp = PROT_ALL & ~PROT_WRITE;
		pl[0] = pp;
		pl[1] = NULL;
		err = 0;
	} else {
		/*
		 * Simply treat it as a vnode fault on the anon vp.
		 */
		(void) splx(s);
		trace3(TR_SEG_GETPAGE, seg, addr, TRC_SEG_ANON);
		err = VOP_GETPAGE(vp, off, PAGESIZE, protp, pl, plsz,
		    seg, addr, rw, cred);
		if (err == 0) {
			for (ppp = pl; (pp = *ppp++) != NULL; ) {
				if (pp->p_offset == off) {
					ap->un.an_page = pp;
					break;
				}
			}
		}
		if (ap->an_refcnt != 1)
			*protp &= ~PROT_WRITE;	/* make read-only */
	}
	AUNLOCK(ap);
	return (err);
}
/*
 * Turn a reference to a shared anon page into a private
 * page with a copy of the data from the original page.
 */
page_t *
anon_private(app, seg, addr, opp, oppflags)
	struct anon **app;
	struct seg *seg;
	addr_t addr;
	page_t *opp;
	u_int oppflags;
{
	register struct anon *old = *app;
	register struct anon *new;
	register page_t *pp;
	struct vnode *vp;
	u_int i, off;

	/* get a locked swap slot */
	new = anon_alloc();
	if (new == (struct anon *)NULL) {
		rm_outofanon();
		return ((page_t *)NULL);	/* out of swap space */
	}

	swap_xlate(new, &vp, &off);
again:
	pp = page_lookup(vp, off);

	if (pp == NULL && (oppflags & STEAL_PAGE) && old == NULL
	  && opp->p_mod == 0 && opp->p_keepcnt == 1) {
		pp = opp;
		hat_unlock(seg, addr);	/* unlock the old translation */
		hat_pageunload(pp);	/* unload all translations    */
		page_hashout(pp);	/* destroy old name for page  */
		trace6(TR_SEG_ALLOCPAGE, seg, addr, TRC_SEG_ANON, vp, off, pp);
		if (page_enter(pp, vp, off))	/* rename as anon page */
			cmn_err(CE_PANIC, "anon private steal");

		/* We've changed the page's identity;
		   we must re-initialize the p_dblist[] fields. */
		for (i = 0; i < PAGESIZE/NBPSCTR; i++)
			pp->p_dblist[i] = -1;

		new->un.an_page = pp;
		*app = new;
		pp->p_mod = 1;
		page_unlock(pp);
		AUNLOCK(new);
		npagesteal++;
		return (pp);
	}

	if (pp == NULL) {
		/*
		 * Normal case, need to allocate new page frame.
		 */
		pp = rm_allocpage(seg, addr, PAGESIZE, P_CANWAIT);
		trace6(TR_SEG_ALLOCPAGE, seg, addr, TRC_SEG_ANON, vp, off, pp);
		if (page_enter(pp, vp, off)) {
			PAGE_RELE(pp);
			goto again;		/* try again */
		}
	} else {
		/*
		 * Already found a page with the right identity - just use it.
		 */
		PAGE_HOLD(pp);
		page_lock(pp);
	}
	new->un.an_page = pp;

	/*
	 * Now copy the contents from the original page,
	 * which is held by the caller;
	 */

	pp->p_intrans = pp->p_pagein = 1;
	ppcopy(opp, pp);
	pp->p_intrans = pp->p_pagein = 0;

	/*
	 * Ok, now we can unlock and unload the old translation info.
	 */
	*app = new;
	hat_unload(seg, addr, PAGESIZE, HAT_UNLOCK);

	pp->p_mod = 1;				/* mark as modified */
	page_unlock(pp);
	AUNLOCK(new);

	/*
	 * If the old page was locked, we need
	 * to move the lock to the new page.
	 */
	if (oppflags & LOCK_PAGE)
		page_pp_useclaim(opp, pp);

	/*
	 * Ok, now release the original page, or else the
	 * process will sleep forever in anon_decref()
	 * waiting for the `keepcnt' to become 0.
	 */
	PAGE_RELE(opp);

	/*
	 * If we copied away from an anonymous page, then
	 * we are one step closer to freeing up an anon slot.
	 */
	if (old != NULL)
		anon_decref(old);
	return (pp);
}

/*
 * Allocate a private zero-filled anon page.
 */

/* ARGSUSED */
page_t *
anon_zero(seg, addr, app)
	struct seg *seg;
	addr_t addr;
	struct anon **app;
{
	register struct anon *ap;
	register page_t *pp;
	struct vnode *vp;
	u_int off;

	/* get a locked swap slot */
	*app = ap = anon_alloc();
	if (ap == NULL) {
		rm_outofanon();
		return ((page_t *)NULL);
	}

	swap_xlate(ap, &vp, &off);
again:
	pp = page_lookup(vp, off);

	if (pp == NULL) {
		/*
		 * Normal case, need to allocate new page frame.
		 */
		pp = rm_allocpage(seg, addr, PAGESIZE, P_CANWAIT);
		trace6(TR_SEG_ALLOCPAGE, seg, addr, TRC_SEG_ANON, vp, off, pp);
		if (page_enter(pp, vp, off)) {
			PAGE_RELE(pp);
			goto again;		/* try again */
		}
	} else {
		/*
		 * Already found a page with the right identity - just use it.
		 */
		PAGE_HOLD(pp);
		page_lock(pp);
	}
	ap->un.an_page = pp;

	pagezero(pp, 0, PAGESIZE);
	cnt.v_zfod++;
	pp->p_mod = 1;		/* mark as modified so pageout writes back */
	page_unlock(pp);
	AUNLOCK(ap);
	return (pp);
}

/* ARGSUSED */
page_t *
anon_zero_aligned(seg, addr, app, align_mask, align_val)
	struct seg *seg;
	addr_t addr;
	struct anon **app;
{
	register struct anon *ap;
	register page_t *pp;
	struct vnode *vp;
	u_int off;
	struct anon *unused_list = NULL;

again:
	/* get a locked swap slot */
	*app = ap = anon_alloc();
	if (ap == NULL) {
		rm_outofanon();
		while (unused_list != NULL) {
			unused_list = (ap = unused_list)->un.an_next;
			ap->un.an_next = NULL;
			AUNLOCK(ap);
			anon_decref(ap);
		}
		return ((page_t *)NULL);
	}

	swap_xlate(ap, &vp, &off);
	pp = page_lookup(vp, off);

	if (pp == NULL) {
		/*
		 * Normal case, need to allocate new page frame.
		 */
		pp = rm_allocpage_aligned(seg, addr, PAGESIZE,
						align_mask, align_val, P_CANWAIT);
		trace6(TR_SEG_ALLOCPAGE, seg, addr, TRC_SEG_ANON, vp, off, pp);
		if (page_enter(pp, vp, off)) {
			PAGE_RELE(pp);
next_anon:
			/* Save this anon so we can free it later */
			ap->un.an_next = unused_list;
			unused_list = ap;
			goto again;		/* try again */
		}
	} else {
		/*
		 * Already found a page with the right identity - just use it.
		 */
		if ((ctob(page_pptonum(pp)) & align_mask) != align_val) {
			/* Wrong alignment; get another one */
			goto next_anon;
		}
		PAGE_HOLD(pp);
		page_lock(pp);
	}

	ap->un.an_page = pp;

	pagezero(pp, 0, PAGESIZE);
	cnt.v_zfod++;
	pp->p_mod = 1;		/* mark as modified so pageout writes back */
	page_unlock(pp);
	AUNLOCK(ap);

	while (unused_list != NULL) {
		unused_list = (ap = unused_list)->un.an_next;
		ap->un.an_next = NULL;
		AUNLOCK(ap);
		anon_decref(ap);
	}

	return (pp);
}

/*
 * This gets calls by the seg_vn driver unload routine
 * which is called by the hat code when it decides to
 * unload a particular mapping.
 */
void
anon_unloadmap(ap, ref, mod)
	struct anon *ap;
	u_int ref, mod;
{
	struct vnode *vp;
	u_int off;

	ALOCK(ap);
	swap_xlate(ap, &vp, &off);
	pvn_unloadmap(vp, off, ref, mod);
	AUNLOCK(ap);
}
