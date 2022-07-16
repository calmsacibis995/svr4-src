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

#ident	"@(#)kern-vm:seg_vpix.c	1.3.2.2"

/*
 * LIM 4.0 support:
 * Copyright (c) 1989 Phoenix Technologies Ltd.
 * All Rights Reserved.
*/


/*
 * SEG_VPIX -- VM support for VP/ix V86 processes.
 *
 * A seg_vpix segment supports the notion of "equivalenced" pages.  This
 * refers to multiple virtual pages in the same segment mapping to the
 * same physical (or anonymous) page.  It also allows given pages to be
 * mapped to specific physical device memory addresses (like seg_dev).
 */

#ifdef VPIX

#include "sys/types.h"
#include "sys/bitmap.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/tss.h"
#include "sys/seg.h"
#include "sys/systm.h"
#include "sys/vnode.h"
#include "sys/mman.h"
#include "sys/cmn_err.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/cred.h"
#include "sys/vmsystm.h"
#include "sys/swap.h"
#include "sys/kmem.h"
#include "sys/disp.h"
#include "sys/inline.h"
#include "sys/tuneable.h"
#include "sys/sysmacros.h"
#include "sys/v86.h"

#include "vm/trace.h"
#include "vm/hat.h"
#include "vm/mp.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_vpix.h"
#include "vm/pvn.h"
#include "vm/anon.h"
#include "vm/page.h"
#include "vm/vpage.h"
#include "vm/seg_kmem.h"

/* Flag to enable 80386 B1 stepping bug workarounds */
extern int	do386b1;

/*
 * Private seg op routines.
 */
STATIC	int segvpix_dup(/* seg, newsegp */);
STATIC	int segvpix_unmap(/* seg, addr, len */);
STATIC	void segvpix_free(/* seg */);
STATIC	faultcode_t segvpix_fault(/* seg, addr, type, rw */);
STATIC	faultcode_t segvpix_faulta(/* seg, addr */);
STATIC	void segvpix_unload(/* seg, addr, ref, mod */);
STATIC	int segvpix_setprot(/* seg, addr, len, prot */);
STATIC	int segvpix_checkprot(/* seg, addr, len, prot */);
STATIC	int segvpix_getprot(/* seg, addr, len, prot */);
STATIC	int segvpix_kluster(/* seg, addr, delta */);
STATIC	u_int segvpix_swapout(/* seg */);
STATIC	int segvpix_sync(/* seg, addr, len, attr, flags */);
STATIC	int segvpix_incore(/* seg, addr, len, vec */);
STATIC	int segvpix_lockop(/* seg, addr, len, attr, op, bitmap, pos */);
STATIC	off_t segvpix_getoffset(/* seg, addr */);
STATIC	int segvpix_gettype(/* seg, addr */);
STATIC	int segvpix_getvp(/* seg, addr, vpp */);

struct	seg_ops segvpix_ops = {
	segvpix_dup,
	segvpix_unmap,
	segvpix_free,
	segvpix_fault,
	segvpix_faulta,
	segvpix_unload,
	segvpix_setprot,
	segvpix_checkprot,
	segvpix_kluster,
	segvpix_swapout,
	segvpix_sync,
	segvpix_incore,
	segvpix_lockop,
	segvpix_getprot,
	segvpix_getoffset,
	segvpix_gettype,
	segvpix_getvp,
};

/*
 * Plain vanilla args structure, provided as a shorthand for others to use.
 */
STATIC
struct segvpix_crargs	vpix_crargs = {
	0,
};

caddr_t vpix_argsp = (caddr_t)&vpix_crargs;

/*
 * Variables for maintaining the free lists of segvpix_data structures.
 */
STATIC struct segvpix_data *segvpix_freelist;
STATIC int segvpix_freeincr = 14;


int
segvpix_create(seg, argsp)
	struct seg *seg;
	caddr_t argsp;
{
	register struct segvpix_crargs *a = (struct segvpix_crargs *)argsp;
	register struct segvpix_data *svd;
	register vpix_page_t *vpg;
	register u_int n, n2;
	vpix_page_t *ovpage;
	u_int osize;
	u_int swresv;

	/*
	 * We make sure the segment is entirely within one page table.
	 * This allows us to depend on the locking of the XTSS to keep
	 * virtual screen memory mappings from getting flushed.
	 */
	if (ptnum(seg->s_base) != ptnum(seg->s_base + seg->s_size - 1))
		return (EINVAL);

	/*
	 * The segment will need private pages; reserve them now.
	 */
	for (swresv = seg->s_size, n = a->n_hole; n-- > 0;) {
		/* Validate the hole: it must fall within the segment. */
		if (a->hole[n].base < seg->s_base ||
		    a->hole[n].base + a->hole[n].size >
				seg->s_base + seg->s_size) {
			return (EINVAL);
		}
		/* Don't reserve swap space for holes. */
		swresv -= a->hole[n].size;
	}

	if (anon_resv(swresv) == 0)
		return (ENOMEM);

	if (seg->s_base != 0) {
		struct seg	*pseg;

		/* If this segment immediately follows another seg_vpix
		 * segment, coalesce them into a single segment.
		 */
		if ((pseg = seg->s_prev) != seg &&
		    pseg->s_ops == &segvpix_ops &&
		    pseg->s_base + pseg->s_size == seg->s_base) {
			svd = (struct segvpix_data *)pseg->s_data;
			ovpage = svd->vpage;
			osize = btop(pseg->s_size);
			seg_free(seg);
			(seg = pseg)->s_size += swresv;
		} else {
			anon_unresv(swresv);
			return (EINVAL);
		}
	} else {
		/* Start a new vpix segment. */
		svd = (struct segvpix_data *)
			kmem_fast_alloc((caddr_t *)&segvpix_freelist,
					sizeof(*segvpix_freelist),
					segvpix_freeincr,
					KM_NOSLEEP);
		lock_init(&svd->lock);
		svd->cred = crgetcred();
		svd->swresv = 0;
		seg->s_ops = &segvpix_ops;
		seg->s_data = (char *)svd;
		ovpage = NULL;
		osize = 0;
	}

	svd->swresv += swresv;

	svd->vpage = (vpix_page_t *)kmem_zalloc((u_int)
		(seg_pages(seg) * sizeof(vpix_page_t)), KM_SLEEP);

	if (ovpage) {
		bcopy((caddr_t)ovpage, (caddr_t)svd->vpage,
			osize * sizeof(vpix_page_t));
		kmem_free((caddr_t)ovpage, osize * sizeof(vpix_page_t));
	}

	for (vpg = &svd->vpage[n = seg_pages(seg)]; n-- > osize;) {
		--vpg;
		vpg->eq_map = vpg->eq_link = vpg->rp_eq_list = n;
		if (do386b1 && n > 0 && n < 16)
			vpg->rp_errata10 = 1;
	}

	/* Change holes to be unmapped. */
	for (n = a->n_hole; n-- > 0;) {
		vpg = &svd->vpage[btop(a->hole[n].base)];
		for (n2 = btopr(a->hole[n].size); n2-- > 0; vpg++) {
			vpg->eq_map = vpg->eq_link = vpg->rp_eq_list = NULLEQ;
			vpg->rp_hole = 1;
		}
	}

	return (0);
}

STATIC int
segvpix_dup(seg, newseg)
	struct seg *seg, *newseg;
{
	ASSERT(newseg && newseg->s_as);
	newseg->s_as->a_size -= seg->s_size;
	seg_free(newseg);
	return(0);
}

STATIC int
segvpix_unmap(seg, addr, len)
	register struct seg *seg;
	register addr_t addr;
	u_int len;
{
	register struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	
	/*
	 * Check for bad sizes
	 */
	if (addr < seg->s_base || addr + len > seg->s_base + seg->s_size ||
	    (len & PAGEOFFSET) || ((u_int)addr & PAGEOFFSET))
		cmn_err(CE_PANIC, "segvpix_unmap");

	/*
	 * Only allow entire segment to be unmapped.
	 */
	if (addr != seg->s_base || len != seg->s_size)
		return (-1);

	/*
	 * Remove any page locks set through this mapping.
	 */
	(void) segvpix_lockop(seg, addr, len, 0, MC_UNLOCK,
				(ulong *)NULL, (size_t)NULL);

	/*
	 * Unload any hardware translations in the range to be taken out.
	 */
	hat_unload(seg, addr, len, HAT_NOFLAGS);

	seg_free(seg);

	return (0);
}

STATIC void
segvpix_free(seg)
	struct seg *seg;
{
	register struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	u_int npages = seg_pages(seg);

	/*
	 * Release anonymous pages.
	 */
	if (svd->vpage) {
		vpix_page_t	*vpage;

		for (vpage = &svd->vpage[npages]; vpage-- != svd->vpage;) {
			if (!vpage->rp_phys && vpage->rp_anon)
				anon_decref(vpage->rp_anon);
			PREEMPT();
		}
	}

	/*
	 * Deallocate the per-page arrays if necessary.
	 */
	if (svd->vpage != NULL)
		kmem_free((caddr_t)svd->vpage, npages * sizeof (vpix_page_t));

	/*
	 * Release swap reservation.
	 */
	if (svd->swresv)
		anon_unresv(svd->swresv);

	/*
	 * Release claim on credentials, and finally free the
	 * private data.
	 */
	crfree(svd->cred);
	kmem_fast_free((caddr_t *)&segvpix_freelist, (caddr_t)svd);
}

/*
 * Do a F_SOFTUNLOCK call over the range requested.
 * The range must have already been F_SOFTLOCK'ed.
 */
STATIC void
segvpix_softunlock(seg, addr, len, rw)
	struct seg *seg;
	addr_t addr;
	u_int len;
	enum seg_rw rw;
{
	register struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register vpix_page_t *vpp;
	u_int vpg, evpg, rp, equiv;
	register vpix_page_t *rpp;
	vpix_page_t *vpage;
	page_t *pp;
	struct vnode *vp;
	u_int offset;

	vpp = svd->vpage;
	vpage = &vpp[vpg = seg_page(seg, addr)];
	evpg = seg_page(seg, addr + len);

	for (; vpg != evpg; vpg++, vpage++) {
		if ((rp = vpage->eq_map) == NULLEQ)
			continue;

		/* If a physical device page, just unlock the translation */
		if ((rpp = &vpp[rp])->rp_phys) {
			/*
			 * Unlock the translation for the page and equivalents
			 */
			equiv = vpg;
			do {
				hat_unlock(seg, ptob(equiv));
			} while ((equiv = vpp[equiv].eq_link) != vpg);
			continue;
		}

		if (rpp->rp_anon == NULL)
			continue;

		swap_xlate(rpp->rp_anon, &vp, &offset);

		/*
		 * For now, we just kludge here by finding the page
		 * ourselves since we would not find the page using
		 * page_exists() if someone has page_abort()'ed it.
		 * XXX - need to redo things to avoid this mess.
		 */
		for (pp = page_hash[PAGE_HASHFUNC(vp, offset)]; pp != NULL;
		    pp = pp->p_hash)
			if (pp->p_vnode == vp && pp->p_offset == offset)
				break;
		if (pp == NULL || pp->p_pagein || pp->p_free)
			cmn_err(CE_PANIC, "segvpix_softunlock");

		if (rw == S_WRITE)
			pp->p_mod = 1;
		if (rw != S_OTHER)
			pp->p_ref = 1;

		/*
		 * Unlock the translation for the page and its equivalents
		 */
		equiv = vpg;
		do {
			hat_unlock(seg, ptob(equiv));
		} while ((equiv = vpp[equiv].eq_link) != vpg);

		PAGE_RELE(pp);
		availrmem++;
		pages_pp_kernel--;
	}
}


/*
 * Handles all the dirty work of getting the right
 * anonymous pages and loading up the translations.
 * This routine is called only from segvpix_fault()
 * when looping over the range of addresses requested.
 *
 * The basic algorithm here is:
 * 	If this is an anon_zero case
 *		Call anon_zero to allocate page
 *	else
 *		Use anon_getpage to get the page
 *	endif
 *	Load up the translation to the page and its equivalents
 *	return
 */
STATIC int
segvpix_faultpage(seg, addr, vp, type, rw)
	struct seg *seg;
	addr_t addr;
	u_int vp;
	enum fault_type type;
	enum seg_rw rw;
{
	register struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register vpix_page_t *vpp;
	register page_t *pp;
	u_int rp, equiv;
	vpix_page_t *vpage, *rpp;
	page_t *anon_pl[1 + 1];
	struct anon **app;
	u_int vpprot = PROT_READ|PROT_WRITE|PROT_USER;
	u_int hat_flags;
	int err;

	vpage = (vpp = svd->vpage) + vp;
	if ((rp = vpage->eq_map) == NULLEQ)
		return (FC_NOMAP);

	hat_flags = (type == F_SOFTLOCK ? HAT_LOCK : HAT_NOFLAGS);

	if ((rpp = &vpp[rp])->rp_phys) {
		/*
		 * This page is mapped to physical device memory.
		 * Load translation for the page and its equivalents.
		 */
		ASSERT(type != F_PROT);	/* since seg_vpix always read/write */
		equiv = vp;
		do {
			hat_devload(seg, ptob(equiv), rpp->rp_pfn,
					vpprot, hat_flags);
		} while ((equiv = vpp[equiv].eq_link) != vp);
		return (0);
	}

	if (type == F_SOFTLOCK) {
		if (availrmem - 1 < tune.t_minarmem)
			return (FC_MAKE_ERR(ENOMEM));	/* out of real memory */
		else { 
			--availrmem;
			pages_pp_kernel++;
		}
	}

	if (*(app = &rpp->rp_anon) == NULL) {
		/*
		 * Allocate a (normally) writable
		 * anonymous page of zeroes.
		 */
		/*
		 * Workaround for 80386 B1 stepping Errata 10 (I/O pages) --
		 * The v86 task has to be at virtual address 0, so we can't
		 * prevent pages 1-15 from being mapped and dirty.  Instead,
		 * we make sure bits 12-15 of the physical address match the
		 * virtual address, and the I/O addresses come out correctly.
		 */
		if (do386b1 && rpp->rp_errata10) {
			/* Search the equivalences to find which one has
			 * the alignment constraint.
			 */
			equiv = vp;
			while (equiv == 0 || equiv > 15) {
				equiv = vpp[equiv].eq_link;
				ASSERT(equiv != vp);
			}
			pp = anon_zero_aligned(seg, addr, app,
						0xF000, ptob(equiv));

			/* Lock this page, since we can't guarantee the
			 * alignment if we have to reload it with anon_getpage.
			 */
			if (page_pp_lock(pp, 0, 0) == 0) {
				PAGE_RELE(pp);
				err = EAGAIN;
				goto out;
			}
			rpp->rp_lock = 1;
		} else
			pp = anon_zero(seg, addr, app);
		if (pp == NULL) {
			err = ENOMEM;
			goto out;	/* out of swap space */
		}
	} else {
		/*
		 * Obtain the page structure via anon_getpage().
		 */
		err = anon_getpage(app, &vpprot, anon_pl, PAGESIZE,
				    seg, addr, rw, svd->cred);
		if (err)
			goto out;
		pp = anon_pl[0];

		ASSERT(pp != NULL);
		ASSERT(pp->p_keepcnt > 0);

		trace4(TR_PG_SEGVN_FLT, pp, pp->p_vnode, pp->p_offset, 0);
	}
	pp->p_ref = 1;

	/*
	 * Load translation for the page and its equivalents
	 */
	equiv = vp;
	do {
		hat_memload(seg, ptob(equiv), pp, vpprot, hat_flags);
	} while ((equiv = vpp[equiv].eq_link) != vp);

	if (type != F_SOFTLOCK)
		PAGE_RELE(pp);

	return (0);

out:
	if (type == F_SOFTLOCK) {
		availrmem++;
		pages_pp_kernel--;
	}
	return (FC_MAKE_ERR(err));
}

/*
 * This routine is called via a machine specific fault handling routine.
 * It is also called by software routines wishing to lock or unlock
 * a range of addresses.
 *
 * Here is the basic algorithm:
 *	If unlocking
 *		Call segvpix_softunlock
 *		Return
 *	endif
 *	Checking and set up work
 *	Loop over all addresses requested
 *		Call segvpix_faultpage to load up translations
 *				and handle anonymous pages
 *	endloop
 */

STATIC faultcode_t
segvpix_fault(seg, addr, len, type, rw)
	struct seg *seg;
	addr_t addr;
	u_int len;
	enum fault_type type;
	enum seg_rw rw;
{
	struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	addr_t a;
	u_int vpg;
	int err;
	
	/*
	 * First handle the easy stuff
	 */
	if (type == F_SOFTUNLOCK) {
		segvpix_softunlock(seg, addr, len, rw);
		return (0);
	}

	vpg = seg_page(seg, addr);

	/*
	 * Ok, now loop over the address range and handle faults
	 */
	for (a = addr; a < addr + len; a += PAGESIZE, ++vpg) {
		err = segvpix_faultpage(seg, a, vpg, type, rw);
		if (err) {
			if (type == F_SOFTLOCK && a > addr)
				segvpix_softunlock(seg, addr, (u_int)(a - addr),
				    S_OTHER);
			return (err);
		}
	}
	return (0);
}

/*
 * This routine is used to start I/O on pages asynchronously.
 */
STATIC faultcode_t
segvpix_faulta(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	register struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register u_int rp;
	register vpix_page_t *rpp;
	struct anon **app;
	int err;

	if ((rp = svd->vpage[seg_page(seg, addr)].eq_map) == NULLEQ)
		return (0);
	if ((rpp = &svd->vpage[rp])->rp_phys)
		return (0);
	if (*(app = &rpp->rp_anon) != NULL) {
		err = anon_getpage(app, (u_int *)NULL,
				(struct page **)NULL, 0, seg, addr, S_READ,
				svd->cred);
		if (err)
			return (FC_MAKE_ERR(err));
		return (0);
	}

	return (0);
}

STATIC void
segvpix_unload(seg, addr, ref, mod)
	struct seg *seg;
	addr_t addr;
	u_int ref, mod;
{
	register struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register u_int rp;
	register vpix_page_t *rpp;

	/*
	 * For now since we are doing a global page replacement
	 * policy, we don't worry about keeping track of virtual
	 * ref and modified bits.
	 */

	rp = svd->vpage[seg_page(seg, addr)].eq_map;

	if (rp != NULLEQ && !(rpp = &svd->vpage[rp])->rp_phys &&
				rpp->rp_anon != NULL)
		anon_unloadmap(rpp->rp_anon, ref, mod);
}

STATIC int
segvpix_setprot(seg, addr, len, prot)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, prot;
{
	if (prot != (PROT_READ|PROT_WRITE))
		return (EACCES);	/* Only R/W protections allowed */
	return (0);
}

STATIC int
segvpix_checkprot(seg, addr, len, prot)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, prot;
{
	return (((prot & (PROT_READ|PROT_WRITE|PROT_USER)) != prot) ? EACCES : 0);
}

STATIC int
segvpix_getprot(seg, addr, len, protv)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, *protv;
{
	u_int pgno = seg_page(seg, addr+len) - seg_page(seg, addr) + 1;

	while (pgno-- > 0)
		*protv++ = PROT_READ|PROT_WRITE;
	return 0;
}

/*
 * Check to see if it makes sense to do kluster/read ahead to
 * addr + delta relative to the mapping at addr.  We assume here
 * that delta is a signed PAGESIZE'd multiple (which can be negative).
 *
 * For seg_vpix, we currently "approve" of the action if we are
 * still in the segment and it maps from the same vp/off.
 */
STATIC int
segvpix_kluster(seg, addr, delta)
	register struct seg *seg;
	register addr_t addr;
	int delta;
{
	register struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register struct anon *oap, *ap;
	register int pd;
	register u_int page;
	u_int rp1, rp2;
	vpix_page_t *rpp1, *rpp2;
	struct vnode *vp1, *vp2;
	u_int off1, off2;

	if (addr + delta < seg->s_base ||
	    addr + delta >= (seg->s_base + seg->s_size))
		return (-1);		/* exceeded segment bounds */

	pd = delta / PAGESIZE;		/* divide to preserve sign bit */
	page = seg_page(seg, addr);

	if ((rp1 = svd->vpage[page].eq_map) == NULLEQ ||
	    (rp2 = svd->vpage[page + pd].eq_map) == NULLEQ)
		return (-1);

	rpp1 = &svd->vpage[rp1];
	rpp2 = &svd->vpage[rp2];

	oap = rpp1->rp_phys ? NULL : rpp1->rp_anon;
	ap = rpp2->rp_phys ? NULL : rpp2->rp_anon;

	if (oap == NULL || ap == NULL)
		return (-1);

	/*
	 * Now we know we have two anon pointers - check to
	 * see if they happen to be properly allocated.
	 */
	swap_xlate(ap, &vp1, &off1);
	swap_xlate(oap, &vp2, &off2);
	if (!VOP_CMP(vp1, vp2) || off1 - off2 != delta)
		return (-1);
	return (0);
}

/*
 * Swap the pages of seg out to secondary storage, returning the
 * number of bytes of storage freed.
 *
 * The basic idea is first to unload all translations and then to call
 * VOP_PUTPAGE for all newly-unmapped pages, to push them out to the
 * swap device.  Pages to which other segments have mappings will remain
 * mapped and won't be swapped.  Our caller (as_swapout) has already
 * performed the unloading step.
 *
 * The value returned is intended to correlate well with the process's
 * memory requirements.  However, there are some caveats:
 * 1)	We assume that the hat layer maintains a large enough translation
 *	cache to capture process reference patterns.
 */
STATIC u_int
segvpix_swapout(seg)
	struct seg *seg;
{
	struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register u_int pgcnt = 0;
	u_int npages;
	register u_int page;
	register struct page *pp;
	register u_int rp;
	register vpix_page_t *rpp;
	struct vnode *vp;
	u_int off;

	/*
	 * Find pages unmapped by our caller and force them
	 * out to the virtual swap device.
	 */
	npages = btop(seg->s_size);
	for (page = 0; page < npages; page++) {
		/*
		 * Obtain <vp, off> pair for the page, then look it up.
		 */
		if ((rp = svd->vpage[page].eq_map) == NULLEQ)
			continue;
		if ((rpp = &svd->vpage[rp])->rp_phys || rpp->rp_anon == NULL)
			continue;
		swap_xlate(rpp->rp_anon, &vp, &off);
		if ((pp = page_exists(vp, off)) == NULL || pp->p_free)
			continue;

		/*
		 * Skip if page is logically unavailable for removal.
		 */
		if (pp->p_lckcnt > 0)
			continue;

		/*
		 * Examine the page to see whether it can be tossed out,
		 * keeping track of how many we've found.
		 */
		if (pp->p_keepcnt != 0) {
			/*
			 * If the page is marked as in transit going out
			 * and has no mappings, it's very likely that
			 * the page is in transit because of klustering.
			   Assume this is so and take credit for it here.
			 */
			if (pp->p_intrans && !pp->p_pagein && !pp->p_mapping)
				pgcnt++;
			continue;
		}

		if (pp->p_mapping != NULL)
			continue;

		/*
		 * Since the keepcnt was 0 the page should not be
		 * in a gone state nor is not directly or indirectly
		 * involved in any IO at this time.
		 */

		/*
		 * No longer mapped -- we can toss it out.  How
		 * we do so depends on whether or not it's dirty.
		 *
		 * XXX:	Need we worry about locking between the
		 * time of the hat_pagesync call and the actions
		 * that depend on its result?
		 */
		if (pp->p_mod && pp->p_vnode) {
			/*
			 * We must clean the page before it can be
			 * freed.  Setting B_FREE will cause pvn_done
			 * to free the page when the i/o completes.
			 * XXX:	This also causes it to be accounted
			 *	as a pageout instead of a swap: need
			 *	B_SWAPOUT bit to use instead of B_FREE.
			 */
			(void) VOP_PUTPAGE(pp->p_vnode, pp->p_offset, PAGESIZE,
			    B_ASYNC | B_FREE, svd->cred);
		} else {
			/*
			 * The page was clean.  Lock it and free it.
			 *
			 * XXX:	Can we ever encounter modified pages
			 *	with no associated vnode here?
			 */
			page_lock(pp);
			page_free(pp, 0);
		}

		/*
		 * Credit now even if i/o is in progress.
		 */
		pgcnt++;
	}

	return (ptob(pgcnt));
}

/*
 * Synchronize primary storage cache with real object in virtual memory.
 */
STATIC int
segvpix_sync(seg, addr, len, attr, flags)
	struct seg *seg;
	register addr_t addr;
	u_int len;
	int attr;
	u_int flags;
{
	register struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register vpix_page_t *vpage;
	u_int rp;
	register vpix_page_t *rpp;
	register struct anon *ap;
	register page_t *pp;
	struct vnode *vp;
	u_int off;
	addr_t eaddr;
	int bflags;
	int err = 0;
	int segtype;
	int pageprot;


	bflags = B_FORCE | ((flags & MS_ASYNC) ? B_ASYNC : 0) |
	    		   ((flags & MS_INVALIDATE) ? B_INVAL : 0);

	if (attr) {
		pageprot = attr & ~(SHARED|PRIVATE);
		segtype = attr & SHARED ? MAP_SHARED : MAP_PRIVATE;

		/*
		 * We are done if the segment types or protections
		 * don't match.
		 */
		if (segtype != MAP_PRIVATE)
			return(0);
		if (pageprot != (PROT_READ|PROT_WRITE))
			return(0);
	}

	vpage = &svd->vpage[seg_page(seg, addr)];

	for (eaddr = addr + len; addr < eaddr; addr += PAGESIZE) {

		if ((rp = (vpage++)->eq_map) == NULLEQ)
			continue;
		if ((rpp = &svd->vpage[rp])->rp_phys)
			continue;
		if ((ap = rpp->rp_anon) == NULL) 
			continue;

		swap_xlate (ap, &vp, &off);

		/*
		 * See if any of these pages are locked --  if so, then we
		 * will have to truncate an invalidate request at the first
		 * locked one.
		 */
		if (flags & MS_INVALIDATE) {
			if ((pp = page_lookup(vp, off)) != NULL)
				if (pp->p_lckcnt || pp->p_cowcnt)
					return EBUSY;
		}

		/*
		* XXX - Should ultimately try to kluster
		* calls to VOP_PUTPAGE for performance.
		*/
		err = VOP_PUTPAGE(vp, off, PAGESIZE, bflags, svd->cred);
		if (err)
			break;
	}
	return (err);
}

/*
 * Determine if we have data corresponding to pages in the
 * primary storage virtual memory cache (i.e., "in core").
 * N.B. Assumes things are "in core" if page structs exist.
 */
STATIC int
segvpix_incore(seg, addr, len, vec)
	struct seg *seg;
	addr_t addr;
	u_int len;
	char *vec;
{
	register struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	u_int rp;
	register vpix_page_t *rpp;
	struct vnode *vp;
	u_int offset;
	u_int p = seg_page(seg, addr);
	u_int ep = seg_page(seg, addr + len);
	u_int v;

	for (v = 0; p < ep; p++, addr += PAGESIZE, v += PAGESIZE) {
		if ((rp = svd->vpage[p].eq_map) != NULLEQ &&
		    !(rpp = &svd->vpage[rp])->rp_phys &&
		    rpp->rp_anon != NULL) {
			swap_xlate(rpp->rp_anon, &vp, &offset);
			*vec++ = (page_exists(vp, offset) != NULL);
		} else
			*vec++ = 0;
	}
	return (v);
}

	
/*
 * Lock down (or unlock) pages mapped by this segment.
 */
STATIC int
segvpix_lockop(seg, addr, len, attr, op, lockmap, pos)
	struct seg *seg;
	addr_t addr;
	u_int len;
	int   attr;
	int op;
	ulong *lockmap;
	size_t pos;
{
	struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register vpix_page_t *vpp, *vpage, *epage, *rpp;
	struct page *pp;
	struct vnode *vp;
	u_int off;
	int segtype;
	int pageprot;
	int err;

	if (attr) {
		pageprot = attr & ~(SHARED|PRIVATE);
		segtype = attr & SHARED ? MAP_SHARED : MAP_PRIVATE;

		/*
		 * We are done if the segment types or protections
		 * don't match.
		 */
		if (segtype != MAP_PRIVATE)
			return(0);
		if (pageprot != (PROT_READ|PROT_WRITE))
			return(0);
	}

	/*
	 * Loop over all pages in the range.  Process if we're locking and
	 * page has not already been locked in this mapping; or if we're
	 * unlocking and the page has been locked.
	 */
	vpp = svd->vpage;
	vpage = &vpp[seg_page(seg, addr)];
	epage = &vpp[seg_page(seg, addr + len)];

	for (; vpage < epage; vpage++, pos++, addr += PAGESIZE) {
		if (vpage->eq_map == NULLEQ)
			continue;
		if ((rpp = &vpp[vpage->eq_map])->rp_phys)
			continue;
		if ((op == MC_LOCK && !rpp->rp_lock)
		   || (op == MC_UNLOCK && rpp->rp_lock)) {
			/* 
			 * If we're locking, softfault the page in memory.
			 */
			if (op == MC_LOCK)
				if (segvpix_fault(seg, addr, PAGESIZE, 
				    F_SOFTLOCK, S_OTHER) != 0)
					return (EIO);

			ASSERT(rpp->rp_anon);	/* SOFTLOCK will force these */

			/*
			 * Get name for page, accounting for
			 * existence of private copy.
			 */
			swap_xlate(rpp->rp_anon, &vp, &off);

			/*
			 * Get page frame.  It's ok if the page is
			 * not available when we're unlocking, as this
			 * may simply mean that a page we locked got
			 * truncated out of existence after we locked it.
			 */
			if ((pp = page_lookup(vp, off)) == NULL)
				if (op == MC_LOCK)
					cmn_err(CE_PANIC, "segvpix_lockop: no page");

			/*
			 * Perform page-level operation appropriate to
			 * operation.  If locking, undo the SOFTLOCK
			 * performed to bring the page into memory
			 * after setting the lock.  If unlocking,
			 * and no page was found, account for the claim
			 * separately.
			 */
			if (op == MC_LOCK) {
				err = page_pp_lock(pp, 0, 0);
				(void) segvpix_fault(seg, addr, PAGESIZE, 
				    F_SOFTUNLOCK, S_OTHER);
				if (!err)
					return (EAGAIN);
				rpp->rp_lock = 1;
				if (lockmap != (ulong *)NULL) {
					BT_SET(lockmap, pos);
				}
			} else {
				if (pp)
					page_pp_unlock(pp, 0, 0);
				rpp->rp_lock = 0;
			}
		}
	}
	return (0);
}

/* ARGSUSED */
STATIC off_t
segvpix_getoffset(seg, addr)
	register struct seg *seg;
	addr_t addr;
{
	return (off_t)0;
}

/* ARGSUSED */
STATIC int
segvpix_gettype(seg, addr)
	register struct seg *seg;
	addr_t addr;
{
 	return MAP_PRIVATE;
}

/* ARGSUSED */
STATIC int
segvpix_getvp(seg, addr, vpp)
	register struct seg *seg;
	addr_t addr;
	struct vnode **vpp;
{
	return -1;
}


/*
 * Map a range of virtual pages in the segment to specific physical
 * (device memory) pages.
 */
int
segvpix_physmap(seg, vpage, ppage, npages)
	register struct seg	*seg;
	u_int		vpage;
	u_int		ppage;
	u_int		npages;
{
	struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register vpix_page_t *vpp;
	register vpix_page_t *rpp;
	register u_int	vpg, equiv;
	u_int		pfn;

	vpp = svd->vpage;

	for (rpp = &vpp[vpage]; npages-- > 0; rpp++, ppage++) {

		/* If this rpage is a "hole", fail */
		if (rpp->rp_hole)
			return (ENXIO);

		/* If this rpage had an anon before, discard it */
		if (!rpp->rp_phys && rpp->rp_anon != NULL)
			anon_decref(rpp->rp_anon);

		/* Indicate this rpage is now physmapped */
		rpp->rp_phys = 1;
		rpp->rp_pfn = ppage;

		/* Get a pointer to one of the vpages for this rp page */
		if ((vpg = rpp->rp_eq_list) == NULLEQ)
			continue;

		/* Validity checks against equivalenced pages */
		if (rpp->rp_errata10) {
			equiv = vpg;
			do {
				if (equiv > 0 && equiv < 16 &&
				    (ppage & 15) != equiv)
					return (EINVAL);
			} while ((equiv = vpp[equiv].eq_link) != vpg);
		}

		/* Unload any previous translations */
		equiv = vpg;
		do {
			hat_unload(seg, ptob(equiv), PAGESIZE, HAT_NOFLAGS);
			/* Load the new translation. [optimization] */
			hat_devload(seg, ptob(equiv), ppage,
					PROT_ALL, HAT_NOFLAGS);
		} while ((equiv = vpp[equiv].eq_link) != vpg);
	}

	return (0);
}


/*
 * Revert a range of virtual pages from a physical mapping to anonymous
 * memory.  This undoes the effect of segvpix_physmap().
 */
int
segvpix_unphys(seg, vpage, npages)
	register struct seg	*seg;
	u_int		vpage;
	u_int		npages;
{
	struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register vpix_page_t *vpp;
	register vpix_page_t *rpp;
	register u_int	vpg, equiv;

	vpp = svd->vpage;

	for (rpp = &vpp[vpage]; npages-- > 0; rpp++) {

		/* Indicate this rpage is no longer physmapped */
		if (!rpp->rp_phys)
			continue;
		rpp->rp_phys = 0;
		rpp->rp_pfn = 0;

		/* Get a pointer to one of the vpages for this rp page */
		if ((vpg = rpp->rp_eq_list) == NULLEQ)
			continue;

		/* Unload any previous translations */
		equiv = vpg;
		do {
			hat_unload(seg, ptob(equiv), PAGESIZE, HAT_NOFLAGS);
		} while ((equiv = vpp[equiv].eq_link) != vpg);
	}

	return (0);
}


/*
 * Set up an equivalence between two virtual pages in the segment.
 * First, vpage_from is unlinked from any equivalence set it might belong
 * to.  Then it is attached to the (possibly unit) set containing
 * rpage_to.  Note that this means that if vpage_from and rpage_to are
 * the same, this will have the effect of unlinking that page from any
 * equivalence set.
 */
int
segvpix_page_equiv(seg, vpage_from, rpage_to)
	register struct seg	*seg;
	u_int		vpage_from;
	u_int		rpage_to;
{
	struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
	register vpix_page_t	*vpp;
	register u_int		rp, equiv, eq2;
	register vpix_page_t	*rpp, *vpg;

	vpg = &(vpp = svd->vpage)[vpage_from];

	if ((rp = vpg->eq_map) == rpage_to) {
		/* Already mapped to the right page. */
		return 0;
	}

	/*
	 * Unlink vpage_from from its old mapping.
	 */
	if (rp != NULLEQ) {
		rpp = &vpp[rp];

		/* Update the rpage's errata10 flag */
		if (do386b1 && rpp->rp_errata10) {
			if (vpage_from > 0 && vpage_from < 16)
				rpp->rp_errata10 = 0;
		}

		/* Unlink from the previous equivalence set */
		ASSERT(vpg->eq_link != NULLEQ);
		for (equiv = vpg->eq_link; vpp[equiv].eq_link != vpage_from;)
			equiv = vpp[equiv].eq_link;
		if (equiv == vpage_from)
			rpp->rp_eq_list = NULLEQ;
		else
			vpp[equiv].eq_link = vpg->eq_link;

		/* Unload any previous translation */
		hat_unload(seg, ptob(vpage_from), PAGESIZE, HAT_NOFLAGS);
	}

	/*
	 * Make sure we don't map to a "hole".
	 */
	rpp = &vpp[rp = vpg->eq_map = rpage_to];
	if (rpp->rp_hole) {
		vpg->eq_map = NULLEQ;
		if (vpage_from != rpage_to)
			return (ENXIO);
		return (0);
	}

	/*
	 * Update the new rpage's errata10 flag.
	 */
	if (do386b1 && vpage_from > 0 && vpage_from < 16) {
		if (rpp->rp_errata10) {
			/* Can't link two errata 10 pages */
			vpg->eq_map = NULLEQ;
			return (EINVAL);
		}
		rpp->rp_errata10 = 1;
	}

	/*
	 * Link to rpage_to.
	 */
	if ((equiv = rpp->rp_eq_list) == NULLEQ)
		rpp->rp_eq_list = vpg->eq_link = vpage_from;
	else {
		vpg->eq_link = vpp[equiv].eq_link;
		vpp[equiv].eq_link = vpage_from;
	}

	return (0);
}


/*
 * This is like segvpix_page_equiv(), but works on a range of pages.
 */
int
segvpix_range_equiv(seg, vpage_from, rpage_to, npages)
	register struct seg	*seg;
	u_int		vpage_from;
	u_int		rpage_to;
	u_int		npages;
{
	register u_int	npg = npages;
	int		err;

	while (npg-- > 0) {
		err = segvpix_page_equiv(seg, vpage_from++, rpage_to++);
		if (err) {
			npages -= npg;
			segvpix_range_equiv(seg, vpage_from - npages,
						 vpage_from - npages,
						 npages);
			return (err);
		}
	}

	return (0);
}


/*
 * Scan a range of memory for modified bits.
 */
int
segvpix_modscan(seg, vpage, npages)
	register struct seg	*seg;
	u_int		vpage;
	u_int		npages;
{
#ifdef DEBUG
	struct segvpix_data *svd = (struct segvpix_data *)seg->s_data;
#endif
	pte_t		*pte;
	int		retval = 0;
	int		bit = 1;

	for (; npages-- > 0; vpage++, bit <<= 1) {

		/* Determine the page table entry for this vpage.
		 * This assumes this routine is called in the process's
		 * context. */

		if (!PG_ISVALID(pte = svtopte(ptob(vpage))))
			continue;

		/* If the page is present and it has been modified,
		 * set the corresponding bit in retval. */

		if (pte->pgm.pg_mod) {
			PG_CLRMOD(pte);
			retval |= bit;
			ASSERT(svd->vpage[svd->vpage[vpage].eq_map].rp_lock);
		}
	}

	return retval;
}

#else	/* VPIX */

int
segvpix_create(seg, argsp)
	struct seg *seg;
	caddr_t argsp;
{
	return ENOSYS;
}

#endif	/* VPIX */
