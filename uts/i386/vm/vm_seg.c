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

#ident	"@(#)kern-vm:vm_seg.c	1.3.1.3"

/*
 * VM - segment management.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/debug.h"
#include "sys/kmem.h"
#include "sys/inline.h"

#include "sys/immu.h"

#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_kmem.h"
#include "vm/mp.h"
#include "sys/vmsystm.h"

/*
 * Variables for maintaining the free list of segment structures.
 */
STATIC struct seg *seg_freelist;
STATIC int seg_freeincr = 32;

/*
 * Allocate a segment to cover [base, base+size)
 * and attach it to the specified address space.
 */
struct seg *
seg_alloc(as, base, size)
	struct as *as;
	register addr_t base;
	register u_int size;
{
	register struct seg *new;
	addr_t segbase;
	u_int segsize;

	segbase = (addr_t)((u_int)base & PAGEMASK);
	segsize =
	    (((u_int)(base + size) + PAGEOFFSET) & PAGEMASK) - (u_int)segbase;

	if (!valid_va_range(&segbase, &segsize, segsize, AH_LO))
		return ((struct seg *)NULL);	/* bad virtual addr range */

	if ( (as != &kas) && !valid_usr_range(segbase, segsize) ) 
		return ((struct seg *)NULL);	/* bad virtual addr range */

	new = (struct seg *)kmem_fast_alloc((caddr_t *)&seg_freelist,
	    sizeof (*seg_freelist), seg_freeincr, KM_SLEEP);
	struct_zero((caddr_t)new, sizeof (*new));
	if (seg_attach(as, segbase, segsize, new) < 0) {
		kmem_fast_free((caddr_t *)&seg_freelist, (char *)new);
		return ((struct seg *)NULL);
	}
	/* caller must fill in ops, data */
	return (new);
}

/*
 * Attach a segment to the address space.  Used by seg_alloc()
 * and for kernel startup to attach to static segments.
 */
int
seg_attach(as, base, size, seg)
	struct as *as;
	addr_t base;
	u_int size;
	struct seg *seg;
{

	seg->s_as = as;
	seg->s_base = base;
	seg->s_size = size;

	/*
	 * as_addseg() will add the segment at the appropraite point
	 * in the list. It will return -1 if there is overlap with
	 * an already existing segment.
	 */

	return (as_addseg(as, seg));
}

/*
 * Unmap a segment and free it from its associated address space.
 * This should be called by anybody who's finished with a whole segment's
 * mapping. Just calls s_ops->unmap() on the whole mapping . It is the
 * responsibility of the segment driver to unlink the the segment
 * from the address space, and to free public and private data structures
 * associated with the segment. (This is typically done by a call to 
 * seg_free()).
 */
void
seg_unmap(seg)
	register struct seg *seg;
{
	/* Shouldn't have called seg_unmap if mapping isn't yet established */
	ASSERT(seg->s_data != NULL);

	/* Unmap the whole mapping */
	(*seg->s_ops->unmap)(seg, seg->s_base, seg->s_size);
}

/*
 * Free the segment from its associated as. This should only be called
 * if a mapping to the segment has not yet been established (e.g., if
 * an error occurs in the middle of doing an as_map when the segment
 * has already been partially set up) or if it has already been deleted
 * (e.g., from a segment driver unmap routine if the unmap applies to the
 * entire segment). If the mapping is currently set up then seg_unmap() should
 * be called instead. 
 */
void
seg_free(seg)
	register struct seg *seg;
{
	register struct as *as = seg->s_as;

	if (as->a_segs == seg)
		as->a_segs = seg->s_next;		/* go to next seg */

	if (as->a_segs == seg)
		as->a_segs = NULL;			/* seg list is gone */
	else {
		seg->s_prev->s_next = seg->s_next;
		seg->s_next->s_prev = seg->s_prev;
	}

	if (as->a_seglast == seg)
		as->a_seglast = as->a_segs;

	/*
	 * If the segment private data field is NULL,
	 * then segment driver is not attached yet.
	 */
	if (seg->s_data != NULL)
		(*seg->s_ops->free)(seg);

	kmem_fast_free((caddr_t *)&seg_freelist, (caddr_t)seg);
}

#ifdef DEBUG

/*
 * Translate addr into page number within segment.
 */
u_int
seg_page(seg, addr)
	struct seg *seg;
	addr_t addr;
{

	return ((u_int)((addr - seg->s_base) >> PAGESHIFT));
}

/*
 * Return number of pages in segment.
 */
u_int
seg_pages(seg)
	struct seg *seg;
{

	return ((u_int)((seg->s_size + PAGEOFFSET) >> PAGESHIFT));
}
#endif
