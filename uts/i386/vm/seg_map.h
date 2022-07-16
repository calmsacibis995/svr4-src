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

#ifndef _VM_SEG_MAP_H
#define _VM_SEG_MAP_H

#ident  "@(#)kern-vm:seg_map.h	1.3.1.3"

#ifndef _SYS_PARAM_H
#include <sys/param.h>
#endif

struct segmap_crargs {
	u_int	prot;
};

/* The following computes the number of pages covered by an smap (see below).
 * Also, PGARRAYSIZE is computed, which is the length of an array of shorts
 * needed to hold enough bits to use one bit per page.
 */

#define PGPERSMAP	(MAXBSIZE / PAGESIZE)
#define NBPS		(NBBY * 2 /* sizeof(u_short) */)
#define PGARRAYSIZE	((PGPERSMAP + NBPS - 1) / NBPS)

/*
 * Each smap struct represents a MAXBSIZE sized mapping to the
 * <sm_vp, sm_off> given in the structure.  The location of the
 * the structure in the array gives the virtual address of the
 * mapping.
 */
struct	smap {
	struct	vnode *sm_vp;		/* vnode pointer (if mapped) */
	u_int	sm_off;			/* file offset for mapping */
	u_short	sm_refcnt;		/* reference count for uses */
	u_short	sm_pgflag[PGARRAYSIZE];	/* per-page flags */
	/*
	 * These next 3 entries could be coded as
	 * u_shorts if we are tight on memory.
	 */
	struct	smap *sm_hash;		/* hash pointer */
	struct	smap *sm_next;		/* next pointer */
	struct	smap *sm_prev;		/* previous pointer */
	u_int	sm_pgowner;	/* owner (proc ptr) of uninitialized pages */
	u_short	sm_pgowncnt;	/* nesting count for getmaps while owned */
};

#if PGARRAYSIZE > 1
#define SMPGIDX(pg)	((pg) / NBPS)
#define SMPGOFF(pg)	((pg) % NBPS)
#else
#define SMPGIDX(pg)	0
#define SMPGOFF(pg)	(pg)
#endif

#define SM_PG_UNINIT(sm, pg)	  /* get uninitialized-page flag */ \
		((sm)->sm_pgflag[SMPGIDX(pg)] & (1 << SMPGOFF(pg)))

#define SM_SET_PG_UNINIT(sm, pg)  /* set uninitialized-page flag */ \
		((sm)->sm_pgflag[SMPGIDX(pg)] |= (1 << SMPGOFF(pg)))

#define SM_CLR_PG_UNINIT(sm, pg)  /* clear uninitialized-page flag */ \
		((sm)->sm_pgflag[SMPGIDX(pg)] &= ~(1 << SMPGOFF(pg)))

/*
 * (Semi) private data maintained by the segmap driver per SEGMENT mapping
 */
struct	segmap_data {
	struct	smap *smd_sm;		/* array of smap structures */
	struct	smap *smd_free;		/* free list head pointer */
	u_char	smd_prot;		/* protections for all smap's */
	u_char	smd_want;		/* smap want flag */
	u_int	smd_hashsz;		/* power-of-two hash table size */
	struct	smap **smd_hash;	/* pointer to hash table */
};

/*
 * These are flags used on release.  Some of these might get handled
 * by segment operations needed for msync (when we figure them out).
 * SM_ASYNC modifies SM_WRITE.  SM_DONTNEED modifies SM_FREE.  SM_FREE
 * and SM_INVAL are mutually exclusive.
 */
#define	SM_WRITE	0x01		/* write back the pages upon release */
#define	SM_ASYNC	0x02		/* do the write asynchronously */
#define	SM_FREE		0x04		/* put pages back on free list */
#define	SM_INVAL	0x08		/* invalidate page (no caching) */
#define	SM_DONTNEED	0x10		/* less likely to be needed soon */

#define MAXBSHIFT	13		/* log2(MAXBSIZE) */

#define MAXBOFFSET	(MAXBSIZE - 1)
#define MAXBMASK	(~MAXBOFFSET)

/*
 * SMAP_HASHAVELEN is the average length desired for this chain, from
 * which the size of the smd_hash table is derived at segment create time.
 * SMAP_HASHVPSHIFT is defined so that 1 << SMAP_HASHVPSHIFT is the
 * approximate size of a vnode struct.
 */
#define	SMAP_HASHAVELEN		4
#define	SMAP_HASHVPSHIFT	6

#define	SMAP_HASHFUNC(smd, vp, off) \
	((((off) >> MAXBSHIFT) + ((int)(vp) >> SMAP_HASHVPSHIFT)) & \
		((smd)->smd_hashsz - 1))

#ifdef _KERNEL

/* the kernel generic mapping segment */
extern struct seg *segkmap;

#if defined(__STDC__)

extern int segmap_create(struct seg *, void *);
extern void segmap_pagecreate(struct seg *, addr_t, u_int, int);
extern addr_t segmap_getmap(struct seg *, struct vnode *, u_int);
extern int segmap_release(struct seg *, addr_t, u_int);

#else

extern int segmap_create();
extern void segmap_pagecreate();
extern addr_t segmap_getmap();
extern int segmap_release();

#endif
#endif /* _KERNEL */

#endif	/* _VM_SEG_MAP_H */
