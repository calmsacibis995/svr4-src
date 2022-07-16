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

#ident  "@(#)kern-vm:seg_map.c	1.3.2.3"

/*
 * VM - generic vnode mapping segment.
 *
 * The segmap driver is used only by the kernel to get faster (than seg_vn)
 * mappings [lower routine overhead; more persistent cache] to random
 * vnode/offsets.  Note than the kernel may (and does) use seg_vn as well.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/vnode.h"
#include "sys/mman.h"
#include "sys/errno.h"
#include "sys/cred.h"
#include "sys/proc.h"
#include "sys/disp.h"
#include "sys/kmem.h"
#include "vm/trace.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"

#include "vm/seg_kmem.h"
#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/seg_map.h"
#include "vm/page.h"
#include "vm/pvn.h"
#include "vm/rm.h"

/*
 * Private seg op routines.
 */
STATIC	void segmap_free(/* seg */);
STATIC	faultcode_t segmap_fault(/* seg, addr, len, type, rw */);
STATIC	faultcode_t segmap_faulta(/* seg, addr */);
STATIC	void segmap_unload(/* seg, addr, ref, mod */);
STATIC	int segmap_checkprot(/* seg, addr, len, prot */);
STATIC	int segmap_getprot(/* seg, addr, len, prot */);
STATIC	int segmap_kluster(/* seg, addr, delta */);
STATIC	off_t segmap_getoffset(/* seg, addr */);
STATIC	int segmap_gettype(/* seg, addr */);
STATIC	int segmap_getvp(/* seg, addr, vpp */);
STATIC	int segmap_badop();

STATIC struct seg_ops segmap_ops = {
	segmap_badop,		/* dup */
	segmap_badop,		/* unmap */
	segmap_free,
	segmap_fault,
	segmap_faulta,
	segmap_unload,
	segmap_badop,		/* setprot */
	segmap_checkprot,
	segmap_kluster,
	(u_int (*)()) NULL,	/* swapout */
	segmap_badop,		/* sync */
	segmap_badop,		/* incore */
	segmap_badop,		/* lockop */
	segmap_getprot,
	segmap_getoffset,
	segmap_gettype,
	segmap_getvp,
};

/*
 * Private segmap routines.
 */
STATIC	void segmap_smapadd(/* smd, smp */);
STATIC	void segmap_smapsub(/* smd, smp */);
STATIC	void segmap_hashin(/* smd, smp, vp, off, flags */);
STATIC	void segmap_hashout(/* smd, smp */);

/*
 * Statistics for segmap operations.
 */
struct segmapcnt {
	int	smc_fault;	/* number of segmap_faults */
	int	smc_faulta;	/* number of segmap_faultas */
	int	smc_getmap;	/* number of segmap_getmaps */
	int	smc_get_use;	/* # of getmaps that reuse an existing map */
	int	smc_get_reclaim; /* # of getmaps that do a reclaim */
	int	smc_get_reuse;	/* # of getmaps that reuse a slot */
	int	smc_rel_async;	/* # of releases that are async */
	int	smc_rel_write;	/* # of releases that write */
	int	smc_rel_free;	/* # of releases that free */
	int	smc_rel_abort;	/* # of releases that abort */
	int	smc_rel_dontneed; /* # of releases with dontneed set */
	int	smc_release;	/* # of releases with no other action */
	int	smc_pagecreate;	/* # of pagecreates */
} segmapcnt;

/*
 * Return number of map pages in segment.
 */
#define	MAP_PAGES(seg)		((seg)->s_size >> MAXBSHIFT)

/*
 * Translate addr into smap number within segment.
 */
#define	MAP_PAGE(seg, addr)	(((addr) - (seg)->s_base) >> MAXBSHIFT)

/*
 * Translate addr in seg into struct smap pointer.
 */
#define	GET_SMAP(seg, addr)	\
	&(((struct segmap_data *)((seg)->s_data))->smd_sm[MAP_PAGE(seg, addr)])

int
segmap_create(seg, argsp)
	struct seg *seg;
	_VOID *argsp;
{
	register struct segmap_data *smd;
	register struct smap *smp;
	struct segmap_crargs *a = (struct segmap_crargs *)argsp;
	register u_int i;
	u_int hashsz;
	addr_t segend;

	/*
	 * No need to notify the hat layer, since the SDT's are
	 * already allocated for seg_map; i.e. no need to call
	 * hat_map().
	 */

	/*
	 * Make sure that seg->s_base and seg->s_base + seg->s_size
	 * are on MAXBSIZE aligned pieces of virtual memory.
	 *
	 * Since we assume we are creating a large segment
	 * (it's just segkmap), trimming off the excess at the
	 * beginning and end of the segment is considered safe.
	 */
	segend = (addr_t)((u_int)(seg->s_base + seg->s_size) & MAXBMASK);
	seg->s_base = (addr_t)roundup((u_int)(seg->s_base), MAXBSIZE);
	seg->s_size = segend - seg->s_base;

	i = MAP_PAGES(seg);

	smd = (struct segmap_data *)kmem_zalloc(sizeof (struct segmap_data), KM_SLEEP);
	smd->smd_prot = a->prot;
	smd->smd_sm = (struct smap *)kmem_zalloc((u_int)(sizeof (struct smap) * i), KM_SLEEP);

	/*
	 * Link up all the slots.
	 */
	for (smp = &smd->smd_sm[i - 1]; smp >= smd->smd_sm; smp--)
		segmap_smapadd(smd, smp);

	/*
	 * Compute hash size rounding down to the next power of two.
	 */
	hashsz = MAP_PAGES(seg) / SMAP_HASHAVELEN;
	for (i = (u_int)0x80 << ((sizeof(int) - 1) * NBBY); i != 0; i >>= 1) {
		if ((hashsz & i) != 0) {
			smd->smd_hashsz = hashsz = i;
			break;
		}
	}
	smd->smd_hash = (struct smap **)kmem_zalloc(hashsz *
	    sizeof (smd->smd_hash[0]), KM_SLEEP);

	seg->s_data = (char *)smd;
	seg->s_ops = &segmap_ops;

	return (0);
}

STATIC void
segmap_free(seg)
	struct seg *seg;
{
	register struct segmap_data *smd = (struct segmap_data *)seg->s_data;

	kmem_free((caddr_t)smd->smd_hash, sizeof (smd->smd_hash[0]) *
	    smd->smd_hashsz);
	kmem_free((caddr_t)smd->smd_sm, sizeof (struct smap) * MAP_PAGES(seg));
	kmem_free((caddr_t)smd, sizeof (*smd));
}

/*
 * Do a F_SOFTUNLOCK call over the range requested.
 * The range must have already been F_SOFTLOCK'ed.
 */
STATIC void
segmap_unlock(seg, addr, len, rw, smp)
	struct seg *seg;
	addr_t addr;
	u_int len;
	enum seg_rw rw;
	register struct smap *smp;
{
	register page_t *pp;
	register addr_t adr;
	u_int off;
	int pg;

	off = smp->sm_off + ((u_int)addr & MAXBOFFSET);
	pg = ((u_int)addr & MAXBOFFSET) >> PAGESHIFT;
	adr = addr;
	for (; adr < addr + len; adr += PAGESIZE, off += PAGESIZE, pg++) {
		/*
		 * For now, we just kludge here by finding the page
		 * ourselves since we would not find the page using
		 * page_find() if someone has page_abort()'ed it.
		 * XXX - need to redo things to avoid this mess.
		 */
		for (pp = page_hash[PAGE_HASHFUNC(smp->sm_vp, off)]; pp != NULL;
		    pp = pp->p_hash)
			if (pp->p_vnode == smp->sm_vp && pp->p_offset == off)
				break;
		if (pp == NULL)
			cmn_err(CE_PANIC, "segmap_unlock NULL");
		if (rw == S_WRITE)
			pp->p_mod = 1;
		if (rw != S_OTHER) {
                        trace4(TR_PG_SEGMAP_FLT, pp, pp->p_vnode, off, 1);
			pp->p_ref = 1;
		}
		if (SM_PG_UNINIT(smp, pg)) {
			/*
			 * We'll release uninitialized pages in segmap_release;
			 * don't do it twice.
			 */
			continue;
		}
		if (pp->p_pagein || pp->p_free)
			cmn_err(CE_PANIC, "segmap_unlock");
		hat_unlock(seg, adr);
		PAGE_RELE(pp);
	}
}

/*
 * This routine is called via a machine specific fault handling
 * routine.  It is also called by software routines wishing to
 * lock or unlock a range of addresses.
 */
STATIC faultcode_t
segmap_fault(seg, addr, len, type, rw)
	struct seg *seg;
	addr_t addr;
	u_int len;
	enum fault_type type;
	enum seg_rw rw;
{
	register u_long sm_off;
	u_long sm_endoff;
	register u_long off;
	register u_long endoff;
	register u_long pp_off;
	register page_t *pp;
	page_t **ppp;
	u_long invar;
	struct segmap_data *smd;
	register struct smap *smp;
	struct vnode *vp;
	page_t *pl[btopr(MAXBSIZE) + 1];
	u_int prot;
	int err;
	int pg;

	segmapcnt.smc_fault++;

	smd = (struct segmap_data *)seg->s_data;
	smp = GET_SMAP(seg, addr);
	vp = smp->sm_vp;

	if (vp == NULL)
		return (FC_MAKE_ERR(EIO));

	ASSERT((((u_int)addr & MAXBOFFSET) + len) <= MAXBSIZE);

	/*
	 * First handle the easy stuff
	 */
	if (type == F_SOFTUNLOCK) {
		segmap_unlock(seg, addr, len, rw, smp);
		return (0);
	}

	/*
	 *
	 * virtual      sm_addr       addr      endaddr       sm_endaddr
	 * addresses       |           |           |               |
	 *                /           /..request../               /
	 *               /           /           /               /
	 *              |           |           |               |
	 * object    sm_off        off        endoff        sm_endoff
	 * offsets
	 *
	 * sm_addr = (u_int)addr & MAXBMASK;
	 * sm_endaddr = sm_addr + MAXBSIZE;
	 * endaddr = addr + len;
	 *
	 */
	sm_off = smp->sm_off;
	sm_endoff = sm_off + MAXBSIZE;
	off = sm_off + ((u_int)addr & MAXBOFFSET);
	endoff = off + len;

	ASSERT(endoff <= sm_endoff);

	/*
	 * Remove invariant from the loop. To compute the virtual
	 * address of the page, we use the following computation:
	 *
	 *	addr =  sm_addr + (pp->p_offset - sm_off)
	 *
	 * Since sm_addr and sm_off do not vary:
	 *
	 *	addr = (invar = (sm_addr - sm_off)) + pp->p_offset
	 */
	invar = ((u_int)addr & MAXBMASK) - sm_off;

	if (type == F_SOFTLOCK && smp->sm_pgowner == (u_int)curproc) {
		/*
		 * Check if any of the pages are uninitialized.
		 * If they are, and we own them, we have to abort them,
		 * so the GETPAGE doesn't hang.  It's OK to abort them,
		 * since we would not have put any data into them yet.
		 */

		pg = ((u_int)addr & MAXBOFFSET) >> PAGESHIFT;
		for (pp_off = off; pp_off < endoff; pp_off += PAGESIZE, pg++) {
			if (!SM_PG_UNINIT(smp, pg))
				continue;

			pp = page_find(vp, pp_off);
			if (pp == NULL)
				cmn_err(CE_PANIC,
					"segmap_fault: uninit page gone");
			ASSERT(pp->p_lock);
			ASSERT(pp->p_intrans && pp->p_pagein);
			ASSERT(pp->p_keepcnt > 0);
			hat_unlock(seg, invar + pp_off);
			pp->p_intrans = pp->p_pagein = 0;
			page_unlock(pp);
			PAGE_RELE(pp);
			page_abort(pp);
			SM_CLR_PG_UNINIT(smp, pg);
		}
	}

	trace3(TR_SEG_GETPAGE, seg, addr, TRC_SEG_SEGKMAP);

	err = VOP_GETPAGE(vp, off, len, &prot, pl, MAXBSIZE, seg, addr,
			  (rw == S_WRITE ? S_OTHER : rw),
			  (struct cred *)NULL);	/* XXX - need real cred val */
	if (err)
		return (FC_MAKE_ERR(err));

	if (rw == S_WRITE)
		prot |= PROT_WRITE;
	prot &= smd->smd_prot;

	for (ppp = pl; (pp = *ppp++) != NULL; ) {
		ASSERT(pp->p_vnode == vp);

		pp_off = pp->p_offset;
		if ((pp_off >= off) && (pp_off < endoff)) {
			/*
			 * Within range requested.
			 */
			pp->p_ref = 1;
			if (type == F_SOFTLOCK) {
				/*
				 * Load up the translation keeping it
				 * locked and don't PAGE_RELE the page.
				 */
				hat_memload(seg, invar + pp_off, pp, prot,
				  HAT_LOCK);
			} else {
				hat_memload(seg, invar + pp_off, pp, prot,
				  HAT_NOFLAGS);
				PAGE_RELE(pp);
			}
		} else if ((pp_off >= sm_off) && (pp_off < sm_endoff)) {
			/*
			 * Within segmap buffer.
			 */
			hat_memload(seg, invar + pp_off, pp, prot, HAT_NOFLAGS);
			PAGE_RELE(pp);
		} else {
			/*
			 * XXX - page is now orphaned !!!
			 */
			PAGE_RELE(pp);
		}
	}

	return (0);
}

/*
 * This routine is used to start I/O on pages asynchronously.
 */
STATIC faultcode_t
segmap_faulta(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	register struct smap *smp;
	int err;

	segmapcnt.smc_faulta++;
	smp = GET_SMAP(seg, addr);
	if (smp->sm_vp == NULL) {
		cmn_err(CE_WARN, "segmap_faulta - no vp");
		return (FC_MAKE_ERR(EIO));
	}
	trace3(TR_SEG_GETPAGE, seg, addr, TRC_SEG_SEGKMAP);
	err = VOP_GETPAGE(smp->sm_vp, smp->sm_off + (u_int)addr & MAXBOFFSET,
	    PAGESIZE, (u_int *)NULL, (page_t **)NULL, 0,
	    seg, addr, S_READ,
	    (struct cred *)NULL);		/* XXX - need real cred val */
	if (err)
		return (FC_MAKE_ERR(err));
	return (0);
}

STATIC void
segmap_unload(seg, addr, ref, mod)
	struct seg *seg;
	addr_t addr;
	u_int ref, mod;
{
	register struct smap *smp;

	smp = GET_SMAP(seg, addr);
	pvn_unloadmap(smp->sm_vp, smp->sm_off + (u_int)addr & MAXBOFFSET,
	    ref, mod);
}

/*ARGSUSED*/
STATIC int
segmap_checkprot(seg, addr, len, prot)
	struct seg *seg;
	addr_t addr;
	u_int len, prot;
{
	struct segmap_data *smd = (struct segmap_data *)seg->s_data;

	return (((smd->smd_prot & prot) != prot) ? EACCES : 0);
}

STATIC int
segmap_getprot(seg, addr, len, protv)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, *protv;
{
 	struct segmap_data *smd = (struct segmap_data *)seg->s_data;
	u_int pgno = seg_page(seg, addr+len) - seg_page(seg, addr) + 1;

        if (pgno != 0) {
		do protv[--pgno] = smd->smd_prot;
		while (pgno != 0);
	}
        return 0;
}

/* ARGSUSED */
STATIC off_t
segmap_getoffset(seg, addr)
	register struct seg *seg;
	addr_t addr;
{
	register struct segmap_data *smd = (struct segmap_data *)seg->s_data;

	return smd->smd_sm->sm_off;
}

/* ARGSUSED */
STATIC int
segmap_gettype(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	return MAP_SHARED;
}

/* ARGSUSED */
STATIC int
segmap_getvp(seg, addr, vpp)
	register struct seg *seg;
	addr_t addr;
	struct vnode **vpp;
{
	register struct segmap_data *smd = (struct segmap_data *)seg->s_data;

	if ((*vpp = smd->smd_sm->sm_vp) == (struct vnode *) NULL)
		return -1;

	return 0;
}

/*
 * Check to see if it makes sense to do kluster/read ahead to
 * addr + delta relative to the mapping at addr.  We assume here
 * that delta is a signed PAGESIZE'd multiple (which can be negative).
 *
 * For segmap we always "approve" of this action from our standpoint.
 */
/*ARGSUSED*/
STATIC int
segmap_kluster(seg, addr, delta)
	struct seg *seg;
	addr_t addr;
	int delta;
{
	return (0);
}

STATIC
segmap_badop()
{
	cmn_err(CE_PANIC, "segmap_badop");
	/*NOTREACHED*/
}

/*
 * Special private segmap operations
 */

/*
 * Add smp to the free list on smd.  If the smp still has a vnode
 * association with it, then it is added to the end of the free list,
 * otherwise it is added to the front of the list.
 */
STATIC void
segmap_smapadd(smd, smp)
	register struct segmap_data *smd;
	register struct smap *smp;
{
	if (smp->sm_refcnt != 0)
		cmn_err(CE_PANIC, "segmap_smapadd");

	if (smd->smd_free == (struct smap *)NULL) {
		smp->sm_next = smp->sm_prev = smp;
	} else {
		smp->sm_next = smd->smd_free;
		smp->sm_prev = (smd->smd_free)->sm_prev;
		(smd->smd_free)->sm_prev = smp;
		smp->sm_prev->sm_next = smp;
	}

	if (smp->sm_vp == (struct vnode *)NULL)
		smd->smd_free = smp;
	else
		smd->smd_free = smp->sm_next;

	/*
	 * XXX - need a better way to do this.
	 */
	if (smd->smd_want) {
		wakeprocs((caddr_t)&smd->smd_free, PRMPT);
		smd->smd_want = 0;
	}
}

/*
 * Remove smp from the smd free list.  If there is an old
 * mapping in effect there, then delete it.
 */
STATIC void
segmap_smapsub(smd, smp)
	register struct segmap_data *smd;
	register struct smap *smp;
{
	if (smd->smd_free == smp)
		smd->smd_free = smp->sm_next;	/* go to next page */

	if (smd->smd_free == smp)
		smd->smd_free = NULL;		/* smp list is gone */
	else {
		smp->sm_prev->sm_next = smp->sm_next;
		smp->sm_next->sm_prev = smp->sm_prev;
	}
	smp->sm_prev = smp->sm_next = smp;	/* make smp a list of one */
	smp->sm_refcnt = 1;
}

STATIC void
segmap_hashin(smd, smp, vp, off)
	register struct segmap_data *smd;
	register struct smap *smp;
	struct vnode *vp;
	u_int off;
{
	register struct smap **hpp;

	/*
	 * Funniness here - we don't increment the ref count on the vnode
	 * even though we have another pointer to it here.  The reason
	 * for this is that we don't want the fact that a seg_map
	 * entry somewhere refers to a vnode to prevent the vnode
	 * itself from going away.  This is because this reference
	 * to the vnode is a "soft one".  In the case where a mapping
	 * is being used by a rdwr [or directory routine?] there already
	 * has to be a non-zero ref count on the vnode.  In the case
	 * where the vp has been freed and the the smap structure is
	 * on the free list, there are no pages in memory that can
	 * refer to the vnode.  Thus even if we reuse the same
	 * vnode/smap structure for a vnode which has the same
	 * address but represents a different object, we are ok.
	 */
	smp->sm_vp = vp;
	smp->sm_off = off;

	hpp = &smd->smd_hash[SMAP_HASHFUNC(smd, vp, off)];
	smp->sm_hash = *hpp;
	*hpp = smp;
}

STATIC void
segmap_hashout(smd, smp)
	register struct segmap_data *smd;
	register struct smap *smp;
{
	register struct smap **hpp, *hp;
	struct vnode *vp;

	vp = smp->sm_vp;
	hpp = &smd->smd_hash[SMAP_HASHFUNC(smd, vp, smp->sm_off)];
	for (;;) {
		hp = *hpp;
		if (hp == NULL)
			cmn_err(CE_PANIC, "segmap_hashout");
		if (hp == smp)
			break;
		hpp = &hp->sm_hash;
	}

	*hpp = smp->sm_hash;
	smp->sm_hash = NULL;
	smp->sm_vp = NULL;
	smp->sm_off = 0;
}

/*
 * Special public segmap operations
 */

/*
 * Create pages (without using VOP_GETPAGE) and load up tranlations to them.
 * If softlock is TRUE, then set things up so that it looks like a call
 * to segmap_fault with F_SOFTLOCK.
 */
void
segmap_pagecreate(seg, addr, len, softlock)
	struct seg *seg;
	register addr_t addr;
	u_int len;
	int softlock;
{
	register page_t *pp;
	register u_int off;
	struct smap *smp;
	struct vnode *vp;
	addr_t eaddr;
	u_int pg;
	u_int prot;

	segmapcnt.smc_pagecreate++;

	eaddr = addr + len;
	addr = (addr_t)((u_int)addr & PAGEMASK);
	smp = GET_SMAP(seg, addr);
	vp = smp->sm_vp;
	off = smp->sm_off + ((u_int)addr & MAXBOFFSET);
	pg = ((u_int)addr & MAXBOFFSET) >> PAGESHIFT;
	prot = ((struct segmap_data *)seg->s_data)->smd_prot;

	ASSERT(smp->sm_pgowner == NULL || smp->sm_pgowner == (u_int)curproc);

	for (; addr < eaddr; addr += PAGESIZE, off += PAGESIZE, pg++) {
		if (SM_PG_UNINIT(smp, pg)) {
			ASSERT(smp->sm_pgowner && smp->sm_pgowncnt > 1);
			continue;
		}
		pp = page_lookup(vp, off);
		if (pp == NULL) {
			pp = rm_allocpage(seg, addr, PAGESIZE, P_CANWAIT);
			trace6(TR_SEG_ALLOCPAGE, seg, addr,
				TRC_SEG_SEGKMAP, vp, off, pp);
			if (page_enter(pp, vp, off))
				cmn_err(CE_PANIC, "segmap_pagecreate page_enter");
			pp->p_intrans = pp->p_pagein = 1;
			hat_memload(seg, addr, pp, prot, HAT_LOCK);
			if (smp->sm_pgowner == NULL) {
				smp->sm_pgowner = (u_int)curproc;
				ASSERT(smp->sm_pgowncnt == 0);
				smp->sm_pgowncnt = 1;
			}
			SM_SET_PG_UNINIT(smp, pg);
		} else {
			if (softlock) {
				PAGE_HOLD(pp);
				hat_memload(seg, addr, pp, prot, HAT_LOCK);
			} else {
				hat_memload(seg, addr, pp, prot, HAT_NOFLAGS);
			}
		}
	}
}

addr_t
segmap_getmap(seg, vp, off)
	struct seg *seg;
	struct vnode *vp;
	u_int off;
{
	register struct segmap_data *smd = (struct segmap_data *)seg->s_data;
	register struct smap *smp, *tsmp;
	extern struct vnode *common_specvp();

	segmapcnt.smc_getmap++;

	if ((off & MAXBOFFSET) != 0)
		cmn_err(CE_PANIC, "segmap_getmap bad offset");

	/*
	 * If this is a block device we have to be sure to use the
	 * "common" block device vnode for the mapping.
	 */
	if (vp->v_type == VBLK)
		vp = common_specvp(vp);

	/*
	 * XXX - keep stats for hash function
	 */
retry:
	for (smp = smd->smd_hash[SMAP_HASHFUNC(smd, vp, off)];
	    smp != NULL; smp = smp->sm_hash)
		if (smp->sm_vp == vp && smp->sm_off == off)
			break;

	if (smp != NULL) {
		if (vp->v_count == 0)			/* XXX - debugging */
			cmn_err(CE_WARN, "segmap_getmap vp count of zero");
		if (smp->sm_refcnt > 0) {
			segmapcnt.smc_get_use++;
			smp->sm_refcnt++;		/* another user */
			/*
			 * Check for nested use when someone owns uninitialized
			 * pages, so we free them in the right segmap_release.
			 */
			if (smp->sm_pgowner == (u_int)curproc)
				smp->sm_pgowncnt++;
		} else {
			segmapcnt.smc_get_reclaim++;
			segmap_smapsub(smd, smp);	/* reclaim */
		}
	} else {
		/*
		 * Allocate a new slot and set it up.
		 */
		while ((smp = smd->smd_free) == NULL) {
			/*
			 * XXX - need a better way to do this.
			 */
			smd->smd_want = 1;
			(void) sleep((caddr_t)&smd->smd_free, PSWP+2);
			/* 
			 * check here for someone else claiming the entry
			 * go back up to hash
			 */
			if (smd->smd_free)
				goto retry;
		}
		segmap_smapsub(smd, smp);
		if (smp->sm_vp != (struct vnode *)NULL) {
			/*
			 * Destroy old vnode association and unload any
			 * hardware translations to the old object.
			 */
			segmapcnt.smc_get_reuse++;
			segmap_hashout(smd, smp);
			/*
			 * Send down the HAT_FREEPP flag to free the pages
			 * if these are the last mappings to them. This is
			 * done to get them on the cache list faster.
			 * Otherwise the pages would be orphaned until the
			 * page daemon finds them.
			 */
			hat_unload(seg, seg->s_base + ((smp - smd->smd_sm) *
			    MAXBSIZE), MAXBSIZE, HAT_FREEPP);
			/* putpage can sleep, have to recheck before hashin */
			for (tsmp = smd->smd_hash[SMAP_HASHFUNC(smd, vp, off)];
			tsmp != NULL; tsmp = tsmp->sm_hash)
				if (tsmp->sm_vp == vp && tsmp->sm_off == off)
					break;
			if (tsmp != NULL)	/* found a dup */
			{
				smp->sm_refcnt = 0;
				segmap_smapadd(smd, smp);
				goto retry;
			}
		}
		segmap_hashin(smd, smp, vp, off);
	}

	trace5(TR_SEG_GETMAP, seg, (u_int)(seg->s_base +
		(smp - smd->smd_sm) * MAXBSIZE) & PAGEMASK,
		TRC_SEG_SEGKMAP, vp, off);

	return (seg->s_base + ((smp - smd->smd_sm) * MAXBSIZE));
}

int
segmap_release(seg, addr, flags)
	struct seg *seg;
	addr_t addr;
	u_int flags;
{
	register struct segmap_data *smd = (struct segmap_data *)seg->s_data;
	register struct smap *smp;
	u_int off;
	u_int pg;
	int error;

	if (addr < seg->s_base || addr >= seg->s_base + seg->s_size ||
	    ((u_int)addr & MAXBOFFSET) != 0)
		cmn_err(CE_PANIC, "segmap_release addr");

	smp = &smd->smd_sm[MAP_PAGE(seg, addr)];
	trace4(TR_SEG_RELMAP, seg, addr, TRC_SEG_SEGKMAP, smp->sm_refcnt);

	/*
	 * Check if any of the pages are uninitialized.
	 * If they are, we can mark them valid now.
	 */
	if (smp->sm_pgowner == (u_int)curproc && --smp->sm_pgowncnt == 0) {
		for (pg = 0, off = smp->sm_off;
				pg < PGPERSMAP; pg++, off += PAGESIZE) {
			if (SM_PG_UNINIT(smp, pg)) {
				page_t	*pp = page_find(smp->sm_vp, off);

				if (pp == NULL)
					cmn_err(CE_PANIC,
						"segmap_release: uninit page gone");
				ASSERT(pp->p_lock);
				ASSERT(pp->p_intrans && pp->p_pagein);
				ASSERT(pp->p_keepcnt > 0);
				hat_unlock(seg, (seg->s_base +
					   ((smp - smd->smd_sm) * MAXBSIZE))
					    + pg * PAGESIZE);
				pp->p_intrans = pp->p_pagein = 0;
				page_unlock(pp);
				PAGE_RELE(pp);
				SM_CLR_PG_UNINIT(smp, pg);
			}
		}
		smp->sm_pgowner = NULL;
	}

	/*
	 * If only the SM_DONTNEED flag or no flags are specified,
	 * then we skip the call to VOP_PUTPAGE all together.
	 */
	if (flags != SM_DONTNEED && flags != 0) {
		int bflags = 0;

		if (flags & SM_WRITE)
			segmapcnt.smc_rel_write++;
		if (flags & SM_ASYNC) {
			bflags |= B_ASYNC;
			segmapcnt.smc_rel_async++;
		}
		if (flags & SM_INVAL) {
			bflags |= B_INVAL;
			segmapcnt.smc_rel_abort++;
		}
		if (smp->sm_refcnt == 1) {
			/*
			 * We only bother doing the FREE and DONTNEED flags
			 * if no one else is still referencing this mapping.
			 */
			if (flags & SM_FREE) {
				bflags |= B_FREE;
				segmapcnt.smc_rel_free++;
			}
			if (flags & SM_DONTNEED) {
				bflags |= B_DONTNEED;
				segmapcnt.smc_rel_dontneed++;
			}
		}
		error = VOP_PUTPAGE(smp->sm_vp, smp->sm_off, MAXBSIZE, bflags,
		    (struct cred *)NULL);	/* XXX - need real cred val */
	} else {
		segmapcnt.smc_release++;
		error = 0;
	}

	if (--smp->sm_refcnt == 0) {
		if (flags & SM_INVAL) {
			segmap_hashout(smd, smp);	/* remove map info */
			/*
			 * Don't bother telling hat_unload() to free the
			 * pages, the VOP_PUTPAGE above took care of that
			 * if the SM_INVAL flag was set.
			 *
			 * XXX - In fact, I don't know why we have to call
			 * hat_unload() in this case because VOP_PUTPAGE
			 * would have called pvn_getdirty() which would
			 * have called hat_pageunload() if the SM_INVAL
			 * flag was set.
			 */
			hat_unload(seg, addr, MAXBSIZE, HAT_NOFLAGS);
		}
		segmap_smapadd(smd, smp);		/* add to free list */
	}

	return (error);
}

void
segmap_flush(seg, vp)
	struct seg *seg;
	register struct vnode *vp;
{
	register struct segmap_data *smd = (struct segmap_data *)seg->s_data;
	register struct smap *smp;
	register u_int	hashsz;
	register int	i;

	hashsz = smd->smd_hashsz;
	for (i=0; i<hashsz; i++) {
		for (smp = smd->smd_hash[i]; smp != NULL; smp = smp->sm_hash) {
			if (smp->sm_vp == vp) {
				segmap_hashout(smd, smp);
				hat_unload(seg, seg->s_base +
				  ((smp - smd->smd_sm) * MAXBSIZE),
				  MAXBSIZE, HAT_NOFLAGS);
			}
		}
	}
}

void
segmap_findmap(vp, off)
	struct vnode *vp;
	u_int off;
{
	struct seg *seg = segkmap;
	register struct segmap_data *smd = (struct segmap_data *)seg->s_data;
	register struct smap *smp;
	extern struct vnode *common_specvp();

	off = off & MAXBMASK;

	/*
	 * If this is a block device we have to be sure to use the
	 * "common" block device vnode for the mapping.
	 */
	if (vp->v_type == VBLK)
		vp = common_specvp(vp);

	/*
	 * XXX - keep stats for hash function
	 */
	for (smp = smd->smd_hash[SMAP_HASHFUNC(smd, vp, off)];
	    smp != NULL; smp = smp->sm_hash)
		if (smp->sm_vp == vp && smp->sm_off == off) {
			cmn_err(CE_CONT, " segmap %x", 
			 seg->s_base + ((smp - smd->smd_sm) * MAXBSIZE));
			 break;
		}
	cmn_err(CE_CONT, "\n");
}

