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

#ident  "@(#)kern-vm:seg_vn.c	1.3.1.8"

/*
 * VM - shared or copy-on-write from a vnode/anonymous memory.
 */

#include "sys/types.h"
#include "sys/bitmap.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/vnode.h"
#include "sys/mman.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/cred.h"
#include "sys/swap.h"
#include "sys/kmem.h"
#include "sys/inline.h"
#include "sys/tuneable.h"
#include "sys/sysmacros.h"
#include "sys/vmsystm.h"

#include "vm/trace.h"
#include "vm/hat.h"
#include "vm/mp.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_vn.h"
#include "vm/pvn.h"
#include "vm/anon.h"
#include "vm/page.h"
#include "vm/vpage.h"
#include "vm/seg_kmem.h"

/*
 * Private seg op routines.
 */
STATIC	int segvn_dup(/* seg, newsegp */);
STATIC	int segvn_unmap(/* seg, addr, len */);
STATIC	void segvn_free(/* seg */);
STATIC	faultcode_t segvn_fault(/* seg, addr, type, rw */);
STATIC	faultcode_t segvn_faulta(/* seg, addr */);
STATIC	void segvn_unload(/* seg, addr, ref, mod */);
STATIC	int segvn_setprot(/* seg, addr, len, prot */);
STATIC	int segvn_checkprot(/* seg, addr, len, prot */);
STATIC	int segvn_getprot(/* seg, addr, len, prot */);
STATIC	off_t segvn_getoffset(/* seg, addr */);
STATIC	int segvn_gettype(/* seg, addr */);
STATIC	int segvn_getvp(/* seg, addr, vpp */);
STATIC	int segvn_kluster(/* seg, addr, delta */);
STATIC	u_int segvn_swapout(/* seg */);
STATIC	int segvn_sync(/* seg, addr, len, attr, flags */);
STATIC	int segvn_incore(/* seg, addr, len, vec */);
void free_vp_pages(/* vp, off, len */);
STATIC	int segvn_lockop(/* seg, addr, len, attr, op, bitmap, pos */);

struct	seg_ops segvn_ops = {
	segvn_dup,
	segvn_unmap,
	segvn_free,
	segvn_fault,
	segvn_faulta,
	segvn_unload,
	segvn_setprot,
	segvn_checkprot,
	segvn_kluster,
	segvn_swapout,
	segvn_sync,
	segvn_incore,
	segvn_lockop,
	segvn_getprot,
	segvn_getoffset,
	segvn_gettype,
	segvn_getvp,
};

/*
 * Common zfod structures, provided as a shorthand for others to use.
 */
STATIC
struct	segvn_crargs zfod_segvn_crargs = {
	(struct vnode *)NULL,
	0,
	(struct cred *)NULL,
	MAP_PRIVATE,
	PROT_ALL,
	PROT_ALL,
	(struct anon_map *)NULL,
};

STATIC
struct	segvn_crargs kzfod_segvn_crargs = {
	(struct vnode *)NULL,
	0,
	(struct cred *)NULL,
	MAP_PRIVATE,
	PROT_ALL & ~PROT_USER,
	PROT_ALL & ~PROT_USER,
	(struct anon_map *)NULL,
};

caddr_t	zfod_argsp = (caddr_t)&zfod_segvn_crargs;	/* user zfod argsp */
caddr_t	kzfod_argsp = (caddr_t)&kzfod_segvn_crargs;	/* kernel zfod argsp */

/*
 * Variables for maintaining the free lists
 * of segvn_data and anon_map structures.
 */
STATIC struct segvn_data *segvn_freelist;
STATIC int segvn_freeincr = 14;
STATIC struct anon_map *anonmap_freelist;
STATIC int anonmap_freeincr = 21;

STATIC	u_int anon_slop = 64*1024;	/* allow segs to expand in place */

STATIC	int segvn_concat(/* seg1, seg2, a */);
STATIC	int segvn_extend_prev(/* seg1, seg2, a */);
STATIC	int segvn_extend_next(/* seg1, seg2, a */);
STATIC	void anonmap_alloc(/* seg, swresv */);
STATIC	void segvn_vpage(/* seg */);

int
segvn_create(seg, argsp)
	struct seg *seg;
	_VOID *argsp;
{
	register struct segvn_crargs *a = (struct segvn_crargs *)argsp;
	register struct segvn_data *svd;
	register u_int swresv = 0;
	register struct cred *cred;
	int error;

	if (a->type != MAP_PRIVATE && a->type != MAP_SHARED)
		cmn_err(CE_PANIC, "segvn_create type");

	/*
	 * Check arguments.  If a shared anon structure is given then
	 * it is illegal to also specify a vp.
	 */
	if (a->amp != NULL && a->vp != NULL)
		cmn_err(CE_PANIC, "segvn_create anon_map");

	/*
	 * If segment may need private pages, reserve them now.
	 */
	if ((a->vp == NULL && a->amp == NULL)
	  || (a->type == MAP_PRIVATE && (a->prot & PROT_WRITE))) {
		if (anon_resv(seg->s_size) == 0)
			return (EAGAIN);
		swresv = seg->s_size;
	}

	/*
	 * Reserve any mapping structures that may be required.
	 * We disallow writeable translations in all cases, even
	 * for shared mappings, to force a protection fault,
	 * so filesystems can allocate blocks if they need to.
	 */
	error = hat_map(seg, a->vp, a->offset & PAGEMASK,
			a->prot & ~PROT_WRITE, HAT_PRELOAD);
	if (error != 0) {
		if (swresv != 0)
			anon_unresv(swresv);
		return(error);
	}

	if (a->cred) {
		cred = a->cred;
		crhold(cred);
	} else {
		cred = crgetcred();
	}
	
	/* Inform the vnode of the new mapping */
	if (a->vp) {
		error = VOP_ADDMAP(a->vp, a->offset & PAGEMASK, seg->s_as,
			seg->s_base, seg->s_size, a->prot,
				a->maxprot, a->type, cred);
		if (error) {
			if (swresv != 0)
				anon_unresv(swresv);
			crfree(cred);
			hat_unload(seg, seg->s_base, seg->s_size, HAT_NOFLAGS);
			return (error);
		}
	}

	/*
	 * If more than one segment in the address space, and
	 * they're adjacent virtually, try to concatenate them.
	 * Don't concatenate if an explicit anon_map structure
	 * was supplied (e.g., SystemV shared memory).
	 *
	 * We also don't try concatenation if this is a segment
	 * for the kernel's address space.  This is a kludge
	 * because the kernel has several threads of control
	 * active at the same time and we can get in trouble
	 * if we reallocate the anon_map while another process
	 * is trying to fill the old anon_map in.
	 * XXX - need as/seg locking to fix the general problem of
	 * multiple threads in an address space instead of this kludge.
	 */
	if ((seg->s_prev != seg) && (a->amp == NULL) && (seg->s_as != &kas)) {
		register struct seg *pseg, *nseg;

		/* first, try to concatenate the previous and new segments */
		pseg = seg->s_prev;
		if (pseg->s_base + pseg->s_size == seg->s_base &&
		    pseg->s_ops == &segvn_ops &&
		    segvn_extend_prev(pseg, seg, a, swresv) == 0) {
			/* success! now try to concatenate with following seg */
			crfree(cred);
			nseg = pseg->s_next;
			if (nseg != pseg && nseg->s_ops == &segvn_ops &&
			    pseg->s_base + pseg->s_size == nseg->s_base)
				(void) segvn_concat(pseg, nseg);
			return (0);
		}
		/* failed, so try to concatenate with following seg */
		nseg = seg->s_next;
		if (seg->s_base + seg->s_size == nseg->s_base &&
		    nseg->s_ops == &segvn_ops &&
		    segvn_extend_next(seg, nseg, a, swresv) == 0) {
			crfree(cred);
			return (0);
		}
	}

	if (a->vp != NULL) {
		VN_HOLD(a->vp);
	}
	svd = (struct segvn_data *)kmem_fast_alloc((caddr_t *)&segvn_freelist,
	    sizeof (*segvn_freelist), segvn_freeincr, KM_SLEEP);
	lock_init(&svd->lock);
	seg->s_ops = &segvn_ops;
	seg->s_data = (char *)svd;

	svd->vp = a->vp;
	svd->offset = a->offset & PAGEMASK;
	svd->prot = a->prot;
	svd->maxprot = a->maxprot;
	svd->pageprot = 0;
	svd->type = a->type;
	svd->vpage = NULL;
	svd->cred = cred;

	if ((svd->amp = a->amp) == NULL) {
		svd->anon_index = 0;
		if (svd->type == MAP_SHARED) {
			svd->swresv = 0;
			/*
			 * Shared mappings to a vp need no other setup.
			 * If we have a shared mapping to an anon_map object
			 * which hasn't been allocated yet,  allocate the
			 * struct now so that it will be properly shared
			 * by remembering the swap reservation there.
			 */
			if (a->vp == NULL) {
				anonmap_alloc(seg, swresv);
			}
		} else {
			/*
			 * Private mapping (with or without a vp).
			 * Allocate anon_map when needed.
			 */
			svd->swresv = swresv;
		}
	} else  {
		u_int anon_num;

		/*
		 * Mapping to an existing anon_map structure without a vp.
		 * For now we will insure that the segment size isn't larger
		 * than the size - offset gives us.  Later on we may wish to
		 * have the anon array dynamically allocated itself so that
		 * we don't always have to allocate all the anon pointer slots.
		 * This of course involves adding extra code to check that we
		 * aren't trying to use an anon pointer slot beyond the end
		 * of the currently allocated anon array.
		 */
		if ((a->amp->size - a->offset) < seg->s_size)
			cmn_err(CE_PANIC, "segvn_create anon_map size");

		anon_num = btopr(a->offset);

		if (a->type == MAP_SHARED) {
			/*
			 * SHARED mapping to a given anon_map.
			 */
			a->amp->refcnt++;
			svd->anon_index = anon_num;
			svd->swresv = 0;
		} else {
			/*
			 * PRIVATE mapping to a given anon_map.
			 * Make sure that all the needed anon
			 * structures are created (so that we will
			 * share the underlying pages if nothing
			 * is written by this mapping) and then
			 * duplicate the anon array as is done
			 * when a privately mapped segment is dup'ed.
			 */
			register struct anon **app;
			register addr_t addr;
			addr_t eaddr;

			anonmap_alloc(seg, 0);
			svd->anon_index = 0;
			svd->swresv = swresv;

			app = &a->amp->anon[anon_num];
			eaddr = seg->s_base + seg->s_size;

			for (addr = seg->s_base; addr < eaddr;
			    addr += PAGESIZE, app++) {
				struct page *pp;

				if (*app != NULL)
					continue;

				/*
				 * Allocate the anon struct now.
				 * Might as well load up translation
				 * to the page while we're at it...
				 */
				pp = anon_zero(seg, addr, app);
				if (*app == NULL)
					cmn_err(CE_PANIC,
						"segvn_create anon_zero");
				hat_memload(seg, addr, pp,
				    svd->prot & ~PROT_WRITE, 0);
			}

			anon_dup(&a->amp->anon[anon_num], svd->amp->anon,
			    seg->s_size);
		}
	}

	return (0);
}

/*
 * Concatenate two existing segments, if possible.
 * Return 0 on success.
 */
STATIC int
segvn_concat(seg1, seg2)
	struct seg *seg1, *seg2;
{
	register struct segvn_data *svd1, *svd2;
	register u_int size;
	register struct anon_map *amp1, *amp2;

	svd1 = (struct segvn_data *)seg1->s_data;
	svd2 = (struct segvn_data *)seg2->s_data;

	/* both segments exist, try to merge them */
#define	incompat(x)	(svd1->x != svd2->x)
	if (incompat(vp) || incompat(maxprot) || incompat(prot) ||
	    incompat(type))
		return (-1);
	/* XXX - should also check cred */
#undef incompat
	/* vp == NULL implies zfod, offset doesn't matter */
	if (svd1->vp != NULL &&
	    svd1->offset + seg1->s_size != svd2->offset)
		return (-1);
	amp1 = svd1->amp;
	amp2 = svd2->amp;
	/* XXX - for now, reject if any private pages.  could merge. */
	if (amp1 != NULL || amp2 != NULL)
		return (-1);
	/* reject if vpage exists.  could merge. */
	if ((svd1->vpage != NULL) || (svd2->vpage != NULL))
		return (-1);
	/* all looks ok, merge second into first */
	svd1->swresv += svd2->swresv;
	svd2->swresv = 0;	/* so seg_free doesn't release */
	size = seg2->s_size;
	seg_free(seg2);
	seg1->s_size += size;
	return (0);
}

/*
 * Extend the previous segment (seg1) to include the
 * new segment (seg2 + a), if possible.
 * Return 0 on success.
 */
STATIC int
segvn_extend_prev(seg1, seg2, a, swresv)
	struct seg *seg1, *seg2;
	register struct segvn_crargs *a;
	u_int swresv;
{
	register struct segvn_data *svd1;
	register u_int size;
	register struct anon_map *amp1;

	/* second segment is new, try to extend first */
	svd1 = (struct segvn_data *)seg1->s_data;
	/* XXX - should also check cred */
	if (svd1->vp != a->vp || svd1->maxprot != a->maxprot ||
	    svd1->prot != a->prot || svd1->type != a->type)
		return (-1);
	/* vp == NULL implies zfod, offset doesn't matter */
	if (svd1->vp != NULL &&
	    svd1->offset + seg1->s_size != (a->offset & PAGEMASK))
		return (-1);
	if (svd1->vpage != NULL) {
		return (-1);
	}
	amp1 = svd1->amp;
	if (amp1) {
		/*
		 * segment has private pages, can
		 * data structures be expanded?
		 */
		if (amp1->refcnt > 1 && amp1->size != seg1->s_size)
			return (-1);
		if (amp1->size - ctob(svd1->anon_index) <
		    seg1->s_size + seg2->s_size) {
			struct anon **aa;
			u_int asize;

			/*
			 * We need a bigger anon array.  Allocate a new
			 * one with anon_slop worth of slop at the
			 * end so it will be easier to expand in
			 * place the next time we need to do this.
			 */
			asize = seg1->s_size + seg2->s_size + anon_slop;
			aa = (struct anon **)
			    kmem_zalloc((u_int)btop(asize) *
			    sizeof (struct anon *), KM_SLEEP);
			bcopy((caddr_t)(amp1->anon + svd1->anon_index),
			    (caddr_t)aa,
			    btop(seg1->s_size) * sizeof (struct anon *));
			kmem_free((caddr_t)amp1->anon,
			    btop(amp1->size) * sizeof (struct anon *));
			amp1->anon = aa;
			amp1->size = asize;
			svd1->anon_index = 0;
		} else {
			/*
			 * Can just expand anon array in place.
			 * Clear out anon slots after the end
			 * of the currently used slots.
			 */
			bzero((caddr_t)(amp1->anon + svd1->anon_index +
			    seg_pages(seg1)),
			    seg_pages(seg2) * sizeof (struct anon *));
		}
	}
	size = seg2->s_size;
	seg_free(seg2);
	seg1->s_size += size;
	svd1->swresv += swresv;
	return (0);
}

/*
 * Extend the next segment (seg2) to include the
 * new segment (seg1 + a), if possible.
 * Return 0 on success.
 */
STATIC int
segvn_extend_next(seg1, seg2, a, swresv)
	struct seg *seg1, *seg2;
	register struct segvn_crargs *a;
	u_int swresv;
{
	register struct segvn_data *svd2 = (struct segvn_data *)seg2->s_data;
	register u_int size;
	register struct anon_map *amp2;

	/* first segment is new, try to extend second */
	/* XXX - should also check cred */
	if (svd2->vp != a->vp || svd2->maxprot != a->maxprot ||
	    svd2->prot != a->prot || svd2->type != a->type)
		return (-1);
	/* vp == NULL implies zfod, offset doesn't matter */
	if (svd2->vp != NULL &&
	    (a->offset & PAGEMASK) + seg1->s_size != svd2->offset)
		return (-1);
	if (svd2->vpage != NULL) {
		return (-1);
	}
	amp2 = svd2->amp;
	if (amp2) {
		/*
		 * segment has private pages, can
		 * data structures be expanded?
		 */
		if (amp2->refcnt > 1)
			return (-1);
		if (ctob(svd2->anon_index) < seg1->s_size) {
			struct anon **aa;
			u_int asize;

			/*
			 * We need a bigger anon array.  Allocate a new
			 * one with anon_slop worth of slop at the
			 * beginning so it will be easier to expand in
			 * place the next time we need to do this.
			 */
			asize = seg1->s_size + seg2->s_size + anon_slop;
			aa = (struct anon **)
			    kmem_zalloc((u_int)btop(asize) *
			    sizeof (struct anon *), KM_SLEEP);
			bcopy((caddr_t)(amp2->anon + svd2->anon_index),
			    (caddr_t)(aa + btop(anon_slop) + seg_pages(seg1)),
			    btop(seg2->s_size) * sizeof (struct anon *));
			kmem_free((caddr_t)amp2->anon,
			    btop(amp2->size) * sizeof (struct anon *));
			amp2->anon = aa;
			amp2->size = asize;
			svd2->anon_index = btop(anon_slop);
		} else {
			/*
			 * Can just expand anon array in place.
			 * Clear out anon slots going backwards
			 * towards the beginning of the array.
			 */
			bzero((caddr_t)(amp2->anon + svd2->anon_index -
			    seg_pages(seg1)),
			    seg_pages(seg1) * sizeof (struct anon *));
			svd2->anon_index -= seg_pages(seg1);
		}
	}
	size = seg1->s_size;
	seg_free(seg1);
	seg2->s_size += size;
	seg2->s_base -= size;
	svd2->offset -= size;
	svd2->swresv += swresv;
	return (0);
}

/*
 * Allocate and initialize an anon_map structure for seg
 * associating the given swap reservation with the new anon_map.
 */
STATIC void
anonmap_alloc(seg, swresv)
	register struct seg *seg;
	u_int swresv;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;

	svd->amp = (struct anon_map *)kmem_fast_alloc(
	    (caddr_t *)&anonmap_freelist,
	    sizeof (*anonmap_freelist), anonmap_freeincr, KM_SLEEP);

	svd->amp->refcnt = 1;
	svd->amp->size = seg->s_size;
	svd->amp->anon = (struct anon **)kmem_zalloc(
		(u_int)(seg_pages(seg) * sizeof (struct anon *)), KM_SLEEP);
	svd->amp->swresv = swresv;
}

STATIC int
segvn_dup(seg, newseg)
	struct seg *seg, *newseg;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct segvn_data *newsvd;
	register u_int npages = seg_pages(seg);
	int error = 0;

	/*
	 * If segment has anon reserved,
	 * reserve more for the new seg.
	 */
	if (svd->swresv && anon_resv(svd->swresv) == 0)
			return (ENOMEM);
	newsvd =
	    (struct segvn_data *)kmem_fast_alloc((caddr_t *)&segvn_freelist,
	    sizeof (*segvn_freelist), segvn_freeincr, KM_SLEEP);
	lock_init(&newsvd->lock);
	newseg->s_ops = &segvn_ops;
	newseg->s_data = (char *)newsvd;

	if ((newsvd->vp = svd->vp) != NULL) {
		VN_HOLD(svd->vp);
	}
	newsvd->offset = svd->offset;
	newsvd->prot = svd->prot;
	newsvd->maxprot = svd->maxprot;
	newsvd->pageprot = svd->pageprot;
	newsvd->type = svd->type;
	newsvd->cred = svd->cred;
	crhold(newsvd->cred);
	newsvd->swresv = svd->swresv;
	if ((newsvd->amp = svd->amp) == NULL) {
		/*
		 * Not attaching to a shared anon object.
		 */
		newsvd->anon_index = 0;
	} else {
		if (svd->type == MAP_SHARED) {
			svd->amp->refcnt++;
			newsvd->anon_index = svd->anon_index;
		} else {
			/*
			 * Allocate and initialize new anon_map structure.
			 */
			anonmap_alloc(newseg, 0);
			newsvd->anon_index = 0;

			hat_chgprot(seg, seg->s_base, seg->s_size, ~PROT_WRITE);

			anon_dup(&svd->amp->anon[svd->anon_index],
			    newsvd->amp->anon, seg->s_size);
		}
	}
	/*
	 * If necessary, create a vpage structure for the new segment.
	 * Do not copy any page lock indications.
	 */
	if (svd->vpage != NULL) {
		register u_int i;
		register struct vpage *ovp = svd->vpage;
		register struct vpage *nvp;

		nvp = newsvd->vpage = (struct vpage *)kmem_alloc(
			(npages * sizeof (struct vpage)), KM_SLEEP);
		for (i = 0; i < npages; i++) {
			*nvp = *ovp++;
			(nvp++)->vp_pplock = 0;
		}
	} else
		newsvd->vpage = NULL;

	/* Inform the vnode of the new mapping */
	if (newsvd->vp)
		error = VOP_ADDMAP(newsvd->vp, newsvd->offset, newseg->s_as,
			newseg->s_base, newseg->s_size, newsvd->prot,
				newsvd->maxprot, newsvd->type, newsvd->cred);
	return (error);
}

STATIC int
segvn_unmap(seg, addr, len)
	register struct seg *seg;
	register addr_t addr;
	u_int len;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct segvn_data *nsvd;
	register struct seg *nseg;
	register u_int	opages,		/* old segment size in pages */
			npages,		/* new segment size in pages */
			dpages;		/* pages being deleted (unmapped)*/
	struct anon **app;
	addr_t nbase;
	u_int nsize;


	/*
	 * Check for bad sizes
	 */
	if (addr < seg->s_base || addr + len > seg->s_base + seg->s_size ||
	    (len & PAGEOFFSET) || ((u_int)addr & PAGEOFFSET))
		cmn_err(CE_PANIC, "segvn_unmap");

	/* Inform the vnode of the unmapping. */
	if (svd->vp) {
		if (VOP_DELMAP(svd->vp, svd->offset, seg->s_as, addr, len,
				svd->prot, svd->maxprot, svd->type, svd->cred) != 0)
			cmn_err(CE_WARN, "segvn_unmap VOP_DELMAP");
	}
	/*
	 * Remove any page locks set through this mapping.
	 */
	(void) segvn_lockop(seg, addr, len, 0, MC_UNLOCK,
				(ulong *)NULL, (size_t)NULL);

	/*
	 * Unload any hardware translations in the range to be taken out.
	 */
	hat_unload(seg, addr, len, HAT_NOFLAGS);

	/*
	 * Check for entire segment
	 */
	if (addr == seg->s_base && len == seg->s_size) {
		seg_free(seg);
		return (0);
	}

	opages = seg_pages(seg);
	dpages = btop(len);
	npages = opages - dpages;

	/*
	 * Check for beginning of segment
	 */
	if (addr == seg->s_base) {
		if (svd->vpage != NULL) {
			register uint nbytes;
			register struct vpage *ovpage;

			ovpage = svd->vpage;	/* keep pointer to vpage */

			nbytes = npages * sizeof (struct vpage);
			svd->vpage = (struct vpage *)kmem_alloc(nbytes, KM_SLEEP);
			bcopy((caddr_t)&ovpage[dpages],
			    (caddr_t)svd->vpage, nbytes);

			/* free up old vpage */
			kmem_free(ovpage, opages * sizeof (struct vpage));
		}
		if (svd->amp != NULL && (svd->amp->refcnt == 1 ||
		    svd->type == MAP_PRIVATE)) {
			/*
			 * Free up now unused parts of anon_map array.
			 */
			app = &svd->amp->anon[svd->anon_index];
			anon_free(app, len);
			svd->anon_index += dpages;
		}
		if (svd->vp != NULL) {
			free_vp_pages(svd->vp, svd->offset, len);
			svd->offset += len;
		}

		if (svd->swresv) {
			anon_unresv(len);
			svd->swresv -= len;
		}

		seg->s_base += len;
		seg->s_size -= len;
		return (0);
	}

	/*
	 * Check for end of segment
	 */
	if (addr + len == seg->s_base + seg->s_size) {
		if (svd->vpage != NULL) {
			register uint nbytes;
			register struct vpage *ovpage;

			ovpage = svd->vpage;	/* keep pointer to vpage */

			nbytes = npages * sizeof (struct vpage);
			svd->vpage = (struct vpage *)kmem_alloc(nbytes, KM_SLEEP);
			bcopy((caddr_t)ovpage, (caddr_t)svd->vpage, nbytes);

			/* free up old vpage */
			kmem_free(ovpage, opages * sizeof (struct vpage));

		}
		if (svd->amp != NULL && (svd->amp->refcnt == 1 ||
		    svd->type == MAP_PRIVATE)) {
			/*
			 * Free up now unused parts of anon_map array
			 */
			app = &svd->amp->anon[svd->anon_index + npages];
			anon_free(app, len);
		}

		if (svd->swresv) {
			anon_unresv(len);
			svd->swresv -= len;
		}

		seg->s_size -= len;
		if (svd->vp != NULL) {
			free_vp_pages(svd->vp, svd->offset + seg->s_size, len);
		}

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
		cmn_err(CE_PANIC, "segvn_unmap seg_alloc");

	nseg->s_ops = seg->s_ops;
	nsvd = (struct segvn_data *)kmem_fast_alloc((caddr_t *)&segvn_freelist,
	    sizeof (*segvn_freelist), segvn_freeincr, KM_SLEEP);
	nseg->s_data = (char *)nsvd;
	lock_init(&nsvd->lock);
	nsvd->pageprot = svd->pageprot;
	nsvd->prot = svd->prot;
	nsvd->maxprot = svd->maxprot;
	nsvd->type = svd->type;
	nsvd->vp = svd->vp;
	nsvd->cred = svd->cred;
	nsvd->offset = svd->offset + nseg->s_base - seg->s_base;
	nsvd->swresv = 0;
	if (svd->vp != NULL) {
		free_vp_pages(svd->vp, svd->offset + seg->s_size, len);
		VN_HOLD(nsvd->vp);
	}
	crhold(svd->cred);

	if (svd->vpage == NULL)
		nsvd->vpage = NULL;
	else {
		/* need to split vpage into two arrays */
		register uint nbytes;
		register struct vpage *ovpage;

		ovpage = svd->vpage;	/* keep pointer to vpage */

		npages = seg_pages(seg);	/* seg has shrunk */
		nbytes = npages * sizeof (struct vpage);
		svd->vpage = (struct vpage *)kmem_alloc(nbytes, KM_SLEEP);

		bcopy((caddr_t)ovpage, (caddr_t)svd->vpage, nbytes);

		npages = seg_pages(nseg);
		nbytes = npages * sizeof (struct vpage);
		nsvd->vpage = (struct vpage *)kmem_alloc(nbytes, KM_SLEEP);

		bcopy((caddr_t)&ovpage[opages - npages],
		    (caddr_t)nsvd->vpage, nbytes);

		/* free up old vpage */
		kmem_free(ovpage, opages * sizeof (struct vpage));
	}

	if (svd->amp == NULL) {
		nsvd->amp = NULL;
		nsvd->anon_index = 0;
	} else {
		/*
		 * Share the same anon_map structure.
		 */
		opages = btop(addr - seg->s_base);
		npages = btop(nseg->s_base - seg->s_base);

		if (svd->amp->refcnt == 1 ||
		    svd->type == MAP_PRIVATE) {
			/*
			 * Free up now unused parts of anon_map array
			 */
			app = &svd->amp->anon[svd->anon_index + opages];
			anon_free(app, len);
		}
		nsvd->amp = svd->amp;
		nsvd->anon_index = svd->anon_index + npages;
		nsvd->amp->refcnt++;
	}
	if (svd->swresv) {
		if (seg->s_size + nseg->s_size + len != svd->swresv)
			cmn_err(CE_PANIC, "segvn_unmap: cannot split swap reservation");
		anon_unresv(len);
		svd->swresv = seg->s_size;
		nsvd->swresv = nseg->s_size;
	}

	/*
	 * Now we do something so that all the translations which used
	 * to be associated with seg but are now associated with nseg.
	 */
	hat_newseg(seg, nseg->s_base, nseg->s_size, nseg);

	return (0);			/* I'm glad that's all over with! */
}

STATIC void
segvn_free(seg)
	struct seg *seg;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct anon **app;
	u_int npages = seg_pages(seg);

	/*
	 * Be sure to unlock pages. XXX Why do things get free'ed instead
	 * of unmapped? XXX
	 */
	(void) segvn_lockop(seg, seg->s_base, seg->s_size,
			0, MC_UNLOCK, (ulong *)NULL, (size_t)NULL);

	/*
	 * Deallocate the vpage and anon pointers if necessary and possible.
	 */
	if (svd->vpage != NULL)
		kmem_free((caddr_t)svd->vpage, npages * sizeof (struct vpage));
	if (svd->amp != NULL) {
		/*
		 * If there are no more references to this anon_map
		 * structure, then deallocate the structure after freeing
		 * up all the anon slot pointers that we can.
		 */
		if (--svd->amp->refcnt == 0) {
			if (svd->type == MAP_PRIVATE) {
				/*
				 * Private - we only need to anon_free
				 * the part that this segment refers to.
				 */
				anon_free(&svd->amp->anon[svd->anon_index],
				    seg->s_size);
			} else {
				/*
				 * Shared - anon_free the entire
				 * anon_map's worth of stuff and 
				 * release any swap reservation.
				 */
				anon_free(svd->amp->anon, svd->amp->size);
				if (svd->amp->swresv)
					anon_unresv(svd->amp->swresv);
			}
			kmem_free((caddr_t)svd->amp->anon,
			    btop(svd->amp->size) * sizeof (struct anon *));
			kmem_fast_free((caddr_t *)&anonmap_freelist,
			    (caddr_t)svd->amp);
		} else if (svd->type == MAP_PRIVATE) {
			/*
			 * We had a private mapping which still has
			 * a held anon_map so just free up all the
			 * anon slot pointers that we were using.
			 */
			app = &svd->amp->anon[svd->anon_index];
			anon_free(app, seg->s_size);
		}
	}

	/*
	 * Release swap reservation.
	 */
	if (svd->swresv)
		anon_unresv(svd->swresv);
	/*
	 * Release claim on vnode, credentials, and finally free the
	 * private data.
	 */
	if (svd->vp != NULL) {
		free_vp_pages(svd->vp, svd->offset, seg->s_size);
		VN_RELE(svd->vp);
	}
	crfree(svd->cred);
	kmem_fast_free((caddr_t *)&segvn_freelist, (caddr_t)svd);
}

/*
 * Do a F_SOFTUNLOCK call over the range requested.
 * The range must have already been F_SOFTLOCK'ed.
 */
STATIC void
segvn_softunlock(seg, addr, len, rw)
	struct seg *seg;
	addr_t addr;
	u_int len;
	enum seg_rw rw;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct anon **app;
	register page_t *pp;
	register struct vpage *vpage;
	register addr_t adr;
	struct vnode *vp;
	u_int offset;

	if (svd->amp != NULL)
		app = &svd->amp->anon[svd->anon_index + seg_page(seg, addr)];
	else
		app = NULL;

	if (svd->vpage != NULL)
		vpage = &svd->vpage[seg_page(seg, addr)];
	else
		vpage = NULL;

	for (adr = addr; adr < addr + len; adr += PAGESIZE) {
		if (app != NULL && *app != NULL)
			swap_xlate(*app, &vp, &offset);
		else {
			vp = svd->vp;
			offset = svd->offset + (adr - seg->s_base);
		}

		/*
		 * For now, we just kludge here by finding the page
		 * ourselves since we would not find the page using
		 * page_find() if someone has page_abort()'ed it.
		 * XXX - need to redo things to avoid this mess.
		 */
		for (pp = page_hash[PAGE_HASHFUNC(vp, offset)]; pp != NULL;
		    pp = pp->p_hash)
			if (pp->p_vnode == vp && pp->p_offset == offset)
				break;
		if (pp == NULL || pp->p_pagein || pp->p_free)
			cmn_err(CE_PANIC, "segvn_softunlock");

		if (vpage != NULL)
			(void) vpage_lock(&svd->lock, vpage);
		if (rw == S_WRITE) {
			pp->p_mod = 1;
			if (vpage != NULL)
				vpage->vp_mod = 1;
		}
		if (rw != S_OTHER) {
			trace4(TR_PG_SEGVN_FLT, pp, vp, offset, 1);
			pp->p_ref = 1;
			if (vpage != NULL)
				vpage->vp_ref = 1;
		}
		hat_unlock(seg, adr);
		PAGE_RELE(pp);
		availrmem++;
		pages_pp_kernel--;
		if (vpage != NULL) {
			vpage_unlock(&svd->lock, vpage);
			vpage++;
		}
		if (app != NULL)
			app++;
	}
}

/*
 * Returns true if the app array has some non-anonymous memory
 * The offp and lenp paramters are in/out paramters.  On entry
 * these values represent the starting offset and length of the
 * mapping.  When true is returned, these values may be modified
 * to be the largest range which includes non-anonymous memory.
 */
STATIC int
non_anon(app, offp, lenp)
	register struct anon **app;
	u_int *offp, *lenp;
{
	register int i, el;
	int low, high;

	low = -1;
	for (i = 0, el = *lenp; i < el; i += PAGESIZE) {
		if (*app++ == NULL) {
			if (low == -1)
				low = i;
			high = i;
		}
	}
	if (low != -1) {
		/*
		 * Found at least one non-anon page.
		 * Set up the off and len return values.
		 */
		if (low != 0)
			*offp += low;
		*lenp = high - low + PAGESIZE;
		return (1);
	}
	return (0);
}

#define	PAGE_HANDLED	((page_t *)-1)

/*
 * Release all the pages in the NULL terminated ppp list
 * which haven't already been converted to PAGE_HANDLED.
 */
STATIC void
segvn_pagelist_rele(ppp)
	register page_t **ppp;
{

	for (; *ppp != NULL; ppp++) {
		if (*ppp != PAGE_HANDLED)
			PAGE_RELE(*ppp);
	}
}

STATIC int stealCOW = 1;

/*
 * Handles all the dirty work of getting the right
 * anonymous pages and loading up the translations.
 * This routine is called only from segvn_fault()
 * when looping over the range of addresses requested.
 *
 * The basic algorithm here is:
 * 	If this is an anon_zero case
 *		Call anon_zero to allocate page
 *		Load up translation
 *		Return
 *	endif
 *	If this is an anon page
 *		Use anon_getpage to get the page
 *	else if page needed
 *		Find page in pl[] list passed in
 *	endif
 *	If not a COW
 *		Load up the translation to the page
 *		return
 *	endif
 *	Call anon_private to handle COW
 *	Transfer lock counts
 *	Load up (writable) translation to new page
 */
STATIC int
segvn_faultpage(seg, addr, off, app, vpage, pl, vpprot, type, rw)
	struct seg *seg;
	addr_t addr;
	u_int off;
	struct anon **app;
	struct vpage *vpage;
	page_t *pl[];
	u_int vpprot;
	enum fault_type type;
	enum seg_rw rw;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register page_t *pp, **ppp;
	int pageflags = 0;
	page_t *anon_pl[1 + 1];
	page_t *opp = NULL;		/* original object page */
	u_int prot;
	int err;
	int COW;
	enum seg_rw arw;

	/*
	 * Initialize protection value for this page.
	 * If we have per page protection values check it now.
	 */
	if (svd->pageprot) {
		u_int protchk;

		switch (rw) {
		case S_READ:
			protchk = PROT_READ;
			break;
		case S_WRITE:
			protchk = PROT_WRITE;
			break;
		case S_EXEC:
			protchk = PROT_EXEC;
			break;
		case S_OTHER:
		default:
			protchk = PROT_READ | PROT_WRITE | PROT_EXEC;
			break;
		}

		prot = vpage->vp_prot;
		if ((prot & protchk) == 0)
			return (FC_PROT);	/* illegal access type */
	} else {
		prot = svd->prot;
	}
	
	if (type == F_SOFTLOCK)
		if (availrmem - 1 < tune.t_minarmem)
			return (FC_MAKE_ERR(ENOMEM));	/* out of real memory */
		else { 
			--availrmem;
			pages_pp_kernel++;
		}

	if (svd->vp == NULL && *app == NULL) {
		/*
		 * Allocate a (normally) writable
		 * anonymous page of zeroes.
		 */
		if ((pp = anon_zero(seg, addr, app)) == NULL) {
			err = ENOMEM;
			goto out;	/* out of swap space */
		}
		pp->p_ref = 1;
		if (type == F_SOFTLOCK) {
			/*
			 * Load up the translation keeping it
			 * locked and don't PAGE_RELE the page.
			 */
			hat_memload(seg, addr, pp, prot, HAT_LOCK);
		} else {
			hat_memload(seg, addr, pp, prot, HAT_NOFLAGS);
			PAGE_RELE(pp);
		}
		return (0);
	}

	/*
	 * Obtain the page structure via anon_getpage() if it is
	 * a private copy of an object (the result of a previous
	 * copy-on-write), or from the pl[] list passed in if it
	 * is from the original object (i.e., not a private copy).
	 */
	if (app != NULL && *app != NULL) {
		arw = rw;
		if (svd->type == MAP_SHARED) {
			/*
			 * If this is a shared mapping to an
			 * anon_map, then force write permissions
			 * returned by anon_getpage().
			 */
			arw = S_WRITE;
		}

		err = anon_getpage(app, &vpprot, anon_pl, PAGESIZE,
		    seg, addr, arw, svd->cred);
		if (err)
			goto out;
		if (svd->type == MAP_SHARED) {
			/*
			 * If this is a shared mapping to an
			 * anon_map, then ignore the write
			 * permissions returned by anon_getpage().
			 * They apply to the private mappings
			 * of this anon_map.
			 */
			vpprot |= PROT_WRITE;
		}
		opp = anon_pl[0];
	} else {
		/*
		 * Find original page.  We must be bringing it in
		 * from the list in pl[].
		 */
		for (ppp = pl; (opp = *ppp) != NULL; ppp++) {
			if (opp == PAGE_HANDLED)
				continue;
			ASSERT(opp->p_vnode == svd->vp); /* XXX */
			if (opp->p_offset == off)
				break;
		}
		if (opp == NULL)
			cmn_err(CE_PANIC,"segvn_faultpage not found");
		*ppp = PAGE_HANDLED;
	}

	ASSERT(opp != NULL);
	ASSERT(opp->p_keepcnt > 0);

	trace4(TR_PG_SEGVN_FLT, opp, opp->p_vnode, opp->p_offset, 0);
	opp->p_ref = 1;

	/*
	 * The fault is treated as a copy-on-write fault if a
	 * write occurs on a private segment and the object
	 * page is write protected.  We assume that fatal
	 * protection checks have already been made.
	 */

	COW = (rw == S_WRITE && svd->type == MAP_PRIVATE &&
	    (vpprot & PROT_WRITE) == 0);

	/*
	 * If not a copy-on-write case load the translation
	 * and return.
	 */
	if (COW == 0) {
		if (type == F_SOFTLOCK) {
			/*
			 * Load up the translation keeping it
			 * locked and don't PAGE_RELE the page.
			 */
			hat_memload(seg, addr, opp, prot & vpprot, HAT_LOCK);
		} else {
			hat_memload(seg, addr, opp, prot & vpprot, HAT_NOFLAGS);
			PAGE_RELE(opp);
		}

		return (0);
	}

	/*
	 * This is the original page we need.
	 * Check to see if we really want to
	 * steal this page.
	 */
	if (stealCOW && app != NULL && *app == NULL &&
	    freemem < minfree && opp->p_keepcnt == 1 &&
	    opp->p_mod == 0 && opp->p_mapping == NULL &&
	    opp->p_lckcnt == 0 && opp->p_cowcnt == 0)
		pageflags |= STEAL_PAGE;

	ASSERT(app != NULL);

	/*
	 * Copy-on-write case: anon_private() will copy the contents
	 * of the original page into a new page.  The page fault which
	 * could occur during the copy is prevented by ensuring that
	 * a translation for the original page is loaded and locked.
	 */
	hat_memload(seg, addr, opp, prot & vpprot, HAT_LOCK);

	/*
	 * If we have a vpage pointer, see if it indicates that we have
	 * ``locked'' the page we map -- if so, tell anon_private to
	 * transfer the locking resource to the new page.  
	 */
	if (vpage != NULL) {
		if (vpage_lock(&svd->lock, vpage) < 0) {
			/*
			 * In the multiprocessor case we should probably
			 * just unlock and return and assume that another
			 * processor got here first.  But for now, we
			 * simply panic in this situation.
			 */
			cmn_err(CE_PANIC, "segvn_faultpage vpage_lock");
		}
		if (vpage->vp_pplock)
			pageflags |= LOCK_PAGE;
	}

	/*
	 * anon_private() will make a new copy of the contents of this
	 * page.  We hold the original page structure.  anon_private
	 * will release the old page if it can make the copy, and unlock
	 * and unload the old translation.
	 */
	pp = anon_private(app, seg, addr, opp, pageflags);

	if (vpage != NULL)
		vpage_unlock(&svd->lock, vpage);

	if (pp == NULL) {
		hat_unlock(seg, addr);
		PAGE_RELE(opp);	/* anon_private did not release it */
		err = ENOMEM;	/* out of swap space */
		goto out;
	}
	
	ASSERT(pp->p_keepcnt > 0);

	if (type == F_SOFTLOCK) {
		/*
		 * Load up the translation keeping it
		 * locked and don't PAGE_RELE the page.
		 */
		hat_memload(seg, addr, pp, prot, HAT_LOCK);
	} else {
		hat_memload(seg, addr, pp, prot, HAT_NOFLAGS);
		PAGE_RELE(pp);
	}

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
 *		Call segvn_softunlock
 *		Return
 *	endif
 *	Checking and set up work
 *	If we will need some non-anonymous pages
 *		Call VOP_GETPAGE over the range of non-anonymous pages
 *	endif
 *	Loop over all addresses requested
 *		Call segvn_faultpage passing in page list
 *		    to load up translations and handle anonymous pages
 *	endloop
 *	Load up translation to any additional pages in page list not
 *	    already handled that fit into this segment
 */

STATIC faultcode_t
segvn_fault(seg, addr, len, type, rw)
	struct seg *seg;
	addr_t addr;
	u_int len;
	enum fault_type type;
	enum seg_rw rw;
{
	struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register page_t **plp, **ppp, *pp;
	struct anon **app;
	u_int off;
	addr_t a;
	struct vpage *vpage;
	u_int vpprot, prot;
	int err;
	page_t *pl[PVN_GETPAGE_NUM + 1];
	u_int plsz, pl_alloc_sz;
	int page;

	/*
	 * First handle the easy stuff
	 */
	if (type == F_SOFTUNLOCK) {
		segvn_softunlock(seg, addr, len, rw);
		return (0);
	}

	/*
	 * If we have the same proections for the entire segment,
	 * insure that the access being attempted is legimate.
	 */
	if (svd->pageprot == 0) {
		u_int protchk;

		switch (rw) {
		case S_READ:
			protchk = PROT_READ;
			break;
		case S_WRITE:
			protchk = PROT_WRITE;
			break;
		case S_EXEC:
			protchk = PROT_EXEC;
			break;
		case S_OTHER:
		default:
			protchk = PROT_READ | PROT_WRITE | PROT_EXEC;
			break;
		}

		if ((svd->prot & protchk) == 0)
			return (FC_PROT);	/* illegal access type */
	}

	/*
	 * Check to see if we need to allocate an anon_map structure.
	 */
	if (svd->amp == NULL && (svd->vp == NULL ||
	    ((type == F_PROT || rw == S_WRITE) && svd->type == MAP_PRIVATE))) {
		/* ok, we need to do it */
		svd->amp = (struct anon_map *)kmem_fast_alloc(
		    (caddr_t *)&anonmap_freelist, sizeof (*anonmap_freelist),
		    anonmap_freeincr, KM_SLEEP);
		svd->amp->refcnt = 1;
		svd->amp->size = seg->s_size;
		svd->amp->anon = (struct anon **)kmem_zalloc((u_int)
		    (seg_pages(seg) * sizeof (struct anon *)), KM_SLEEP);
	}

	page = seg_page(seg, addr);
	if (svd->amp == NULL)
		app = NULL;
	else
		app = &svd->amp->anon[svd->anon_index + page];

	if (svd->vpage == NULL)
		vpage = NULL;
	else
		vpage = &svd->vpage[page];

	plp = pl;
	*plp = (page_t *)NULL;
	pl_alloc_sz = 0;
	off = svd->offset + (addr - seg->s_base);
	/*
	 * See if we need to call VOP_GETPAGE for
	 * *any* of the range being faulted on.
	 * We can skip all of this work if there
	 * was no original vnode.
	 */
	if (svd->vp != NULL) {
		u_int vp_off, vp_len;
		int dogetpage;

		if (len > ptob((sizeof (pl) / sizeof (pl[0])) - 1)) {
			/*
			 * Page list won't fit in local array,
			 * allocate one of the needed size.
			 */
			pl_alloc_sz = (btop(len) + 1) * sizeof (page_t *);
			plp = (page_t **)kmem_zalloc(pl_alloc_sz, KM_SLEEP);
			plsz = len;
		} else
			plsz = PVN_GETPAGE_SZ;

		vp_off = off;
		vp_len = len;

		if (app == NULL)
			dogetpage = 1;
		else if (len <= PAGESIZE)
			dogetpage = (*app == NULL);	/* inline non_anon() */
		else
			dogetpage = non_anon(app, &vp_off, &vp_len);

		if (dogetpage) {
			enum seg_rw arw;

			/*
			 * Need to get some non-anonymous pages.
			 * We need to make only one call to GETPAGE to do
			 * this to prevent certain deadlocking conditions
			 * when we are doing locking.  In this case
			 * non_anon() should have picked up the smallest
			 * range which includes all the non-anonymous
			 * pages in the requested range.  We have to
			 * be careful regarding which rw flag to pass in
			 * because on a private mapping, the underlying
			 * object is never allowed to be written.
			 */
			if (rw == S_WRITE && svd->type == MAP_PRIVATE) {
				arw = S_READ;
			} else {
				arw = rw;
			}
			trace3(TR_SEG_GETPAGE, seg, addr, TRC_SEG_FILE);
			err = VOP_GETPAGE(svd->vp, vp_off, vp_len, &vpprot,
			    plp, plsz, seg, addr + (vp_off - off), arw,
			    svd->cred);
			if (err) {
				segvn_pagelist_rele(plp);
				if (pl_alloc_sz)
					kmem_free((caddr_t)plp, pl_alloc_sz);
				return (FC_MAKE_ERR(err));
			}
			if (svd->type == MAP_PRIVATE)
				vpprot &= ~PROT_WRITE;
		}
	}

	/*
	 * N.B. at this time the plp array has all the needed non-anon
	 * pages in addition to (possibly) having some adjacent pages.
	 */

	/*
	 * Ok, now loop over the address range and handle faults
	 */
	for (a = addr; a < addr + len; a += PAGESIZE, off += PAGESIZE) {
		err = segvn_faultpage(seg, a, off, app, vpage, plp, vpprot,
		    type, rw);
		if (err) {
			if (type == F_SOFTLOCK && a > addr)
				segvn_softunlock(seg, addr, (u_int)(a - addr),
				    S_OTHER);
			segvn_pagelist_rele(plp);
			if (pl_alloc_sz)
				kmem_free((caddr_t)plp, pl_alloc_sz);
			return (err);
		}
		if (app)
			app++;
		if (vpage)
			vpage++;
	}

	/*
	 * Now handle any other pages in the list returned.
	 * If the page can be used, load up the translations now.
	 * Note that the for loop will only be entered if "plp"
	 * is pointing to a non-NULL page pointer which means that
	 * VOP_GETPAGE() was called and vpprot has been initialized.
	 */
	if (svd->pageprot == 0)
		prot = svd->prot & vpprot;
	for (ppp = plp; (pp = *ppp) != NULL; ppp++) {
		int diff;

		if (pp == PAGE_HANDLED)
			continue;

		diff = pp->p_offset - svd->offset;
		if (diff >= 0 && diff < seg->s_size) {
			ASSERT(svd->vp == pp->p_vnode);

			page = btop(diff);
			if (svd->pageprot)
				prot = svd->vpage[page].vp_prot & vpprot;

			if (svd->amp == NULL ||
			    svd->amp->anon[svd->anon_index + page] == NULL) {
				hat_memload(seg, seg->s_base + diff, pp,
				    prot, HAT_NOFLAGS);
			}
		}
		PAGE_RELE(pp);
	}

	if (pl_alloc_sz)
		kmem_free((caddr_t)plp, pl_alloc_sz);
	return (0);
}
/*
 * This routine is used to start I/O on pages asynchronously.
 */
STATIC faultcode_t
segvn_faulta(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct anon **app;
	int err;

	if (svd->amp != NULL) {
		app = &svd->amp->anon[svd->anon_index + seg_page(seg, addr)];
		if (*app != NULL) {
			err = anon_getpage(app, (u_int *)NULL,
			    (page_t **)NULL, 0, seg, addr, S_READ,
			    svd->cred);
			if (err)
				return (FC_MAKE_ERR(err));
			return (0);
		}
	}

	if (svd->vp == NULL)
		return (0);			/* zfod page - do nothing now */

	trace3(TR_SEG_GETPAGE, seg, addr, TRC_SEG_FILE);
	err = VOP_GETPAGE(svd->vp, svd->offset + (addr - seg->s_base),
	    PAGESIZE, (u_int *)NULL, (page_t **)NULL, 0, seg, addr,
	    S_OTHER, svd->cred);
	if (err)
		return (FC_MAKE_ERR(err));
	return (0);
}

STATIC void
segvn_unload(seg, addr, ref, mod)
	struct seg *seg;
	addr_t addr;
	u_int ref, mod;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register int page;
	register struct vpage *vpage;

	if (svd->vpage == NULL) {
		/*
		 * For now since we are doing a global page replacement
		 * policy, we don't worry about keeping track of virtual
		 * ref and modified bits.
		 */
		return;
	}

	page = seg_page(seg, addr);
	vpage = &svd->vpage[page];
	if (svd->amp != NULL) {
		register struct anon *ap;

		ap = svd->amp->anon[svd->anon_index + page];
		if (ap) {
			anon_unloadmap(ap, ref, mod);
			goto unloaded;
		}
	}
	pvn_unloadmap(svd->vp, svd->offset + (addr - seg->s_base), ref, mod);
unloaded:
	vpage->vp_mod |= mod;
	vpage->vp_ref |= ref;
}

STATIC int
segvn_setprot(seg, addr, len, prot)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, prot;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct vpage *svp, *evp;
	struct vnode *vp;

	if ((svd->maxprot & prot) != prot)
		return (EACCES);			/* violated maxprot */

	/*
	 * If it's a private mapping and we're making it writable
	 * and no swap space has been reserved, have to reserve
	 * it all now.  If it's private and we're removing write
	 * permission for the whole mapping and we haven't 
	 * modified any pages, we can release the swap space.
	 */
	if (svd->type == MAP_PRIVATE && (prot & PROT_WRITE) != 0
	  && svd->swresv == 0) {
		if (anon_resv(seg->s_size) == 0)
			return (EAGAIN);
		svd->swresv = seg->s_size;
	}

	if (addr == seg->s_base && len == seg->s_size && svd->pageprot == 0) {
		if (svd->type == MAP_PRIVATE && (prot & PROT_WRITE) == 0
		  && svd->swresv > 0 && svd->amp == NULL) {
			anon_unresv(svd->swresv);
			svd->swresv = 0;
		}
		if (svd->prot == prot)
			return (0);			/* all done */
		svd->prot = (u_char)prot;
	} else {
		struct anon **app;
		page_t *pp;
		u_int offset, off;
		/*
		 * A vpage structure exists or else the change does not
		 * involve the entire segment.  Establish a vpage structure
		 * if none is there.  Then, for each page in the range,
		 * adjust its individual permissions.  Note that write-
		 * enabling a MAP_PRIVATE page can affect the claims for
		 * locked down memory.  Overcommitting memory terminates
		 * the operation.  
		 */
		segvn_vpage(seg);
		if (svd->amp == NULL)
			app = NULL;
		else
			app = &svd->amp->anon[svd->anon_index + 
				seg_page(seg, addr)];
		offset = svd->offset + (addr - seg->s_base);
		evp = &svd->vpage[seg_page(seg, addr + len)];
		for (svp = &svd->vpage[seg_page(seg, addr)]; svp < evp; svp++) {
			if (svp->vp_pplock && (svp->vp_prot != prot) 
			  && (svd->type == MAP_PRIVATE)) {
				if (app && *app) 
					swap_xlate(*app, &vp, &off);
				else {
					vp = svd->vp;
					off = offset;
				}
				if ((pp = page_lookup(vp, off)) == NULL)
					cmn_err(CE_PANIC, " no page");
				if ((svp->vp_prot ^ prot) & PROT_WRITE)
					if (prot & PROT_WRITE) {
						if (!page_addclaim(pp)) 
							break;
					} else
						page_subclaim(pp);
			}
			svp->vp_prot = prot;
			offset += PAGESIZE;
			if (app)
				app++;
		}

		/*
		 * Did we terminate prematurely?  If so, simply unload
		 * the translations to the things we've updated so far.
		 */
		if (svp != evp) {
			len = (svp - &svd->vpage[seg_page(seg, addr)]) * 
			    PAGESIZE;
			if (len != 0)
				hat_unload(seg, addr, len, HAT_NOFLAGS);
			return (EAGAIN);
		}
	}

	if ((svd->type == MAP_PRIVATE && (prot & PROT_WRITE) != 0) ||
	    prot == 0) {
		/*
		 * Either private data with write access (in which case
		 * we need to throw out all former translations so that
		 * we get the right translations set up on fault and we
		 * don't allow write access to any copy-on-write pages
		 * that might be around) or we don't have permission
		 * to access the memory at all (in which case we have to
		 * unload any current translations that might exist).
		 */
		hat_unload(seg, addr, len, HAT_NOFLAGS);
	} else {
		/*
		 * A shared mapping or a private mapping in which write
		 * protection is going to be denied - just change all the
		 * protections over the range of addresses in question.
		 * Note that we can't actually enable write permissions
		 * here, since we have to take a protection fault in order
		 * to give filesystems a chance to allocate disk blocks.
		 */
		hat_chgprot(seg, addr, len, prot & ~PROT_WRITE);
	}

	return (0);
}

STATIC int
segvn_checkprot(seg, addr, len, prot)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, prot;
{
	struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct vpage *vp, *evp;

	/*
	 * If segment protection can be used, simply check against them.
	 */
	if (svd->pageprot == 0)
		return (((svd->prot & prot) != prot) ? EACCES : 0);

	/*
	 * Have to check down to the vpage level.
	 */
	evp = &svd->vpage[seg_page(seg, addr + len)];
	for (vp = &svd->vpage[seg_page(seg, addr)]; vp < evp; vp++)
		if ((vp->vp_prot & prot) != prot)
			return (EACCES);

	return (0);
}

STATIC int
segvn_getprot(seg, addr, len, protv)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, *protv;
{
 	struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	u_int pgno = seg_page(seg, addr+len) - seg_page(seg, addr) + 1;

        if (pgno != 0) {
		if (svd->pageprot == 0) {
                        do protv[--pgno] = svd->prot;
                        while (pgno != 0);
		} else {
                        register pgoff = seg_page(seg, addr);
			do {
                                pgno--;
                                protv[pgno] = svd->vpage[pgno+pgoff].vp_prot;
			} while (pgno != 0);
		}
	}
	return 0;
}

/* ARGSUSED */
STATIC off_t
segvn_getoffset(seg, addr)
	register struct seg *seg;
	addr_t addr;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;

	return (addr - seg->s_base + svd->offset);
}

/* ARGSUSED */
STATIC int
segvn_gettype(seg, addr)
	register struct seg *seg;
	addr_t addr;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;

	return svd->type;
}

/* ARGSUSED */
STATIC int
segvn_getvp(seg, addr, vpp)
	register struct seg *seg;
	addr_t addr;
	struct vnode **vpp;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;

	if ((*vpp = svd->vp) == (struct vnode *) NULL)
		return -1;

	return 0;
}

/*
 * Check to see if it makes sense to do kluster/read ahead to
 * addr + delta relative to the mapping at addr.  We assume here
 * that delta is a signed PAGESIZE'd multiple (which can be negative).
 *
 * For segvn, we currently "approve" of the action if we are
 * still in the segment and it maps from the same vp/off.
 */
STATIC int
segvn_kluster(seg, addr, delta)
	register struct seg *seg;
	register addr_t addr;
	int delta;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct anon *oap, *ap;
	register int pd;
	register u_int page;
	struct vnode *vp1, *vp2;
	u_int off1, off2;

	if (addr + delta < seg->s_base ||
	    addr + delta >= (seg->s_base + seg->s_size))
		return (-1);		/* exceeded segment bounds */

	pd = delta / PAGESIZE;		/* divide to preserve sign bit */
	page = seg_page(seg, addr);

	if (svd->type == MAP_SHARED)
		return (0);		/* shared mapping - all ok */

	if (svd->amp == NULL)
		return (0);		/* off original vnode */

	page += svd->anon_index;
	oap = svd->amp->anon[page];
	ap = svd->amp->anon[page + pd];

	if ((oap == NULL && ap != NULL) || (oap != NULL && ap == NULL))
		return (-1);		/* one with and one without an anon */

	if (oap == NULL)		/* implies that ap == NULL */
		return (0);		/* off original vnode */

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
 * 1)	When given a shared segment as argument, this routine will
 *	only succeed in swapping out pages for the last sharer of the
 *	segment.  (Previous callers will only have decremented mapping
 *	reference counts.)
 * 2)	We assume that the hat layer maintains a large enough translation
 *	cache to capture process reference patterns.
 */
STATIC u_int
segvn_swapout(seg)
	struct seg *seg;
{
	struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	struct anon_map *amp = svd->amp;
	register u_int pgcnt = 0;
	u_int npages;
	register u_int page;

	/*
	 * Find pages unmapped by our caller and force them
	 * out to the virtual swap device.
	 */
	npages = seg->s_size >> PAGESHIFT;
	for (page = 0; page < npages; page++) {
		register page_t *pp;
		register struct anon **app;
		struct vnode *vp;
		u_int off;
		register int s;

		/*
		 * Obtain <vp, off> pair for the page, then look it up.
		 *
		 * Note that this code is willing to consider regular
		 * pages as well as anon pages.  Is this appropriate here?
		 */
		if (amp != NULL && *(app = &amp->anon[svd->anon_index + page])
					!= NULL)
			swap_xlate(*app, &vp, &off);
		else if ((vp = svd->vp) != NULL) {
			off = svd->offset + ptob(page);
		} else
			continue;	/* untouched zfod page */

		s = splvm();
		if ((pp = page_exists(vp, off)) == NULL || pp->p_free) {
			(void) splx(s);
			continue;
		}
		(void) splx(s);

		/*
		 * Skip if page is logically unavailable for removal.
		 */
		if (pp->p_lckcnt > 0 || pp->p_cowcnt > 0) 
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
			(void) VOP_PUTPAGE(vp, off, PAGESIZE, B_ASYNC | B_FREE,
			    svd->cred);
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
segvn_sync(seg, addr, len, attr, flags)
	struct seg *seg;
	register addr_t addr;
	u_int len;
	int attr;
	u_int flags;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct anon **app;
	register struct anon *ap;
	register struct vpage *vpp = svd->vpage;
	register page_t *pp;
	u_int offset;
	struct vnode *vp;
	u_int off;
	addr_t eaddr;
	int bflags;
	int err = 0;
	int segtype;
	int pageprot;
	int prot;


	offset = svd->offset + (addr - seg->s_base);
	bflags = B_FORCE | ((flags & MS_ASYNC) ? B_ASYNC : 0) |
	    		   ((flags & MS_INVALIDATE) ? B_INVAL : 0);

	if (attr) {
		pageprot = attr & ~(SHARED|PRIVATE);
		segtype = attr & SHARED ? MAP_SHARED : MAP_PRIVATE;

		/*
		 * We are done if the segment types don't match
		 * or if we have segment level protections and
		 * they don't match.
		 */
		if (svd->type != segtype)
			return(0);
		if (vpp == NULL) {
			if (svd->prot != pageprot)
				return(0);
			prot = svd->prot;
		} else
			vpp = &svd->vpage[seg_page(seg, addr)];

	} else if (svd->vp && svd->amp == NULL && (flags & MS_INVALIDATE) == 0){

		/*
		 * No attributes, no anonymous pages and MS_INVALIDATE flag
		 * is not on, just use one big request.
		 */
		err = VOP_PUTPAGE(svd->vp, offset, len, bflags, svd->cred);
		return (err);
	}

	if (svd->amp == NULL)
		app = NULL;
	else
		app = &svd->amp->anon[svd->anon_index + seg_page(seg, addr)];

	for (eaddr = addr + len; addr < eaddr; addr += PAGESIZE) {

		if (app != NULL && (ap = *app++) != NULL) 
			swap_xlate (ap, &vp, &off);
		else {
			vp = svd->vp;
			off = offset;
		}
		offset += PAGESIZE;

		if (attr) {
			if (vpp)
				prot = vpp++->vp_prot;
			if ( prot != pageprot )
				continue;
		}

		if (vp == NULL)		/* untouched zfod page */
			continue;

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
segvn_incore(seg, addr, len, vec)
	struct seg *seg;
	register addr_t addr;
	u_int len;
	register char *vec;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct anon **app;
	struct vnode *vp;
	u_int offset;
	u_int p, ep;
	register int ret;
	register struct vpage *vpp;
	register page_t *pp;
	u_int start;

	if (svd->amp == NULL && svd->vp == NULL) {
		bzero(vec, btoc(len));
		return (len);	/* no anonymous pages created yet */
	}

	p = seg_page(seg, addr);
	ep = seg_page(seg, addr + len);
	vpp = (svd->vpage) ? &svd->vpage[p]: NULL;
	start = svd->vp ? 0x10 : 0;

	for ( ; p < ep; p++, addr += PAGESIZE) {
		ret = start;
		if ((svd->amp != NULL) &&
		    (*(app = &svd->amp->anon[svd->anon_index + p]) != NULL)) {
			swap_xlate(*app, &vp, &offset);
			ret |= 0x20;
			pp = page_exists(vp, offset);
		} else if ((vp = svd->vp) != NULL) {
			offset = svd->offset + (addr - seg->s_base);
			pp = page_exists(vp, offset);
		} else
			pp = NULL;	/* untouched zfod page */

		if (pp != NULL) {
			ret |= 0x1;
			if (pp->p_lckcnt)
				ret |= 0x8;
			if (pp->p_cowcnt)
				ret |= 0x4;
		}
		if (vpp && ((vpp++)->vp_pplock))
			ret |= 2;
		*vec++ = (char) ret;
	}
	return (len);
}

/*
 * Lock down (or unlock) pages mapped by this segment.
 */
STATIC int
segvn_lockop(seg, addr, len, attr, op, lockmap, pos)
	struct seg *seg;
	addr_t addr;
	u_int len;
	int   attr;
	int op;
	ulong *lockmap;
	size_t pos;
{
	struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct vpage *vpp = svd->vpage;
	register struct vpage *evp;
	page_t *pp;
	struct anon **app;
	u_int offset;
	u_int off;
	int segtype;
	int pageprot;
	int claim;
	int err;
	struct vnode *vp;

	if (attr) {
		pageprot = attr & ~(SHARED|PRIVATE);
		segtype = attr & SHARED ? MAP_SHARED : MAP_PRIVATE;

		/*
		 * We are done if the segment types don't match
		 * or if we have segment level protections and
		 * they don't match.
		 */
		if (svd->type != segtype)
			return(0);
		if (svd->pageprot == 0 && svd->prot != pageprot)
			return(0);

	}

	/*
	 * If we're locking, then we must create a vpage structure if
	 * none exists.  If we're unlocking, then check to see if there
	 * is a vpage --  if not, then we could not have locked anything.
	 */

	if (vpp == NULL) {
		if (op == MC_LOCK)
			segvn_vpage(seg);
		else
			return (0);
	}

	if ( op == MC_LOCK && svd->amp == NULL && svd->vp == NULL) {
		svd->amp = (struct anon_map *)kmem_fast_alloc(
		    (caddr_t *)&anonmap_freelist,
		    sizeof (*anonmap_freelist), anonmap_freeincr, KM_SLEEP);

		svd->amp->refcnt = 1;
		svd->amp->size = seg->s_size;
		svd->amp->anon = (struct anon **)kmem_zalloc(seg_pages(seg) *
		    sizeof (struct anon *), KM_SLEEP);
	}

	/*
	 * Set up bounds for looping over the range of pages.
	 */
	if (svd->amp == NULL)
		app = NULL;
	else
		app = &svd->amp->anon[svd->anon_index + seg_page(seg, addr)];

	offset = svd->offset + (addr - seg->s_base);
	evp = &svd->vpage[seg_page(seg, addr + len)];

	/*
	 * Loop over all pages in the range.  Process if we're locking and
	 * page has not already been locked in this mapping; or if we're
	 * unlocking and the page has been locked.
	 */

	for (vpp = &svd->vpage[seg_page(seg, addr)]; vpp < evp; vpp++, pos++) {
		if ((attr == 0 || vpp->vp_prot == pageprot)
		  && ((op == MC_LOCK && !vpp->vp_pplock)
		   || (op == MC_UNLOCK && vpp->vp_pplock))) {

			/* 
			 * If we're locking, softfault the page in memory.
			 */
			if (op == MC_LOCK) 
				if (segvn_fault(seg, addr, PAGESIZE, 
				    F_SOFTLOCK, S_OTHER) != 0)
					return (EIO);

			/*
			 * Get name for page, accounting for
			 * existence of private copy.
			 */
			if (app != NULL && *app != NULL)
				swap_xlate(*app, &vp, &off);
			else {
				vp = svd->vp;
				off = offset;
			}
			claim = ((vpp->vp_prot & PROT_WRITE) != 0) &
			    (svd->type == MAP_PRIVATE);

			/*
			 * Get page frame.  It's ok if the page is
			 * not available when we're unlocking, as this
			 * may simply mean that a page we locked got
			 * truncated out of existence after we locked it.
			 */
			if ((pp = page_lookup(vp, off)) == NULL)
				if (op == MC_LOCK)
					cmn_err(CE_PANIC, "segvn_lockop: no page");

			/*
			 * Perform page-level operation appropriate to
			 * operation.  If locking, undo the SOFTLOCK
			 * performed to bring the page into memory
			 * after setting the lock.  If unlocking,
			 * and no page was found, account for the claim
			 * separately.
			 */
			if (op == MC_LOCK) {
				err = page_pp_lock(pp, claim, 0);
				(void) segvn_fault(seg, addr, PAGESIZE, 
				    F_SOFTUNLOCK, S_OTHER);
				if (!err)
					return (EAGAIN);
				vpp->vp_pplock = 1;
				if (lockmap != (ulong *)NULL) {
					BT_SET(lockmap, pos);
				}
			} else {
				if (pp)
					page_pp_unlock(pp, claim, 0);
				vpp->vp_pplock = 0;
			}
		}
		addr += PAGESIZE;
		offset += PAGESIZE;
		if (app)
			app++;
	}
	return (0);
}

/*
 * Create a vpage structure for this seg.
 */
STATIC void
segvn_vpage(seg)
	struct seg *seg;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;
	register struct vpage *vp, *evp;

	/*
	 * If no vpage structure exists, allocate one.  Copy the protections
	 * from the segment itself to the individual pages.
	 */
	if (svd->vpage == NULL) {
		svd->pageprot = 1;
		svd->vpage = (struct vpage *)
		    kmem_zalloc(seg_pages(seg) * sizeof (struct vpage),  KM_SLEEP);
		evp = &svd->vpage[seg_page(seg, seg->s_base + seg->s_size)];
		for (vp = svd->vpage; vp < evp; vp++)
			vp->vp_prot = svd->prot;
	}
}

/*
 * Return 1 if addr is anon, else 0.
 */
int
segvn_isanon(seg, addr)
	register struct seg *seg;
	addr_t addr;
{
	register struct segvn_data *svd = (struct segvn_data *)seg->s_data;

	if (svd->vp == NULL)
		return 1;
	if (svd->amp == NULL)
		return 0;

	return (svd->amp->anon[svd->anon_index + seg_page(seg, addr)] != NULL);
}
