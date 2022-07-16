/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-vm:seg_dummy.c	1.3"

/*
 * VM - dummy segment driver.
 *
 * This segment driver is used to reserve sections of address space
 * without actually making any mappings.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/mman.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/kmem.h"
#include "sys/cmn_err.h"
#include "sys/vnode.h"
#include "sys/debug.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/sysmacros.h"

#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"

/*
 * Private seg op routines.
 */
STATIC	int segdummy_dup(/* seg, newsegp */);
STATIC	int segdummy_unmap(/* seg, addr, len */);
STATIC	void segdummy_free(/* seg */);
STATIC	faultcode_t segdummy_fault(/* seg, addr, len, type, rw */);
STATIC	faultcode_t segdummy_faulta(/* seg, addr */);
STATIC	void segdummy_unload(/* seg, addr, ref, mod */);
STATIC	int segdummy_setprot(/* seg, addr, size, len */);
STATIC	int segdummy_checkprot(/* seg, addr, size, len */);
STATIC	int segdummy_getprot(/* seg, addr, size, len */);
STATIC	off_t segdummy_getoffset(/* seg, addr */);
STATIC	int segdummy_gettype(/* seg, addr */);
STATIC	int segdummy_getvp(/* seg, addr, vpp */);
STATIC	int segdummy_badop();
STATIC	int segdummy_incore(/* seg, addr, size, vec */);
STATIC	int segdummy_ctlops(/* seg, addr, size, [flags] */);

STATIC struct seg_ops segdummy_ops = {
	segdummy_dup,
	segdummy_unmap,
	segdummy_free,
	segdummy_fault,
	segdummy_faulta,
	segdummy_unload,
	segdummy_setprot,
	segdummy_checkprot,
	segdummy_badop,		/* kluster */
	(u_int (*)()) NULL,	/* swapout */
	segdummy_ctlops,		/* sync */
	segdummy_incore,
	segdummy_ctlops,		/* lockop */
	segdummy_getprot,
	segdummy_getoffset,
	segdummy_gettype,
	segdummy_getvp,
};

STATIC u_int	segdummy_data;


/*
 * Create a device segment.
 */
/*ARGSUSED*/
int
segdummy_create(seg, argsp)
	struct seg *seg;
	_VOID *argsp;
{
	seg->s_ops = &segdummy_ops;

	/* s_data isn't used, so just point all of them to a single location */
	seg->s_data = (_VOID *)&segdummy_data;

	return 0;
}

/*
 * Duplicate seg and return new segment in newsegp.
 */
/*ARGSUSED*/
STATIC int
segdummy_dup(seg, newseg)
	struct seg *seg, *newseg;
{
	return segdummy_create(newseg, NULL);
}

/*
 * Unmap a segment at addr for length len.
 */
/*ARGSUSED*/
STATIC int
segdummy_unmap(seg, addr, len)
	register struct seg *seg;
	register addr_t addr;
	u_int len;
{
	register struct seg	*nseg;	/* new segment, for split case */
	register addr_t		nbase;	/* base addr of new seg */
	register u_int		nsize;	/* size of new seg */

	/*
	 * Check for entire segment
	 */
	if (addr == seg->s_base && len == seg->s_size) {
		seg_free(seg);
		return (0);
	}

	/*
	 * Check for beginning of segment
	 */
	if (addr == seg->s_base) {
		seg->s_base += len;
		seg->s_size -= len;
		return (0);
	}

	/*
	 * Check for end of segment
	 */
	if (addr + len == seg->s_base + seg->s_size) {
		seg->s_size -= len;
		return (0);
	}

	/*
	 * The section to go is in the middle of the segment,
	 * have to make it into two segments.  nseg is made for
	 * the high end while seg is cut down at the low end.
	 */
	nbase = addr + len;				/* new seg base */
	nsize = (seg->s_base + seg->s_size) - nbase;	/* new seg size */
	seg->s_size = addr - seg->s_base;		/* shrink old seg */
	nseg = seg_alloc(seg->s_as, nbase, nsize);
	if (nseg == NULL)
		cmn_err(CE_PANIC, "segdummy_unmap seg_alloc");

	return segdummy_create(nseg, NULL);
}

/*
 * Free a segment.
 */
/*ARGSUSED*/
STATIC void
segdummy_free(seg)
	struct seg *seg;
{
	/* no-op */
}

/*
 * Handle a fault on a device segment.
 */
/*ARGSUSED*/
STATIC faultcode_t
segdummy_fault(seg, addr, len, type, rw)
	struct seg *seg;
	addr_t addr;
	u_int len;
	enum fault_type type;
	enum seg_rw rw;
{
	return (FC_MAKE_ERR(EFAULT));
}

/*
 * Asynchronous page fault.
 */
/*ARGSUSED*/
STATIC faultcode_t
segdummy_faulta(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	return (FC_MAKE_ERR(EFAULT));
}

/*ARGSUSED*/
STATIC void
segdummy_unload(seg, addr, ref, mod)
	struct seg *seg;
	addr_t addr;
	u_int ref, mod;
{
	/* no-op */
}

/*ARGSUSED*/
STATIC int
segdummy_setprot(seg, addr, len, prot)
	struct seg *seg;
	addr_t addr;
	u_int len, prot;
{
	return (0);
}

/*ARGSUSED*/
STATIC int
segdummy_checkprot(seg, addr, len, prot)
	struct seg *seg;
	addr_t addr;
	u_int len, prot;
{
	return (0);
}

/*ARGSUSED*/
STATIC int
segdummy_getprot(seg, addr, len, protv)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, *protv;
{
	u_int pgno = seg_page(seg, addr+len) - seg_page(seg, addr) + 1;

	while (pgno-- > 0)
		*protv++ = PROT_NONE;
	return 0;
}

/* ARGSUSED */
STATIC off_t
segdummy_getoffset(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	return (off_t)0;
}

/* ARGSUSED */
STATIC int
segdummy_gettype(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	return MAP_PRIVATE;
}

/* ARGSUSED */
STATIC int
segdummy_getvp(seg, addr, vpp)
	struct seg *seg;
	addr_t addr;
	struct vnode **vpp;
{
	*vpp = NULL;
	return -1;
}


STATIC
segdummy_badop()
{
	cmn_err(CE_PANIC, "segdummy_badop");
	/*NOTREACHED*/
}

/*
 * segdummy pages are not in the cache, and thus can't really be controlled.
 * syncs, locks, and advice are simply always successful.
 */
/*ARGSUSED*/
STATIC int
segdummy_ctlops(seg, addr, len, flags)
	struct seg *seg;
	addr_t addr;
	u_int len, flags;
{
	return (0);
}

/*
 * segdummy pages are never "in core".
 */
/*ARGSUSED*/
STATIC int
segdummy_incore(seg, addr, len, vec)
	struct seg *seg;
	addr_t addr;
	u_int len;
	char *vec;
{
	bzero(vec, len = btoc(len));
	return ctob(len);
}
