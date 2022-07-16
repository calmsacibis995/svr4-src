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

#ident	"@(#)kern-vm:seg_objs.c	1.3"

/*
 * VM - segment of a mapped kernel memory resource.
 *
 * Used for mapping kernel memory resources into user space.
 * The primary usage currently is the X/win queue data object
 * that is shared between the driver for a virtual terminal and the
 * X server for that VT (see code in io/ws/xque.c).
 *
 * This code was based on the seg_dev segment driver for 
 * device memory mapping since it, too, is kernel virtual
 * address space that is sharable with user applications
 * and is always in core. The only difference, essentially,
 * is that seg_dev uses hat_devload() to load translations,
 * seg_objs uses hat_pteload(), and seg_dev has a vnode association,
 * seg_objs does not. Since we use hat_pteload, we may or may not have
 * page structures associated with the memory to which
 * we are mapping.
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

#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_objs.h"
#include "vm/pvn.h"
#include "vm/vpage.h"

/*
 * Private seg op routines.
 */
STATIC	int segobjs_dup(/* seg, newsegp */);
STATIC	int segobjs_unmap(/* seg, addr, len */);
STATIC	void segobjs_free(/* seg */);
STATIC	faultcode_t segobjs_fault(/* seg, addr, len, type, rw */);
STATIC	faultcode_t segobjs_faulta(/* seg, addr */);
STATIC	void segobjs_unload(/* seg, addr, ref, mod */);
STATIC	int segobjs_setprot(/* seg, addr, size, len */);
STATIC	int segobjs_checkprot(/* seg, addr, size, len */);
STATIC	int segobjs_getprot(/* seg, addr, size, len */);
STATIC	int segobjs_badop();
STATIC	int segobjs_incore(/* seg, addr, size, vec */);
STATIC	int segobjs_ctlops(/* seg, addr, size, [flags] */);
STATIC	off_t segobjs_getoffset(/* seg, addr */);
STATIC	int segobjs_gettype(/* seg, addr */);
STATIC	int segobjs_getvp(/* seg, addr, vpp */);

STATIC	struct	seg_ops segobjs_ops = {
	segobjs_dup,
	segobjs_unmap,
	segobjs_free,
	segobjs_fault,
	segobjs_faulta,
	segobjs_unload,
	segobjs_setprot,
	segobjs_checkprot,
	segobjs_badop,		/* kluster */
	(u_int (*)()) NULL,	/* swapout */
	segobjs_ctlops,		/* sync */
	segobjs_incore,
	segobjs_ctlops,		/* lockop */
	segobjs_getprot,
	segobjs_getoffset,
	segobjs_gettype,
	segobjs_getvp,
};



/* ARGSUSED */
STATIC off_t
segobjs_getoffset(seg, addr)
	register struct seg *seg;
	addr_t addr;
{
	register struct segobjs_data *sdp = (struct segobjs_data *)seg->s_data;

	return sdp->offset;
}

/* ARGSUSED */
STATIC int
segobjs_gettype(seg, addr)
	register struct seg *seg;
	addr_t addr;
{
	return MAP_SHARED; /* Always a shared object */
}


/* ARGSUSED */
STATIC int
segobjs_getvp(seg, addr, vpp)
	register struct seg *seg;
	addr_t addr;
	struct vnode **vpp;
{
	*vpp = NULL; /* no vnode association can be made with these pages */
	return -1;
}


/*
 * Create a shared kernel object segment.
 */
int
segobjs_create(seg, argsp)
	struct seg *seg;
	_VOID *argsp;
{
	register struct segobjs_data *sdp;
	register struct segobjs_crargs *a = (struct segobjs_crargs *)argsp;
	register int error = 0;

	/* No need to call hat_map */

	sdp = (struct segobjs_data *)kmem_alloc(sizeof (struct segobjs_data),KM_SLEEP);
	sdp->mapfunc = a->mapfunc;
	sdp->offset = a->offset;
	sdp->prot = a->prot;
	sdp->maxprot = a->maxprot;
	sdp->pageprot = 0;
	sdp->vpage = NULL;
	sdp->arg = a->arg;

	seg->s_ops = &segobjs_ops;
	seg->s_data = (char *)sdp;

	return (0);
}

/*
 * Duplicate seg and return new segment in newsegp.
 */
STATIC int
segobjs_dup(seg, newseg)
	struct seg *seg, *newseg;
{
	register struct segobjs_data *sdp = (struct segobjs_data *)seg->s_data;
	register struct segobjs_data *newsdp;
	struct segobjs_crargs a;
	int error;

	a.mapfunc = sdp->mapfunc;
	a.arg = sdp->arg;
	a.offset = sdp->offset;
	a.prot = sdp->prot;
	a.maxprot = sdp->maxprot;

	error = segobjs_create(newseg, (caddr_t)&a);
	if (error != 0)
		return (error);
	newsdp = (struct segobjs_data *)newseg->s_data;
	newsdp->pageprot = sdp->pageprot;
	if (sdp->vpage != NULL) {
		register u_int nbytes = seg_pages(seg) * sizeof (struct vpage);

		if (newsdp->vpage == NULL)
			newsdp->vpage = (struct vpage *)kmem_alloc(nbytes,KM_SLEEP);
		bcopy((caddr_t)sdp->vpage, (caddr_t)newsdp->vpage, nbytes);
	}

	return (error);
}

/*
 * Split a segment at addr for length len.
 */
/*ARGSUSED*/
STATIC int
segobjs_unmap(seg, addr, len)
	register struct seg *seg;
	register addr_t addr;
	u_int len;
{
	register struct segobjs_data *sdp = (struct segobjs_data *)seg->s_data;
	register struct segobjs_data *nsdp;
	register struct seg *nseg;
	register u_int	opages,		/* old segment size in pages */
			npages,		/* new segment size in pages */
			dpages;		/* pages being deleted (unmapped)*/

	addr_t nbase;
	u_int nsize;

	/*
	 * Check for bad sizes
	 */
	if (addr < seg->s_base || addr + len > seg->s_base + seg->s_size ||
	    (len & PAGEOFFSET) || ((u_int)addr & PAGEOFFSET))
		cmn_err(CE_PANIC, "segobjs_unmap");

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
		if (sdp->vpage != NULL) {
			register uint nbytes;
			register struct vpage *ovpage;

			ovpage = sdp->vpage;	/* keep pointer to vpage */

			nbytes = npages * sizeof (struct vpage);
			sdp->vpage = (struct vpage *)kmem_alloc(nbytes, KM_SLEEP);
			bcopy((caddr_t)&ovpage[dpages],
                            (caddr_t)sdp->vpage, nbytes);

			/* free up old vpage */
			kmem_free(ovpage, opages * sizeof (struct vpage));
		}
		sdp->offset += len;

		seg->s_base += len;
		seg->s_size -= len;
		return (0);
	}

	/*
	 * Check for end of segment
	 */
	if (addr + len == seg->s_base + seg->s_size) {
		if (sdp->vpage != NULL) {
			register uint nbytes;
			register struct vpage *ovpage;

			ovpage = sdp->vpage;	/* keep pointer to vpage */

			nbytes = npages * sizeof (struct vpage);
			sdp->vpage = (struct vpage *)kmem_alloc(nbytes, KM_SLEEP);
			bcopy((caddr_t)ovpage, (caddr_t)sdp->vpage, nbytes);

			/* free up old vpage */
			kmem_free(ovpage, opages * sizeof (struct vpage));

		}
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
		cmn_err(CE_PANIC, "segobjs_unmap seg_alloc");

	nseg->s_ops = seg->s_ops;
	nsdp = (struct segobjs_data *)kmem_alloc(sizeof (struct segobjs_data),KM_SLEEP);
	nseg->s_data = (char *)nsdp;
	nsdp->pageprot = sdp->pageprot;
	nsdp->prot = sdp->prot;
	nsdp->maxprot = sdp->maxprot;
	nsdp->mapfunc = sdp->mapfunc;
	nsdp->offset = sdp->offset + nseg->s_base - seg->s_base;

	if (sdp->vpage == NULL)
		nsdp->vpage = NULL;
	else {
		/* need to split vpage into two arrays */
		register uint nbytes;
		register struct vpage *ovpage;

		ovpage = sdp->vpage;	/* keep pointer to vpage */

		npages = seg_pages(seg);	/* seg has shrunk */
		nbytes = npages * sizeof (struct vpage);
		sdp->vpage = (struct vpage *)kmem_alloc(nbytes, KM_SLEEP);

		bcopy((caddr_t)ovpage, (caddr_t)sdp->vpage, nbytes);

		npages = seg_pages(nseg);
		nbytes = npages * sizeof (struct vpage);
		nsdp->vpage = (struct vpage *)kmem_alloc(nbytes, KM_SLEEP);

		bcopy((caddr_t)&ovpage[opages - npages],
                    (caddr_t)nsdp->vpage, nbytes);

		/* free up old vpage */
		kmem_free(ovpage, opages * sizeof (struct vpage));
	}

	/*
	 * Now we do something so that all the translations which used
	 * to be associated with seg but are now associated with nseg.
	 */
	hat_newseg(seg, nseg->s_base, nseg->s_size, nseg);

	return (0);
}

/*
 * Free a segment.
 */
STATIC void
segobjs_free(seg)
	struct seg *seg;
{
	register struct segobjs_data *sdp = (struct segobjs_data *)seg->s_data;
	u_int npages = seg_pages(seg);

	if (sdp->vpage != NULL)
		kmem_free((caddr_t)sdp->vpage, npages * sizeof (struct vpage));

	kmem_free((char *)sdp, sizeof (*sdp));
}

/*
 * Handle a fault on a shared kernel object segment.
 */
STATIC faultcode_t
segobjs_fault(seg, addr, len, type, rw)
	register struct seg *seg;
	register addr_t addr;
	u_int len;
	enum fault_type type;
	enum seg_rw rw;
{
	register struct segobjs_data *sdp = (struct segobjs_data *)seg->s_data;
	register addr_t adr;
	register u_int prot, protchk;
	int pf;
	struct vpage *vpage;

	if (type == F_PROT) {
		/*
		 * Since the seg_objs driver does not implement copy-on-write,
		 * this means that a valid translation is already loaded,
		 * but we got an fault trying to access the device.
		 * Return an error here to prevent going in an endless
		 * loop reloading the same translation...
		 */
		return (FC_PROT);
	}

	if (type != F_SOFTUNLOCK) {
		if (sdp->pageprot == 0) {
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
			prot = sdp->prot;
			if ((prot & protchk) == 0)
				return (FC_PROT);
			vpage = NULL;
		} else {
			vpage = &sdp->vpage[seg_page(seg, addr)];
		}
	}

	for (adr = addr; adr < addr + len; adr += PAGESIZE) {
		if (type == F_SOFTUNLOCK) {
			hat_unlock(seg, adr);
			continue;
		}
		if (vpage != NULL) {
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
			vpage++;
			if ((prot & protchk) == 0)
				return (FC_PROT);
		}

		pf = (*sdp->mapfunc)(sdp->arg,
		    sdp->offset + (adr - seg->s_base), prot);
		if (pf == -1)
			return (FC_MAKE_ERR(EFAULT));

		hat_pteload(seg, adr, page_numtopp(pf), pf, prot, type == F_SOFTLOCK);
	}

	return (0);
}

/*
 * Asynchronous page fault.  We simply do nothing since this
 * entry point is not supposed to load up the translation.
 */
/*ARGSUSED*/
STATIC faultcode_t
segobjs_faulta(seg, addr)
	struct seg *seg;
	addr_t addr;
{

	return (0);
}

/*ARGSUSED*/
STATIC void
segobjs_unload(seg, addr, ref, mod)
	struct seg *seg;
	addr_t addr;
	u_int ref, mod;
{

	/* cannot use ref and mod bits on shared kernel data objects,
	 * so ignore 'em
	 */
}

STATIC int
segobjs_setprot(seg, addr, len, prot)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, prot;
{
	register struct segobjs_data *sdp = (struct segobjs_data *)seg->s_data;
	register struct vpage *vp, *evp;

	if ((sdp->maxprot & prot) != prot)
		return (EACCES);			/* violated maxprot */

	if (addr == seg->s_base && len == seg->s_size && sdp->pageprot == 0) {
		if (sdp->prot == prot)
			return (0);			/* all done */
		sdp->prot = (u_char)prot;
	} else {
		sdp->pageprot = 1;
		if (sdp->vpage == NULL) {
			/*
			 * First time through setting per page permissions,
			 * initialize all the vpage structures to prot
			 */
			sdp->vpage = (struct vpage *)kmem_zalloc(seg_pages(seg)
					* sizeof (struct vpage), KM_SLEEP);
			evp = &sdp->vpage[seg_pages(seg)];
			for (vp = sdp->vpage; vp < evp; vp++)
				vp->vp_prot = sdp->prot;
		}
		/*
		 * Now go change the needed vpages protections.
		 */
		evp = &sdp->vpage[seg_page(seg, addr + len)];
		for (vp = &sdp->vpage[seg_page(seg, addr)]; vp < evp; vp++)
			vp->vp_prot = prot;
	}

	if (prot == 0)
		hat_unload(seg, addr, len, HAT_NOFLAGS);
	else
		hat_chgprot(seg, addr, len, prot);
	return (0);
}

STATIC int
segobjs_checkprot(seg, addr, len, prot)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, prot;
{
	struct segobjs_data *sdp = (struct segobjs_data *)seg->s_data;
	register struct vpage *vp, *evp;

	/*
	 * If segment protection can be used, simply check against them
	 */
	if (sdp->pageprot == 0)
		return (((sdp->prot & prot) != prot) ? EACCES : 0);

	/*
	 * Have to check down to the vpage level
	 */
	evp = &sdp->vpage[seg_page(seg, addr + len)];
	for (vp = &sdp->vpage[seg_page(seg, addr)]; vp < evp; vp++)
		if ((vp->vp_prot & prot) != prot)
			return (-1);

	return (0);
}

STATIC int
segobjs_getprot(seg, addr, len, protv)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, *protv;
{
 	struct segobjs_data *sdp = (struct segobjs_data *)seg->s_data;
	u_int pgno = seg_page(seg, addr+len) - seg_page(seg, addr) + 1;

        if (pgno != 0) {
		if (sdp->pageprot == 0) {
                        do protv[--pgno] = sdp->prot;
                        while (pgno != 0);
		} else {
                        register pgoff = seg_page(seg, addr);
			do {
                                pgno--;
                                protv[pgno] = sdp->vpage[pgno+pgoff].vp_prot;
			} while (pgno != 0);
		}
	}
	return 0;
}

STATIC
segobjs_badop()
{
	cmn_err(CE_PANIC, "segobjs_badop");
	/*NOTREACHED*/
}

/*
 * segobjs pages are not in the cache, and thus can't really be controlled.
 * syncs, locks, and advice are simply always successful.
 */
/*ARGSUSED*/
STATIC int
segobjs_ctlops(seg, addr, len, flags)
	struct seg *seg;
	addr_t addr;
	u_int len, flags;
{

	return (0);
}

/*
 * segobjs pages are always "in core".
 */
/*ARGSUSED*/
STATIC int
segobjs_incore(seg, addr, len, vec)
	struct seg *seg;
	addr_t addr;
	register u_int len;
	register char *vec;
{
	u_int v = 0;

	for (len = (len + PAGEOFFSET) & PAGEMASK; len; len -= PAGESIZE,
	    v += PAGESIZE)
		*vec++ = 1;
	return (v);
}
