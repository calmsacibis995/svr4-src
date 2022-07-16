/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-vm:vm_hat.c	1.4.2.11"

/*
 * The hat layer manages the address translation hardware as a cache
 * driven by calls from the higher levels in the VM system.  Nearly
 * all the details of how the hardware is managed should not be visible
 * above this layer except for miscellaneous machine specific functions
 * (e.g. mapin/mapout) that work in conjunction with this code.  Other
 * than a small number of machine specific places, the hat data
 * structures seen by the higher levels in the VM system are opaque
 * and are only operated on by the hat routines.  Each address space
 * contains a struct hat and a page contains an opaque pointer which
 * is used by the hat code to hold a list of active translations to
 * that page.
 *
 * Notes:
 *
 *	The p_mapping list hanging off the page structure is protected
 *	by spl's in the Sun code. Currently, we do not have to do this
 *	because our mappings are not interrupt replaceable.
 *
 *	It is assumed by this code:
 *
 *		- no load/unload requests will span kernel/user boundaries.
 *		- all addresses to be mapped are on page boundaries.
 *		- all segment sizes, s_size, are multiples of the page
 *		  size.
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/immu.h"
#include "sys/vmparam.h"
#include "vm/vm_hat.h"
#include "vm/hat.h"
#include "vm/seg.h"
#include "vm/rm.h"
#include "vm/as.h"
#include "vm/page.h"
#include "vm/anon.h"
#include "vm/seg_vn.h"
#include "sys/mman.h"
#include "sys/bitmasks.h"
#include "sys/tuneable.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/sysmacros.h"
#include "sys/inline.h"
#include "sys/errno.h"
#include "sys/vnode.h"

#include "sys/tss.h"
#include "sys/proc.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/kmem.h"

#include "sys/disp.h"	/* added for preemption point PREEMPT() */
#ifdef WEITEK
#include "sys/weitek.h"
#endif

#ifdef DEBUG

/*
 * Checks to make sure lock/unlock operations are valid; i.e.
 * no unlock is done on a mapping that is not locked.
 *
 * NOTE:
 *
 * There is one limitation to this checking. The counter for the
 * number of SOFTLOCKs on a mapping is only 3 bits, hence only
 * 7 locks can be placed on a single mapping. This has proved
 * sufficient for debugging the 3b2.
 */
#define	HAT_LCK_CHCK	0

#endif

extern struct seg *segkmap;	/* kernel generic mapping segment */
extern struct seg *segu;

extern pte_t kpd0[];		/* the single page directory for the system */
extern struct proc *old_curproc;
extern struct proc *curproc;

extern page_t	*pages;
extern page_t	*epages;

#ifdef __STDC__
paddr_t svirtophys(u_long);
STATIC hatpt_t *hat_ptalloc(u_int);
STATIC void	hat_ptfree(hatpt_t *);
STATIC hatpt_t *hat_ptsteal(u_int);
STATIC void	hat_pteload(struct seg *, addr_t, page_t *, u_int, u_int, u_int);
#else
paddr_t svirtophys();
STATIC hatpt_t *hat_ptalloc();
STATIC void	hat_ptfree();
STATIC hatpt_t *hat_ptsteal();
STATIC void	hat_pteload();
#endif

#ifdef DEBUG
/*
 * Helps determine where a mapping was established.
 *
 * The PTE can only be tagged when HAT_LCK_CHCK is off !
 */
#define	HAT_TAG_PTE	1

#if HAT_TAG_PTE == 1

#define PG_DUP1	0x200
#define PG_DUP2	0x400
#define PG_DUP3	0x800
#define PG_MAP	0x400
#define PG_LD1	0xC00
#define PG_LD2	0xE00

#endif

/*
 * General debugging.
 */
u_int hat_debug = 0x4;
u_int hat_duppts = 1;
u_int hat_mappts = 1;
u_int hat_pgfail = 0;
u_int	hat_ptstolen = 0;
u_int	hat_mcflpcnt = 0;
#endif

STATIC int nptalloced;
STATIC int nhatfree;
STATIC int nhatptavail;

STATIC hatmc_t *hat_mcalloc();
STATIC void	hat_mcfree();

#define	HATPTCHKSZ	PAGESIZE
STATIC int hatpt_freeincr = HATPTCHKSZ/sizeof(hatpt_t);
STATIC hatpt_t *hatpt_freelist;
STATIC hatpt_t hat_activept;	/* list of active page tables */

extern int ext_freemem_wait;	/* use when waiting for free memory */

STATIC page_t mcfreelist;	/* list of pages with free mapping chunks */
STATIC int nmcalloced;
STATIC int nmcfree;
STATIC int nmcpalloced;

struct as kas;

/*
 * Initialize the hardware address translation structures.
 * Called by startup() after the vm structures have been allocated
 * and mapped in.
 */
void
hat_init()
{
	/*
	 * Initialize hatpt and mapping chunk page freelists.
	 * Initialize active page table list.
	 */

	ASSERT(HAT_MCPP*HAT_EPMC == 1024);
	ASSERT(PAGESIZE == MMU_PAGESIZE);
	hatpt_freelist = (hatpt_t *)NULL;
	mcfreelist.p_prev = mcfreelist.p_next = (page_t *)&mcfreelist;
	hat_activept.hatpt_next = hat_activept.hatpt_prev 
				= (hatpt_t *)&hat_activept;
}


STATIC void
unlink_ptap(as, ptap)
	register struct as	*as;
	register hatpt_t	*ptap;
{
	register struct hat	*hatp = &(as->a_hat);

	if (ptap->hatpt_forw == ptap) {
		hatp->hat_pts = hatp->hat_ptlast = (hatpt_t *)NULL;
	} else {
		ptap->hatpt_back->hatpt_forw = ptap->hatpt_forw;
		ptap->hatpt_forw->hatpt_back = ptap->hatpt_back;
		if (hatp->hat_pts == ptap) 
			hatp->hat_pts = ptap->hatpt_forw;
		if (hatp->hat_ptlast == ptap) 
			hatp->hat_ptlast = hatp->hat_pts;
	}
}


/*
 * Allocate hat structure for address space (as).
 * Called from as_alloc() when address space is being set up.
 * This is a null routine since as_alloc() has zero'd out
 * the as structure before calling hat_alloc().
 * There is no need to zero out the hat struct inside of as 
 * structure again.
 */
void
hat_alloc(as)
struct as *as;
{
	/*
	 * no page tables allocated as yet.
	 */
	/*
	register struct hat *hatp = &as->a_hat;

	hatp->hat_pts = (hatpt_t *) NULL;
	hatp->hat_ptlast = (hatpt_t *) NULL;
	*/
}

/*
 * Free all of the translation resources for the specified address space.
 * Called from as_free when the address space is being destroyed.
 */

void
hat_free(as)
register struct as *as;
{
	ASSERT(as != (struct as *)NULL);
	ASSERT(as != &kas);

	if (as != u.u_procp->p_as)
		hat_freeas(as, 0); 
	else if (hat_freeas(as, 1)) 
		flushtlb();
}

/* hat_remptel(): Internal HAT layer subroutine to remove
 * a PTE from the mapping linked list for its page.
 * It ORs the reference and modify bits into the page
 * structure as a side-effect (saves caller from doing
 * a separate page_numtopp()).
 */

STATIC
void
hat_remptel(mapp, ptep)
hatmap_t *mapp;
register pte_t *ptep;
{
	register page_t *pp;
	register hatmap_t *nextmap;

	if ((pp = (page_t *) page_numtopp(ptep->pgm.pg_pfn)) != (page_t *) NULL)
	{
		pp->p_ref |= ptep->pgm.pg_ref;
		pp->p_mod |= ptep->pgm.pg_mod;
		nextmap = (hatmap_t *)(&(pp->p_mapping));
		while (nextmap->hat_mnext != mapp) {
			if (nextmap->hat_mnext == (hatmap_t *)NULL) {
				/* Translation not in p_mapping list
				   (e.g. /dev/mem) */
				ASSERT(mapp->hat_mapv == 0);
				return;
			}
			nextmap = nextmap->hat_mnext;
		}
		ASSERT(mapptoptep(mapp) == ptep);
		nextmap->hat_mnext = mapp->hat_mnext;
		mapp->hat_mapv = 0;
	}
}

/* hat_freeas(): provides common processing for hat_free()
 * and hat_swapout() to do complete image wipes.
 */

STATIC
hat_freeas(as, zap_pdt)
struct as *as;
int zap_pdt;			/* 1 = invoked in context of current process */
				/* 0 = not invoked in context of current process */
{
	register hatpt_t *ptap;
	hatpt_t	 *eptap;
	register pte_t *ptep;
	register hatmap_t *mapp;
	int      mcndx;
	int      mcnum;
	hatptcp_t *ptcp;
	hatpgt_t *pt;
	struct hat *hatp = &as->a_hat;
	int ec;
	hatmcp_t *mcp;
	int didazap = 0;

	ptap = hatp->hat_pts;
	if (ptap == (hatpt_t *)NULL) {
		ASSERT(hatp->hat_ptlast == (hatpt_t *)NULL);
		return(0);
	}
	do {
		/*
		 * save current hatpt pointer in eptap here
		 * so the do loop will stop properly
		 */
		eptap = ptap;
		ASSERT(ptap && ptap->hatpt_aec);
		ASSERT(ptap->hatpt_pde.pg_pte && ptap->hatpt_pdtep);

		if (!zap_pdt) {		/* if not in context of current process */
			if (ptap->hatpt_locks != 0) {
#ifdef DEBUG
				cmn_err(CE_CONT, "hat_freeas: skipping locked page table !\n");
#endif
				ptap = ptap->hatpt_forw;
				continue;
			}
		}

		ASSERT(ptap->hatpt_locks == 0);

		pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);
		mcp = ptap->hatpt_mcp;
		for (mcnum = 0; mcnum < HAT_MCPP && ptap->hatpt_aec > 0; mcp++, mcnum++) {
			if (mcp->hat_mcp == 0) {
#ifdef DEBUG
				ptep = pt->hat_pgtc[mcnum].hat_pte;
				for (mcndx = 0; mcndx < HAT_EPMC; ptep++, mcndx++)
					ASSERT(ptep->pg_pte == 0);
#endif
				continue;
			}
			mapp = mcptomapp(mcp);
			ASSERT(((uint)mapp & ~HATMAP_ADDR) == 0);
			ptcp = mapptoptcp(mapp);
			ec = ptcp->ptp.hat_mcaec;
			ASSERT(ec);
			ASSERT(ptap->hatpt_aec >= ec);
			ptap->hatpt_aec -= ec;
			ptcp->ptp.hat_mcaec = 0;
			ptep = pt->hat_pgtc[mcnum].hat_pte;
			ASSERT(ptcptoptep(ptcp) == ptep);
			for (mcndx = 0; mcndx < HAT_EPMC && ec > 0; mapp++, ptep++, mcndx++) {
				if (ptep->pg_pte == 0) {
					ASSERT(mapp->hat_mapv == 0);
					continue;
				}

#if DEBUG && HAT_LCK_CHCK == 1
				ASSERT(!PG_ISLOCKED(ptep)); 
#endif

				as->a_rss--;

				/*
				 * Must remove pte from its p_mapping list
				 */
				hat_remptel(mapp, ptep);

				/*
				 * Until a translation cache exists:
				 * Don't have to NULL out the pte
				 * because we are going to hat_ptfree()
				 * the whole page table. hat_ptalloc()
				 * will zero it before it is reallocated.
				 *
				 * Once cache exists, zero as we go
				 * since that will be much less work.
				 */
				ec--;
			}
			ASSERT(ec == 0);
#ifdef DEBUG
			for ( ; mcndx < HAT_EPMC; ptep++, mcndx++, mapp++) {
				ASSERT(ptep->pg_pte == 0);
				ASSERT(mapp->hat_mapv == 0);
			}
#endif
			hat_mcfree(--mapp); /* free this mapping chunk */
		}
		ASSERT(ptap->hatpt_aec == 0);
#ifdef DEBUG
		for (; mcnum < HAT_MCPP; mcp++, mcnum++) {
			ptep = pt->hat_pgtc[mcnum].hat_pte;
			for (mcndx = 0; mcndx < HAT_EPMC; ptep++, mcndx++)
				ASSERT(ptep->pg_pte == 0);
			ASSERT(mcp->hat_mcp == 0);
		}
#endif

		/*
		 * if we need to zap pdt, zero out pd entry 
		 * and set a flag to do flushtlb later.
		 */
		if (zap_pdt) {		/* if in context of current process */
			ASSERT(ptap->hatpt_pdtep->pgm.pg_pfn ==
						 ptap->hatpt_pde.pgm.pg_pfn);
			(ptap->hatpt_pdtep)->pg_pte = 0;
			didazap = 1;
		}
	
		ptap = ptap->hatpt_forw;
		ASSERT(ptap);
		hat_ptfree(ptap->hatpt_back);
	} while (ptap != eptap);
	return(didazap);
}

/*
 * Set up any translation structures, for the specified address space,
 * that are needed or preferred when the process is being swapped in.
 *
 * For the 386, we
 * don't do anything here. The page tables will be allocated by hat_memload.
 */
void
hat_swapin(as)
struct as *as;
{
}

/*
 * Free all of the translation resources, for the specified address space,
 * that can be freed while the process is swapped out. Called from as_swapout.
 *
 */
void
hat_swapout(as)
struct as *as;
{
	ASSERT(as != (struct as *)NULL);
	ASSERT(as != u.u_procp->p_as);
	ASSERT(as != &kas);

	(void) hat_freeas(as, 0);
}


/*
 * Unload all of the hardware translations that map page pp.
 */
void
hat_pageunload(pp)
	register page_t *pp;
{
	register pte_t *ptep;
	register hatmap_t *mapp, *nmapp;
	page_t *ptpp;
	register hatptcp_t *ptcp;
	paddr_t	ptcpphys;
	hatpt_t *ptap;
	int mcnx;
	struct as *as;
	struct as *cur_as = u.u_procp->p_as;
	int flush = 0;

	ASSERT(pp >= pages && pp < epages);
	mapp = (hatmap_t *)(pp->p_mapping);
	pp->p_mapping = (caddr_t)NULL;

	while (mapp != NULL) {
		nmapp = mapp->hat_mnext;
		mapp->hat_mapv = 0;
		mcnx = HATMAPMCNDX(mapp);
		ptcp = mapptoptcp(mapp);
		ASSERT(ptcp->ptp.hat_mcaec);
		ptep = (pte_t *)(ptcp->hat_ptpv & HATPTC_ADDR) + mcnx;
				/* "& HATPT_ADDR" to get rid of hat_mcaec */ 
		ASSERT(ptep->pg_pte != 0);
		ASSERT(page_numtopp(ptep->pgm.pg_pfn) == pp);
		pp->p_ref |= ptep->pgm.pg_ref;
		pp->p_mod |= ptep->pgm.pg_mod;

		/* 
	 	 * find the hatpt structure associated 
	 	 * with this map
	 	 */
		ptcpphys = svirtophys(ptcp->hat_ptpv);	
		ptpp = page_numtopp(btoct(ptcpphys));
		ASSERT(ptpp >= pages && ptpp < epages);
		ptap = ptpp->phat_ptap;

#if DEBUG && HAT_LCK_CHCK == 1
		ASSERT(!PG_ISLOCKED(ptep));
#endif

		ptep->pg_pte = 0;	/* we can clear pte now */

		as = ptap->hatpt_as;
		as->a_rss--;

		if (--(ptcp->ptp.hat_mcaec) == 0) {
#ifdef DEBUG
		   ptep -= mcnx;  mapp -= mcnx;
		   for (mcnx = 0; mcnx < HAT_EPMC; mcnx++, ptep++, mapp++) {
			ASSERT(ptep->pg_pte == 0);
			ASSERT(mapp->hat_mapv == 0);
		   }
		   --mapp;
#endif
	    	   ptap->hatpt_mcp[HATMAPMCNO(ptcp->hat_ptpv)].hat_mcp = 0;
	    	   hat_mcfree(mapp);
		}

		ASSERT(ptap->hatpt_aec);
		if (--(ptap->hatpt_aec) == 0) {
#ifdef DEBUG
			u_int	idx;

			ptep = (pte_t *)pfntokv(ptap->hatpt_pde.pgm.pg_pfn);
			for (idx = 0; idx < NPGPT; idx++, ptep++)
				ASSERT(ptep->pg_pte == 0);
			for (idx = 0; idx < HAT_MCPP; idx++)
				ASSERT(ptap->hatpt_mcp[idx].hat_mcp == 0);
#endif

			if (ptap->hatpt_locks == 0) {

				/*
				 * Check if we are freeing the current process' 
				 * page table. If we are, we need to clear the
				 * corresponding pd entry.  
				 */
				if (as == cur_as || as == &kas) {
					ASSERT(ptap->hatpt_pdtep->pgm.pg_pfn ==
						ptap->hatpt_pde.pgm.pg_pfn);
					ptap->hatpt_pdtep->pg_pte = 0;
				}

				hat_ptfree(ptap);
			}
		}
		mapp = nmapp;

		if (as == cur_as || as == &kas)
			++flush;
	}

	if (flush)
		flushtlb();
}


/*
 * Remove write permissions, if any, from all existing mappings
 * to a given page.  This is called from page_rdonly when a filesystem
 * creates new "holes" in a page (by skipping past the EOF).
 */

void
hat_rdonly(pp)
	register page_t *pp;
{
	register pte_t *ptep;
	register hatmap_t *mapp, *nmapp;
	page_t *ptpp;
	register hatptcp_t *ptcp;
	paddr_t	ptcpphys;
	hatpt_t *ptap;
	int mcnx;
	struct as *as;
	struct as *cur_as = u.u_procp->p_as;
	int flush = 0;

	ASSERT(pp >= pages && pp < epages);
	mapp = (hatmap_t *)(pp->p_mapping);

	while (mapp != NULL) {
		nmapp = mapp->hat_mnext;
		mcnx = HATMAPMCNDX(mapp);
		ptcp = mapptoptcp(mapp);
		ASSERT(ptcp->ptp.hat_mcaec);
		ptep = (pte_t *)(ptcp->hat_ptpv & HATPTC_ADDR) + mcnx;
				/* "& HATPT_ADDR" to get rid of hat_mcaec */ 
		ASSERT(ptep->pg_pte != 0);
		ASSERT(page_numtopp(ptep->pgm.pg_pfn) == pp);

		/* 
	 	 * find the hatpt structure associated 
	 	 * with this map
	 	 */
		ptcpphys = svirtophys(ptcp->hat_ptpv);	
		ptpp = page_numtopp(btoct(ptcpphys));
		ptap = ptpp->phat_ptap;
		as = ptap->hatpt_as;

		mapp = nmapp;

		if (ptep->pg_pte & PG_RW) {
			ptep->pg_pte &= ~PG_RW;
			if (as == cur_as || as == &kas)
				++flush;
		}
	}

	if (flush)
		flushtlb();
}

/*
 * Unload all of the mappings in the range [addr,addr+len] .
 */
void
hat_unload(seg, addr, len, flags)
	struct seg	*seg;
	register	addr_t addr;
	u_int		len;
	u_int		flags;
{
	struct hat		*hatp = &seg->s_as->a_hat;
	addr_t			endaddr;
	register hatpt_t	*ptap;
	hatpt_t			*eptap;
	register pte_t		*ptep;
	register hatmap_t	*mapp;
	hatptcp_t		*ptcp;
	int			mcnum, mcndx;
	hatpgt_t		*pt;
	hatmcp_t		*mcp;
	int			didazap = 0;
	addr_t			endmc, endpg;
	pte_t			*vpdte;
	u_int			free_page = flags & HAT_FREEPP;
	u_int			put_page = flags & (HAT_PUTPP|HAT_FREEPP);
	u_int			rele_page = flags & HAT_RELEPP;
	page_t			*pp;

	/* addr must be page aligned */
	ASSERT(((u_long) addr & POFFMASK) == 0);

	ptap = hatp->hat_pts;
	if (ptap == (hatpt_t *)NULL) {
		ASSERT(hatp->hat_ptlast == (hatpt_t *)NULL);
		return;
	}
	endaddr = addr + len;
	ASSERT(endaddr >= addr);
	vpdte = kpd0 + ptnum(addr);
	do {
		ASSERT(ptap && ptap->hatpt_aec);
		ASSERT(ptap->hatpt_pde.pg_pte && ptap->hatpt_pdtep);

		/*
		 * save current hatpt pointer in eptap here
		 * so the do loop will stop properly
		 */
		eptap = ptap;

		if (addr >= endaddr) break;
		if (vpdte > ptap->hatpt_pdtep) {
			ptap = ptap->hatpt_forw;
			ASSERT(ptap && ptap->hatpt_aec);
			ASSERT(ptap->hatpt_pde.pg_pte && ptap->hatpt_pdtep);
			if (ptap == hatp->hat_pts)
				break;
			continue;
		}
		while (vpdte < ptap->hatpt_pdtep) {
			addr = (addr_t) ((++vpdte - kpd0) * VPTSIZE);
			if (addr >= endaddr) goto done;
		}
		pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);
		mcp = ptap->hatpt_mcp;
		mcnum = HATMCNO(addr);
		mcp += mcnum;
		mcndx = HATMCNDX(addr);
		for (; mcnum < HAT_MCPP; mcp++, mcnum++) {
			if (mcp->hat_mcp == 0) {
				mcndx = 0;
				addr = (addr_t)(((uint)addr + HATMCSIZE)
						& HATVMC_ADDR);
				if (addr >= endaddr) goto done;
				continue;
			}
			mapp = mcptomapp(mcp);
			ASSERT(((uint)mapp & ~HATMAP_ADDR) == 0);
			ptcp = mapptoptcp(mapp);
			ASSERT(ptcp->ptp.hat_mcaec);
			ASSERT((uint)ptap->hatpt_aec >= ptcp->ptp.hat_mcaec);
			ptep = pt->hat_pgtc[mcnum].hat_pte;
			ASSERT(ptcptoptep(ptcp) == ptep);
			mapp += mcndx;
			ptep += mcndx;
			for (; mcndx < HAT_EPMC; addr += PAGESIZE, mapp++, ptep++, mcndx++) {
				if (addr >= endaddr) goto done;
				if (ptep->pg_pte == 0) {
					ASSERT(mapp->hat_mapv == 0); 
					continue;
				}

				if (flags & HAT_UNLOCK) {
					ASSERT(ptap->hatpt_locks > 0);
					--ptap->hatpt_locks;
#if DEBUG && HAT_LCK_CHCK == 1
					ASSERT(PG_ISLOCKED(ptep));
					PG_CLRLOCK(ptep);
#endif
				}
#if DEBUG && HAT_LCK_CHCK == 1
				else {
					ASSERT(!PG_ISLOCKED(ptep));
				}
#endif

				if ((pp = (page_t *) page_numtopp(ptep->pgm.pg_pfn)) != (page_t *) NULL) {
					/*
				 	* Must remove pte from its p_mapping list
				 	*/
					hat_remptel(mapp, ptep);

					if (rele_page) {
						PAGE_RELE(pp);
					}

					if (put_page &&
						(pp->p_keepcnt == 0) &&
						(pp->p_mapping == 0) &&
						(pp->p_lock == 0) &&
						(pp->p_free == 0) &&
						(pp->p_intrans == 0) &&
						(pp->p_lckcnt == 0) &&
						(pp->p_cowcnt == 0)) {
						/* The following may sleep,
						   so lock down the page table. */
						ptap->hatpt_locks++;
						if (pp->p_mod && pp->p_vnode) {
							(void)
							VOP_PUTPAGE(pp->p_vnode,
							pp->p_offset, PAGESIZE,
							B_ASYNC | B_FREE,
							(struct cred *) 0);
						} else if (free_page) {
							page_lock(pp);
							page_free(pp, 0);
						}
						ptap->hatpt_locks--;
					}
				}

				ASSERT(seg->s_as->a_rss > 0);
				seg->s_as->a_rss--;

				ptep->pg_pte = 0;
				didazap = 1;
				ASSERT(ptcp->ptp.hat_mcaec);
				ASSERT(ptap->hatpt_aec);
				ptap->hatpt_aec--;
				if (--ptcp->ptp.hat_mcaec == 0) {
					mcp->hat_mcp = 0;
					hat_mcfree(mapp);
					if (ptap->hatpt_aec == 0) {
					    ptap = ptap->hatpt_forw;
					    addr = (addr_t)(((uint)addr
							+ VPTSIZE) & PT_ADDR);
					    if (eptap->hatpt_locks == 0) {
						if (seg->s_as == &kas ||
						    seg->s_as == u.u_procp->p_as) {
				   ASSERT(eptap->hatpt_pdtep->pgm.pg_pfn
					== eptap->hatpt_pde.pgm.pg_pfn);
						    (eptap->hatpt_pdtep)->pg_pte = 0;
						}
						hat_ptfree(eptap);
					    }
					    goto endptloop;
					}
					addr = (addr_t)(((uint)addr
						+ HATMCSIZE) & HATVMC_ADDR);
					/* Reset mcndx here so initial
					 * entry into the loops can be
					 * in the middle of a chunk.
					 */
					mcndx = 0;
					goto endmcloop;
				}
				ASSERT(ptap->hatpt_aec);
			}
			/* Can fall through when we start in middle of chunk */
			mcndx = 0;
endmcloop:		;
		}
		/* Can fall through when we start in middle of page table */
		ASSERT(ptap->hatpt_aec != 0);
		ptap = ptap->hatpt_forw;
endptloop:	;	
	} while (ptap != eptap);
done:
	if (didazap && (seg->s_as == &kas || u.u_procp->p_as == seg->s_as))
		flushtlb();
}

/* A FEW WORDS ON PROTECTIONS ON THE 386:
 *
 * The 32-bit virtual addresses we all know and love are
 * mapped identically to the linear address space (ignoring
 * floating point emulation), so that 0 virtual is 0 linear.
 * The mapping occurs via segment descriptors.
 * There are separate segments for user text (0 thru 0xBFFFFFFF),
 * user data (0 thru 0xFFFFFFFF), kernel text (0 thru 0xFFFFFFFF),
 * and kernel data (0 thru 0xFFFFFFFF).
 * The segment descriptors are the only place that executability
 * can be specified and the segments must be fixed for 32-bit
 * pointers to work as expected.
 * Thus the HAT layer only manipulates the linear address space,
 * which consists of page tables and the page directory table.
 * PROT_EXEC is treated as PROT_READ, since the 386 does not provide
 * execute permission control at the page level.
 * Besides the present bit, there are two bits for protection.
 * One bit controls user access, and the other controls writability.
 * The way the hardware works in practice, the kernel can write
 * all valid pages, some combinations do not really exist, but
 * we can pretend.
 * So there are four permission bit states: KR, KW, UR, UW.
 * Of these, KR and KW are effectively the same.
 *
 * hat_vtop_prot() implements our choices.
 * It returns the value for the two bits in PTE position,
 * along with the valid bit (PG_V), and all other bits zero
 * (to be used along with ~(PTE_PROTMASK|PG_V) to modify the valid PTEs).
 */

u_int
hat_vtop_prot(vprot)
register u_int vprot;
{
	vprot &= PROT_ALL;
	if (vprot & PROT_EXEC) {
		vprot &= ~PROT_EXEC;
		vprot |= PROT_READ;
	}
	if (vprot & PROT_WRITE)
		vprot &= ~PROT_READ;
	switch(vprot) {
	case PROT_READ:
		return(PG_V);		/* KR */
	case PROT_WRITE:
		return(PG_RW|PG_V);	/* KW */
	case PROT_USER|PROT_READ:
		return(PG_US|PG_V);	/* UR */
	case PROT_USER|PROT_WRITE:
		return(PTE_RW|PG_V);	/* UW */
	default:
		/* PROT_NONE and PROT_USER|PROT_NONE */
		return(PG_US);		/* page not valid */
			/*
			 * The reason we return PG_US instead of 0 for the
			 * PROT_NONE case, is to get around a potential
			 * problem if this protection is used with page
			 * frame number 0, since we might end up with a 0
			 * pte which is really mapped.  Since the kernel
			 * never users PROT_NONE, the only way this can
			 * happen is if a user maps physical address 0
			 * through /dev/*mem and then uses mprotect()
			 * to set its protection to PROT_NONE.
			 */
	}
}

/*
 * hat_vtokp_prot(vprot):
 * used for kernel segments to check the requested virtual page
 * protections and to convert them to the physical protections.
 * Only ro/rw permissions are available in the page table entry
 * (using the read-write bit).
 * So, only kernel level permissions are permitted.
 * The value returned is the value to be placed in the read-write bit.
 */
u_int
hat_vtokp_prot(vprot)
register u_int	vprot;
{
	if (vprot & PROT_USER) {
		cmn_err(CE_PANIC, "hat_vtokp_prot: user addr in kernel space");
		/* NOTREACHED */
	}

	switch (vprot) {
		case 0:
			cmn_err(CE_PANIC, "hat_vtokp_prot: null permission");
			/* NOTREACHED */
		case PROT_READ:
		case PROT_EXEC:
		case PROT_READ | PROT_EXEC:
			return(0);
		case PROT_WRITE:
		case PROT_READ | PROT_WRITE:
		case PROT_READ | PROT_WRITE | PROT_EXEC:
			return(1);
		default:
			cmn_err(CE_PANIC, "hat_vtokp_prot: bad prot");
			/* NOTREACHED */
	}
}

/*
 * Change the protections for the virtual address range [addr,addr+len]
 * to the protection prot.
 */
void
hat_chgprot(seg, addr, len, prot)
struct seg *seg;
addr_t addr;
u_int len;
u_int prot;
{
	struct hat *hatp = &seg->s_as->a_hat;
	addr_t endaddr;
	register hatpt_t *ptap;
	hatpt_t  *eptap;
	register pte_t *ptep;
	register hatmap_t *mapp;
	int mcndx;
#ifdef DEBUG
	hatptcp_t *ptcp;
#endif
	int mcnum;
	hatpgt_t *pt;
	hatmcp_t *mcp;
	pte_t *vpdte;
	u_int pprot, pmask;
	int   s;

	/* addr must be page aligned */
	ASSERT(((u_long) addr & POFFMASK) == 0);

	ptap = eptap = hatp->hat_pts;
	if (ptap == (hatpt_t *)NULL) {
		ASSERT(hatp->hat_ptlast == (hatpt_t *)NULL);
		return;
	}

	if (prot == ~PROT_WRITE) {
		pmask = ~PG_RW;
		pprot = 0;
	} else {
		pmask = ~(PTE_PROTMASK|PG_V);
		pprot = hat_vtop_prot(prot);
	}

	endaddr = addr + len;
	ASSERT(endaddr >= addr);
	vpdte = kpd0 + ptnum(addr);
	do {
		ASSERT(ptap && ptap->hatpt_aec);
		ASSERT(ptap->hatpt_pde.pg_pte && ptap->hatpt_pdtep);
		if (addr >= endaddr) break;
		if (vpdte > ptap->hatpt_pdtep) {
			ptap = ptap->hatpt_forw;
			ASSERT(ptap && ptap->hatpt_aec);
			ASSERT(ptap->hatpt_pde.pg_pte && ptap->hatpt_pdtep);
			continue;
		}
		while (vpdte < ptap->hatpt_pdtep) {
			addr = (addr_t) ((++vpdte - kpd0) * VPTSIZE);
			if (addr >= endaddr) goto done;
		}
		pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);
		mcp = ptap->hatpt_mcp;
		mcnum = HATMCNO(addr);
		mcp += mcnum;
		mcndx = HATMCNDX(addr);
		for (; mcnum < HAT_MCPP; mcp++, mcnum++) {
			if (mcp->hat_mcp == 0) {
				mcndx = 0;
				addr = (addr_t)(((uint)addr + HATMCSIZE)
						& HATVMC_ADDR);
				if (addr >= endaddr) goto done;
				continue;
			}
			mapp = mcptomapp(mcp);
			ASSERT(((uint)mapp & ~HATMAP_ADDR) == 0);
#ifdef DEBUG
			ptcp = mapptoptcp(mapp);
			ASSERT(ptcp->ptp.hat_mcaec);
			ASSERT((uint)ptap->hatpt_aec >= ptcp->ptp.hat_mcaec);
#endif
			ptep = pt->hat_pgtc[mcnum].hat_pte;
#ifdef DEBUG
			ASSERT(ptcptoptep(ptcp) == ptep);
#endif
			mapp += mcndx;
			ptep += mcndx;
			for (; mcndx < HAT_EPMC; addr += PAGESIZE, mapp++, ptep++, mcndx++) {
				if (addr >= endaddr) goto done;
				if (ptep->pg_pte == 0) {
					ASSERT(mapp->hat_mapv == 0);
					continue;
				}
				/* 
				 * no need to check for valid pte here
				 * since we're just clearing valid bit
				 * or setting protection.
				 */
				ptep->pg_pte = pprot | (ptep->pg_pte & pmask);
				ASSERT(ptep->pg_pte != 0);
			}
			/* reset mcndx here so we can start in midchunk */
			mcndx = 0;
		}
		ptap = ptap->hatpt_forw;
	} while (ptap != eptap);
done:
	if (seg->s_as == &kas || seg->s_as == u.u_procp->p_as) 
		flushtlb();
}

/*
 * Get all the hardware dependent attributes for a page struct
 */
void
hat_pagesync(pp)
register page_t *pp;
{
	register pte_t *ptep;
	register hatmap_t *mapp;

	register struct as *cur_as = u.u_procp->p_as;
	register paddr_t ptcpphys;
	register hatpt_t *ptap;
	page_t *ptpp;
	int flush = 0;

	ASSERT(pp >= pages && pp < epages);

	mapp = (hatmap_t *)pp->p_mapping;

	while (mapp != (hatmap_t *)NULL) {
		ptep = mapptoptep(mapp);
		ASSERT(page_pptonum(pp) == ptep->pgm.pg_pfn);
		pp->p_ref |= ptep->pgm.pg_ref;
		pp->p_mod |= ptep->pgm.pg_mod;

		ptcpphys = svirtophys((u_long)ptep);
		ptpp = page_numtopp(btoct(ptcpphys));
		ASSERT(ptpp);
		ptap = ptpp->phat_ptap;
		ASSERT(ptap);
		ASSERT((u_int)ptap->hatpt_pde.pgm.pg_pfn ==
			(u_int)btoct(ptcpphys));
		if ((ptap->hatpt_as == cur_as || ptap->hatpt_as == &kas) &&
				(ptep->pg_pte & (PG_REF|PG_M)))
			flush = 1;
		ptep->pg_pte &= ~(PG_REF|PG_M);

		mapp = mapp->hat_mnext;
	}

	if (flush)
		flushtlb();
}

/*
 * Set up addr to map to page pp with protection prot.
 */
/*ARGSUSED*/
void
hat_memload(seg, addr, pp, prot, lock)
struct seg *seg;
addr_t addr;
page_t *pp;
u_int  prot;
u_int  lock;
{
	ASSERT(pp >= pages && pp < epages);

	hat_pteload(seg, addr, pp, page_pptonum(pp), prot, lock);
}

/*ARGSUSED*/
void
hat_devload(seg, addr, ppid, prot, lock)
struct seg *seg;
addr_t addr;
u_int ppid;
u_int prot;
u_int lock;
{
	hat_pteload(seg, addr, NULL, ppid, prot, lock);
}

/* 
 * Set up addr to map pfn with protection prot.
 * May or may not have an associated pp structure.
 */

STATIC void
hat_pteload(seg, addr, pp, pf, prot, lock)
struct seg *seg;
addr_t addr;
register page_t *pp;
u_int  pf;
u_int  prot;
u_int  lock;
{
	struct hat *hatp = &seg->s_as->a_hat;
	register hatpt_t *ptap;
	hatpt_t  *eptap, *new_ptap;
	register pte_t *ptep;
	register hatmap_t *mapp;
	hatptcp_t *ptcp;
	int mcndx;
	int mcnum;
	hatpgt_t *pt;
	hatmcp_t *mcp;
	pte_t *vpdte;
	u_int mode;
	struct as	*cur_as = seg->s_as;

	/* addr must be page aligned */
	ASSERT(((u_int) addr & POFFMASK) == 0);
	
	mode = hat_vtop_prot(prot);
	
	vpdte = kpd0 + ptnum(addr);

	/* only segkmap and segu are allowed to do hat_pteload if kas */
	/* panic if kernel address, but not segkmap nor segu */
	ASSERT(!(cur_as == &kas && seg != segkmap && seg != segu));

	/* 
	 * First: find or allocate the page table.
	 * Note that hat_ptalloc() allocates both the page table
	 * and the hatpt_t and sets up hatpt_pde.
	 */

	new_ptap = (hatpt_t *)NULL;
pt_search:
	if ((ptap = hatp->hat_pts) == (hatpt_t *)NULL) {
		ASSERT(hatp->hat_ptlast == (hatpt_t *)NULL);
		if (new_ptap == (hatpt_t *)NULL) {
			/*
			 * If the thing we are mapping is associated with a
			 * pp, we must bump the keepcnt to prevent it from
			 * being stolen because it is possible to sleep in
			 * hat_ptalloc().
			 */
			if (pp) {
				++pp->p_keepcnt;
				new_ptap = hat_ptalloc(HAT_CANWAIT);
				--pp->p_keepcnt;
			} else {
				new_ptap = hat_ptalloc(HAT_CANWAIT);
			}
			ASSERT(hatp->hat_pts == (hatpt_t *)NULL);
			goto pt_search;
		}
		ptap = new_ptap;

		/* only user page tables on the active PT list */
		if (cur_as != &kas)
			APPEND_PT(ptap, hat_activept);

		ptap->hatpt_as = cur_as;
		ptap->hatpt_forw = ptap->hatpt_back = ptap;
		hatp->hat_pts = hatp->hat_ptlast = ptap;
		ptap->hatpt_pdtep = vpdte;
		if (cur_as == &kas || cur_as == u.u_procp->p_as) {
			ASSERT(vpdte->pg_pte == 0);
			vpdte->pg_pte = ptap->hatpt_pde.pg_pte;
		}
	} else if (ptap->hatpt_pdtep != vpdte) {
		eptap = ptap;
		do {
			ASSERT(ptap);
			if (ptap->hatpt_pdtep == vpdte)
				goto found_ptap;
			if (ptap->hatpt_pdtep > vpdte) break;
			ptap = ptap->hatpt_forw;
		} while (ptap != eptap);

		if (new_ptap == (hatpt_t *)NULL) {
			/* 
			 * If the thing we are mapping is associated with a
			 * pp, we must bump the keepcnt to prevent it from
			 * being stolen because it is possible to sleep in
			 * hat_ptalloc().
			 */
			if (pp) {
				++pp->p_keepcnt;
				new_ptap = hat_ptalloc(HAT_CANWAIT);
				--pp->p_keepcnt;
			} else {
				new_ptap = hat_ptalloc(HAT_CANWAIT);
			}
			goto pt_search;
		}

		ASSERT(ptap);
		eptap = ptap;
		ptap = new_ptap;

		/*
		 * now eptap is after the insertion place because
		 * hatpt_pdtep is bigger or because it is the
		 * low entry (which is after the highest entry
		 * in the circular list) and all entries are
		 * lower than needed.
		 */

		/* only user page tables on the active PT list */

		if (cur_as != &kas)
			APPEND_PT(ptap, hat_activept);

		ptap->hatpt_as = cur_as;
		ptap->hatpt_pdtep = vpdte;
		if (cur_as == &kas || cur_as == u.u_procp->p_as) {
			ASSERT(vpdte->pg_pte == 0);
			vpdte->pg_pte = ptap->hatpt_pde.pg_pte;
		}
		hatp->hat_ptlast = ptap;
		ptap->hatpt_forw = eptap;
		ptap->hatpt_back = eptap->hatpt_back;
		eptap->hatpt_back->hatpt_forw = ptap;
		eptap->hatpt_back = ptap;
		if (hatp->hat_pts->hatpt_pdtep > vpdte)
			hatp->hat_pts = ptap;
	}
	else {
found_ptap:
		if (new_ptap != (hatpt_t *)NULL) {
			/* Somebody else allocated a page
			   table for this address while we
			   were asleep trying to allocate
			   one ourselves, so free up the
			   one we allocated. */
			new_ptap->hatpt_as = NULL;
			hat_ptfree(new_ptap);
		}
		hatp->hat_ptlast = ptap;
	}

	ASSERT(ptap);
	ASSERT(hatp->hat_ptlast == ptap);

	pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);

	/* Now setup the mapping chunk pointers */
	mcnum = HATMCNO(addr);
	mcndx = HATMCNDX(addr);
	mcp = ptap->hatpt_mcp + mcnum;
	ptep = pt->hat_pgtc[mcnum].hat_pte + mcndx;

	if ((mapp = (hatmap_t *)mcp->hat_mcp) == 0) {
		/* Need a new chunk */
		/*
		 * If the thing we are mapping is associated with a
		 * pp, we must bump the keepcnt to prevent it from
		 * being stolen because it is possible to sleep in
		 * hat_mcalloc().  We also lock the page table.
		 */
		if (pp)
			++pp->p_keepcnt;
		++ptap->hatpt_locks;
		mapp = (hatmap_t *)hat_mcalloc(mcp, pt->hat_pgtc + mcnum,
						HAT_CANWAIT);
		--ptap->hatpt_locks;
		if (pp)
			--pp->p_keepcnt;
		ASSERT(ptep->pg_pte == 0);
	}
	ASSERT(((uint)mapp & ~HATMAP_ADDR) == 0);
	mapp += mcndx;
	ptcp = mapptoptcp(mapp);

	if (ptep->pg_pte != 0) {
		ASSERT(ptcp->ptp.hat_mcaec);
		ASSERT(ptap->hatpt_aec);

		if ((uint)ptep->pgm.pg_pfn == pf) {
#if DEBUG && HAT_TAG_PTE == 1
			ptep->pg_pte |= PG_LD1;
#endif

			ptep->pg_pte = mode |
				(ptep->pg_pte & ~(PTE_PROTMASK|PG_V));
			ASSERT(ptep->pg_pte != 0);

			/* and not:
		 	 * pte->pg_pte = (u_int)mkpte(mode, page_pptonum(pp));
		 	 * as on the 3B2, preserve other bits not specified.
		 	 * The lock bit is handled separately below.
		 	 * Some other bits might be used for various ageing
		 	 * mechanisms or for other purposes.
		 	 */

			if (lock & HAT_LOCK) {
				++ptap->hatpt_locks;
#if DEBUG && HAT_LCK_CHK == 1
				PG_SETLOCK(ptep);
#endif
			}
			flushtlb();
			return; /* success */
		} else {
			page_t	*opp = page_numtopp(ptep->pgm.pg_pfn);

			if (opp == NULL || !opp->p_gone) {
				cmn_err(CE_PANIC,
		"hat_pteload - trying to change existing mapping: pte = %x\n",
					ptep->pg_pte);
			}
			hat_remptel(mapp, ptep);
		}
	}

#if DEBUG && HAT_TAG_PTE == 1
	mode |= PG_LD2;
#endif

	ptep->pg_pte = (u_int)mkpte(mode, pf);
	ASSERT(ptep->pg_pte != 0);

	if (lock & HAT_LOCK) {
		++ptap->hatpt_locks;
#if DEBUG && HAT_LCK_CHCK == 1
		PG_SETLOCK(ptep);
#endif
	}

	/* add to p_mapping list */

	if (pp) {
		mapp->hat_mnext = (hatmap_t *)pp->p_mapping;
		pp->p_mapping = (caddr_t)mapp;
	}
	
	/* increment active pte count for chunk */
	ptcp->ptp.hat_mcaec++;

	/* increment active pte count for hatpt */
	ptap->hatpt_aec++;

 	seg->s_as->a_rss++; 
}

/*
 * The following code has the knowledge of segment list and
 * hatpt list of an address space being sorted in ascending
 * virtual address order. 
 */

#ifdef DEBUG
STATIC int hat_dup_lost = 0;
STATIC int hat_dup_orphan = 0;
#endif

int
hat_dup(as, new_as)
struct as *as;
struct as *new_as;
{
	register addr_t addr;
	register addr_t endaddr;
	register pte_t *pte;
	pte_t	*new_pte;
	hatpgt_t *pt, *newpt;
	u_int	copyflag;
	page_t	*pp;
	page_t	*opp;
	struct seg	*sseg;
	struct seg	*seg;		/* parent seg ptr */
	struct seg	*new_seg;	/* child seg ptr */
	struct segvn_data *svd;		/* parent segment private data */
	struct segvn_data *new_svd;	/* child segment private data */
	struct anon_map	*amp;		/* parent anon map ptr */
	struct anon_map	*new_amp;	/* child anon map ptr */
	struct anon	**anon;		/* parent anon entry ptr */
	struct anon	**new_anon;	/* child anon entry ptr */
	u_int	index;
	struct vnode	*svp;
	u_int	soff;
	hat_t *hatp = &as->a_hat;
	hat_t *nhatp = &new_as->a_hat;
	hatpt_t *ptap, *eptap;
	hatpt_t *lptap;
	hatpt_t *nptap;
	pte_t	*vpdte;
	pte_t	*endvpdte;
	hatmcp_t *mcp;
	hatmcp_t *new_mcp;
	hatmap_t *mapp;
	hatmap_t *new_mapp;
#ifdef DEBUG
	hatptcp_t *ptcp;
#endif
	hatptcp_t *new_ptcp;
	int	mcndx;
	int	mcnum;
#if DEBUG && HAT_TAG_PTE == 1
	u_int	dbgflg = 0;
#endif

#ifdef DEBUG
	if (hat_duppts == 0)
		return(0);
#endif

	/* 
	 * Optimize behavior for seg_vn type segments.
	 *
	 * Initialize the page tables of the new address space <new_as>
	 * by copying the valid seg_vn mappings from the original address
	 * space <as>.
	 *
	 * As a heuristic, if a MAP_PRIVATE page has already been
	 * copy-on-write'd we will give a private copy of the page
	 * to the child. In general, we believe the child will write
	 * the same pages that the parent has already written. Our
	 * performance studies show that this RADICALLY reduces the
	 * number of c-o-w faults caused by the shell.
	 *
	 * NOTES:
	 *
	 *	We do not inform any file system that might manage a
	 *	particular page that a new reference has been made to
	 *	it.
	 *
	 *	We're only guessing that the process will need the
	 *	copied mappings. We don't bother sleeping for any
	 *	mapping resources.
	 *
	 *	In the future, we might try some other heuristics
	 *	to cut down on unnecessary copying (for example,
	 *	instead of copying all stack pages, only copy the
	 *	current one which we find based on the current
	 *	(parent) stack pointer).
	 */

	sseg = seg = as->a_segs;
	new_seg = new_as->a_segs;

	ASSERT(seg != NULL && new_seg != NULL);
	ASSERT(nhatp->hat_pts == NULL);
	ASSERT(nhatp->hat_ptlast == NULL);
	ASSERT(new_as != &kas);

	/*
	 * Segment list of an address space is sorted by ascending
	 * virtual address; so is hatpt list. Since we are duplicating
	 * the entire as, we start from the first hatpt.
	 */
	eptap = ptap = hatp->hat_pts;

	/*
	 *	By the time fork() was called and we came to hat_dup() we
	 *	could have slept -- the page tables could have been stolen
	 *	by now.
	 */
	if (ptap == NULL) {
		if (hatp->hat_ptlast != NULL)
			cmn_err(CE_PANIC,"hat_dup: hat_pts 0, hat_ptlast %x\n",
				hatp->hat_ptlast);
		return(0);
	}

	/* Lock down the ending page table, so we don't lose it
	 * during a preemption.
	 */

	eptap->hatpt_locks++;

	/*
	 * The following loops are indented with 4 space characters,
	 * instead of a tab, in an attempt to keep the code readable
	 * on an 80 column terminal.
	 */

    do {
	/*
	 * Make sure the seg lists are 1-1.  If not, just bail out.
	 */
	if (seg->s_base != new_seg->s_base ||
	    seg->s_size != new_seg->s_size)
		break;
	ASSERT(seg->s_ops == new_seg->s_ops);

	/* 
	 * Only copy the mappings if this is a vnode-type segment.
	 * For now, in order to tell whether a segment is a
	 * vnode-type segment, we check what functions are used
	 * to manipulate it.  Ultimately, we would like a type field 
	 * to let us know what kind of segment it is.  Or we can
	 * take the object-oriented approach and add a function
	 * into the seg_ops that will do this duplication for us.
	 * There may eventually be other types of segments that
	 * we can 'duplicate' on fork.
	 */
	if (seg->s_ops != &segvn_ops)
	    continue;

	svd = (struct segvn_data *)(seg->s_data);

	/*
	 * If the original segment has per page protections
	 * (a vpage array), play safe and do not copy the mappings.
	 */
	if (svd->pageprot)
	    continue;

	/*
	 * If the original segment is MAP_PRIVATE, has write
	 * permission and has the anon map set up, then we will
	 * anticipate the c-o-w faults and make a copy of the
	 * pages.
	 */
	if ((svd->type == MAP_PRIVATE) &&
	    (svd->prot & PROT_WRITE) &&
	    ((amp = svd->amp) != NULL)) {
	    copyflag = 1;

	    /*
	     * The child's anon structures should have been set
	     * up in segvn_dup().
	     */
	    new_svd = (struct segvn_data *)(new_seg->s_data);
	    ASSERT(new_svd != NULL);
	    new_amp = new_svd->amp;
	    ASSERT(new_amp != NULL);
	    ASSERT(amp->anon != NULL);
	    ASSERT(new_amp->anon != NULL);
	} else {
	    copyflag = 0;
	}

	/*
	 * Calculate parameters and find data structures of the
	 * mappings in the original segment.
	 */

	addr = seg->s_base;
	endaddr = addr + seg->s_size - 1;

	/* page aligned */
	ASSERT(((u_long)addr & POFFMASK) == 0);

	vpdte = kpd0 + ptnum(addr);
	endvpdte = kpd0 + ptnum(endaddr);

	do {
	    if (ptap->hatpt_pdtep < vpdte) 
		continue;
	    else if (ptap->hatpt_pdtep > endvpdte) 
		goto endseg;
	    while (vpdte < ptap->hatpt_pdtep) {
		addr = (addr_t) ((++vpdte - kpd0) * VPTSIZE);
	    }
	    ASSERT(vpdte == ptap->hatpt_pdtep);
	    pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);

	    /* Lock down the current page table, so we don't lose it
	     * during a preemption.
	     */

	    ptap->hatpt_locks++;

	    /* 
	     * if need to have a new page table for new_as 
	     * allocate one.
	     *
	     * The following code depends on hatpt list
	     * and the segment list of as being sorted in
	     * ascending virtual address order.
	     */
	    if (nhatp->hat_pts == NULL ||
		    (lptap = nhatp->hat_pts->hatpt_back)->hatpt_pdtep != vpdte) {
		if ((nptap = hat_ptalloc(HAT_NOSLEEP | HAT_NOSTEAL)) == NULL) {
		    ptap->hatpt_locks--;
		    goto done;
		}

		APPEND_PT(nptap, hat_activept);
		nptap->hatpt_as = new_as;
		nptap->hatpt_pdtep = vpdte;
		if (nhatp->hat_pts == NULL) {
		    nptap->hatpt_forw = nptap->hatpt_back = nptap;
		    nhatp->hat_pts = nhatp->hat_ptlast = nptap;
		}
		else {
		    /* add it to the end of hatpt list */
		    nptap->hatpt_forw = nhatp->hat_pts;
		    nptap->hatpt_back = lptap;
		    nhatp->hat_pts->hatpt_back = nptap;
		    lptap->hatpt_forw = nptap;
		}
		lptap = nptap;
	    }

	    /* Lock down the destination page table, so we don't lose it
	     * during a preemption.
	     */

	    lptap->hatpt_locks++;

	    newpt = (hatpgt_t *) ptetokv(lptap->hatpt_pde.pg_pte);

	    mcp = ptap->hatpt_mcp;
	    mcnum = HATMCNO(addr);
	    mcp += mcnum;
	    mcndx = HATMCNDX(addr);

	    /*
	     * Walk through page table, copying valid entries.
	     */
	    for (; mcnum < HAT_MCPP; mcp++, mcnum++) {
		if (mcp->hat_mcp == 0) {
		    mcndx = 0;
		    addr = (addr_t)(((uint)addr + HATMCSIZE) & HATVMC_ADDR);
		    if (addr >= endaddr) {
			/* unlock page table */
			ptap->hatpt_locks--;
			lptap->hatpt_locks--;
			goto endseg;
		    }
		    continue;
		}
		/* 
		 * there is a mapping chunk allocated,
		 * there exists at least a valid pte.
		 */
		new_mcp = lptap->hatpt_mcp + mcnum;
		if (new_mcp->hat_mcp == 0) {
		    /* Need a new chunk */
		    if (hat_mcalloc(new_mcp, newpt->hat_pgtc + mcnum,
			    HAT_NOSTEAL | HAT_NOSLEEP) == NULL) {
			/* unlock page table */
			ptap->hatpt_locks--;
			lptap->hatpt_locks--;
			/*
			 * free newly allocated PT if
			 * nothing has been loaded. 
			 */
			if (lptap->hatpt_aec == 0)
			    hat_ptfree(lptap);
			goto done;
		    }
		}

		mapp = mcptomapp(mcp);
		ASSERT(((uint)mapp & ~HATMAP_ADDR) == 0);
#ifdef DEBUG
		ptcp = mapptoptcp(mapp);
		ASSERT(ptcp->ptp.hat_mcaec);
		ASSERT((uint)ptap->hatpt_aec >= ptcp->ptp.hat_mcaec);
#endif
		pte = pt->hat_pgtc[mcnum].hat_pte + mcndx;
		mapp += mcndx;
		new_mapp = mcptomapp(new_mcp);
		ASSERT(((uint)new_mapp & ~HATMAP_ADDR) == 0);
		new_ptcp = mapptoptcp(new_mapp);
		new_pte = newpt->hat_pgtc[mcnum].hat_pte + mcndx;
		new_mapp += mcndx;
		if (copyflag) {
		    index = (addr - seg->s_base) >> PNUMSHFT;
		    anon = amp->anon + svd->anon_index + index;
		    new_anon = new_amp->anon + new_svd->anon_index + index;
		}
		while (mcndx < HAT_EPMC) {
		    if (addr >= endaddr) {
			/* unlock page table */
			ptap->hatpt_locks--;
			lptap->hatpt_locks--;
			goto endseg;
		    }
		    if (pte->pg_pte == 0) {
			ASSERT(mapp->hat_mapv == 0);
			goto nextpage;
		    }
		    ASSERT(new_pte->pg_pte == 0);

		    if (copyflag && *anon) {
			/*
			 * Allocate a new locked anon structure, the anon page,
			 * copy the page contents, and tie everything together.
			 */
			ASSERT(*anon == *new_anon);
			*new_anon = anon_alloc();
			if (*new_anon == NULL) {
			    /*
			     * Don't bother breaking the c-o-w's anymore,
			     * because we're low on resources.
			     *
			     * Also, we don't bump the current loop
			     * variables (addr, pte, anon, ...), because
			     * we'll do the current pte over again
			     * without copyflag set.
			     */
			    copyflag = 0;
			    *new_anon = *anon;
			    continue;
			}

#if DEBUG && HAT_TAG_PTE == 1
			dbgflg = PG_DUP1;
#endif

			swap_xlate(*new_anon, &svp, &soff);
try_again:
			pp = page_lookup(svp, soff);
			if (pp == NULL) {
			    pp = page_get(PAGESIZE, P_NOSLEEP);
			    if (pp == NULL) {
				/*
				 * See above...
				 */
				copyflag = 0;
				AUNLOCK(*new_anon);
				anon_free(new_anon, NBPP);
				*new_anon = *anon;
				continue;
			    }
			    if (page_enter(pp, svp, soff)) {
				PAGE_RELE(pp);
				goto try_again;
			    }
			} else {
			    /*
			     * Already found a page with the right identity -
			     * just use it.
			     */
			    PAGE_HOLD(pp);
			    page_lock(pp);
			}

			pp->p_intrans = pp->p_pagein = 1;

			if (pte->pg_pte == 0) {
				/*
				 * Must have slept in page_lookup() or page_lock()
				 * and the parent's page was stolen.
				 * Nothing to do with this page since
				 * no translation is available.  
				 * Skip to the next page.
				 */

#ifdef DEBUG
printf("hat_dup:page_lookup slept and parent's page is gone %d\n",
				++hat_dup_lost);
#endif
				
				ASSERT(mapp->hat_mapv == 0);
				AUNLOCK(*new_anon);
				anon_free(new_anon, NBPP);
				*new_anon = *anon;
				pp->p_intrans = pp->p_pagein = 0;
				page_unlock(pp);
				PAGE_RELE(pp);
				if (pp->p_keepcnt == 0 &&
				    pp->p_mapping == NULL) {
#ifdef DEBUG
					hat_dup_orphan++;
#endif
					page_abort(pp);
				}
				goto nextpage;
			}

#if DEBUG && HAT_TAG_PTE == 1
			dbgflg |= PG_DUP2;
#endif

			(*new_anon)->un.an_page = pp;
			opp = page_numtopp(pte->pgm.pg_pfn);
			ppcopy(opp, pp);
			pp->p_mod = 1;
			pp->p_intrans = 0;
			pp->p_pagein = 0;
			/*
			 * The child's page should be valid and writeable.
			 */
			new_pte->pg_pte = (u_int)
			    mkpte(PG_V | PG_RW | (pte->pg_pte & PG_US),
				page_pptonum(pp));

#if DEBUG && HAT_TAG_PTE == 1
			new_pte->pg_pte |= dbgflg;
#endif

			/*
			 * Add to pp's p_mapping list.
			 */
			new_mapp->hat_mnext = (hatmap_t *)pp->p_mapping;
			pp->p_mapping = (caddr_t)new_mapp;

			/* increment active pte count for chunk */
			new_ptcp->ptp.hat_mcaec++;

			/* increment active pte count for hatpt */
			lptap->hatpt_aec++;

			++new_as->a_rss;

			page_unlock(pp);
			PAGE_RELE(pp);
			AUNLOCK(*new_anon);

			/* 
			 * If the parent refcount is 1, cancel the c-o-w.
			 *
			 * Note:
			 *
			 * We don't have to flush the TLB because segvn_dup()
			 * called hat_chgprot(), to set the c-o-w, and that
			 * flushes the TLB.
			 */
			if ((--(*anon)->an_refcnt) == 1)
			    PG_SETPROT(pte, PG_RW);
		    } else {
			/*
			 * The fault-on-write bit has already been set in
			 * the parent's pte, if it's needed, by segvn_dup().
			 */
			new_pte->pg_pte = pte->pg_pte;

#if DEBUG && HAT_TAG_PTE == 1
			new_pte->pgm.pg_tag = 0;
			dbgflg |= PG_DUP3;
			new_pte->pg_pte |= dbgflg;
			dbgflg = PG_DUP3;
#endif

			/*
			 * Add to p_mapping list.
			 */
			new_mapp->hat_mnext = mapp->hat_mnext;
			mapp->hat_mnext = new_mapp;

			/* increment active pte count for chunk */
			new_ptcp->ptp.hat_mcaec++;

			/* increment active pte count for hatpt */
			lptap->hatpt_aec++;

			++new_as->a_rss;
		    }
#if DEBUG && HAT_LCK_CHCK == 1
		    /*
		     * Child should not inherit SOFTLOCKs.
		     */
		    new_pte->pgm.pg_tag = 0;
#endif

nextpage:
		    ++pte;  ++new_pte;
		    ++mapp;  ++new_mapp;
		    ++anon;  ++new_anon;
		    ++mcndx;
		    addr += PAGESIZE;

		    /* These loops can take long;
			give other processes a chance */
		    PREEMPT();
		}
		mcndx = 0;
	    }
	    ASSERT(lptap->hatpt_aec);

	    /* Unlock current page table */
	    ptap->hatpt_locks--;
	    lptap->hatpt_locks--;

	} while (++vpdte, (ptap = ptap->hatpt_forw) != eptap);
endseg:	;

    } while (new_seg = new_seg->s_next,
		(seg = seg->s_next) != sseg && ptap != eptap);

done:
	/* Unlock ending page table */
	eptap->hatpt_locks--;

	return (0);
}

/*
 * hat_map(seg, vp, vp_base, prot, flags)
 *
 * Allocate any hat resources needed for a new segment.
 *
 * This routine is invoked by the seg_create() routines 
 * in the segment drivers.
 *
 * For vnode type of segments, we load up translations 
 * to any pages of the segment that are already in core.
 */
u_int
hat_map(seg, vp, vp_base, prot, flags)
	struct seg *seg;
	struct vnode *vp;
	off_t	vp_base;
	u_int	prot;
	u_int	flags;
{
	register addr_t	addr;
	register page_t	*pp;
	register hatpt_t *ptap;
	hatpt_t  *eptap;
	pte_t	*ptep;
	pte_t	*vpdte;
	hat_t	*hatp;
	struct	as *as;
	int	seg_size;
	long	vp_off;		/* offset of page in file */
	long	vp_end;		/* end of VM segment in file */
	long	addr_off;	/* loop invariant; see comments below. */
	hatmap_t *mapp;
	hatptcp_t *ptcp;
	int mcndx;
	int mcnum;
	hatpgt_t *pt;
	hatmcp_t *mcp;
	u_int	mode;
	int	wasfree;
	int	oldpri;
	page_t	*spp, *nextpp;

#ifdef DEBUG
	if (hat_mappts == 0)
		return(0);
#endif

	if (vp == NULL || (flags & HAT_PRELOAD) == 0)
		return(0);

	/*
	 * Set up as many of the hardware address translations as we can.
	 * If the segment is a vnode-type segment and it has a non-NULL vnode, 
	 * we walk down the list of incore pages associated with the vnode.
	 * For each page on that list, if the page maps into the range managed
	 * by the segment we calculate the corresponding virtual address and
	 * set up the hardware translation tables to point to the page.
	 *
	 * Note: the file system that owns the vnode is not informed about the
	 * new references we have made to the page.
	 */

	/*
	 * Calculate the protections we need for the pages.
	 * (i.e. whether to set fault on write bit or not).
	 */
	if (!((mode = hat_vtop_prot(prot)) & PG_V))
		return(0);		/* don't bother */

#if DEBUG && HAT_TAG_PTE == 1
	mode |= PG_MAP;
#endif

	addr = seg->s_base;
	seg_size = seg->s_size;

	ASSERT(((u_long)addr & POFFMASK) == 0);	/* page aligned */

	as = seg->s_as;
	ASSERT(as == u.u_procp->p_as || as == &kas);
	hatp = &(as->a_hat);

	/*
	 * Remove invariant from the loop. To compute the virtual
	 * address of the page, we use the following computation:
	 *
	 *	addr = seg->s_base + (vp_off - vp_base)
	 *
	 * Since seg->s_base and vp_base do not vary for this segment:
	 *
	 *	addr = vp_off + (addr_off = (seg->s_base - vp_base))
	 *
	 * Note: currently addr = seg->s_base;
	 */
	addr_off = (u_int)addr - vp_base;
	vp_end = vp_base + seg_size;
	
	/*
	 * Walk down the list of pages associated with the vnode,
	 * setting up the translation tables if the page maps into
	 * addresses in this segment.
	 *
	 * This is not an easy task because the pages on the list
	 * can disappear as we traverse it. See comments in
	 * pvn_vplist_dirty() in vm/vm_pvn.c .
	 *
	 * Use a dummy page as a marker of our start point
	 * to terminate our search of the page list.
	 * Code that scans this list must skip strange pages.
	 */

	spp = kmem_zalloc(sizeof (page_t), KM_SLEEP);
	spp->p_vnode = (struct vnode *)&hat_map;
#ifdef DEBUG
	spp->p_offset = (u_int)u.u_procp;
#endif

	oldpri = splhi();
	if ((pp = vp->v_pages) == NULL) {
		splx(oldpri);
		kmem_free(spp, sizeof (page_t));
		return 0;
	}

	/*
	 * Insert a start marker at the front of the v_pages list,
	 * so we'll know when we've traversed the entire list.
	 */
	spp->p_vpnext = pp;
	spp->p_vpprev = pp->p_vpprev;
	pp->p_vpprev = spp;
	spp->p_vpprev->p_vpnext = spp;
	vp->v_pages = spp;

	while (pp != spp) {

		/*
		 * See if the vp offset is within the range mapped
		 * by the segment.
		 *
		 * If page is intrans, ignore it.  (There may be 
		 * races if we try to use it.)
		 *
		 * NOTE: We do these checks here, before possibly taking
		 * the page off the freelist, to avoid unecessarily
		 * reclaiming the page. As a consequence, we must check
		 * the vnodes. If the page got reused somehow, we could
		 * find ourselves traversing the wrong list.
		 */
		vp_off = pp->p_offset;
		if (pp->p_vnode != vp || pp->p_gone ||
		    (vp_off < (u_int)vp_base) || (vp_off >= vp_end) ||
		    (pp->p_intrans)) {
			pp = pp->p_vpnext;
			continue;
		}

		/* NOTE: if mon_enter() were real, a marker
		 * would be needed in front of pp.
		 */
		if (pp->p_free) {
			wasfree = 1;
			page_reclaim(pp);
		} else {
			wasfree = 0;
		}

		ASSERT(pp->p_vnode == vp && pp->p_offset == vp_off);
		ASSERT(!pp->p_gone);

		PAGE_HOLD(pp);
		splx(oldpri);

		addr = (addr_t) (addr_off + vp_off);
		ASSERT((vp_off & POFFMASK) == 0);
		ASSERT(((u_int)addr & POFFMASK) == 0);

		/*
		 * Compute the address of page directory entry for the 
		 * page. Search the hatpt list to find a page table
		 * that has a matching pde pointer. If it is 
		 * not found, it means the page table not present.
		 * Allocate and set up the page table.  Then get 
		 * the page table entry for the page.
		 */
		vpdte = kpd0 + ptnum(addr);

		if ((ptap = hatp->hat_pts) == (hatpt_t *)NULL) {
			ASSERT(hatp->hat_ptlast == (hatpt_t *)NULL);
			ptap = hat_ptalloc(HAT_NOSLEEP | HAT_NOSTEAL);
			if (ptap == NULL) {
				/*
				 * We're not going to use the page, put it back
				 * if we took it off the freelist. Bail out !
				 */
				PAGE_RELE(pp);
				if (wasfree && pp->p_keepcnt == 0 &&
				    pp->p_mapping == NULL)
					page_free(pp, 0);
				break;
			}

			/* only user page tables on the active PT list */
			if (as != &kas)
				APPEND_PT(ptap, hat_activept);

			ptap->hatpt_as = as;
			ptap->hatpt_forw = ptap->hatpt_back = ptap;
			hatp->hat_pts = hatp->hat_ptlast = ptap;
			ptap->hatpt_pdtep = vpdte;
			vpdte->pg_pte = ptap->hatpt_pde.pg_pte;
		} else if (ptap->hatpt_pdtep != vpdte) {
			eptap = ptap;
			do {
				ASSERT(ptap);
				if (ptap->hatpt_pdtep == vpdte) {
					hatp->hat_ptlast = ptap;
					goto foundpt;
				}
				if (ptap->hatpt_pdtep > vpdte) break;
				ptap = ptap->hatpt_forw;
			} while (ptap != eptap);

			ASSERT(ptap);
			eptap = ptap;

			/*
		 	 * now eptap is after the insertion place because
		 	 * hatpt_pdtep is bigger or because it is the
		 	 * low entry (which is after the highest entry
		 	 * in the circular list) and all entries are
		 	 * lower than needed.
		 	 */
			ptap = hat_ptalloc(HAT_NOSLEEP | HAT_NOSTEAL);
			if (ptap == NULL) {
				/*
				 * We're not going to use the page, put it back
				 * if we took it off the freelist. Bail out !
				 */
				PAGE_RELE(pp);
				if (wasfree && pp->p_keepcnt == 0 &&
				    pp->p_mapping == NULL)
					page_free(pp, 0);
				break;
			}

			/* only user page tables on the active PT list */
			if (as != &kas)
				APPEND_PT(ptap, hat_activept);

			ptap->hatpt_as = as;
			ptap->hatpt_pdtep = vpdte;
			vpdte->pg_pte = ptap->hatpt_pde.pg_pte;
			hatp->hat_ptlast = ptap;
			ptap->hatpt_forw = eptap;
			ptap->hatpt_back = eptap->hatpt_back;
			eptap->hatpt_back->hatpt_forw = ptap;
			eptap->hatpt_back = ptap;
			if (hatp->hat_pts->hatpt_pdtep > vpdte) 
				hatp->hat_pts = ptap;
		}
		else
			hatp->hat_ptlast = ptap;
foundpt:
		ASSERT(ptap);
		ASSERT(hatp->hat_ptlast == ptap);

		pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);

		/* Now setup the mapping chunk pointers */
		mcnum = HATMCNO(addr);
		mcndx = HATMCNDX(addr);
		mcp = ptap->hatpt_mcp + mcnum;
		if (mcp->hat_mcp == 0) {
			/* Need a new chunk */
			if (hat_mcalloc(mcp, pt->hat_pgtc + mcnum,
					HAT_NOSTEAL | HAT_NOSLEEP) == NULL) {
				/*
			  	 * free newly allocated PT if nothing
			  	 * has been loaded. 
			  	 */
				if (ptap->hatpt_aec == 0)
					hat_ptfree(ptap);
				/*
				 * We're not going to use the page, put it back
				 * if we took it off the freelist. Bail out !
				 */
				PAGE_RELE(pp);
				if (wasfree && pp->p_keepcnt == 0 &&
				    pp->p_mapping == NULL)
					page_free(pp, 0);
				break;
			}
		}
		mapp = mcptomapp(mcp);
		ASSERT(((uint)mapp & ~HATMAP_ADDR) == 0);
		mapp += mcndx;
		ptcp = mapptoptcp(mapp);

		ptep = pt->hat_pgtc[mcnum].hat_pte + mcndx;

		ASSERT(ptep->pg_pte == 0); 

		/* Could check p_gone here, but that could be messy.
		 * (Would we need to free the page table if pt_inuse == 0?
		 *  If so, would we need a marker?)
		 * Instead, map it and let PAGE_RELE() abort it.
		 */

		ptep->pg_pte = (u_int)mkpte(mode, page_pptonum(pp));

		/*
		 * Finally, add this reference to the p_mapping list.
		 * If page is on the freelist and we mapped it into
		 * the process address space, take it off the freelist.
		 * Then get the next page on the vnode page list.
		 */

		mapp->hat_mnext = (hatmap_t *)pp->p_mapping;
		pp->p_mapping = (caddr_t)mapp;
	
		/* increment active pte count for chunk */
		ptcp->ptp.hat_mcaec++;

		/* increment active pte count for hatpt */
		ptap->hatpt_aec++;

 		as->a_rss++; 

		oldpri = splhi();

		nextpp = pp->p_vpnext;
		PAGE_RELE(pp);
		pp = nextpp;
	}

	/*
	 * Remove the marker page.
	 */
	if (spp->p_vpnext == spp)
		vp->v_pages = NULL;
	else {
		spp->p_vpnext->p_vpprev = spp->p_vpprev;
		spp->p_vpprev->p_vpnext = spp->p_vpnext;
		if (vp->v_pages == spp)
			vp->v_pages = spp->p_vpnext;
	}
	splx(oldpri);
	kmem_free(spp, sizeof(page_t));

	return (0);
}

/*
 * Unlock translation at addr. 
 * This just means checking that the xlation
 * was locked and decrementing the page table
 * lock counts in the hatpt structure.
 */
/*ARGSUSED*/
void
hat_unlock(seg, addr)
struct seg *seg;
addr_t addr;
{
	struct hat *hatp = &seg->s_as->a_hat;
	register hatpt_t *ptap;
	register hatpt_t  *eptap;
	register pte_t *vpdte;

	/* addr is page aligned */
	ASSERT(((int)addr & POFFMASK) == 0);

	/* only segkmap and segu are allowed to do hat_unlock if kas */
	/* panic if kernel address, but not segkmap nor segu */
	ASSERT(!(seg->s_as == &kas && seg != segkmap && seg != segu));

	vpdte = kpd0 + ptnum(addr);
	if ((seg->s_as == &kas || u.u_procp->p_as == seg->s_as) &&
		!PG_ISVALID(vpdte))
		cmn_err(CE_PANIC, "hat_unlock: invalid pde %x as %x addr %x\n",
				vpdte, seg->s_as, addr);

	/* find the hatpt struct */

	ptap = hatp->hat_ptlast;
	if (ptap == (hatpt_t *)NULL) {
		cmn_err(CE_PANIC, "hat_unlock: empty hatpt list\n");
	}
	for (eptap = ptap; ptap->hatpt_pdtep != vpdte;) {
		ptap = ptap->hatpt_forw;
		if (ptap == eptap)
			cmn_err(CE_PANIC, "hat_unlock: can't find hatpt\n");
	}

	if (! ptap->hatpt_pde.pgm.pg_v) {
		if (seg->s_as == &kas || u.u_procp->p_as == seg->s_as)
			cmn_err(CE_PANIC,"hat_unlock: vpdte %x hat_pde %x addr %x as %x\n",
				vpdte, ptap->hatpt_pde.pg_pte, addr, seg->s_as);
		else	cmn_err(CE_PANIC,"hat_unlock: as %x addr %x hat_pde %x\n",
					seg->s_as, addr, ptap->hatpt_pde.pg_pte);
	}

	hatp->hat_ptlast = ptap;

#if DEBUG && HAT_LCK_CHCK == 1
	{
		int mcnum, mcndx;
		pte_t *ptep;
		hatpgt_t *pt;

		pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);
		mcnum = HATMCNO(addr);
		mcndx = HATMCNDX(addr);
		ptep = pt->hat_pgtc[mcnum].hat_pte + mcndx;
		PG_CLRLOCK(ptep);

		/* 
		 * No need to do a flushtlb since 
		 * lock bit is a software bit 
		 */
	}
#endif

	ASSERT(ptap->hatpt_locks > 0);
	if ((--ptap->hatpt_locks) == 0 && ext_freemem_wait) {
		wakeprocs((caddr_t)&ext_freemem_wait, PRMPT);
		ext_freemem_wait = 0;
	}
}

/*
 * Associate all the mappings in the range [addr..addr+len] with
 * segment seg. Since we don't cache segments in this hat implementation,
 * this routine is a noop.
 */
/*ARGSUSED*/
void
hat_newseg(seg, addr, len, nseg)
struct seg *seg;
addr_t addr;
u_int len;
struct seg *nseg;
{
}

/*
 * Return the physical page ID corresponding to the virtual 
 * address vaddr.  This is a required interface defined by DKI.
 * For the 386, we use the page frame number as the physical page ID.
 */
u_int
hat_getkpfnum(vaddr)
caddr_t vaddr;
{
	pte_t	*pt = svtopte(vaddr);

	if (!PG_ISVALID(pt))
		return NOPAGE;
	return pt->pgm.pg_pfn;
}

/*
 * Return the physical page ID corresponding to the physical
 * address paddr.  This is a required interface defined by DKI.
 * For the 386, we use the page frame number as the physical page ID.
 */
u_int
hat_getppfnum(paddr, pspace)
caddr_t paddr;
u_int pspace;
{
	if (pspace != PSPACE_MAINSTORE)
		return NOPAGE;
	return btoct((u_int)paddr);
}

/*
 * hat_ptalloc(vpdte, flag)
 *
 * flag can be the followings:
 * HAT_NOSLEEP	-> return immediately if no memory
 * HAT_CANWAIT	-> wait if no memory currently available
 * HAT_NOSTEAL	-> don't steal a page table from another process
 *
 * Allocate a page table and a hatpt_t for it.
 * set up pte for page table in the hatpt_t, zero the page table,
 * add the hatpt to the activept list and return a pointer to the hatpt_t.
 *
 * The following code assumes the forward link pointer is
 * the first field of the hatpt structure.
 */

STATIC hatpt_t *
hat_ptalloc(flag)
u_int	 flag;
{
	register hatpt_t *ptap;
	register page_t *pp;
	u_int pfn;
	caddr_t ptaddr;

again:

	if (availrmem - 1  < tune.t_minarmem ||
	    availsmem - 1  < tune.t_minasmem) {
#ifdef DEBUG
		++hat_pgfail;
#endif
	}
	else {
		--availrmem;
		--availsmem;
		++pages_pp_kernel;

		/* allocate a physical page for the page table */
		if ((pp = page_get(PAGESIZE, P_NOSLEEP)) == NULL) {
			++availrmem;
			++availsmem;
			--pages_pp_kernel;
#ifdef DEBUG
			++hat_pgfail;
#endif
		}
		else {
			ptap = (hatpt_t *)kmem_fast_zalloc((caddr_t *)&hatpt_freelist,
				sizeof(hatpt_t), hatpt_freeincr,
				(flag & HAT_CANWAIT) ? KM_SLEEP : KM_NOSLEEP);
			if (ptap == (hatpt_t *)NULL) {
				PAGE_RELE(pp);
				++availsmem;
				++availrmem;
				--pages_pp_kernel;
				cmn_err(CE_NOTE, "Unable to allocate hatpt structure");
			}
			else {
				/*
				 * allocated page table and hatpt
				 */
				pfn = page_pptonum(pp);
				ptap->hatpt_pde.pg_pte =
		 	        	(u_int)mkpte(PTE_RW|PG_V,pfn);
				ptaddr = (caddr_t)phystokv(ctob(pfn));
				struct_zero(ptaddr, PAGESIZE);
				pp->phat_ptap = ptap;
				return(ptap);
			}

		}
	}

	/* Try to steal a page table from other process */

	if ((flag & HAT_NOSTEAL) == 0) {

#ifdef DEBUG
		++hat_ptstolen;
#endif
		if ((ptap = hat_ptsteal(HAT_PTUSED)) != (hatpt_t *)NULL) {
			ptap->hatpt_pde.pg_pte =
				(ptap->hatpt_pde.pg_pte & PG_ADDR) |
					PTE_RW | PG_V;
			/*
		 	 * return the page table and hatpt.
		 	 * pte's have already been cleared above
		 	 */
			return(ptap);
		}
	}

	if ((flag & HAT_CANWAIT) == 0)
		return(NULL);

#ifdef DEBUG
	if (hat_debug)
		cmn_err(CE_NOTE, "hat_ptalloc - have to wait\n");
#endif
	++ext_freemem_wait;
	sleep((caddr_t)&ext_freemem_wait, PSWP+2);
	goto again;
}

/*
 * Free previously allocated HAT pages and hatpt struct
 *
 * The following code assumes the forward link pointer is
 * the first field of the hatpt structure.
 */
STATIC void
hat_ptfree(ptap)
register hatpt_t *ptap;
{
	register page_t *pp;
	struct	 as	*as;

	/* Free page table page */
	pp = page_numtopp(ptap->hatpt_pde.pgm.pg_pfn);
	if (pp < pages || pp >= epages)
		cmn_err(CE_PANIC, "hat_ptfree: no page struct for page to be freed");
	PAGE_RELE(pp);
	++availsmem;
	++availrmem;
	--pages_pp_kernel;

	/* Unlink from address space */
	if ((as = ptap->hatpt_as) != NULL) {
		if (as != &kas)
			REMOVE_PT(ptap);
		unlink_ptap(as, ptap);
	}

	/* Free hatpt struct */
	kmem_fast_free((caddr_t *)&hatpt_freelist, (caddr_t)ptap);

	/* wake up any process waiting for a free page table or mapping chunk */
	if (ext_freemem_wait) {
		wakeprocs((caddr_t)&ext_freemem_wait, PRMPT);
		ext_freemem_wait = 0;
	}
}

/*
 * hat_ptsteal(flag)
 *
 * flag can be the following:
 * HAT_PTFREE - stolen page table is not to be reused, 
 *		called from hat_mcalloc(). 
 * HAT_PTUSED - stolen page table is to be reused immediately,
 *		called from hat_ptalloc().
 *
 * Steal a page table and its associated hatpt structure
 * from a process other than current process.
 */

STATIC hatpt_t *
hat_ptsteal(flag)
u_int	flag;
{
	register hatpt_t   *ptap;
	register struct as *cur_as = u.u_procp->p_as;
	register struct as *as;
	register pte_t *ptep;
	page_t *pp;
	register hatmap_t *mapp;
	hatptcp_t *ptcp;
	int mcndx;
	int mcnum;
	int ec;
	hatpgt_t *pt;
	hatmcp_t *mcp;

	for (ptap = hat_activept.hatpt_next; ptap != &hat_activept;
	     ptap = ptap->hatpt_next) {
		as = ptap->hatpt_as;
		ASSERT(as != NULL && as != &kas);

		/* XXX - TEMPORARY
		 * if (ptap->hatpt_locks == 0 && as != cur_as) {
		 */
		if (ptap->hatpt_locks == 0) {
			pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);
			mcp = ptap->hatpt_mcp;
			for (mcnum = 0; mcnum < HAT_MCPP && ptap->hatpt_aec > 0; mcp++, mcnum++) {
				if (mcp->hat_mcp == 0) 
					continue;
				mapp = mcptomapp(mcp);
				ASSERT(((uint)mapp & ~HATMAP_ADDR) == 0);
				ptcp = mapptoptcp(mapp);
				ec = ptcp->ptp.hat_mcaec;
				ASSERT(ec);
				ASSERT(ptap->hatpt_aec >= ec);
				ptap->hatpt_aec -= ec;
				ptcp->ptp.hat_mcaec = 0;
				ptep = pt->hat_pgtc[mcnum].hat_pte;
				ASSERT(ptcptoptep(ptcp) == ptep);
				for (mcndx = 0; mcndx < HAT_EPMC && ec > 0; mapp++, ptep++, mcndx++) {
					if (ptep->pg_pte == 0) {
						ASSERT(mapp->hat_mapv == 0);
						continue;
					}

#if DEBUG && HAT_LCK_CHCK == 1
					ASSERT(!PG_ISLOCKED(ptep)); 
#endif

					as->a_rss--;

					/*
				 	 * Must remove pte from its p_mapping list
				 	 */
					hat_remptel(mapp, ptep);
					ptep->pg_pte = 0;

					ec--;
				}
				ASSERT(ec == 0);
				mcp->hat_mcp = 0;
				hat_mcfree(--mapp); /* free this mapping chunk */
			}
			ASSERT(ptap->hatpt_aec == 0);

			/*
		  	 * remove hatpt from active list
		  	 */
			REMOVE_PT(ptap);
			unlink_ptap(as, ptap);
		
			if (as == cur_as) {
				ptap->hatpt_pdtep->pg_pte = 0;
				flushtlb();
			}

			/*
			 * if the stolen PT is not to be reused immediately, 
			 * release the physical page and free hatpt structure
			 */
			if (flag & HAT_PTFREE) {
				pp = page_numtopp(ptap->hatpt_pde.pgm.pg_pfn);
				if (pp < pages || pp >= epages)
					cmn_err(CE_PANIC, "hat_ptsteal: no page struct for page to be freed");
				PAGE_RELE(pp);
				++availsmem;
				++availrmem;
				--pages_pp_kernel;

				/* Free hatpt struct */
				kmem_fast_free((caddr_t *)&hatpt_freelist, (caddr_t)ptap);

				/* NOTE: In HAT_PTFREE case, the caller tests
				   return value for non-NULL, but must NOT
				   use the value as a pointer. */
			}
			return(ptap);
		}
	}
	return(NULL);
}

STATIC u_int
hatlowbit(u)
register u_int u;
{
 	register int ndx = 1;

	ASSERT(u != 0);
	ASSERT((u & 1) == 0);
	while (((u >>= 1) & 1) == 0)
		ndx++;
	ASSERT(ndx < HAT_MCPP);
	return(ndx);
}

/*
 * hat_mcalloc(pgtcp, flag)
 *
 * flag can be the followings:
 * HAT_NOSLEEP	-> return immediately if no memory
 * HAT_CANWAIT	-> wait if no memory currently available
 * HAT_NOSTEAL	-> don't steal a page table from another process
 */

STATIC hatmc_t *
hat_mcalloc(mcp, pgtcp, flag)
hatmcp_t  *mcp;
hatpgtc_t *pgtcp;
u_int	 flag;
{
	register page_t *pp;
	register u_int bits;
	u_int pfn;
	u_int bno;
	register hatmcpg_t *cpgp;
#ifdef DEBUG
	hatmap_t *mapp;
#endif

	ASSERT(mcp->hat_mcp == NULL);
loop:
	if ((pp = mcfreelist.p_next) == (page_t *)&mcfreelist) {
		if (availrmem - 1  >= tune.t_minarmem &&
	    	    availsmem - 1  >= tune.t_minasmem) {
			--availrmem;
			--availsmem;
			++pages_pp_kernel;

			/* allocate a physical page for the page table */
			if ((pp = page_get(PAGESIZE, P_NOSLEEP)) == NULL) {
				++availrmem;
				++availsmem;
				--pages_pp_kernel;
#ifdef DEBUG
				cmn_err(CE_NOTE, "hat_mcalloc: page_get request failed");
#endif
			}
			else {
				ASSERT(mcfreelist.p_next == (page_t *)&mcfreelist); 
				mcfreelist.p_next = pp;
				mcfreelist.p_prev = pp;
				pp->p_next = pp->p_prev = (page_t *)&mcfreelist;
				pfn = page_pptonum(pp);
				pp->phat_mcpgp = cpgp = (hatmcpg_t *) phystokv(ctob(pfn));
				struct_zero((caddr_t)cpgp, PAGESIZE);
				/* treat accounting as allocated chunk */
				cpgp->hat_mcpga.hat_mpgabits = 1;	
#ifdef DEBUG
				ASSERT(!hat_mcflpcnt);
				hat_mcflpcnt++;
#endif				
				goto gotmcp;
			}
		}
	}
	else
		goto gotmcp;

	/* Try to steal a page table from other process */

	if ((flag & HAT_NOSTEAL) == 0) {

#ifdef DEBUG
		if (hat_debug)
			cmn_err(CE_NOTE, "hat_mcalloc: about to steal a page table\n");
#endif
		if (hat_ptsteal(HAT_PTFREE) != NULL) 
			goto loop;
#ifdef DEBUG
		if (hat_debug)
			cmn_err(CE_NOTE, "hat_mcalloc: failed to steal a page table\n");
#endif
	}
	if ((flag & HAT_CANWAIT) == 0) 
		return(NULL);
	
#ifdef DEBUG
	if (hat_debug)
		cmn_err(CE_NOTE, "hat_mcalloc - have to wait\n");
#endif
	++ext_freemem_wait;
	sleep((caddr_t)&ext_freemem_wait, PSWP+2);
	/* If another process allocated the chunk while we slept, use it */
	if (mcp->hat_mcp != NULL)
		return(mcp->hat_mcp);
	goto loop;

gotmcp:
	cpgp = pp->phat_mcpgp;
	bits = cpgp->hat_mcpga.hat_mpgabits;
	ASSERT(bits != HATMCFULL);
	ASSERT(bits & 1);
	bno = hatlowbit(~bits);
	ASSERT(cpgp->hat_mcpga.hatptcp[bno].hat_ptpv == 0);
	cpgp->hat_mcpga.hatptcp[bno].hat_pgtcp = pgtcp;
	cpgp->hat_mcpga.hat_mpgabits |= sbittab[bno];
#ifdef DEBUG
	mapp = cpgp->hat_mc[bno].hat_map;
	for (bits = 0; bits < HAT_MCPP; mapp++, bits++)
		ASSERT(mapp->hat_mapv == 0);
#endif
	if (cpgp->hat_mcpga.hat_mpgabits == HATMCFULL) {
		/*
		 * This page is now full, so take it off the mcfreelist.
		 */
		mcfreelist.p_next = pp->p_next;
		pp->p_next->p_prev = pp->p_prev;
		pp->p_next = pp->p_prev = (page_t *)NULL;
#ifdef DEBUG
 		hat_mcflpcnt--;
#endif

	}
	return(mcp->hat_mcp = cpgp->hat_mc + bno);
}

/*
 * mcp passed can be anywhere in a mapping chunk.
 * need to adjust to the beginning of the chunk
 * when zero out the mapping chunk.
 */
STATIC void
hat_mcfree(mcp)
hatmap_t *mcp;
{
	register hatmcpg_t	*pgp;
	register uint		bits;
	register page_t		*pp;
	int	 mcnum;

	pgp = (hatmcpg_t *) ((uint)mcp & ~POFFMASK);
	bits = pgp->hat_mcpga.hat_mpgabits;

	/* 
	 * clear allocation bit and the pointer to the page table chunk.
	 */
	mcnum = HATMAPMCNO(mcp);
	pgp->hat_mcpga.hat_mpgabits = bits &= ~sbittab[mcnum];
	pgp->hat_mcpga.hatptcp[mcnum].hat_pgtcp = (hatpgtc_t *)NULL; 

	/* 
	 * find this page's page structure
	 */
	pp = page_numtopp(btoct(svirtophys((u_long)pgp)));
	if (pp < pages || pp >= epages)
		cmn_err(CE_PANIC, "hat_mcfree: no page struct for mc to be freed");

	/*
	 * check if this mapping chunk page is 
	 * unused. If yes, check if there is at
	 * least another page on the mcfreelist.
	 * For now, we don't bother to check how
	 * many free slots left on the mcfreelist.
	 * As long as there is another page left
	 * on the mcfreelist, we will release this
	 * page back to page pool.
	 */
	if (bits == 1) {
		if (pp->p_next != (page_t *)NULL 
		    && ((mcfreelist.p_next != pp) 
		    || (mcfreelist.p_prev != pp))) { 
#ifdef DEBUG
			hat_mcflpcnt--;
#endif
			pp->p_prev->p_next = pp->p_next;
			pp->p_next->p_prev = pp->p_prev;
			pp->p_next = pp->p_prev = (page_t *)NULL;
			PAGE_RELE(pp);
			++availsmem;
			++availrmem;
			--pages_pp_kernel;
			return;
		}
		else if (pp->p_next == (page_t *)NULL &&
		         mcfreelist.p_next != (page_t *)&mcfreelist) {
			PAGE_RELE(pp);
			++availsmem;
			++availrmem;
			--pages_pp_kernel;
			return;
		}
	}

	/*
	 * the mapping chunk page is not on the mcfreelist
	 * add this page to the end of the mcfreelist
	 */
	if (pp->p_next == (page_t *)NULL) {
		pp->p_next = (page_t *)&mcfreelist;
		pp->p_prev = mcfreelist.p_prev;
		mcfreelist.p_prev->p_next = pp;
		mcfreelist.p_prev = pp;
#ifdef DEBUG
		hat_mcflpcnt++;
#endif
	}
	if (ext_freemem_wait) {
		wakeprocs((caddr_t)&ext_freemem_wait, PRMPT);
		ext_freemem_wait = 0;
	}
}

/* Routine to translate virtual addresses to physical addresses
 * Used by the floppy, Integral, and Duart driver to access the dma
 * If an invalid address is received vtop returns a 0
 */

#define EBADDR	0

paddr_t
vtop(vaddr, p)
	register caddr_t	vaddr;
	proc_t	*p;
{
	register pte_t *ptp, *pdtep;
	register paddr_t retval;
	struct	hat    *hatp;
	hatpt_t	*ptap, *eptap;
	int mcndx;
	int mcnum;
	hatpgt_t *pt;
	int s;
	paddr_t	svirtophys();

	s = splhi();
	if ((UVUBLK <= (u_long)vaddr) &&
				((u_long)vaddr < (UVUBLK + ctob(p->p_usize)))) {
		vaddr = (caddr_t)PTOU(p) + ((u_long)vaddr - UVUBLK);
		goto kernel_addr;
	}
	else if (KADDR((u_long)vaddr) || (p == u.u_procp)) {
kernel_addr:
		splx(s);
		return(svirtophys((u_long)vaddr));
	}

	/*
	 * Here vaddr is in the context of some process other than
	 * curproc.  Since there is no per proc page directory, we
	 * scan its hatpts and the associated page tables.
	 */

	pdtep = kpd0 + ptnum(vaddr);
	hatp = &(p->p_as->a_hat);
	
	/*
	 * Should we take advantage of hat_ptlast here?
	 */
	ptap = eptap = hatp->hat_ptlast;
	if (ptap == (hatpt_t *)NULL) {
		ASSERT(hatp->hat_pts == (hatpt_t *)NULL);
			goto error;
	} else {
		do {
			ASSERT(ptap && ptap->hatpt_aec);
			ASSERT(ptap->hatpt_pde.pg_pte && ptap->hatpt_pdtep);
			if (pdtep > ptap->hatpt_pdtep) {
				ptap = ptap->hatpt_forw;
				continue;
			}
			else
				break;
		} while (ptap != eptap);
		
		if (pdtep == ptap->hatpt_pdtep) { 
			hatp->hat_ptlast = ptap;
			pt = (hatpgt_t *) ptetokv(ptap->hatpt_pde.pg_pte);
			mcnum = HATMCNO(vaddr);
			mcndx = HATMCNDX(vaddr);
			ptp = pt->hat_pgtc[mcnum].hat_pte;
			ptp += mcndx;
		}
		else {
#ifdef DEBUG
			cmn_err(CE_NOTE, "vtop didn't find pdtep");
#endif
			goto error;
		}
	}
	if (!PG_ISVALID(ptp)) {
		goto error;
	}
	splx(s);
	return ctob(ptp->pgm.pg_pfn) + PAGOFF(vaddr);
error:
	splx(s);
	cmn_err(CE_CONT,"vtop(%x,%x) FAILED, curproc = %x\n", vaddr,p,curproc);
	return((paddr_t)EBADDR);
}


paddr_t
svirtophys(vaddr)
unsigned long vaddr;
{
	register paddr_t retval;
	register pte_t *pt;
	register int	prio;

	prio = splhi();
	pt = svtopte(vaddr);
	if (!PG_ISVALID(pt))
		cmn_err(CE_PANIC, "svirtophys - not present.");
	retval = ctob(pt->pgm.pg_pfn) + PAGOFF(vaddr);
	splx(prio);
	return((paddr_t) retval);
}

/*
 * Load the page directory entries for curproc's s user address
 * space.  The user space page directory entries are zeroed
 * and rewritten.
 */

#ifdef WEITEK
extern int map_weitek_pt();
extern int unmap_weitek_pt();
#endif

void
restorepd()
{
	register struct	hat    *hatp;
	register hatpt_t       *ptap, *eptap;

	if (old_curproc->p_as != (struct as *)NULL) {
		hatp = &(old_curproc->p_as->a_hat);
		ptap = eptap = hatp->hat_pts;
		if (ptap != (hatpt_t *)NULL) {
			do {
				ptap->hatpt_pdtep->pg_pte = (uint)0;
				ptap = ptap->hatpt_forw;
			} while (ptap != eptap);
		}
	}
	if (curproc->p_as != (struct as *)NULL) {
#ifdef WEITEK
		/*	No way to know right now whether the process being switched
 		 *	in is "weitek_proc" or not. We are not even running in
		 *	the context of the new ublock - we are actually but a
		 *	flushtlb() was not done - hence the reference is still
		 *	in the context of the old process's ublock. So we look
		 *	at the u_weitek field through floating u block address
		 *	space.
 		 */
 
 		if (PTOU(curproc)->u_weitek == WEITEK_HW) {
 			ASSERT(weitek_pt > 0);
 			/* if (old_curpoc != weitek_proc) */
 			map_weitek_pt();
 		}
		else	unmap_weitek_pt();
#endif
		hatp = &(curproc->p_as->a_hat);
		ptap = eptap = hatp->hat_pts;
		if (ptap != (hatpt_t *)NULL) {
			do {
				ptap->hatpt_pdtep->pg_pte = ptap->hatpt_pde.pg_pte;
				ptap = ptap->hatpt_forw;
			} while (ptap != eptap);
		}
	}
out:
	flushtlb();
}

/*
 * hat_exec - move page tables and hat structures for the new stack image
 * from the old address space to the new address space.
 */

int
hat_exec(oas, ostka, stksz, nas, nstka, hatflag)
	struct as	*oas;
	addr_t		ostka;
	int		stksz;
	struct as	*nas;
	addr_t		nstka;
	u_int		hatflag;
{
	ASSERT(PAGOFF(stksz) == 0);
	ASSERT(PAGOFF((u_int)ostka) == 0);
	ASSERT(PAGOFF((u_int)nstka) == 0);
	ASSERT(nas->a_segs->s_next == nas->a_segs);
	ASSERT(nas->a_hat.hat_pts == (hatpt_t *)NULL);

	if (hatflag) {
		/* Move the page tables themselves as the flag
		 * states that they contain only pages to be moved
		 * and the pages are properly aligned in the table.
		 */
		register hatpt_t *ptap;
		register pte_t *ovpdte;
		register pte_t *nvpdte;
		register pte_t *endvpdte;
		register struct hat *hatp;

		ovpdte = kpd0 + ptnum(ostka);
		nvpdte = kpd0 + ptnum(nstka);
		endvpdte = kpd0 + ptnum(nstka + stksz - 1);

		ptap = (hatp = &oas->a_hat)->hat_pts;
		while (ptap->hatpt_pdtep < ovpdte) {
			if ((ptap = ptap->hatpt_forw) == hatp->hat_pts)
				return(0);
		}
		hatp = &nas->a_hat;
		for (; nvpdte <= endvpdte; ++ovpdte, ++nvpdte) {
			if (!PG_ISVALID(ovpdte))
				continue;
			ASSERT(ptap->hatpt_pdtep == ovpdte);
			unlink_ptap(oas, ptap);
			ptap->hatpt_as = nas;
			if (hatp->hat_pts == (hatpt_t *)NULL) {
				ptap->hatpt_forw = ptap->hatpt_back = ptap;
				hatp->hat_pts = ptap;
			} else {
				ptap->hatpt_forw = hatp->hat_pts;
				ptap->hatpt_back = hatp->hat_ptlast;
				hatp->hat_ptlast->hatpt_forw =
					hatp->hat_pts->hatpt_back = ptap;
			}
			hatp->hat_ptlast = ptap;
			ptap->hatpt_pdtep = nvpdte;
			oas->a_rss -= ptap->hatpt_aec;
			nas->a_rss += ptap->hatpt_aec;
			ptap = ptap->hatpt_forw;
			ovpdte->pg_pte = 0;
		}
		return(0);
	}

	/* In the case of non-aligned PTEs, the PTEs would have to be
	 * copied to new page table(s).  Since this is an extremely
	 * complex operation, for a case which is so rare that it will
	 * probably never occur, we just unload (and swap out) the old
	 * mapping and let references via the new address space fault
	 * the pages back in.
	 */

#ifdef DEBUG
	cmn_err(CE_NOTE, "hat_exec: couldn't do special case - unload instead");
#endif

	{ struct seg	fake_seg;
		fake_seg = *nas->a_segs;
		fake_seg.s_as = oas;
		hat_unload(&fake_seg, ostka, stksz, HAT_FREEPP);
	}
	return(0);
}

/*
 * hat_asload -- load the current process's address space into the
 * mmu (page directory).  Called by as_exec() after setting up the stack.
 */
void
hat_asload()
{
	register hatpt_t	*ptap, *eptap;
	register struct hat	*hatp;

	hatp = &(u.u_procp->p_as->a_hat);
	ptap = eptap = hatp->hat_pts;
	if (ptap != (hatpt_t *)NULL) {
		do {
			ptap->hatpt_pdtep->pg_pte = ptap->hatpt_pde.pg_pte;
			ptap = ptap->hatpt_forw;
		} while (ptap != eptap);
	}
	flushtlb();
}
