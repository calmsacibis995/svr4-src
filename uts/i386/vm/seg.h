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

#ifndef _VM_SEG_H
#define _VM_SEG_H

#ident	"@(#)kern-vm:seg.h	1.3"

#include "vm/faultcode.h"
#include "vm/mp.h"

/*
 * VM - Segments.
 */

/*
 * An address space contains a set of segments, managed by drivers.
 * Drivers support mapped devices, sharing, copy-on-write, etc.
 *
 * The seg structure contains a lock to prevent races, the base virtual
 * address and size of the segment, a back pointer to the containing
 * address space, pointers to maintain a circularly doubly linked list
 * of segments in the same address space, and procedure and data hooks
 * for the driver.  The seg list on the address space is sorted by
 * ascending base addresses and overlapping segments are not allowed.
 *
 * After a segment is created, faults may occur on pages of the segment.
 * When a fault occurs, the fault handling code must get the desired
 * object and set up the hardware translation to the object.  For some
 * objects, the fault handling code also implements copy-on-write.
 *
 * When the hat wants to unload a translation, it can call the unload
 * routine which is responsible for processing reference and modify bits.
 */

/*
 * Fault information passed to the seg fault handling routine.
 * The F_SOFTLOCK and F_SOFTUNLOCK are used by software
 * to lock and unlock pages for physical I/O.
 */
enum fault_type {
	F_INVAL,		/* invalid page */
	F_PROT,			/* protection fault */
	F_SOFTLOCK,		/* software requested locking */
	F_SOFTUNLOCK		/* software requested unlocking */
};

/*
 * seg_rw gives the access type for a fault operation
 */
enum seg_rw {
	S_OTHER,		/* unknown or not touched */
	S_READ,			/* read access attempted */
	S_WRITE,		/* write access attempted */
	S_EXEC 			/* execution access attempted */
};

struct seg {
	mon_t	s_lock;
	addr_t	s_base;			/* base virtual address */
	u_int	s_size;			/* size in bytes */
	struct	as *s_as;		/* containing address space */
	struct	seg *s_next;		/* next seg in this address space */
	struct	seg *s_prev;		/* prev seg in this address space */
	struct	seg_ops {
#if defined(__STDC__)
		int (*dup)(struct seg *, struct seg *);
		int (*unmap)(struct seg *, addr_t, u_int);
		void (*free)(struct seg *);
		faultcode_t (*fault)(struct seg *, addr_t, u_int, 
			enum fault_type, enum seg_rw);
		faultcode_t (*faulta)(struct seg *, addr_t);
		void (*unload)(struct seg *, addr_t, u_int, u_int);
		int (*setprot)(struct seg *, addr_t, u_int, u_int);
		int (*checkprot)(struct seg *, addr_t, u_int, u_int);
		int (*kluster)(struct seg *, addr_t, int);
		u_int (*swapout)(struct seg *);
		int (*sync)(struct seg *, addr_t, u_int, int, u_int);
		int (*incore)(struct seg *, addr_t, u_int, char *);
		int (*lockop)(struct seg *, addr_t, u_int, int, int, 
			ulong *, u_int);
		int (*getprot)(struct seg *, addr_t, u_int, u_int *);
		off_t (*getoffset)(struct seg *, addr_t);
		int (*gettype)(struct seg *, addr_t);
		int (*getvp)(struct seg *, addr_t, struct vnode **);		
#else
		int	(*dup)();
		int	(*unmap)();
		void	(*free)();
		faultcode_t	(*fault)();
		faultcode_t	(*faulta)();
		void	(*unload)();
		int	(*setprot)();
		int	(*checkprot)();
		int	(*kluster)();
		u_int	(*swapout)();
		int	(*sync)();
		int	(*incore)();
		int	(*lockop)();
		int	(*getprot)();
		off_t	(*getoffset)();
		int	(*gettype)();
		int	(*getvp)();		
#endif
	} *s_ops;
	_VOID *s_data;			/* private data for instance */
};

#ifdef _KERNEL
/*
 * Generic segment operations
 */
struct	seg *seg_alloc(/* as, base, size */);
int	seg_attach(/* as, base, size, seg */);
void	seg_free(/* seg */);

#ifdef DEBUG

u_int	seg_page(/* seg, addr */);
u_int	seg_pages(/* seg */);

#else

#define seg_page(seg, addr) \
 	((u_int)(((addr) - (seg)->s_base) >> PAGESHIFT))

#define seg_pages(seg) \
	((u_int)(((seg)->s_size + PAGEOFFSET) >> PAGESHIFT))

#endif

#if defined(__STDC__)
extern void seg_unmap(struct seg *);
#else
extern void seg_unmap();
#endif	/* __STDC__ */

#endif /* _KERNEL */

#endif	/* _VM_SEG_H */
