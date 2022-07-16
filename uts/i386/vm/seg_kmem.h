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

#ifndef _VM_SEG_KMEM_H
#define _VM_SEG_KMEM_H

#ident	"@(#)kern-vm:seg_kmem.h	1.3"

/*
 * VM - Kernel Segment Driver
 */

/*
 * These variables should be put in a place which
 * is guaranteed not to get paged out of memory.
 */
extern struct as kas;			/* kernel's address space */
extern struct seg kpseg;		/* kernel's "ptov" segment */
extern struct seg kvseg;		/* kernel's "sptalloc" segment */
extern struct seg ktextseg;		/* kernel's "most everything else" segment */
extern struct seg kpioseg;		/* kernel's "pioseg" segment */

#if defined(__STDC__)

extern int sptalloc(int, int, caddr_t, int);
extern void sptfree(caddr_t, int, int);

/*
 * For segkmem_create, the argsp is actually a pointer to the
 * optional array of pte's used to map the given segment.
 */
extern int segkmem_create(struct seg *, void *);

extern caddr_t kseg(int);
extern void unkseg(caddr_t);

#else

extern int sptalloc();
extern void sptfree();
extern int segkmem_create();
extern caddr_t kseg();
extern void unkseg();

#endif /* __STDC__ */

/*
 * Flags to pass to segkmem_mapin().
 */
#define PTELD_LOCK	0x01
#define PTELD_INTREP	0x02
#define PTELD_NOSYNC	0x04

typedef struct kseg_list {
caddr_t	kseg_vaddr;			/* virtual address of kseg */
int kseg_size;				/* size of kseg request in clicks */
struct kseg_list *kseg_next;			/* link */
} kseg_list_t;
#endif	/* _VM_SEG_KMEM_H */
