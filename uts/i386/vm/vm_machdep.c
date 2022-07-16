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

#ident	"@(#)kern-vm:vm_machdep.c	1.3"

/*
 * UNIX machine dependent virtual memory support.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/vm.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/immu.h"

#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/page.h"
#include "vm/seg_vn.h"
#include "vm/seg_kmem.h"

#include "sys/immu.h"
#include "vm/cpu.h"

/*
 * map_addr() is the routine called when the system is to
 * chose an address for the user.  We will  pick an address
 * range which is just above UVSHM.
 *
 * addrp is a value/result parameter. On input it is a hint from the user
 * to be used in a completely machine dependent fashion. We decide to
 * completely ignore this hint.
 *
 * On output it is NULL if no address can be found in the current
 * processes address space or else an address that is currently
 * not mapped for len bytes with a page of red zone on either side.
 * If align is true, then the selected address will obey the alignment
 * constraints of a vac machine based on the given off value.
 *
 * On the 3b2, we don't have a virtual address cache, so this arg is
 * ignored.
 *
 * We try to map everything into secton 3 of the virtual address space.
 * This was one to allow the heap to grow to the section boundary. Also,
 * this makes the segment table for section 2 smaller while section 3 is
 * the one that grows.
 */
/*ARGSUSED*/
void
map_addr(addrp, len, off, align)
        addr_t *addrp;
        register u_int len;
        off_t off;
        int align;
{
	register struct as *as = u.u_procp->p_as;
	addr_t	base;
	u_int	slen;

	base = (addr_t) UVSHM;
	slen = (addr_t) UVEND - base;
	len = (len + PAGEOFFSET) & PAGEMASK;

	/*
	 * Redzone for each side of the request. This is done to leave
	 * one page unmapped between segments. This is not required, but
	 * it's useful for the user because if their program strays across
	 * a segment boundary, it will catch a fault immediately making
	 * debugging a little easier.
	 */
	len += 2 * PAGESIZE;

	/*
	 * Look for a large enough hole starting above UVSHM.
	 * After finding it, use the lower part.
	 */
	if (as_gap(as, len, &base, &slen, AH_LO, (addr_t)NULL) == 0)
		*addrp = (addr_t)((u_int)base + PAGESIZE);
        else
                *addrp = NULL;  /* no more virtual space */
}

/*
 * Determine whether [base, base+len) contains a mapable range of
 * addresses at least minlen long. base and len are adjusted if
 * required to provide a mapable range.
 */

/* ARGSUSED */
int
valid_va_range(basep, lenp, minlen, dir)
	register addr_t *basep;
	register u_int *lenp;
	register u_int minlen;
	register int dir;
{

	register addr_t hi, lo;

	lo = *basep;
	hi = lo + *lenp;
	if (hi < lo ) 		/* overflow */
		return(0);
	if (hi - lo < minlen)
		return (0);
	return (1);
}

/*
 * Determine whether [addr, addr+len) are vaild user address.
 */

int
valid_usr_range(addr, len)
	register addr_t addr;
	register size_t len;
{
	register addr_t eaddr = addr + len;

	if (eaddr <= addr || addr < (addr_t)UVBASE || eaddr >= (addr_t)KVBASE)
		return(0);
	return(1);
}
/*
 * Move pages from one kernel virtual address to another.
 * Both addresses are assumed to reside in the Sysmap,
 * and size must be a multiple of CLBYTES.
 */

/* ARGSUSED */
pagemove(from, to, size)
	register caddr_t from, to;
	int size;
{
}

/*
 * Locate the next free virtual address (above UVSHM) within the given process.
 */
caddr_t
findvaddr(p)
	struct proc *p;
{
	register struct as *as;
	register struct seg *seg;
	register caddr_t vaddr;
	register caddr_t vaddrt;

	as = p->p_as;

	vaddr = (caddr_t) UVSHM;

	seg = as->a_segs;
	if (seg != NULL) {
		/*
		 * segs are sorted by base virtual address,
		 * and are in a circular list, so we can 
		 * easily get to the highest used virtual address.
		 */
		seg = seg->s_prev; 
		vaddrt = seg->s_base + seg->s_size;
		if (vaddr < vaddrt)
			vaddr = vaddrt;
	}

	/*
	 * Align on Page (4K on 386) boundary. VM invalidates old assumption
	 * that alignment should be Page Table aligned (4MB on 386).
	 */

	vaddr = (caddr_t)((uint)(vaddr + NBPP - 1) & ~POFFMASK);
	return vaddr;
}

/*
 *	Fast inline data copy. Data sizes MUST be an exact multiple
 *	of 64 bytes, and <src> and <dest> addresses must be word aligned.
 *
 *	Cautions:  dest and src must be word aligned. len must be an integral
 *		   number of words.
 *
 *	Assumptions: Direction flag for string copy is the default one - from
 *		     low to high memory, i.e., we do not do a "cld". Anyone
 *		     who had changed it should restore it. %ecx is not saved -
 *		     since it is a throw away register.
 */

#if defined(lint)

#define inlinecopy(src, dest, len)	bcopy(src, dest, len)

#else
int	asm
inlinecopy(src, dest, len)
{
%mem	src, dest, len;
	pushl	%esi
	pushl	%edi
	movl	src, %esi
	movl	dest, %edi
	movl	len, %ecx
	shrl	$2, %ecx
	repz
	smovl
	popl	%edi
	popl	%esi
}

#endif

/*
 * Copy the data from the physical page represented by "frompp" to
 * that represented by "topp".
 */
void
ppcopy(frompp, topp)
	page_t *frompp;
	page_t *topp;
{
	ASSERT(frompp != NULL && topp != NULL);
	ASSERT(frompp >= pages && frompp < epages && topp >= pages && topp < epages);

	inlinecopy((addr_t) phystokv(ctob(page_pptonum(frompp))),
		   (addr_t) phystokv(ctob(page_pptonum(topp))),
		   PAGESIZE);
}

/*
 * pagecopy() and pagezero() assume that they will not be called at
 * interrupt level.
 */

/*
 * Copy the data from `addr' to physical page represented by `pp'.
 * `addr' is a (user) virtual address which we might fault on.
 */
void
pagecopy(addr, pp)
	addr_t addr;
	page_t *pp;
{
	register addr_t va;

	ASSERT(pp != NULL);
	va = (addr_t) pfntokv(page_pptonum(pp));
	(void) copyin((caddr_t)addr, va, PAGESIZE);
}

/*
 * Zero the physical page from off to off + len given by `pp'
 * without changing the reference and modified bits of page.
 * pagezero uses global CADDR2 and assumes that no one uses this
 * map at interrupt level and no one sleeps with an active mapping there.
 */
pagezero(pp, off, len)
	page_t *pp;
	u_int off, len;
{
	register caddr_t va;

	ASSERT((int)len > 0 && (int)off >= 0 && off + len <= PAGESIZE);
	ASSERT(pp != NULL);

	va = (caddr_t) pfntokv(page_pptonum(pp));
	(void) bzero(va + off, len);
}

/* Find an isolated hole in the user address space to use to
 * build the new stack image.
 * Prefer a hole that is aligned properly within a page table
 * so the page table(s) can be moved later.
 */

caddr_t
execstk_addr(size, hatflagp)
	int	size;
	u_int	*hatflagp;	/* to tell the hat whether a PT move or PTE copy
				 * is needed */
{
	addr_t	hstart, secend;
	int	hsize;
	struct seg *sseg, *seg;
	addr_t	base, eaddr, align, aligned_start;
	addr_t	svhstart = NULL;

	/* We are looking for a hole of 'size' bytes with a free page
	 * both before and aft (so segment concatenation does not occur).
	 * We first look for a slot with its end aligned properly for the
	 * user stack and falling inside of page table(s) which aren't used
	 * for anything else, so that later, we can use the page table
	 * directly at the new location, instead of copying pte's.
	 * If we can't find one, then we drop the alignment constraint.
	 *
	 * Since our stack grows downwards, we must be careful not to use
	 * slots where spilling of the aft end (into the free page) doesn't
	 * look like stack growth.  This allows the copyarglist code to
	 * skip string length checks.
	 */
	*hatflagp = 1;
	hstart = (addr_t) MINUVADR;
	if ((sseg = seg = u.u_procp->p_as->a_segs) == NULL)
		return hstart;
	secend = (addr_t) (MAXUVADR - PAGESIZE);
	align = (addr_t)((u.u_userstack + sizeof(int) - size) & (VPTSIZE - 1));
	size += PAGESIZE;	/* for free page at end */

	do {
		if ((ulong)(base = seg->s_base) == u.u_sub) {
			/* Pretend stack segment is one page bigger, to avoid
			 * the stack growth case if copyarglist spills over.
			 */
			base -= NBPP;
		}
		if (base > hstart) {
			if (base > secend)
				hsize = secend - hstart;
			else hsize = base - hstart;
			if (hsize >= size && svhstart == NULL)
				svhstart = hstart;
			if (base <= secend) {
				/* Make sure the end of the page table is free */
				aligned_start = (addr_t) ((u_int)base & ~(VPTSIZE-1));
				if (base != aligned_start) {
					hsize -= (base - aligned_start)
						- PAGESIZE; /* the extra page */
				}
			}
			aligned_start = (((u_int)hstart + (VPTSIZE-1)) & ~(VPTSIZE-1))
					+ align;
			hsize -= aligned_start - hstart;
			if (hsize >= size)
				return aligned_start;
		}
		eaddr = seg->s_base + seg->s_size;
		if (eaddr >= hstart)
			/* need free page before hole */
			hstart = eaddr + PAGESIZE;
		if (hstart > secend)
			break;
		if (seg->s_next == sseg) {
			/* it is the last segment, so all
			 * the rest is fair game.
			 */
			hsize = secend - hstart;
			if (hsize >= size && svhstart == NULL)
				svhstart = hstart;
			aligned_start = (((u_int)hstart + (VPTSIZE-1)) & ~(VPTSIZE-1))
					+ align;
			hsize -= aligned_start - hstart;
			if (hsize >= size)
				return(aligned_start);
		}
	} while (seg = seg->s_next, seg != sseg);

	*hatflagp = 0;
	return(svhstart);
}
