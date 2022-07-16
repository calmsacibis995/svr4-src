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

#ifndef _VM_SEG_VN_H
#define _VM_SEG_VN_H

#ident	"@(#)kern-vm:seg_vn.h	1.3"

#include "vm/mp.h"

/*
 * A pointer to this structure is passed to segvn_create().
 */
struct segvn_crargs {
	struct	vnode *vp;	/* vnode mapped from */
	u_int	offset;		/* starting offset of vnode for mapping */
	struct	cred *cred;	/* credentials */
	u_char	type;		/* type of sharing done */
	u_char	prot;		/* protections */
	u_char	maxprot;	/* maximum protections */
	struct	anon_map *amp;	/* anon mapping to map to */
};

/*
 * The anon_map structure is used by the seg_vn driver to manage
 * unnamed (anonymous) memory.   When anonymous memory is shared,
 * then the different segvn_data structures will point to the
 * same anon_map structure.  Also, if a segment is unmapped
 * in the middle where an anon_map structure exists, the
 * newly created segment will also share the anon_map structure,
 * although the two segments will use different ranges of the
 * anon array.  When mappings are private (or shared with
 * a reference count of 1), an unmap operation will free up
 * a range of anon slots in the array given by the anon_map
 * structure.  Because of fragmentation due to this unmapping,
 * we have to store the size of the anon array in the anon_map
 * structure so that we can free everything when the referernce
 * count goes to zero.
 */
struct anon_map {
	u_int	refcnt;		/* reference count on this structure */
	u_int	size;		/* size in bytes mapped by the anon array */
	struct	anon **anon;	/* pointer to an array of anon * pointers */
	u_int	swresv;		/* swap space reserved for this anon_map */
};

/*
 * (Semi) private data maintained by the seg_vn driver per segment mapping.
 */
struct	segvn_data {
	mon_t	lock;
	u_char	pageprot;	/* true if per page protections present */
	u_char	prot;		/* current segment prot if pageprot == 0 */
	u_char	maxprot;	/* maximum segment protections */
	u_char	type;		/* type of sharing done */
	struct	vnode *vp;	/* vnode that segment mapping is to */
	u_int	offset;		/* starting offset of vnode for mapping */
	u_int	anon_index;	/* starting index into anon_map anon array */
	struct	anon_map *amp;	/* pointer to anon share structure, if needed */
	struct	vpage *vpage;	/* per-page information, if needed */
	struct	cred *cred;	/* mapping credentials */
	u_int	swresv;		/* swap space reserved for this segment */
};

#ifdef _KERNEL

#if defined(__STDC__)
extern int segvn_create(struct seg *, void *);
#else
extern int segvn_create();
#endif

extern	struct seg_ops segvn_ops;

/*
 * Provided as shorthand for creating user zfod segments.
 */
extern	caddr_t zfod_argsp;
extern	caddr_t kzfod_argsp;
#endif /* _KERNEL */

#endif	/* _VM_SEG_VN_H */
