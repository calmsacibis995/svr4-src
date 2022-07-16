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

#ifndef _VM_ANON_H
#define _VM_ANON_H

#ident	"@(#)kern-vm:anon.h	1.3"

/*
 * VM - Anonymous pages.
 */

/*
 * Each page which is anonymous, either in memory or in swap,
 * has an anon structure.  The structure's primary purpose is
 * to hold a reference count so that we can detect when the last
 * copy of a multiply-referenced copy-on-write page goes away.
 * When on the free list, un.next gives the next anon structure
 * in the list.  Otherwise, un.page is a ``hint'' which probably
 * points to the current page.  This must be explicitly checked
 * since the page can be moved underneath us.  This is simply
 * an optimization to avoid having to look up each page when
 * doing things like fork.
 */
struct anon {
	int	an_refcnt;
	union {
		struct	page *an_page;	/* ``hint'' to the real page */
		struct	anon *an_next;	/* free list pointer */
	} un;
	struct anon *an_bap;		/* pointer to real anon */
	short	an_flag;		/* see below */
	short	an_use;			/* for debugging code */
};

/* an_flag values */
#define ALOCKED	0x1
#define AWANT	0x2

#ifdef DEBUG
/* an_use values */
#define AN_NONE		0
#define AN_DATA		1
#define AN_UPAGE	2
#endif

struct anoninfo {
	u_int	ani_max;	/* maximum anon pages available */
	u_int	ani_free;	/* number of anon pages currently free */
	u_int	ani_resv;	/* number of anon pages reserved */
};

#ifdef _KERNEL
extern	struct anoninfo anoninfo;

struct	anon *anon_alloc();
void	anon_dup(/* old, new, size */);
void	anon_free(/* app, size */);
int	anon_getpage(/* app, protp, pl, sz, seg, addr, rw, cred */);
struct	page *anon_private(/* app, seg, addr, ppsteal */);
struct	page *anon_zero(/* seg, addr, app */);
struct	page *anon_zero_aligned(/* seg, addr, app, align_mask, align_val */);
void	anon_unloadmap(/* ap, ref, mod */);
int	anon_resv(/* size */);
void	anon_unresv(/* size */);

#define ALOCK(ap) { \
	while ((ap)->an_flag & ALOCKED) { \
		(ap)->an_flag |= AWANT; \
		(void) sleep((caddr_t)(ap), PINOD); \
	} \
	(ap)->an_flag |= ALOCKED; \
}

#define AUNLOCK(ap) { \
	ASSERT((ap)->an_flag & ALOCKED); \
	(ap)->an_flag &= ~ALOCKED; \
	if ((ap)->an_flag & AWANT) { \
		(ap)->an_flag &= ~AWANT; \
		wakeprocs((caddr_t)(ap), PRMPT); \
	} \
}

/* flags to anon_private */
#define STEAL_PAGE 0x1
#define LOCK_PAGE 0x2

#endif /* _KERNEL */

#endif	/* _VM_ANON_H */
