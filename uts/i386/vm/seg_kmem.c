#ident	"@(#)seg_kmem.c	1.2	92/06/28	JPB"

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

#ident  "@(#)kern-vm:seg_kmem.c	1.3.1.4"

/*
 * VM - kernel segment routines
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/vm.h"
#include "sys/user.h"
#include "sys/mman.h"
#include "sys/map.h"
#include "sys/tuneable.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/immu.h"
#include "sys/kmem.h"

#include "vm/seg_kmem.h"
#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/anon.h"
#include "vm/rm.h"
#include "vm/page.h"
#ifdef AT386	/* 16 MB support */
#include "sys/dmaable.h"
#endif	/* 16 MB support */

extern u_int	hat_vtokp_prot();
extern int	ptmalloc();
int availkmem;

/*
 * Private seg op routines.
 */

#if defined(__STDC__)
STATIC void segkmem_unload(struct seg *, addr_t, u_int, u_int);
STATIC int segkmem_setprot(struct seg *, addr_t, u_int, u_int);
STATIC int segkmem_checkprot(struct seg *, addr_t, u_int, u_int);
STATIC int segkmem_getprot(struct seg *, addr_t, u_int, u_int *);
STATIC int segkmem_fault(struct seg *, addr_t, u_int, enum fault_type, enum seg_rw);
STATIC int segkmem_faulta(struct seg *, addr_t);
STATIC off_t segkmem_getoffset(struct seg *, addr_t);
STATIC int segkmem_gettype(struct seg *, addr_t);
STATIC int segkmem_getvp(struct seg *, addr_t, struct vnode **);
STATIC void segkmem_badop(void);
STATIC int segkmem_alloc(struct seg *, addr_t, u_int, int);
STATIC void segkmem_free(struct seg *, addr_t, u_int);
STATIC void segkmem_mapin(struct seg *, addr_t, u_int, u_int, u_int, int);
STATIC int segkmem_mapout(struct seg *, addr_t, u_int);
#else
STATIC void segkmem_unload();
STATIC int segkmem_setprot();
STATIC int segkmem_checkprot();
STATIC int segkmem_getprot();
STATIC int segkmem_fault();
STATIC int segkmem_faulta();
STATIC off_t segkmem_getoffset();
STATIC int segkmem_gettype();
STATIC int segkmem_getvp();
STATIC void segkmem_badop();
STATIC int segkmem_alloc();
STATIC void segkmem_free();
STATIC void segkmem_mapin();
STATIC int segkmem_mapout();
#endif

/*
 * Machine specific public segments.
 */
struct seg ktextseg;
struct seg kvseg;
struct seg kpseg;
struct seg kpioseg;
struct seg kdvmaseg;

struct	seg_ops segkmem_ops = {
	(int(*)())segkmem_badop,		/* dup */
	(int(*)())segkmem_badop,		/* split */
	(void(*)())segkmem_badop,		/* free */
	segkmem_fault,
	segkmem_faulta,
	segkmem_unload,
	segkmem_setprot,
	segkmem_checkprot,
	(int(*)())segkmem_badop,		/* kluster */
	(u_int (*)())segkmem_badop,		/* swapout */
	(int(*)())segkmem_badop,		/* sync */
	(int(*)())segkmem_badop,		/* incore */
	(int(*)())segkmem_badop,		/* lockop */
	segkmem_getprot,
	segkmem_getoffset,
	segkmem_gettype,
	segkmem_getvp,
};

/*
 * The segkmem driver will (optional) use an array of pte's to back
 * up the mappings for compatibility reasons.  This driver treates
 * argsp as a pointer to the pte array to be used for the segment.
 */
int
segkmem_create(seg, argsp)
	struct seg *seg;
	caddr_t argsp;
{
	/*
	 * No need to notify the hat layer, since the SDT's are
	 * already allocated for seg_kmem; i.e. no need to call
	 * hat_map().
	 */

	seg->s_ops = &segkmem_ops;

	seg->s_data = seg->s_base;	/* must be set to something */

	return (0);
}

/*ARGSUSED*/
STATIC int
segkmem_fault(seg, addr, len, type, rw)
	struct seg *seg;
	addr_t addr;
	u_int len;
	enum fault_type type;
	enum seg_rw rw;
{
	if (type == F_SOFTLOCK || type == F_SOFTUNLOCK) {
		return (0);
	} else {
		return (-1);
	}
}

/*ARGSUSED*/
STATIC int
segkmem_faulta(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	return (-1);
}

/*ARGSUSED*/
STATIC void
segkmem_unload(seg, addr, ref, mod)
	struct seg *seg;
	addr_t addr;
	u_int ref, mod;
{

	/* ends up being called for mapout, ignore */
}

STATIC int
segkmem_setprot(seg, addr, len, prot)
	struct seg *seg;
	addr_t addr;
	u_int len, prot;
{
	register pte_t *ppte;
	register addr_t eaddr;
	register u_int pprot;

	ASSERT(seg->s_as == &kas);
	ASSERT(len <= seg->s_size);
	ASSERT(addr <= addr+len);
	ASSERT(addr >= seg->s_base && addr+len <= seg->s_base + seg->s_size);
	ASSERT(seg->s_ops == &segkmem_ops);
	if (prot)
		pprot = hat_vtokp_prot(prot);
	/*
	 *  386 does not have non-paged segment.
	 */
	ASSERT(seg->s_data);
	ppte = svtopte(addr);
	for (eaddr = addr + len; addr < eaddr; addr += PAGESIZE) {
		if (prot == 0) {
			/* invalidate software pte (if any) */

			/* XXX - *ppte++ = mmu_pteinvalid; */
			(ppte++)->pg_pte = 0;

		} else {
			/* assume physical segment has kernel write permissions */
			(ppte++)->pgm.pg_rw = pprot;
		}
		if (((ulong)ppte & PTMASK) == 0) {
			/* new page table */
			ppte = svtopte(addr);
		}
	}
	return 0;
}

STATIC int
getprot(ppte)
register pte_t *ppte;
{
	if (!PG_ISVALID(ppte))
		return (u_int) 0;
	else if (ppte->pgm.pg_rw)
		return (u_int) (PROT_READ | PROT_EXEC | PROT_WRITE);
	else
		return (u_int) (PROT_READ | PROT_EXEC);
}

STATIC int
segkmem_checkprot(seg, addr, len, prot)
	register struct seg *seg;
	register addr_t addr;
	u_int len;
	register u_int prot;
{
	register pte_t *ppte;
	pte_t tpte;
	addr_t eaddr;

	ASSERT(seg->s_as == &kas);
	ASSERT(len <= seg->s_size);
	ASSERT(addr <= addr+len);
	ASSERT(addr >= seg->s_base && addr+len <= seg->s_base + seg->s_size);
	ASSERT(seg->s_ops == &segkmem_ops);
	/*
	 *  386 does not have non-paged segment.
	 */
	ASSERT(seg->s_data);
	ppte = svtopte(addr);

	for (eaddr = addr + len; addr < eaddr; addr += PAGESIZE) {
		tpte = *ppte++;		/* structure assignment */
		if ((getprot(&tpte) & prot) != prot)
			return (EACCES);
		if (((ulong)ppte & PTMASK) == 0) {
			/* new page table */
			ppte = svtopte(addr);
		}
	}
        return (0);
}

STATIC int
segkmem_getprot(seg, addr, len, protv)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, *protv;
{
	u_int pgno = seg_page(seg, addr+len) - seg_page(seg, addr) + 1;
	register pte_t *ppte;
	register i;
	pte_t tpte;

	if (seg->s_data == NULL)
		return -1;

	ppte = svtopte(addr);

	for (i = 0; i < pgno; i++) {
		tpte = *ppte++;
		protv[i] = getprot(&tpte);
		if (((ulong)ppte & PAGEMASK) == 0) {
			/* new page table */
			ppte = svtopte(addr);
		}
	}

	return 0;
}

/* ARGSUSED */
STATIC off_t
segkmem_getoffset(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	return 0;
}

/* ARGSUSED */
STATIC int
segkmem_gettype(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	return MAP_SHARED;
}

/* ARGSUSED */
STATIC int
segkmem_getvp(seg, addr, vpp)
	register struct seg *seg;
	addr_t addr;
	struct vnode **vpp;
{
	*vpp = NULL;
	return -1;
}

STATIC void
segkmem_badop()
{

	cmn_err(CE_PANIC, "segkmem_badop");
}

/*
 * Special public segkmem routines
 */

/*
 * Allocate physical pages for the given kernel virtual address.
 * Performs most of the work of the old memall/vmaccess combination.
 */
STATIC int
#ifdef AT386	/* 16 MB support */
segkmem_alloc(seg, addr, len, flag)
#else
segkmem_alloc(seg, addr, len, canwait)
#endif	/* 16 MB support */
	struct seg *seg;
	addr_t addr;
	u_int len;
#ifdef AT386	/* 16 MB support */
	int flag;
#else
	int canwait;
#endif	/* 16 MB support */
{
	page_t *pp;
	register pte_t *ppte;
	pte_t tpte;
	int val;
	int flg;

	tpte.pg_pte = PG_V;
	ASSERT(seg->s_as == &kas);
	ASSERT(len <= seg->s_size);
	ASSERT(addr <= addr+len);
	ASSERT(addr >= seg->s_base && addr+len <= seg->s_base + seg->s_size);
	ASSERT(seg->s_ops == &segkmem_ops);
	/*
	 *  386 does not have non-paged segment.
	 */
	ASSERT(seg->s_data);

#ifdef AT386	/* 16 MB support */
	flg = ((flag & NOSLEEP) ? P_NOSLEEP : P_CANWAIT);
	if (!(flag & KM_NO_DMA))
		flg |= P_DMA;
#else
	flg = (canwait ? P_CANWAIT : P_NOSLEEP);
#endif	/* 16 MB support */

	flg |= P_NORESOURCELIM;

	pp = rm_allocpage(seg, addr, len, flg);

	if (pp != (page_t *)NULL) {
		ppte = svtopte(addr);
		while (pp != (page_t *)NULL) {
			ASSERT(!PG_ISVALID(ppte));
			tpte.pgm.pg_pfn = page_pptonum(pp);
			/* the Sun code would do a
			 * hat_pteload(seg, addr, pp, tpte, PTELD_LOCK);
			 * but we are not allocating the page table
			 * space for the p_mapping links.
			 * Those links would make this code
			 * unnecessarily messy (page table would
			 * alternate with mapping tables every segment).
			 * Instead, we rely on the p_keepcnt not getting
			 * decremented to mark the page as non-swappable.
			 * The page comes in with p_keepcnt == 1.
			 */
#ifdef DEBUG
			ASSERT(pp->p_uown == NULL);
			pp->p_uown = (struct proc *) addr;
			ASSERT(pp->p_vnode == NULL);
#endif
			page_sub(&pp, pp);
			*ppte++ = tpte;	/* structure assignment */
			addr += PAGESIZE;
			if (((ulong)ppte & PTMASK) == 0) {
				/* new page table */
				ppte = svtopte(addr);
			}
		}
		val = 1;
	} else
		val = 0;

	flushtlb();

	return (val);
}

/*ARGSUSED*/
STATIC void
segkmem_free(seg, addr, len)
	register struct seg *seg;
	addr_t addr;
	u_int len;
{
	page_t *pp;
	register pte_t *ppte;
	pte_t tpte;
	addr_t sv_addr = addr;
	u_int sv_len = len;

	ASSERT(seg->s_as == &kas);
	ASSERT(len <= seg->s_size);
	ASSERT(addr <= addr+len);
	ASSERT(addr >= seg->s_base && addr+len <= seg->s_base + seg->s_size);
	ASSERT(seg->s_ops == &segkmem_ops);
	/*
	 *  386 does not have non-paged segment.
	 */
	ASSERT(seg->s_data);
	ppte = svtopte(addr);

	for (; (int)len > 0; len -= PAGESIZE, addr += PAGESIZE) {
		tpte = *ppte;		/* structure assignment */
		ASSERT(PG_ISVALID(ppte));

		/* XXX - *ppte++ = mmu_pteinvalid; */
		(ppte++)->pg_pte = 0;

		pp = page_numtookpp(tpte.pgm.pg_pfn);
		if (pp == NULL)
			cmn_err(CE_PANIC, "segkmem_free");
#ifdef DEBUG
		ASSERT(pp->p_uown == (struct proc *) addr);
		pp->p_uown = NULL;
		ASSERT(pp->p_vnode == NULL);
		ASSERT(pp->p_keepcnt == 1);
#endif
		/*
		 * and release the keep count.
		 */
		PAGE_RELE(pp);			/* this should free the page */

		if (((ulong)ppte & PTMASK) == 0) {
			/* new page table */
			ppte = svtopte(addr);
		}
	}
	flushtlb();

}

/*
 * segkmem_mapin() and segkmem_mapout() are for manipulating kernel
 * addresses only. Since some users of segkmem_mapin() forget to unmap,
 * this is done implicitly.
 * NOTE: addr and len must always be multiples of the mmu page size. Also,
 * this routine cannot be used to set invalid translations.
 */
STATIC void
segkmem_mapin(seg, addr, len, vprot, pcookie, flags)
	struct seg *seg;
	register addr_t addr;
	register u_int len;
	u_int vprot;
	u_int pcookie;
	int flags;
{
	register pte_t *ppte;
	page_t *pp, *opp;
	pte_t apte;

	ASSERT(seg->s_as == &kas);
	ASSERT(len <= seg->s_size);
	ASSERT(addr <= addr+len);
	ASSERT(addr >= seg->s_base && addr+len <= seg->s_base + seg->s_size);
	ASSERT(seg->s_ops == &segkmem_ops);
	ASSERT(seg->s_data);
	ppte = svtopte(addr);

	apte.pg_pte = pcookie;
	if (vprot == PROT_NONE)
		cmn_err(CE_PANIC, "segkmem_mapin -- invalid ptes");
	apte.pgm.pg_rw = hat_vtokp_prot(vprot);
	apte.pgm.pg_v = 1;

	/*
	 * Always lock the mapin'd translations.
	 */

	for (; len != 0; addr += MMU_PAGESIZE, len -= MMU_PAGESIZE) {
		/*
		 * Determine the page frame we're mapping to allow
		 * the translation layer to manage cache consistency.
		 * If we're replacing a valid mapping, then ask the
		 * translation layer to unload the mapping and update
		 * data structures (necessary to maintain the information
		 * for cache consistency).  We use page_numtookpp here
		 * instead of page_numtopp so that we don't get a page
		 * struct for physical pages in transition.
		 */
		pp = page_numtookpp(apte.pgm.pg_pfn);
		ASSERT(pp == page_numtopp(apte.pgm.pg_pfn));

		if ((flags & PTELD_INTREP) == 0) {
			/*
			 * Because some users of mapin don't mapout things
			 * when they are done, we check for a currently
			 * valid translation.  If we find one, then we
			 * unlock the old translation now.
			 */
			if (PG_ISVALID(ppte)) {
				opp = page_numtookpp(ppte->pgm.pg_pfn);
				ASSERT(opp == page_numtopp(ppte->pgm.pg_pfn));
				/*
				 * If the page is different than the one
				 * already loaded, unload it now.  We test
				 * to see if the page is different before
				 * doing this since we can blow up if this
				 * virtual page contains a page struct used
				 * by hat_unload and we are just reloading
				 * going from non-cached to cached.
				 */
				if ((ppte->pg_pte &
				    (PG_V | PG_ADDR)) ==
				    (apte.pg_pte & (PG_V | PG_ADDR)))
					goto next_page;
#ifdef DEBUG
				if (opp) {
					ASSERT(opp->p_uown == (struct proc *) addr);
					opp->p_uown = NULL;
					ASSERT(opp->p_vnode == NULL);
				}
#endif
				if (opp) PAGE_RELE(opp);
			}
		}

#ifdef DEBUG
		if (pp) {
			/* the audit won't handle
			 * kmem/ublk multiple maps
			 */
			ASSERT(pp->p_keepcnt == 0);
			ASSERT(pp->p_uown == NULL);
			pp->p_uown = (struct proc *) addr;
			ASSERT(pp->p_vnode == NULL);
		}
#endif
		if (pp) pp->p_keepcnt++;

next_page:
		*ppte++ = apte;
		if (((ulong)ppte & PTMASK) == 0) {
			/* new page table */
			ppte = svtopte(addr);
		}
		apte.pgm.pg_pfn++;
	}
	flushtlb();
}

/*
 * Release mapping for the kernel. This frees pmegs, which are a critical
 * resource. The segment must be backed by software ptes. The pages
 * specified are only freed if they are valid. This allows callers to
 * clear out virtual space without knowing if it's mapped or not.
 * NOTE: addr and len must always be multiples of the page size.
 */
STATIC int
segkmem_mapout(seg, addr, len)
	struct seg *seg;
	register addr_t addr;
	register u_int len;
{
	register pte_t *ppte;
	page_t *pp;
	addr_t sv_addr = addr;
	u_int sv_len = len;
	int free_page = 0;

	ASSERT(seg->s_as == &kas);
	ASSERT(len <= seg->s_size);
	ASSERT(addr <= addr+len);
	ASSERT(addr >= seg->s_base && addr+len <= seg->s_base + seg->s_size);
	ASSERT(seg->s_ops == &segkmem_ops);
	if (seg->s_as != &kas)
		cmn_err(CE_PANIC,"segkmem_mapout: bad as");
	/*
	 *  386 does not have non-paged segment.
	 */
	ASSERT(seg->s_data);
	ppte = svtopte(addr);
	for (; len != 0; addr += MMU_PAGESIZE, len -= MMU_PAGESIZE) {
		if (!PG_ISVALID(ppte))
			goto next_page;
		PG_CLRVALID(ppte);
		pp = page_numtookpp(ppte->pgm.pg_pfn);
		ASSERT(pp == page_numtopp(ppte->pgm.pg_pfn));
#ifdef DEBUG
		if (pp) {
			ASSERT(pp->p_keepcnt == 1);
			ASSERT(pp->p_uown == (struct proc *) addr);
			pp->p_uown = NULL;
			ASSERT(pp->p_vnode == NULL);
		}
#endif
		if (pp) 
		{
			PAGE_RELE(pp);
			free_page++;
		}

next_page:
		if (((ulong)++ppte & PTMASK) == 0) {
			/* new page table */
			ppte = svtopte(addr);
		}
	}
	flushtlb();
	return(free_page);
}

/*
 * Allocate system virtual address space and allocate or link pages.
 */

int
sptalloc(size, mode, base, flag)
	register int size, mode;
	register caddr_t base;
	int flag;
{
	register i, sp;

	if (base == 0) {
		if (flag & NOSLEEP)
			i = tune.t_minakmem;
		else
			i = 0;

		if (size > availkmem - i)
			return NULL;
	}

	/*
	 * Allocate system virtual address space
	 */
	if ((sp = malloc(sptmap, size)) == 0) {
#ifdef DEBUG
		cmn_err(CE_WARN, "No kernel virtual space.");
		cmn_err(CE_CONT, "\tsize=%d, mode=%d, base=%d\n",
		  size, mode, base);
#endif
		return NULL;
	}

	/*
	 * Backwards compatibility: some drivers used PG_LOCK flag.
	 */
	mode &= ~O_PG_LOCK;

	/*
	 * Allocate and fill in pages
	 */
	if (base == 0) {

#ifdef AT386	/* 16 MB support */
		if (! segkmem_alloc(&kvseg, (addr_t)ctob(sp), ctob(size), flag))
#else
		if (! segkmem_alloc(&kvseg, (addr_t)ctob(sp), ctob(size),
			!(flag & NOSLEEP)))
#endif	/* 16 MB support */
		{
			rmfree(sptmap,size,sp);
			return (NULL);
		}
		else {
			availkmem -= size;
		}
	} else {
		segkmem_mapin(&kvseg, (addr_t)ctob(sp), ctob(size), PROT_WRITE,
			mkpte(0, (u_int)base), !(flag & NOSLEEP));
	}

	/*
	 *  Maintain backwards compatibility.
	 *  Fix protections for the kernel page table (SYSSEGS).
	 */

	for (i = 0; i < size; i++)
		*(int *)svtopte(ctob(sp + i)) |= mode;

	/*
	 *  Flush the translation lookaside buffer since modes get affected.
	 */

	flushtlb();

#ifdef AT386	/* 16 MB support */
#ifdef DEBUG
	if (dma_check_on && base == 0 && ! (flag & KM_NO_DMA)) {
		char * vaddr;

		for (vaddr = (char *)ctob(sp);
			vaddr < ((char *) ctob(sp)) + ctob(size); vaddr += PAGESIZE)
			if (! DMA_BYTE(vtop(vaddr, 0)))
				cmn_err(CE_PANIC,"sptalloc: dma: addr %x vtop %x\n",
						vaddr, vtop(vaddr, 0));
	}
#endif
#endif	/* 16 MB support */

	return(ctob(sp));
}

void
sptfree(vaddr, size, flag)
	caddr_t vaddr;
	register int size;
	int flag;
{
	register i, sp;

	sp = btoc(vaddr);
	if (flag) {
		availkmem += segkmem_mapout(&kvseg, vaddr, ctob(size));
	} else {
			for (i = 0; i < size; i++)
				*(int *)svtopte(ctob(sp + i)) = 0;
	}
	rmfree(sptmap, size, sp);
	flushtlb();
}


/*	Add a new segment to the kernel.  Returns the new
**	kernel segment virtual address.
*/

STATIC kseg_list_t ksegtbl;

char *
kseg(sz)
register int	sz;
{
	register char *va;
	register pfn;
	register kseg_list_t *kseg_new;

	if (sz <= 0 || sz > MAXKSEG)
		return(0);
	if ((pfn = ptmalloc(sptmap, sz, 0)) == 0)
		return(0);


	if (availrmem - sz < tune.t_minarmem  ||
	   availsmem - sz < tune.t_minasmem) {
		rmfree(sptmap, sz, pfn);
		nomemmsg("kseg", sz, 0, 0);
		return(0);
	}
	availrmem -= sz;
	availsmem -= sz;
	pages_pp_kernel += sz;

#ifdef AT386	/* 16 MB support */
	if (!segkmem_alloc(&kvseg, (addr_t)ctob(pfn), ctob(sz), 0))
#else
	if (!segkmem_alloc(&kvseg, (addr_t)ctob(pfn), ctob(sz), 1))
#endif	/* 16 MB support */
		cmn_err(CE_PANIC, "kseg - segkmem_alloc failed");

	va = (char *) ctob(pfn);
	kseg_new = (kseg_list_t *)kmem_alloc(sizeof(*kseg_new), KM_SLEEP);
	kseg_new->kseg_vaddr = va;
	kseg_new->kseg_size = sz;
	kseg_new->kseg_next = ksegtbl.kseg_next;
	ksegtbl.kseg_next = kseg_new;
	bzero(va, ctob(sz));

#ifdef AT386	/* 16 MB support */
#ifdef DEBUG
	{
		if (dma_check_on) {
		char *vaddr;

		for (vaddr = va; vaddr < (va + ctob(sz)); vaddr += PAGESIZE)
			if (! DMA_BYTE(vtop(vaddr, 0)))
				cmn_err(CE_PANIC,"kseg: dma: vaddr %x\n vtop %x\n",
						vaddr, vtop(vaddr, 0));
		}
	}
#endif
#endif	/* 16 MB support */

	return(va);
}

/* Free previously allocated kernel segment.
 */

void
unkseg(va)
register char	*va;
{
	register sz = 0, pfn, sn;
	register kseg_list_t *kseg_list = ksegtbl.kseg_next;
	register kseg_list_t *kseg_prev;

	if (kseg_list == NULL)
	{
		cmn_err(CE_WARN,"unkseg called without previous kseg");
		return;
	}
	else
		kseg_prev = &ksegtbl;

	while(kseg_list)
	{
		if (kseg_list->kseg_vaddr == va)
		{
			sz = kseg_list->kseg_size;
			kseg_prev->kseg_next = kseg_list->kseg_next;
			kmem_free(kseg_list, sizeof(*kseg_list));
			break;
		}
		else
		{
			kseg_prev = kseg_list;
			kseg_list = kseg_list->kseg_next;
		}
	}
	pfn = btoct(va);
	ASSERT(sz);
	segkmem_free(&kvseg, va, ctob(sz));
	availrmem += sz;
	availsmem += sz;
	pages_pp_kernel -= sz;
	flushtlb();
	rmfree(sptmap, sz, pfn);
}
