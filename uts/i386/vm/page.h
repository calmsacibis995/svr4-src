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

#ifndef _VM_PAGE_H
#define _VM_PAGE_H

#ident	"@(#)kern-vm:page.h	1.3.1.4"

#include "vm/vm_hat.h"

/*
 * VM - Ram pages.
 *
 * Each physical page has a page structure, which is used to maintain
 * these pages as a cache.  A page can be found via a hashed lookup
 * based on the [vp, offset].  If a page has an [vp, offset] identity,
 * then it is entered on a doubly linked circular list off the
 * vnode using the vpnext/vpprev pointers.   If the p_free bit
 * is on, then the page is also on a doubly linked circular free
 * list using next/prev pointers.  If the p_intrans bit is on,
 * then the page is currently being read in or written back.
 * In this case, the next/prev pointers are used to link the
 * pages together for a consecutive IO request.  If the page
 * is in transit and the the page is coming in (pagein), then you
 * must wait for the IO to complete before you can attach to the page.
 * 
 */
typedef struct page {
	u_int	p_lock: 1,		/* locked for name manipulation */
		p_want: 1,		/* page wanted */
		p_free: 1,		/* on free list */
		p_intrans: 1,		/* data for [vp, offset] intransit */
		p_gone: 1,		/* page has been released */
		p_mod: 1,		/* software copy of modified bit */
		p_ref: 1,		/* software copy of reference bit */
		p_pagein: 1,		/* being paged in, data not valid */
		p_nc: 1,		/* do not cache page */
		p_age: 1;		/* on page_freelist */
	u_int	p_nio : 6;		/* # of outstanding io reqs needed */
	u_short	p_keepcnt;		/* number of page `keeps' */
	struct	vnode *p_vnode;		/* logical vnode this page is from */
	u_int	p_offset;		/* offset into vnode for this page */
	struct page *p_hash;		/* hash by [vnode, offset] */
	struct page *p_next;		/* next page in free/intrans lists */
	struct page *p_prev;		/* prev page in free/intrans lists */
	struct page *p_vpnext;		/* next page in vnode list */
	struct page *p_vpprev;		/* prev page in vnode list */
	struct phat p_hat;		/* One of two hat-specific structures.
					   This structure is for those fields
					   which cannot be overloaded; i.e.
					   they must be valid for all pages.
					   One field, in particular, must be
					   part of this structure: mappings,
					   whose exact semantics are hat-
					   specific, but which must always,
					   generically, be zero if the page
					   has any translations mapped to it,
					   and non-zero otherwise. */
	u_short	p_lckcnt;		/* number of locks on page data */
	u_short	p_cowcnt;		/* number of copy on write lock */
	union {
	  daddr_t  _p_dblist[PAGESIZE/NBPSCTR]; /* disk storage for the page */
	  struct phat2  _p_hat2;	/* Another hat-specific structure.
					   This one is for fields which can
					   be overloaded on top of p_dblist,
					   e.g., for special pages such as
					   page tables. */
	} _p_db;
#ifdef DEBUG
	struct proc *p_uown;		/* process owning it as u-page */
#endif
} page_t;

#define p_mapping	p_hat.mappings
#define p_dblist	_p_db._p_dblist
#define p_hat2		_p_db._p_hat2

/*
 * Maximum of noncontiguous Memory Segments.
 * XXX - We must change this to be a tuneable. page_init() must handle this.
 */
#define	MAX_MEM_SEG	10

struct pageac {			/* page pool accounting structure */
	struct pageac *panext;	/* pointer to next contiguous
				 * chunk in the ram pool.
				 * It must be the first entry in
				 * this structure for page_init() to work.
				 */
	uint	num;		/* number of logical pages in chunk */
	uint	firstpfn;	/* page frame number of first page in chunk. */
	uint	endpfn;		/* firstpfn + num*(PAGESIZE/MMU_PAGESIZE) */
				/* NOTE:This page frame number DOES NOT exits */
	struct	page *firstpp;	/* pointer to first page structure */
	struct	page *endpp;	/* firstpp + num */
} ;

/* global mapping table for noncontiguous user free memory chunks.
*/
extern struct pageac pageac_table[];

/* 1st page structure address for 1st free user page in the system.
*/
extern page_t *pages;

/* Last page structure address for (Last free user page in system - 1). 
/* Logically this structure does not exists.
*/
extern page_t *epages;



#ifdef _KERNEL
#define PAGE_HOLD(pp)	(pp)->p_keepcnt++
#define PAGE_RELE(pp)	page_rele(pp)

/*
 * page_get() request flags.
 */
#ifndef P_NOSLEEP
#define	P_NOSLEEP	0x0000
#define	P_CANWAIT	0x0001
#define	P_PHYSCONTIG	0x0002
#define	P_DMA		0x0004
#define P_NORESOURCELIM	0x0008
#endif

#define	PAGE_HASHSZ	page_hashsz

extern	int page_hashsz;
extern	page_t **page_hash;

struct pageac *pageahead;
uint pagepoolsize;

#ifdef sun386
extern	page_t *epages2;		/* end of absolutely all pages */
extern	u_int	pages_base2;		/* page # for compaq expanded mem */
#endif

/*
 * Variables controlling locking of physical memory.
 */
extern	u_int	pages_pp_locked;	/* physical pages actually locked */
extern	u_int	pages_pp_claimed;	/* physical pages reserved */
extern	u_int	pages_pp_kernel;	/* physical page locks by kernel */
extern	u_int	pages_pp_maximum;	/* tuning: lock + claim <= max */

/*
 * Page frame operations.
 */

void	page_init(/* pap */);
void	page_reclaim(/* pp */);
page_t *page_exists(/* vp, off */);
page_t *page_find(/* vp, off */);
page_t *page_lookup(/* vp, off */);
int	page_enter(/* pp, vp, off */);
void	page_abort(/* pp */);
void	page_free(/* pp */);
page_t *page_get(/* bytes, flags */);
page_t *page_get_aligned(/* bytes, align_mask, align_val, flags */);
int	page_pp_lock(/* pp, claim, kernel */);
void	page_pp_unlock(/* pp, claim, kernel */);
void	page_pp_useclaim(/* opp, npp */);
int	page_addclaim(/* claim */);
void	page_subclaim(/* claim */);
void	page_hashin(/* pp, vp, offset, lock */);
void	page_hashout(/* pp */);
void	page_sub(/* ppp, pp */);
void	page_sortadd(/* ppp, pp */);
void	page_wait(/* pp */);
page_t *page_numtookpp(/* pfnum */);

#if defined(DEBUG) || defined(sun386) || defined(i386)

	/*
	 *	Since we now handle non-contiguous physical memory segments
	 *	on the 80386 we need these routines.
	 */

u_int	page_pptonum(/* pp */);
page_t *page_numtopp(/* pfnum */);

extern	u_int	pages_base;	/* on 80386 this is pfn for pages */
extern	uint	pages_end;	/* on 80386 this is pfn for epages */
				/* But this base and end may not be contiguous */

#else

	/*
	 *	Generalized simple case, e.g. on 3b2. Here we assume that
	 *	we have a single physically contiguous memory segment.
	 */

extern	u_int	pages_base;		/* page # for pages[0] */
extern	uint	pages_end;

#define page_pptonum(pp) \
	(((uint)((pp) - pages) * \
		(PAGESIZE/MMU_PAGESIZE)) + pages_base)

#define page_numtopp(pfn) \
	((pfn) < pages_base || (pfn) >= pages_end) ? \
		((struct page *) NULL) : \
		((struct page *) (&pages[(uint) ((pfn) - pages_base) / \
			(PAGESIZE/MMU_PAGESIZE)]))

#endif

#include "vm/mp.h"

#ifdef DEBUG

void	page_rele(/* pp */);
void	page_lock(/* pp */);
void	page_unlock(/* pp */);

#else

extern	mon_t	page_mplock;		/* lock for manipulating page links */

#define page_rele(pp) { \
	mon_enter(&page_mplock); \
	\
	if (--((struct page *)(pp))->p_keepcnt == 0) { \
		while (((struct page *)(pp))->p_want) { \
			cv_broadcast(&page_mplock, (char *)(pp)); \
			((struct page *)(pp))->p_want = 0; \
		} \
	} \
	\
	mon_exit(&page_mplock); \
	\
	if (((struct page *)(pp))->p_keepcnt == 0 \
		&& (((struct page *)(pp))->p_gone \
		|| ((struct page *)(pp))->p_vnode == NULL)) \
		page_abort(pp); \
}
#define page_lock(pp) { \
	mon_enter(&page_mplock); \
	while (pp->p_lock) \
		page_cv_wait(pp); \
	pp->p_lock = 1; \
	mon_exit(&page_mplock); \
}
#define page_unlock(pp) { \
	mon_enter(&page_mplock); \
	((struct page *)(pp))->p_lock = 0; \
	while (((struct page *)(pp))->p_want) { \
		((struct page *)(pp))->p_want = 0; \
		cv_broadcast(&page_mplock, (char *)(pp)); \
	} \
	mon_exit(&page_mplock); \
}

#endif

#endif /* _KERNEL */

/*
 * Page hash table is a power-of-two in size, externally chained
 * through the hash field.  PAGE_HASHAVELEN is the average length
 * desired for this chain, from which the size of the page_hash
 * table is derived at boot time and stored in the kernel variable
 * page_hashsz.  In the hash function it is given by PAGE_HASHSZ.
 * PAGE_HASHVPSHIFT is defined so that 1 << PAGE_HASHVPSHIFT is
 * the approximate size of a vnode struct.
 */
#define	PAGE_HASHAVELEN		4
#define	PAGE_HASHVPSHIFT	6
#define	PAGE_HASHFUNC(vp, off) \
	((((off) >> PAGESHIFT) + ((int)(vp) >> PAGE_HASHVPSHIFT)) & \
		(PAGE_HASHSZ - 1))

#if defined(__STDC__)
extern void page_cv_wait(page_t *);
extern void ppcopy(page_t *, page_t *);
#else
extern void page_cv_wait();
extern void ppcopy();
#endif /* __STDC__ */

#endif	/* _VM_PAGE_H */
