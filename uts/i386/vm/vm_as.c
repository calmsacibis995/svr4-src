/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *		PROPRIETARY NOTICE (Combined)
 *
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 *
 *
 *
 *		Copyright Notice
 *
 * Notice of copyright on this source code product does not indicate
 * publication.
 *
 *	(c) 1986,1987,1988,1989	 Sun Microsystems, Inc
 *	(c) 1983,1984,1985,1986,1987,1988,1989	AT&T.
 *		  All rights reserved.
 *
 */

#ident  "@(#)kern-vm:vm_as.c	1.3.1.4"

/*
 * VM - address spaces.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/systm.h"
#include "sys/mman.h"
#include "sys/sysmacros.h"
#include "sys/debug.h"
#include "sys/sysinfo.h"
#include "sys/kmem.h"
#include "sys/inline.h"
#include "sys/user.h"
#include "sys/cmn_err.h"
#include "sys/uio.h"
#include "sys/vnode.h"
#include "sys/swap.h"

#include "sys/immu.h"
#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_vn.h"
#include "vm/page.h"
#include "vm/seg_kmem.h"

extern int	valid_va_range();

/*
 * Variables for maintaining the free list of address space structures.
 */
STATIC struct as *as_freelist;
STATIC int as_freeincr = 8;

/*
 * Find a segment containing addr.  as->a_seglast is used as a
 * cache to remember the last segment hit we had here.  We
 * start looking there and go circularly around the seglist.
 */
struct seg *
as_segat(as, addr)
	register struct as *as;
	register addr_t addr;
{
	register struct seg *seg, *sseg;

	if (as->a_seglast == NULL)
		as->a_seglast = as->a_segs;

	sseg = seg = as->a_seglast;
	if (seg != NULL) {
		do {
			if (seg->s_base <= addr &&
			    addr < (seg->s_base + seg->s_size)) {
				as->a_seglast = seg;
				return (seg);
			}
			seg = seg->s_next;
		} while (seg != sseg);
	}
	return (NULL);
}

/*
 * Allocate and initialize an address space data structure.
 * We call hat_alloc to allow any machine dependent
 * information in the hat structure to be initialized.
 */
struct as *
as_alloc()
{
	register struct as *as;

	as = (struct as *)kmem_fast_alloc((caddr_t *)&as_freelist,
	    sizeof (*as_freelist), as_freeincr, KM_SLEEP);
	struct_zero((caddr_t)as, sizeof (*as));
	hat_alloc(as);
	return (as);
}

/*
 * Free an address space data structure.
 * Need to free the hat first and then
 * all the segments on this as and finally
 * the space for the as struct itself.
 */
void
as_free(as)
	register struct as *as;
{

	hat_free(as);
	while (as->a_segs != NULL)
		seg_unmap(as->a_segs);
	kmem_fast_free((caddr_t *)&as_freelist, (caddr_t)as);
}

struct as *
as_dup(as)
	register struct as *as;
{
	register struct as *newas;
	register struct seg *seg, *sseg, *newseg;

	newas = as_alloc();
	sseg = seg = as->a_segs;

	if (seg != NULL) {
		do {
			newseg = seg_alloc(newas, seg->s_base, seg->s_size);
			if (newseg == NULL) {
				as_free(newas);
				return (NULL);
			}
			if ((*seg->s_ops->dup)(seg, newseg) != 0) {
				/*
				 * We call seg_free() on the new seg
				 * because the segment is not set up
				 * completely; i.e. it has no ops.
				 */
				seg_free(newseg);
				as_free(newas);
				return (NULL);
			}
			newas->a_size += seg->s_size;	
			seg = seg->s_next;
		} while (seg != sseg);
	}

	if (hat_dup(as, newas) != 0) {
		as_free(newas);
		return(NULL);
	}

	return (newas);
}

/*
 * Add a new segment to the address space, sorting
 * it into the proper place in the linked list.
 */
int
as_addseg(as, newseg)
	register struct as *as;
	register struct seg *newseg;
{
	register struct seg *seg;
	register addr_t base;
	register addr_t eaddr;

	seg = as->a_segs;
	if (seg == NULL) {
		newseg->s_next = newseg->s_prev = newseg;
		as->a_segs = newseg;
	} else {
		/*
		 * Figure out where to add the segment to keep list sorted
		 */
		base = newseg->s_base;
		eaddr = base + newseg->s_size;
		do {
			if (base < seg->s_base) {
				if (eaddr > seg->s_base)
					return (-1);
				break;
			}
			if (base < seg->s_base + seg->s_size)
				return (-1);
			seg = seg->s_next;
		} while (seg != as->a_segs);

		newseg->s_next = seg;
		newseg->s_prev = seg->s_prev;
		seg->s_prev = newseg;
		newseg->s_prev->s_next = newseg;

		if (base < as->a_segs->s_base)
			as->a_segs = newseg;		/* newseg is at front */
	}

	return (0);
}

/*
 * Handle a ``fault'' at addr for size bytes.
 */
faultcode_t
as_fault(as, addr, size, type, rw)
	struct as *as;
	addr_t addr;
	u_int size;
	enum fault_type type;
	enum seg_rw rw;
{
	register struct seg *seg;
	register addr_t raddr;			/* rounded down addr */
	register u_int rsize;			/* rounded up size */
	register u_int ssize;
	register faultcode_t res = 0;
	addr_t addrsav;
	struct seg *segsav;

	switch (type) {

	case F_SOFTLOCK:
		vminfo.v_sftlock++;
		break;

	case F_PROT:
		vminfo.v_pfault++;
		break;

	case F_INVAL:
		vminfo.v_vfault++;
		break;
	}

	raddr = (addr_t)((u_int)addr & PAGEMASK);
	rsize = (((u_int)(addr + size) + PAGEOFFSET) & PAGEMASK) - (u_int)raddr;

	seg = as_segat(as, raddr);
	if (seg == NULL)
		return (FC_NOMAP);

	addrsav = raddr;
	segsav = seg;

	do {
		if (raddr >= seg->s_base + seg->s_size) {
			seg = seg->s_next;	/* goto next seg */
			if (raddr != seg->s_base) {
				res = FC_NOMAP;
				break;
			}
		}
		if (raddr + rsize > seg->s_base + seg->s_size)
			ssize = seg->s_base + seg->s_size - raddr;
		else
			ssize = rsize;

		res = (*seg->s_ops->fault)(seg, raddr, ssize, type, rw);
		if (res != 0)
			break;

		raddr += ssize;
		rsize -= ssize;
	} while (rsize != 0);

	/*
	 * If were SOFTLOCKing and we encountered a failure,
	 * we must SOFTUNLOCK the range we already did.
	 */
	if (res != 0 && type == F_SOFTLOCK) {
		for (seg = segsav; addrsav < raddr; addrsav += ssize) {
			if (addrsav >= seg->s_base + seg->s_size)
				seg = seg->s_next;	/* goto next seg */
			/*
			 * Now call the fault routine again to perform the
			 * unlock using S_OTHER instead of the rw variable
			 * since we never got a chance to touch the pages.
			 */
			if (raddr > seg->s_base + seg->s_size)
				ssize = seg->s_base + seg->s_size - addrsav;
			else
				ssize = raddr - addrsav;
			(void) (*seg->s_ops->fault)(seg, addrsav, ssize,
			    F_SOFTUNLOCK, S_OTHER);
		}
	}

	return (res);
}

/*
 * Asynchronous ``fault'' at addr for size bytes.
 */
faultcode_t
as_faulta(as, addr, size)
	struct as *as;
	addr_t addr;
	u_int size;
{
	register struct seg *seg;
	register addr_t raddr;			/* rounded down addr */
	register u_int rsize;			/* rounded up size */
	register faultcode_t res;

	raddr = (addr_t)((u_int)addr & PAGEMASK);
	rsize = (((u_int)(addr + size) + PAGEOFFSET) & PAGEMASK) - (u_int)raddr;

	seg = as_segat(as, raddr);
	if (seg == NULL)
		return (FC_NOMAP);

	do {
		if (raddr >= seg->s_base + seg->s_size) {
			seg = seg->s_next;	/* goto next seg */
			if (raddr != seg->s_base)
				return (FC_NOMAP);
		}

		res = (*seg->s_ops->faulta)(seg, raddr);
		if (res != 0)
			return (res);

		raddr += PAGESIZE;
		rsize -= PAGESIZE;
	} while (rsize != 0);

	return (0);
}

/*
 * Set the virtual mapping for the interval from [addr : addr + size)
 * in address space `as' to have the specified protection.
 * It is ok for the range to cross over several segments,
 * as long as they are contiguous.
 */

int
as_setprot(as, addr, size, prot)
	struct as *as;
	addr_t addr;
	u_int size;
	u_int prot;
{
	register struct seg *seg;
	register u_int ssize;
	register addr_t raddr;			/* rounded down addr */
	register u_int rsize;			/* rounded up size */
	int error = 0;

	raddr = (addr_t)((u_int)addr & PAGEMASK);
	rsize = (((u_int)(addr + size) + PAGEOFFSET) & PAGEMASK) - (u_int)raddr;

	seg = as_segat(as, raddr);
	if (seg == NULL)
		return (ENOMEM);

	do {
		if (raddr >= seg->s_base + seg->s_size) {
			seg = seg->s_next;	/* goto next seg */
			if (raddr != seg->s_base)
				return (ENOMEM);
		}
		if ((raddr + rsize) > (seg->s_base + seg->s_size))
			ssize = seg->s_base + seg->s_size - raddr;
		else
			ssize = rsize;

		error =  (*seg->s_ops->setprot)(seg, raddr, ssize, prot);
		if (error != 0)
			return (error);

		raddr += ssize;
		rsize -= ssize;
	} while (rsize != 0);

	return (0);
}

/*
 * Check to make sure that the interval from [addr : addr + size)
 * in address space `as' has at least the specified protection.
 * It is ok for the range to cross over several segments, as long
 * as they are contiguous.
 */
int
as_checkprot(as, addr, size, prot)
	struct as *as;
	addr_t addr;
	u_int size;
	u_int prot;
{
	register struct seg *seg;
	register u_int ssize;
	register addr_t raddr;			/* rounded down addr */
	register u_int rsize;			/* rounded up size */
	int error;

	raddr = (addr_t)((u_int)addr & PAGEMASK);
	rsize = (((u_int)(addr + size) + PAGEOFFSET) & PAGEMASK) - (u_int)raddr;

	seg = as_segat(as, raddr);
	if (seg == NULL)
		return (ENOMEM);

	do {
		if (raddr >= seg->s_base + seg->s_size) {
			seg = seg->s_next;	/* goto next seg */
			if (raddr != seg->s_base)
				return (ENOMEM);
		}
		if ((raddr + rsize) > (seg->s_base + seg->s_size))
			ssize = seg->s_base + seg->s_size - raddr;
		else
			ssize = rsize;

		error = (*seg->s_ops->checkprot)(seg, raddr, ssize, prot);
		if (error != 0)
			return (error);

		rsize -= ssize;
		raddr += ssize;
	} while (rsize != 0);

	return (0);
}

int
as_unmap(as, addr, size)
	register struct as *as;
	addr_t addr;
	u_int size;
{
	register struct seg *seg, *seg_next;
	register addr_t raddr, eaddr;
	register u_int ssize;
	addr_t obase;

	raddr = (addr_t)((u_int)addr & PAGEMASK);
	eaddr = (addr_t)(((u_int)(addr + size) + PAGEOFFSET) & PAGEMASK);

	seg_next = as->a_segs;
	if (seg_next != NULL) {
		do {
			/*
			 * Save next segment pointer since seg can be
			 * destroyed during the segment unmap operation.
			 * We also have to save the old base.
			 */
			seg = seg_next;
			seg_next = seg->s_next;
			obase = seg->s_base;

			if (raddr >= seg->s_base + seg->s_size)
				continue;		/* not there yet */

			if (eaddr <= seg->s_base)
				break;			/* all done */

			if (raddr < seg->s_base)
				raddr = seg->s_base;	/* skip to seg start */

			if (eaddr > (seg->s_base + seg->s_size))
				ssize = seg->s_base + seg->s_size - raddr;
			else
				ssize = eaddr - raddr;

			if ((*seg->s_ops->unmap)(seg, raddr, ssize) != 0)
				return (-1);

			as->a_size -= ssize;
			raddr += ssize;

			/*
			 * Check to see if we have looked at all the segs.
			 *
			 * We check a_segs because the unmaps above could
			 * have unmapped the last segment.
			 */
		} while (as->a_segs != NULL && obase < seg_next->s_base);
	}

	return (0);
}

int
as_map(as, addr, size, crfp, argsp)
	struct as *as;
	addr_t addr;
	u_int size;
	int (*crfp)();
	caddr_t argsp;
{
	register struct seg *seg;
	register addr_t raddr;			/* rounded down addr */
	register u_int rsize;			/* rounded up size */
	int error;

	raddr = (addr_t)((u_int)addr & PAGEMASK);
	rsize = (((u_int)(addr + size) + PAGEOFFSET) & PAGEMASK) - (u_int)raddr;

	if (as->a_size + rsize > u.u_rlimit[RLIMIT_VMEM].rlim_cur)
		return (ENOMEM);

	seg = seg_alloc(as, addr, size);
	if (seg == NULL)
		return (ENOMEM);

	/*
	 * Remember that this was the most recently touched segment.
	 * If the create routine merges this segment into an existing
	 * segment, seg_free will adjust the a_seglast hint.
	 */
	as->a_seglast = seg;

	error = (*crfp)(seg, argsp);
	if (error != 0) {
		seg_free(seg);
	} else {
		/*
		 * add size now so as_unmap will work if as_ctl fails
		 */
		as->a_size += rsize;

		if (as->a_paglck) {
			error = as_ctl(as, addr, size, MC_LOCK, 0,
				(caddr_t)NULL, (ulong *)NULL, (size_t)NULL);
			if (error != 0)
				(void) as_unmap(as, addr, size);
		}
	}

	return (error);
}

/*
 * Find a hole of at least size minlen within [base, base+len).
 * If flags specifies AH_HI, the hole will have the highest possible address
 * in the range. Otherwise, it will have the lowest possible address.
 * If flags specifies AH_CONTAIN, the hole will contain the address addr.
 * If an adequate hole is found, base and len are set to reflect the part of
 * the hole that is within range, and 0 is returned. Otherwise,
 * -1 is returned.
 * XXX This routine is not correct when base+len overflows addr_t.
 */
/* VARARGS5 */
int
as_gap(as, minlen, basep, lenp, flags, addr)
	struct as *as;
	register u_int minlen;
	addr_t *basep;
	u_int *lenp;
	int flags;
	addr_t addr;
{
	register addr_t lobound, hibound;
	register addr_t lo, hi;
	register struct seg *seg, *sseg;

	lobound = *basep;
	hibound = lobound + *lenp;
	if (lobound > hibound)		/* overflow */ 
		return (-1);

	sseg = seg = as->a_segs;
	if (seg == NULL) {
		if (valid_va_range(basep, lenp, minlen, flags & AH_DIR))
			return (0);
		else
			return (-1);
	}

	if ((flags & AH_DIR) == AH_LO) {	/* search from lo to hi */
		lo = lobound;
		do {
			hi = seg->s_base;
			if (hi > lobound && hi > lo) {
				*basep = MAX(lo, lobound);
				*lenp = MIN(hi, hibound) - *basep;
				if (valid_va_range(basep,lenp,minlen,AH_LO) &&
				    ((flags & AH_CONTAIN) == 0 ||
				    (*basep <= addr && *basep + *lenp > addr)))
					return (0);
			}
			lo = seg->s_base + seg->s_size;
		} while (lo < hibound && (seg = seg->s_next) != sseg);

		if (hi < lo) 
			hi = hibound;

		/* check against upper bound */
		if (lo < hibound) {
			*basep = MAX(lo, lobound);
			*lenp = MIN(hi, hibound) - *basep;
			if (valid_va_range(basep, lenp, minlen, AH_LO) &&
			    ((flags & AH_CONTAIN) == 0 ||
			    (*basep <= addr && *basep + *lenp > addr)))
				return (0);
		}
	} else {				/* search from hi to lo */
		seg = seg->s_prev;
		hi = hibound;
		do {
			lo = seg->s_base + seg->s_size;
			if (lo < hibound && hi > lo) {
				*basep = MAX(lo, lobound);
				*lenp = MIN(hi, hibound) - *basep;
				if (valid_va_range(basep,lenp,minlen,AH_HI) &&
				    ((flags & AH_CONTAIN) == 0 ||
				    (*basep <= addr && *basep + *lenp > addr)))
					return (0);
			}
			hi = seg->s_base;
		} while (hi > lobound && (seg = seg->s_prev) != sseg);

		if (lo > hi)
			lo = lobound;

		/* check against lower bound */
		if (hi > lobound) {
			*basep = MAX(lo, lobound);
			*lenp = MIN(hi, hibound) - *basep;
			if (valid_va_range(basep, lenp, minlen, AH_HI) &&
			    ((flags & AH_CONTAIN) == 0 ||
			    (*basep <= addr && *basep + *lenp > addr)))
				return (0);
		}
	}
	return (-1);
}

/*
 * Return the next range within [base, base+len) that is backed
 * with "real memory".  Skip holes and non-seg_vn segments.
 * We're lazy and only return one segment at a time.
 */
int
as_memory(as, basep, lenp)
	struct as *as;
	addr_t *basep;
	u_int *lenp;
{
	register struct seg *seg, *sseg;
	register addr_t addr, eaddr;
	addr_t segend;
	struct seg *cseg = NULL;

	/* XXX - really want as_segatorabove? */
	if (as->a_seglast == NULL)
		as->a_seglast = as->a_segs;

	addr = *basep;
	eaddr = addr + *lenp;

	sseg = seg = as->a_seglast;
	if (seg == NULL)
		return(EINVAL);

	do {
		if (seg->s_ops != &segvn_ops)
			continue;
		if (seg->s_base <= addr &&
		    addr < (segend = (seg->s_base + seg->s_size))) {
			/* found a containing segment */
			as->a_seglast = seg;
			*basep = addr;
			if (segend > eaddr)
				*lenp = eaddr - addr;
			else
				*lenp = segend - addr;
			return (0);
		} else if (seg->s_base > addr) {
			if (cseg == NULL || cseg->s_base > seg->s_base) {
				/*
				 * Save closest seg above the range.
				 * We have to keep scanning because
				 * we started with the hint instead
				 * of the beginning of the list.
				 */
				cseg = seg;
			}
		}
	} while ((seg = seg->s_next) != sseg);

	if (cseg == NULL)		/* no valid segs within range */
		return (EINVAL);

	/*
	 * Only found a close segment, see if there's
	 * a valid range we can return.
	 */
	if (cseg->s_base >= eaddr)	/* closest segment is out of range */
		return (ENOMEM);

	as->a_seglast = cseg;		/* reset hint */

	*basep = cseg->s_base;
	if (cseg->s_base + cseg->s_size > eaddr)
		*lenp = eaddr - cseg->s_base;	/* segment contains eaddr */
	else
		*lenp = cseg->s_size;	/* seg is between addr and eaddr */
	return (0);
}

/*
 * Swap the pages associated with the address space as out to
 * secondary storage, returning the number of bytes actually
 * swapped.
 *
 * The value returned is intended to correlate well with the process's
 * memory requirements.  Its usefulness for this purpose depends on
 * how well the segment-level routines do at returning accurate
 * information.
 */
u_int
as_swapout(as)
	register struct as *as;
{
	register struct seg *seg, *sseg;
	register u_int swpcnt = 0;

	/*
	 * Kernel-only processes have given up their address
	 * spaces.  Of course, we shouldn't be attempting to
	 * swap out such processes in the first place...
	 */
	if (as == NULL)
		return (0);

	/*
	 * Free all mapping resources associated with the address
	 * space.  The segment-level swapout routines capitalize
	 * on this unmapping by scavanging pages that have become
	 * unmapped here.
	 */
	hat_swapout(as);

	/*
	 * Call the swapout routines of all segments in the address
	 * space to do the actual work, accumulating the amount of
	 * space reclaimed.
	 */
	sseg = seg = as->a_segs;
	if (seg != NULL) {
		do {
			register struct seg_ops *ov = seg->s_ops;

			/*
			 * We have to check to see if the seg has
			 * an ops vector because the seg may have
			 * been in the middle of being set up when
			 * the process was picked for swapout.
			 */
			if ((ov != NULL) && (ov->swapout != NULL))
				swpcnt += (*ov->swapout)(seg);
		} while ((seg = seg->s_next) != sseg);
	}

	return (swpcnt);
}

/*
 * Determine whether data from the mappings in interval [addr : addr + size)
 * are in the primary memory (core) cache.
 */
int
as_incore(as, addr, size, vec, sizep)
	struct as *as;
	addr_t addr;
	u_int size;
	char *vec;
	u_int *sizep;
{
	register struct seg *seg;
	register u_int ssize;
	register addr_t raddr;		/* rounded down addr */
	register u_int rsize;		/* rounded up size */
	u_int isize;			/* iteration size */

	*sizep = 0;
	raddr = (addr_t)((u_int)addr & PAGEMASK);
	rsize = ((((u_int)addr + size) + PAGEOFFSET) & PAGEMASK) - (u_int)raddr;
	seg = as_segat(as, raddr);
	if (seg == NULL)
		return (-1);
	for (; rsize != 0; rsize -= ssize, raddr += ssize) {
		if (raddr >= seg->s_base + seg->s_size) {
			seg = seg->s_next;
			if (raddr != seg->s_base)
				return (-1);
		}
		if ((raddr + rsize) > (seg->s_base + seg->s_size))
			ssize = seg->s_base + seg->s_size - raddr;
		else
			ssize = rsize;
		*sizep += isize =
		    (*seg->s_ops->incore)(seg, raddr, ssize, vec);
		if (isize != ssize)
			return (-1);
		vec += btoc(ssize);
	}
	return (0);
}

/*
 * Cache control operations over the interval [addr : addr + size) in 
 * address space "as".
 */
int
as_ctl(as, addr, size, func, attr, arg, lock_map, pos)
	struct as *as;
	addr_t addr;
	u_int size;
	int func;
	int attr;
	caddr_t arg;
	ulong *lock_map;
	size_t pos;
{
	register struct seg *seg;	/* working segment */
	register struct seg *sseg;	/* first segment of address space */
	register addr_t raddr;		/* rounded down addr */
	register u_int rsize;		/* rounded up size */
	register u_int ssize;		/* size of seg */
	int error;			/* result */

	/*
	 * If these are address space lock/unlock operations, loop over
	 * all segments in the address space, as appropriate.
	 */
	if (func == MC_LOCKAS) {
		if ((int)arg & MCL_FUTURE)
			as->a_paglck = 1;
		if (((int)arg & MCL_CURRENT) == 0)
			return (0);

		sseg = seg = as->a_segs;
		if (seg == NULL)
			return(0);
		do {
			error = (*seg->s_ops->lockop)(seg, seg->s_base,
				seg->s_size, attr, MC_LOCK, lock_map, pos);
			if (error != 0)
				return (error);
			pos += seg_pages(seg);
		} while((seg = seg->s_next) != sseg);

		return (0);
	} else if (func == MC_UNLOCKAS) {
		as->a_paglck = 0;

		sseg = seg = as->a_segs;
		if (seg == NULL)
			return(0);
		do {
			error = (*seg->s_ops->lockop)(seg, seg->s_base,
				seg->s_size, attr, MC_UNLOCK, NULL, 0);
			if (error != 0)
				return (error);
		} while((seg = seg->s_next) != sseg);

		return (0);
	}

	/*
	 * Normalize addresses and sizes.
	 */
	raddr = (addr_t)((u_int)addr & PAGEMASK);
	rsize = (((u_int)(addr + size) + PAGEOFFSET) & PAGEMASK) - (u_int)raddr;

	/*
	 * Get initial segment.
	 */
	if ((seg = as_segat(as, raddr)) == NULL)
		return (ENOMEM);

	/*
	 * Loop over all segments.  If a hole in the address range is
	 * discovered, then fail.  For each segment, perform the appropriate
	 * control operation.
	 */

	while (rsize != 0) {

		/*
		 * Make sure there's no hole, calculate the portion
		 * of the next segment to be operated over.
		 */
		if (raddr >= seg->s_base + seg->s_size) {
			seg = seg->s_next;
			if (raddr != seg->s_base) 
				return (ENOMEM);
		}
		if ((raddr + rsize) > (seg->s_base + seg->s_size))
			ssize = seg->s_base + seg->s_size - raddr;
		else
			ssize = rsize;

		/*
		 * Dispatch on specific function.
		 */
		switch (func) {

		/*
		 * Synchronize cached data from mappings with backing
		 * objects.
		 */
		case MC_SYNC:
			if (error = (*seg->s_ops->sync)
			    (seg, raddr, ssize, attr, (u_int)arg))
				return (error);
			break;

		/*
		 * Lock pages in memory.
		 */
		case MC_LOCK:
			if (error = (*seg->s_ops->lockop)(seg, raddr, ssize, attr, func, lock_map, pos))
				return (error);
			break;

		/*
		 * Unlock mapped pages.
		 */
		case MC_UNLOCK:
			(void) (*seg->s_ops->lockop)(seg, raddr, ssize, attr, func, (ulong *)NULL, (size_t)NULL);
			break;

		/*
		 * Can't happen.
		 */
		default:
			cmn_err(CE_PANIC, "as_ctl: bad operation %d", func);
			/* NOTREACHED */
		}

		rsize -= ssize;
		raddr += ssize;
	}
	return (0);
}

u_int
as_getprot(as, addr, naddr)
	struct as *as;
	register caddr_t addr;
	caddr_t *naddr;
{
	register struct seg *seg;
 	caddr_t eaddr;
	u_int prot;
	u_int nprot;

	seg = as_segat(as, addr);

        eaddr = seg->s_base + seg->s_size;
	(*seg->s_ops->getprot)(seg, addr, 0, &prot);
	while ((addr += PAGESIZE) < eaddr) {
		(*seg->s_ops->getprot)(seg, addr, 0, &nprot);
		if (nprot != prot)
                        break;
	}
	*naddr = addr;
	return prot;
}

/* Special code for exec to move the stack segment from its interim
 * place in the old address to the right place in the new address space.
 */

int
as_exec(oas, ostka, stksz, nas, nstka, hatflag)
 struct as *oas;
 addr_t ostka;
 int stksz;
 struct as *nas;
 addr_t nstka;
 u_int hatflag;
{
	struct seg *stkseg;

	stkseg = as_segat(oas, ostka);
	ASSERT(stkseg != NULL);
	ASSERT(stkseg->s_base == ostka && stkseg->s_size == stksz);
	if (oas->a_segs == stkseg)
		oas->a_segs = stkseg->s_next;
	if (oas->a_segs == stkseg)
		oas->a_segs = NULL;
	else {
		stkseg->s_prev->s_next = stkseg->s_next;
		stkseg->s_next->s_prev = stkseg->s_prev;
	}
	if (oas->a_seglast == stkseg)
		oas->a_seglast = oas->a_segs;
	stkseg->s_as = nas;
	stkseg->s_base = nstka;
	nas->a_size += stkseg->s_size;
	oas->a_size -= stkseg->s_size;
	(void) as_addseg(nas, stkseg);
	return(hat_exec(oas, ostka, stksz, nas, nstka, hatflag));
}

/*
 * We are going to write iosize bytes from this uio
 * to the vnode vp at offset uiop->uio_offset.  If 
 * any whole pages are being written, if we are
 * writing from the beginning of any page past old EOF,
 * or if we are writing from the beginning of the page containing
 * the old EOF and are writing at least through the EOF,
 * then we normally will just create these pages
 * when we call segmap_pagelist_create.
 * There are two problems to worry about:
 * the A-A problem and the A-B deadlock.
 *
 * The A-B deadlock occurs when the from addresses contain
 * pages from another (non-SWAP) vnode.
 * The pages created by segmap_pagelist_create()
 * have p_pagein set which is both an I/O and an use lock.
 * If the the uiomove has to fault in a "from" page from secondary
 * media, that corresponds to a second p_pagein lock.
 * Another process can be trying to do these pages with
 * the "from/to" roles reversed, reversing the locking order
 * and precipitating a deadlock.
 *
 * The A-A problem
 * occurs when the from address(es) are also
 * mapped to these same pages.
 * It can manifest as either a deadlock or a content issue.
 *
 * as_iolock() provide a solution to both problems.
 * It relies on the fact that segmap_pagelist_create()
 * uses existing pages in the page pool.
 * It goes thru all the pages of the from addresses to see if
 * they are the non-SWAP vnode pages.
 * If so, it faults them in and keeps them, so that 
 * segmap_pagelist_creat will find and use them.
 * This also solves the A-B problem by forcing the p_pagein
 * locks to go single-file for the "from" and "to" parts.
 */
int
as_iolock(uiop, pl, iosize, to_vp, to_filesize, pagecreate_p)
	register struct uio *uiop;
	page_t **pl;
	struct vnode *to_vp;
	u_int iosize;
	off_t to_filesize;
	int *pagecreate_p;
{
	register u_int n, sz;
	page_t **ppp;
	register addr_t addr, raddr, eaddr;
	register off_t foff;
	register struct iovec *iov;
	struct as *as;
	struct seg *seg, *oseg;
	struct vnode *svp;
	int error;
	int cpsz;

	*pagecreate_p = 0;
	*(ppp = pl) = NULL;

	if (uiop->uio_segflg == UIO_SYSSPACE)
		return iosize;
	as = u.u_procp->p_as;
	ASSERT(as != NULL);

	if ((uiop->uio_offset & PAGEOFFSET) != 0 || IS_SWAPVP(to_vp))
		return iosize;

	while ((iov = uiop->uio_iov)->iov_len == 0) {
		uiop->uio_iov++;
		uiop->uio_iovcnt--;
		ASSERT(uiop->uio_iovcnt > 0);
	}
	if ((n = iosize) > iov->iov_len)
		n = iov->iov_len;
	if (uiop->uio_offset + n < to_filesize) {
		if ((n &= PAGEMASK) == 0)
			return iosize;
	}

	addr = iov->iov_base;
	raddr = (addr_t)((u_int)addr & PAGEMASK);
	eaddr = addr + n;
	cpsz = PAGESIZE - ((u_int)addr & PAGEOFFSET);

	sz = 0;

	/*
	 * Set oseg to avoid search for the simple
	 * single segment case.  Within a segment,
	 * the vnode offsets always ascend.
	 * But when multiple segments become involved,
	 * A-B deadlocks can occur if offsets for a vnode
	 * occur in reverse order.  These deadlocks are
	 * with respect to p_keepcnt, which isn't strict locking,
	 * but some places, such as pvn_getdirty do wait for
	 * p_keepcnt.
	 */
	oseg = seg = as_segat(as, raddr);

	for (; raddr < eaddr; sz += cpsz, cpsz = PAGESIZE, raddr += PAGESIZE) {

		if (raddr + PAGESIZE > eaddr)
			cpsz -= raddr + PAGESIZE - eaddr;
		if (seg == NULL && (seg = as_segat(as, raddr)) == NULL)
			continue;
		if (raddr >= seg->s_base + seg->s_size) {
			if ((seg = as_segat(as, raddr)) == NULL)
				continue;
		}

		/*
		 * Any (from) page that is not anon
		 * will have to be read in.
		 */
		if (seg->s_ops != &segvn_ops)
			continue;
		if (segvn_isanon(seg, raddr))
			continue;

		(void) (*seg->s_ops->getvp) (seg, raddr, &svp);
		foff = (*seg->s_ops->getoffset) (seg, raddr);
		if (seg != oseg && ppp != pl) {
			/*
			 * It is the multisegment case,
			 * so we have to check for offset reversal
			 * or multiple vnodes.
			 */
			if (svp != ppp[-1]->p_vnode ||
			    foff < ppp[-1]->p_offset)
					break;
			if (foff == ppp[-1]->p_offset)
				continue;
		}

		error = VOP_GETPAGE(svp, foff, PAGESIZE, NULL, ppp,
			PAGESIZE, seg, raddr, S_READ, 
			(struct cred *)NULL);
		if (error)
			goto err;
		ppp++;
	}

	if (sz < n) {
		*pagecreate_p = ((sz & PAGEOFFSET) == 0 ||
				 uiop->uio_offset + sz >= to_filesize);
	} else
		*pagecreate_p = 1;
	*ppp = NULL;
	return sz;

err:
	*ppp = NULL;
	for (ppp = pl; *ppp; ppp++)
		PAGE_RELE(*ppp);
	pl[0] = NULL;
	return 0;
}
