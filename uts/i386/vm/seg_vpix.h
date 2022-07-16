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

#ifndef _VM_SEG_VPIX_H
#define _VM_SEG_VPIX_H

#ident	"@(#)kern-vm:seg_vpix.h	1.3.1.1"

#include "vm/seg.h"
#include "vm/anon.h"
#include "vm/mp.h"

/*
 * Structure whose pointer is passed to the segvpix_create routine
 */
struct segvpix_crargs {
	u_int		n_hole;		/* # of "holes" - these are regions
					   in the segment which will never
					   have real memory associated with
					   them. */
	struct vpix_hole {
		caddr_t		base;	/* start addr of hole */
		u_int		size;	/* size (in bytes) of hole */
	} hole[4];
};

/*
 * Note: we don't need a shareable anon_map, because 1) seg_vpix segments
 * are always private, and 2) we don't allow seg_vpix segments to be
 * partially unmapped.  We don't need protection information since seg_vpix
 * pages are always read/write.
 */

/*
 * A seg_vpix segment supports the notion of "equivalenced" pages.  This
 * refers to multiple virtual pages in the same segment mapping to the
 * same physical (or anonymous) page.  We represent the equivalences by
 * keeping a circular linked list for each equivalence set.  One of the
 * pages in the equivalence set is (arbitrarily) considered the "primary";
 * This is kept,
 * along with some other per-page information, in a vpix_page structure.
 */

typedef struct vpix_page vpix_page_t;
struct vpix_page {	/* one for every virtual page */
	u_short		eq_map;		/* "real" page this is mapped to */
	u_short		eq_link;	/* link to next equivalenced vpage */
	u_short		rp_eq_list;	/* 1st vpage mapped to this rpage */
	u_short		rp_phys : 1;	/* this rpage is physmapped */
	u_short		rp_lock : 1;	/* we locked the physical page */
	u_short		rp_errata10 : 1;/* some vpage in 0x1000 - 0xF000 range,
					   and thus subject to B1 Errata 10 */
	u_short		rp_hole : 1;	/* this rpage has no real memory */
	union {				/* actual page assignment for rpage */
		struct anon *rpu_anon;		/* anonymous backing */
		u_int	     rpu_pfn;		/* physical map pfn */
	} rp_un;
};

#define rp_anon	rp_un.rpu_anon
#define rp_pfn	rp_un.rpu_pfn

#define NULLEQ	((u_short)-1)

/*
 * (Semi) private data maintained by the seg_vn driver per segment mapping
 */
struct	segvpix_data {
	mon_t	lock;
	short	s_vpix;		/* hook to the v86tab structure */
	vpix_page_t *vpage;	/* per-page information */
	struct cred *cred;	/* mapping credentials */
	u_int	swresv;		/* swap space reserved for this segment */
};

#ifdef _KERNEL

#if defined(__STDC__)
extern int segvpix_create(struct seg *, void *);
extern int segvpix_physmap(struct seg *, u_int, u_int, u_int);
extern int segvpix_unphys(struct seg *, u_int, u_int);
extern int segvpix_page_equiv(struct seg *, u_int, u_int);
extern int segvpix_range_equiv(struct seg *, u_int, u_int, u_int);
extern int segvpix_modscan(struct seg *, u_int, u_int);
#else
extern int segvpix_create();
extern int segvpix_physmap();
extern int segvpix_unphys();
extern int segvpix_page_equiv();
extern int segvpix_range_equiv();
extern int segvpix_modscan();
#endif

extern	struct seg_ops segvpix_ops;

/*
 * Provided as shorthand for creating a vanilla vpix segment.
 */
extern caddr_t vpix_argsp;

#endif /* _KERNEL */

#endif	/* _VM_SEG_VPIX_H */
