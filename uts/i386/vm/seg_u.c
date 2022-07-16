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

#ident  "@(#)kern-vm:seg_u.c	1.4.1.5"

/*
 * VM - u-area segment routines
 *
 * XXX:	Many of the (struct segu_data *) arguments to the routines
 *	below should become (struct seg *)s when the seg_u segment
 *	type becomes more fully realized.
 *
 * Current model:
 *	Each u-area is described the SVR3.2 way in the proc structure,
 *
 * Desired model:
 *	segu_data describes nproc u-areas and the segment ops
 *	manipulate individual slots in segu_data, so that (e.g.)
 *	copying a u-area upon process creation turns into
 *	transcribing parts of segu_data from one place to another.
 *
 * Alternative design possibilities:
 *	When is swap space allocated?  We can do it at process creation
 *	time, as part of setting up the u-area, or upon demand immediately
 *	before swapping out the u-area.  In the latter case, we should
 *	reserve swap space at u-area creation time.
 *
 *	Another possibility still is to use lazy allocation and defer
 *	grabbing swap space for a given u-area until we're about to
 *	swap it out, but subsequently keep hold of it until process
 *	destruction time.
 *
 *	We currently use the last of these techniques since the others
 *	have been observed to cause races.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/mman.h"
#include "sys/vnode.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/tuneable.h"
#include "sys/inline.h"
#include "sys/kmem.h"
#include "sys/user.h"
#include "sys/seg.h"

#include "vm/anon.h"
#include "vm/rm.h"
#include "vm/page.h"
#include "vm/seg.h"
#include "vm/as.h"
#include "sys/swap.h"
#include "sys/proc.h"

#include "sys/immu.h"
#include "vm/hat.h"
#include "vm/vm_hat.h"
#include "vm/seg_u.h"

#if defined(__STDC__)
STATIC int segu_getslot(struct seg *, addr_t, u_int);
#else
STATIC int segu_getslot();
#endif

STATIC int segu_debug = 0;		/* patchable for debugging */

#define UPAGE_PROT	(PROT_READ | PROT_WRITE)

#ifdef DEBUG
#define anon_alloc	anon_upalloc
#define anon_decref	anon_updecref
#define anon_free	anon_upfree
extern struct anon *anon_upalloc();
extern void	anon_updecref();
extern void	anon_upfree();
#else
extern void	anon_decref();	/* XXX */
#endif

extern char	runout;

STATIC	faultcode_t segu_fault(/* seg, addr, type, rw */);
STATIC	int segu_checkprot(/* seg, addr, len, prot */);
STATIC	int segu_getprot(/* seg, addr, len, prot */);
STATIC	off_t segu_getoffset(/* seg, addr */);
STATIC	int segu_gettype(/* seg, addr */);
STATIC	int segu_getvp(/* seg, addr, vpp */);
STATIC	int segu_kluster(/* seg, addr, delta */);
STATIC	void segu_badop();

STATIC struct	seg_ops segu_ops = {
	(int(*)())segu_badop,		/* dup */
	(int(*)())segu_badop,		/* unmap */
	segu_badop,			/* free */
	segu_fault,
	(int(*)())segu_badop,		/* faulta */
	segu_badop,			/* unload */
	(int(*)())segu_badop,		/* setprot */
	segu_checkprot,
	segu_kluster,
	(u_int (*)()) segu_badop,	/* swapout */
	(int(*)())segu_badop,		/* sync */
	(int(*)())segu_badop,		/* incore */
	(int(*)())segu_badop,		/* lockop */
	segu_getprot,
	segu_getoffset,
	segu_gettype,
	segu_getvp,
};

#ifdef __STDC__
STATIC int segu_softunload(struct seg *, addr_t, u_int, int, u_int);
STATIC int segu_softload(struct seg *, addr_t, u_int, int, u_int);
#else
STATIC int segu_softunload();
STATIC int segu_softload();
#endif


STATIC void
segu_badop()
{
	cmn_err(CE_PANIC, "seg_badop");
}

/*
 * Check to see if it makes sense to do kluster/read ahead to
 * addr + delta relative to the mapping at addr.  We assume here
 * that delta is a signed PAGESIZE'd multiple (which can be negative).
 *
 * For seg_u we always "approve" of this action from our standpoint.
 */
/* ARGSUSED */
STATIC int
segu_kluster(seg, addr, delta)
	struct seg	*seg;
	addr_t		addr;
	int		delta;
{
	return (0);
}

/*
 * Segment operations specific to the seg_u segment type.
 */

/*
 * Create the overall segu segment.
 *
 * For the moment, we have no use for the argsp argument.
 */
/* ARGSUSED */
int
segu_create(seg, argsp)
	struct seg	*seg;
	caddr_t		argsp;
{
	register u_int			numslots;
	register int			i, prev;
	register struct segu_segdata	*sdp;
	caddr_t		vbase;

	/*
	 * No need to notify the hat layer, since the SDT's are
	 * already allocated for seg_u; i.e. no need to call
	 * hat_map(). Seg_u does most of it's own dirty work,
	 * setting up it's own mappings.
	 */

	/*
	 * Trim the segment's size down to the largest multiple of
	 * SEGU_PAGES that's no larger than the original value.
	 *
	 * XXX:	Does it matter that we're discarding virtual address
	 *	space off the end with no record of how much there was?
	 */
	numslots = seg->s_size / ptob(SEGU_PAGES);
	seg->s_size = numslots * ptob(SEGU_PAGES);

	/*
	 * Allocate segment-specific information.
	 */
	seg->s_data = (caddr_t) kmem_alloc(sizeof (struct segu_segdata), KM_NOSLEEP);
	if (seg->s_data == NULL)
		return (ENOMEM);
	sdp = (struct segu_segdata *)seg->s_data;

	/*
	 * Allocate the slot array.
	 */
	sdp->usd_slots = (struct segu_data *)
		kmem_alloc(numslots * sizeof (struct segu_data), KM_NOSLEEP);
	if (sdp->usd_slots == NULL) {
		kmem_free(seg->s_data, sizeof (struct segu_segdata));
		return (ENOMEM);
	}

	/*
	 * Set up the slot free list, marking each slot as unallocated.
	 */
	vbase = seg->s_base;
	sdp->usd_slots[0].su_flags = 0;
	for (prev = 0, i = 1; i < numslots; i++) {
		vbase += ptob(SEGU_PAGES);
		sdp->usd_slots[i].su_flags = 0;
		/* If slot crosses page table boundary, skip it */
		if (VPTSIZE - ((ulong)vbase % VPTSIZE) < ptob(SEGU_PAGES)) {
			sdp->usd_slots[i].su_next = NULL;
			continue;
		}
		sdp->usd_slots[prev].su_next = &sdp->usd_slots[i];
		prev = i;
	}
	sdp->usd_slots[prev].su_next = NULL;
	sdp->usd_free = &sdp->usd_slots[0];

	seg->s_ops = &segu_ops;
	return (0);
}


/*
 * Swap in a process's ublock.
 */
int
swapinub(p)
	register proc_t	*p;
{
	register int		i;
	register faultcode_t	fc;
	register int		usize = p->p_usize;

	/*
	 * If the u-block is already in-core, there's no work to do.
	 */
	if (p->p_flag & SULOAD)
		return (1);

	/*
	 * 386 ublock can constitute a variable number of pages.
	 */
	ASSERT((MINUSIZE <= usize) && (usize <= MAXUSIZE));

	if (segu_debug)
		cmn_err(CE_CONT, "swapinub(%x) [pid %d]\n", p, p->p_pid);

	fc = segu_fault(segu, (addr_t)p->p_segu, ptob(usize),
		F_SOFTLOCK, S_OTHER);
	if (fc != 0) {
		cmn_err(CE_PANIC,
			"swapinub: segu_fault() failed to swap in the u area");
	}

	/*
	 * Update the pointer to the page table entries for this ublock,
	 * in case we moved to a new page table.
	 */
	p->p_ubptbl = svtopte(segu_stom((addr_t)p->p_segu));

	p->p_flag |= SULOAD;
	return (1);
}

/*
 * Swap out a process u-block.
 */
int
swapoutub(p)
	register proc_t *p;
{
	register int		i;
	register faultcode_t	fc;
	register int		usize = p->p_usize;

	/*
	 *	386 has variable sized ublocks.
	 */
	ASSERT((MINUSIZE <= usize) && (usize <= MAXUSIZE));

	if (segu_debug)
		cmn_err(CE_CONT, "swapoutub(%x) [pid %d]\n", p, p->p_pid);

	fc = segu_fault(segu, (addr_t)p->p_segu, ptob(usize),
		F_SOFTUNLOCK, S_OTHER);
	if (fc != 0) {
		cmn_err(CE_PANIC,
			"swapoutub: seg_fault() failed to swap u area out");
	}

	p->p_ubptbl = NULL;

	p->p_flag &= ~SULOAD;
	return (1);
}

/*
 * Lock arbitration to ensure that we don't try to swap out a u-block
 * at the same time as it's being swapped in.
 */
void
ub_lock(p)
	register proc_t *p;
{
	while (p->p_flag & SUSWAP) {
		p->p_flag |= SUWANT;
		sleep(((caddr_t)&p->p_flag)+1, PSWP);
	}
	p->p_flag |= SUSWAP;
}

void
ub_rele(p)
	register proc_t *p;
{
	register int s;

	p->p_flag &= ~SUSWAP;
	if (p->p_flag & SUWANT) {
		p->p_flag &= ~SUWANT;
		wakeprocs(((caddr_t)&p->p_flag)+1, PRMPT);
	}
	/*
	 * If sched() went to sleep waiting for a process to swap in,
	 * wake it up (since we might have prevented a swap-in while
	 * we had the u-block locked).
	 */
	s = splhi();
	if (runout) {
		runout = 0;
		setrun(proc_sched);
	}
	splx(s);
}


/*
 * Handle a fault on an address corresponding to one of the
 * slots in the segu segment.
 */
/* ARGSUSED */
STATIC faultcode_t
segu_fault(seg, vaddr, len, type, rw)
	struct seg	*seg;
	addr_t		vaddr;
	u_int		len;
	enum fault_type	type;
	enum seg_rw	rw;
{
	struct segu_segdata	*sdp = (struct segu_segdata *)seg->s_data;
	struct segu_data	*sup;
	int			slot;
	addr_t			vbase;
	int			err;

	/*
	 * Sanity checks.
	 */
	if (seg != segu)
		cmn_err(CE_PANIC,"segu_fault: wrong segment");
	if (type == F_PROT)
		cmn_err(CE_PANIC,"segu_fault: unexpected F_PROT fault");

	/*
	 * Verify that the range specified by vaddr and len falls
	 * completely within the mapped part of a single allocated
	 * slot, calculating the slot index and slot pointer while
	 * we're at it.
	 */
	slot = segu_getslot(seg, vaddr, len);
	if (slot == -1)
		return (FC_MAKE_ERR(EFAULT));
	sup = &sdp->usd_slots[slot];

	vbase = seg->s_base + ptob(SEGU_PAGES) * slot;

	/*
	 * The F_SOFTLOCK and F_SOFTUNLOCK cases have more stringent
	 * range requirements: the given range must exactly coincide
	 * with the slot's mapped portion.
	 */
	if (type == F_SOFTLOCK || type == F_SOFTUNLOCK) {
		if (vaddr != segu_stom(vbase)
				|| len < ptob(sup->su_proc->p_usize)
				|| len > ptob(SEGU_PAGES))
			return (FC_MAKE_ERR(EFAULT));
	}

	if (type == F_SOFTLOCK) {
		/*
		 * Somebody is trying to lock down this slot, e.g., as
		 * part of swapping in a u-area contained in the slot.
		 */

		/*
		 * It is erroneous to attempt to lock when already locked.
		 *
		 * XXX:	Possibly this shouldn't be a panic.  It depends
		 *	on what assumptions we're willing to let clients
		 *	make.
		 */
		if (sup->su_flags & SEGU_LOCKED)
			cmn_err(CE_PANIC,"segu_fault: locking locked slot");

		err = segu_softload(seg, segu_stom(vbase),
				len, slot, 1);
		if (err)
			return (FC_MAKE_ERR(err));

		sup->su_flags |= SEGU_LOCKED;
		return (0);
	}

	if (type == F_INVAL) {
		/*
		 * Normal fault (e.g., by accessing PTOU(p)).  The processing
		 * required is quite similar to that for the F_SOFTLOCK case in
		 * that we have to drag stuff in and make sure it's mapped.  It
		 * differs in that we don't lock it down.
		 */

		if (segu_debug)
			cmn_err(CE_CONT, "segu_fault(%x, %x, %d)\n",
				vaddr, len, type);

		/*
		 * If the slot is already locked, the only way we
		 * should fault is by referencing the red zone.
		 *
		 * XXX:	Probably should tighten this check and verify
		 *	that it's really a red zone reference.
		 * XXX:	Is this the most appropriate error code?
		 */
		if (sup->su_flags & SEGU_LOCKED)
			return (FC_MAKE_ERR(EINVAL));

		err = segu_softload(seg, vaddr, len, slot, 0);
		return (err ? FC_MAKE_ERR(err) : 0);
	}

	if (type == F_SOFTUNLOCK) {
		/*
		 * Somebody is trying to swap out this slot, e.g., as
		 * part of swapping out a u-area contained in this slot.
		 */

		/*
		 * It is erroneous to attempt to unlock when not
		 * currently locked.
		 */
		if (!(sup->su_flags & SEGU_LOCKED))
			cmn_err(CE_PANIC,"segu_fault: unlocking unlocked slot");
		sup->su_flags &= ~SEGU_LOCKED;

		err = segu_softunload(seg, vaddr, len, slot, 1);
		return (err ? FC_MAKE_ERR(err) : 0);
	}

	cmn_err(CE_PANIC,"segu_fault: bogus fault type");
	/* NOTREACHED */
}

/*
 * Check that the given protections suffice over the range specified by
 * vaddr and len.  For this segment type, the only issue is whether or
 * not the range lies completely within the mapped part of an allocated slot.
 *
 * We let segu_getslot do all the dirty work.
 */
/* ARGSUSED */
STATIC int
segu_checkprot(seg, vaddr, len, prot)
	struct seg	*seg;
	addr_t		vaddr;
	u_int		len;
	u_int		prot;
{
	register int	slot = segu_getslot(seg, vaddr, len);

	return (slot == -1 ? EACCES : 0);
}

static int
segu_getprot(seg, addr, len, protv)
	register struct seg *seg;
	register addr_t addr;
	register u_int len, *protv;
{
	u_int pgno = seg_page(seg, addr+len) - seg_page(seg, addr) + 1;

	if (pgno != 0) {
		do protv[--pgno] = (u_int)UPAGE_PROT;
		while (pgno != 0);
	}
	return 0;
}

/* ARGSUSED */
static off_t
segu_getoffset(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	return 0;
}

/* ARGSUSED */
static int
segu_gettype(seg, addr)
	struct seg *seg;
	addr_t addr;
{
	return MAP_SHARED;
}

/* ARGSUSED */
segu_getvp(seg, addr, vpp)
	struct seg *seg;
	addr_t addr;
	struct vnode **vpp;
{
	*vpp = NULL;
	return -1;
}


/*
 * Allocate resources for a single slot.
 *
 * When used for u-area, called at process creation time.
 *
 * Assumption: no resources exist as yet for this slot, so there's
 * no possibility of (say) some swap space already being allocated.
 * (This is a change from the previous implementation.)
 *
 * For system processes, no swap space is allocated, since they will
 * never get swapped.
 */
addr_t
segu_get(cp, no_swap)
	struct proc *cp;	/* The child's proc. structure */
	int	no_swap;	/* Flag: don't allocate swap space */
{
	struct segu_segdata	*sdp = (struct segu_segdata *)segu->s_data;
	page_t			*pp;
	addr_t			vbase;
	addr_t			va;
	struct segu_data	*sup;
	int			slot;
	int			i;
	register int		usize = cp->p_usize;	/* Should be set by now */
							/* for the child */

	/*
	 *  386 ublock can constitute of a variable number of pages.
	 */
	ASSERT((MINUSIZE <= usize) && (usize <= MAXUSIZE));

	/*
	 * Allocate virtual space.  This amounts to grabbing a free slot.
	 */
	if ((sup = sdp->usd_free) == NULL)
		return (NULL);
	sdp->usd_free = sup->su_next;
	slot = sup - sdp->usd_slots;

	vbase = segu->s_base + ptob(SEGU_PAGES) * slot;

	ASSERT(ptnum(vbase) == ptnum(vbase + ctob(SEGU_PAGES) - 1));

	/*
	 * Reserve sufficient swap space for this slot.  We'll
	 * actually allocate it in the loop below, but reserving it
	 * here allows us to back out more gracefully than if we
	 * had an allocation failure in the body of the loop.
	 *
	 * Note that we don't need swap space for the red zone page.
	 */
	if (no_swap) {
		if (availrmem - usize < tune.t_minarmem ||
		    availsmem - usize < tune.t_minasmem) {
			nomemmsg("segu_get", usize, 0, 0);
			return(NULL);
		}
		availrmem -= usize;
		availsmem -= usize;
		pages_pp_kernel += usize;
	}
	else if (anon_resv(ptob(usize)) == 0) {
		if (segu_debug)
			cmn_err(CE_CONT, "segu_get: no swap space available\n");
		sup->su_next = sdp->usd_free;
		sdp->usd_free = sup;
		return (NULL);
	}

	/*
	 * Allocate pages, avoiding allocating one for the red zone.
	 */
	pp = rm_allocpage(segu, segu_stom(vbase), ptob(usize), P_CANWAIT);
	if (pp == NULL) {
		if (segu_debug)
			cmn_err(CE_CONT, "segu_get: no pages available\n");
		/*
		 * Give back the resources we've acquired.
		 */
		if (no_swap) {
			availrmem += usize;
			availsmem += usize;
			pages_pp_kernel -= usize;
		} else
			anon_unresv(ptob(usize));
		sup->su_next = sdp->usd_free;
		sdp->usd_free = sup;
		return (NULL);
	}

	/*
	 * Allocate swap space.
	 *
	 * Because the interface for getting swap slots is designed
	 * to handle only one page at a time, we must deal with each
	 * page in the u-area individually instead of allocating a
	 * contiguous chunk of swap space for the whole thing as we
	 * would prefer.
	 *
	 * This being the case, we actually do more in this loop than
	 * simply allocate swap space.  As we handle each page, we
	 * complete its setup.
	 */
	for (i = 0, va = vbase; i < usize; i++, va += ptob(1)) {
		register struct anon	*ap;
		struct vnode		*vp;
		u_int			off;

		/*
		 * Sanity check.
		 */
		if (pp == NULL)
			cmn_err(CE_PANIC,"segu_get: not enough pages");

		if (no_swap) {
			page_lock(pp);
		} else {
			/*
			 * Get a locked swap slot.
			 */
			if ((ap = anon_alloc()) == NULL) {
				cmn_err(CE_PANIC,
					"segu_get: swap allocation failure");
			}
			sup->su_swaddr[i] = ap;

			/*
			 * Tie the next page to the swap slot.
			 */
			swap_xlate(ap, &vp, &off);
			while (page_enter(pp, vp, off)) {
				/*
				 * The page was already tied to something
				 * else that we have no record of.  Since
				 * the page we wish be named by <vp, off>
				 * already exists, we abort the old page.
				 */
				page_t	*p1;

				p1 = page_lookup(vp, off);
				if (p1 != NULL)
					page_abort(p1);
			}
			AUNLOCK(ap);
		}

		/*
		 * Mark the page for long term keep.
		 *
		 * XXX:	Until we switch over to using a separate long
		 *	term keep count field, we need do nothing.
		 * XXX:	When the page structure is modified to include a
		 *	long-term keep count field, we'll want to use
		 *	that field.  In the meantime, we have to make do
		 *	with the (short term) p_keepcnt field.  This
		 *	introduces possibilities for deadlock unless we're
		 *	very careful to constrain ourselves elsewhere
		 *	(e.g., in how we say the pages should be released
		 *	at segmap_release time). 
		 */

		/*
		 * Load and lock an MMU translation for the page.
		 */
		hat_memload(segu, va, pp, UPAGE_PROT, HAT_LOCK);

		page_unlock(pp);

		/*
		 * Prepare to use the next page.
		 */
		page_sub(&pp, pp);
	}

	/*
	 * Keep a pointer to the page table entries for this ublock,
	 * to increase speed of context switch.
	 */
	cp->p_ubptbl = svtopte(segu_stom(vbase));

	/*
	 * Finally, mark this slot as allocated and locked.
	 */
	sup->su_flags = SEGU_ALLOCATED | SEGU_LOCKED;
	sup->su_proc = cp;

	/*
	 * Return the address of the base of the mapped part of
	 * the slot.
	 */
	return (segu_stom(vbase));
}

/*
 * Reclaim resources for a single slot.
 *
 * When used for u-area, called at process destruction time.
 *
 * N.B.: Since this routine deallocates all of the slot's resources,
 * callers can't count on the resources remaining accessible.  In
 * particular, any stack contained in the slot will vanish, so we'd
 * better not be running on that stack.
 *
 * We can't simply undo everything that segu_get did directly,
 * because someone else may have acquired a reference to one or
 * more of the associated pages in the meantime.
 */
void
segu_release(p)
	struct proc *p;
{
	struct segu_segdata	*sdp = (struct segu_segdata *)segu->s_data;
	addr_t			vbase;
	addr_t			vaddr;
	struct segu_data	*sup;
	int			slot;
	int			i;
	register int		usize = p->p_usize;

	ASSERT(p != NULL);

	vaddr = (addr_t)p->p_segu;
	vbase = segu_mtos(vaddr);
	p->p_segu = (struct seguser *)NULL;

	/*
	 * Get the slot corresponding to this virtual address.
	 */
	slot = (vbase - segu->s_base) / ptob(SEGU_PAGES);
	sup = &sdp->usd_slots[slot];

	/*
	 * XXX:	Do we need to lock this slot's pages while we're
	 *	messing with them?  What can happen once we decrement
	 *	the keep count below?
	 */

	/*
	 * If this slot is locked, unlock it, unlock the mmu
	 * translation, and decrement the keep count.
	 */
	if (sup->su_flags & SEGU_LOCKED) {
		sup->su_flags &= ~SEGU_LOCKED;
		hat_unload(segu, vaddr, ptob(usize), HAT_UNLOCK|HAT_RELEPP);
	}
	else {
		hat_unload(segu, vaddr, ptob(usize), HAT_NOFLAGS);
	}
	p->p_ubptbl = (pte_t *)NULL;

	if (p->p_flag & SSYS) {
		availrmem += usize;
		availsmem += usize;
		pages_pp_kernel -= usize;
	} else {
		/*
		 * Release our claim on swap space for the pages controlled
		 * by this slot.
		 */
#ifdef DEBUG
		anon_free(sup->su_swaddr, ptob(usize));
#else
		register int 	i;
		register struct anon	**app = sup->su_swaddr;

		/* Can't call anon_free() directly, since it does a PREEMPT().
		   Expand it inline here, w/o the PREEMPT(). */
		i = usize;
		while (i-- > 0) {
			if (*app != NULL) {
				anon_decref(*app);
				*app = NULL;
			}
			app++;
		}
#endif
		anon_unresv(ptob(usize));
	}

	/*
	 * Mark the slot as unallocated and put it back on the free list.
	 */
	sup->su_flags &= ~SEGU_ALLOCATED;
	sup->su_next = sdp->usd_free;
	sdp->usd_free = sup;

	ASSERT(p->p_parent);

	/*
	 *	Since the proc structure entries are now dynamically allocted
	 *	we could not free the proc structure during pswtch().
	 *	Instead we delay it and remove it here. This is 386 specific
	 *	and we need to get the right amount of pages allocated for
	 *	the ublock - which is kept in the proc structure.
	 */
	if (p->p_parent->p_flag & SNOWAIT)
		freeproc(p);
}

/*
 * Private routines for use by seg_u operations.
 */

/*
 * Verify that the range designated by vaddr and len lies completely
 * within the mapped part of a single allocated slot.  If so, return
 * the slot's index; otherwise return -1.
 */
STATIC int
segu_getslot(seg, vaddr, len)
	register struct seg	*seg;
	addr_t			vaddr;
	u_int			len;
{
	register int			slot;
	register struct segu_segdata	*sdp;
	register struct segu_data	*sup;
	addr_t				vlast;
	addr_t				vmappedbase;

	sdp = (struct segu_segdata *)seg->s_data;

	/*
	 * Make sure the base is in range of the segment as a whole.
	 */
	if (vaddr < seg->s_base || vaddr >= seg->s_base + seg->s_size)
		return (-1);

	/*
	 * Figure out what slot the address lies in.
	 */
	slot = (vaddr - seg->s_base) / ptob(SEGU_PAGES);
	sup = &sdp->usd_slots[slot];

	/*
	 * Make sure the end of the range falls in the same slot.
	 */
	vlast = vaddr + len - 1;
	if ((vlast - seg->s_base) / ptob(SEGU_PAGES) != slot)
		return (-1);

	/*
	 * Nobody has any business touching this slot if it's not currently
	 * allocated.
	 */
	if (!(sup->su_flags & SEGU_ALLOCATED))
		return (-1);

	/*
	 * Finally, verify that the range is completely in the mapped part
	 * of the slot.
	 */
	vmappedbase = segu_stom(seg->s_base + ptob(SEGU_PAGES) * slot);
	if (vaddr < vmappedbase || vlast >= vmappedbase + ptob(SEGU_PAGES))
		return (-1);

	return (slot);
}

/*
 * Unload intra-slot resources in the range given by vaddr and len.
 * Assumes that the range is known to fall entirely within the mapped
 * part of the slot given as argument and that the slot itself is
 * allocated.
 */
STATIC int
segu_softunload(seg, vaddr, len, slot, locked)
	struct seg	*seg;
	addr_t		vaddr;
	u_int		len;
	int		slot;
	u_int		locked;
{
	register struct segu_data	*sup;

	sup = &((struct segu_segdata *)segu->s_data)->usd_slots[slot];

	ASSERT(sup->su_proc != NULL);
	ASSERT(!(sup->su_proc->p_flag & SSYS));

	vaddr = (addr_t)((u_int)vaddr & PAGEMASK);
	len = roundup(len, ptob(1));

	/*
	 * We tried calling hat_unload with HAT_FREEPP to put the pages on
	 * the free list if there weren't anymore mappings, but it hurt
	 * performance slightly, so use HAT_PUTPP to just put out dirty pages
	 * without freeing clean ones.
	 */
	if (locked) {
		hat_unload(seg, vaddr, len, HAT_UNLOCK|HAT_RELEPP|HAT_PUTPP);
	} else {
		hat_unload(seg, vaddr, len, HAT_RELEPP|HAT_PUTPP);
	}

	return (0);
}


/*
 * Load and possibly lock intra-slot resources in the range given
 * by vaddr and len.  Assumes that the range is known to fall entirely
 * within the mapped part of the slot given as argument and that the
 * slot itself is allocated.
 */
STATIC int
segu_softload(seg, vaddr, len, slot, lock)
	struct seg	*seg;
	addr_t		vaddr;
	u_int		len;
	int		slot;
	u_int		lock;
{
	struct segu_segdata	*sdp = (struct segu_segdata *)segu->s_data;
	register struct segu_data
				*sup = &sdp->usd_slots[slot];
	register addr_t		va;
	addr_t			vlim;
	register u_int		i;

	ASSERT(sup->su_proc != NULL);
	ASSERT(!(sup->su_proc->p_flag & SSYS));

	/*
	 * Loop through the pages in the given range.
	 */
	va = vaddr = (addr_t)((u_int)vaddr & PAGEMASK);
	len = roundup(len, ptob(1));
	vlim = va + len;
	/* Calculate starting page index within slot. */
	i = (va - (seg->s_base + slot * ptob(SEGU_PAGES))) / ptob(1);
	for ( ; va < vlim; va += ptob(1), i++) {
		page_t		*pl[2];
		struct vnode	*vp;
		u_int		off;
		register int	err;
		struct anon	*ap;

		/*
		 * Summon the page.  If it's not resident, arrange
		 * for synchronous i/o to pull it in.
		 *
		 * XXX:	Need read credentials value; for now we punt.
		 */
		ap = sup->su_swaddr[i];
		ALOCK(ap);
		swap_xlate(ap, &vp, &off);
		err = VOP_GETPAGE(vp, off, ptob(1), (u_int *)NULL,
			pl, ptob(1), seg, va, S_WRITE, (struct ucred *)NULL);
		AUNLOCK(ap);
		if (err) {
			/*
			 * Back out of what we've done so far.
			 */
			if ((len = va - vaddr) != 0) {
				(void) segu_softunload(seg, vaddr, len, 
						slot, lock);
			}
			return (err);
		}
		/*
		 * The returned page list will have exactly one entry,
		 * which is returned to us already kept.
		 */

		/*
		 * Load an MMU translation for the page.
		 */
		hat_memload(seg, va, pl[0], UPAGE_PROT, lock);

		/*
		 * If we're locking down resources, we need to increment
		 * the page's long term keep count.  In any event, we
		 * need to decrement the (short term) keep count.
		 *
		 * XXX:	For the moment the long and short term keep counts
		 *	are the same thing, so in the locking case, the
		 *	count manipulations cancel.
		 */
		if (!lock)
			PAGE_RELE(pl[0]);
	}

	return (0);
}

/* XENIX Support */
/*
 *	segu_expand (newsz) expands the current proc's ublock to accomodate
 *	newsz pages in its ldt[]; Returns 1 if successful, 0 otherwise.
 *
 */
segu_expand(newsz)
register unsigned int newsz;
{
	register struct proc *p = u.u_procp;
	register ushort npages;
	ushort   oldsz = p->p_usize;
	ushort   tmp;
	int	 slot;
	struct	 segu_segdata *sdp = (struct segu_segdata *) segu->s_data;
	struct	 segu_data	*sup;
	page_t	 *pp;
	addr_t	 vaddr, vbase, va;
	register int	usize = p->p_usize;

	vaddr = (addr_t)p->p_segu;


	if (newsz > SEGU_PAGES) {	/* and not... MAXUSIZE - since this driver */
		return(0);		/* can handle only SEG_PAGES "slots".	   */
	}

	if ((npages = newsz - oldsz) <= 0) {	/* Nothing to expand? */
		return(1);			/* Successful */
	}

	if (anon_resv(ptob(npages)) == 0) {
		if (segu_debug)
			cmn_err(CE_CONT,"segu_expand: No swap for expanding u-block\n");
		return(0);
	}

	if (availrmem - (int)npages < tune.t_minarmem
	  || availsmem - (int)npages < tune.t_minasmem) {
		nomemmsg("segu_expand",npages, 0, 0);
		anon_unresv(ptob(npages));
		return(0);
	}
	else {
		if (p->p_flag & SSYS) {
			availrmem -= npages;
			pages_pp_kernel += npages;
		}
		availsmem -= npages;
	}

	/*
	 *  First get the virtual address (in segvu) and its corresponding "slot"
	 *  for the existing pages in the ublock.
	 */

	vbase = segu_mtos(vaddr);
	slot = (vbase - segu->s_base) / ptob(SEGU_PAGES);
	sup = &sdp->usd_slots[slot];

	/*
	 *  ASSERT: The slot must previously been allocated - since current ublock.
	 *  ASSERT: This slot belongs to the current process expanding the ublock.
	 */
	
	ASSERT((sup->su_flags & SEGU_ALLOCATED) && (sup->su_proc == p));

	/*
	 *  Find the virtual offset into the "slot". Needed since we want to do
	 *  a hat_memload() for each virtual address in segvu virtual segment.
	 */

	vaddr += ptob(oldsz);
	vbase = segu_mtos(vaddr);
	pp = rm_allocpage(segu, segu_stom(vbase), ptob(usize), 1);
	if (pp == NULL) {
		if (segu_debug)
			cmn_err(CE_CONT,"segu_expand: no pages available\n");
		anon_unresv(ptob(npages));
		return(0);
	}

	for (tmp = oldsz, va = vaddr; tmp < newsz; tmp++, va += PAGESIZE) {
		register struct anon *ap;
		struct vnode	     *vp;
		u_int		     off;

		ASSERT(pp != NULL);

		if ((ap = anon_alloc()) == NULL)
			cmn_err(CE_PANIC,"segu_expand: swap allocation failure");
		sup->su_swaddr[tmp] = ap;

		swap_xlate(ap, &vp, &off);
		while (page_enter(pp, vp, off)) {
			struct page *pl;

			pl = page_lookup(vp, off);
			if (pl != NULL)
				page_abort(pl);
		}
		AUNLOCK(ap);

		hat_memload(segu, va, pp, UPAGE_PROT, 1);
		page_unlock(pp);
		usertable[tmp].pg_pte = mkpte(PG_V, page_pptonum(pp));
		page_sub(&pp, pp);
	}
	flushtlb();

	p->p_usize = newsz;
	struct_zero((caddr_t)&u + ctob(oldsz), ctob(npages));
	return(1);
}

/*
 *	segu_shrink(ldtlim) reduces the current proc's ublock to accomodate
 *	only ldtlim entries in its ldt[];
 */

segu_shrink(ldtlim)
	ushort ldtlim;
{
	register struct proc	*p = u.u_procp;
	register short		npages;
	register short		newsz;
	register pte_t		*ptep;
	register page_t		*pp;
	ushort			oldsz;
	int			tmp;
	struct segu_segdata	*sdp = (struct segu_segdata *)segu->s_data;
	addr_t			vbase, vaddr, va;
	struct segu_data	*sup;
	int			slot;

	vaddr = (addr_t)p->p_segu;

	newsz = btoc(p->p_ldt - vaddr + (ldtlim + 1) * sizeof(struct dscr));
	oldsz = btoc(p->p_ldt - vaddr +
				 (u.u_ldtlimit + 1) * sizeof(struct dscr));

	if (newsz < MINUSIZE) {
		cmn_err(CE_PANIC, "segu_shrink: newsz < MINUSIZE");
	}
	if ((npages = oldsz - newsz) <= 0) {
		return;
	}

	/* Clear entries in &u mapping */
	for (tmp = oldsz; tmp < newsz; tmp++) {
		usertable[tmp].pg_pte = 0;
	}
	p->p_usize = newsz;

	vbase = segu_mtos(vaddr);
	slot = (vbase - segu->s_base) / ptob(SEGU_PAGES);
	sup = &sdp->usd_slots[slot];

	/*
	 *  ASSERT: The slot must previously been allocated - since current ublock.
	 *  ASSERT: This slot belongs to the current process shrinking the ublock.
	 */

	ASSERT((sup->su_flags & SEGU_ALLOCATED) && (sup->su_proc == p));

	/*
	 * Find the virtual offset into the "slot". Needed since we want to do
	 * a hat_unload() for each virtual address in segu virtual segment.
	 */
	vaddr += ptob(oldsz);

	/*
	 *  Unload the mmu translations for portion(s) of this slot
	 *  and release the corresponding pages.
	 */
	hat_unload(segu, vaddr, ptob(npages), HAT_UNLOCK|HAT_RELEPP);

	ASSERT(!(p->p_flag & SSYS));

	/*
	 * Release our claim on swap space for the pages thrown out.
	 */
	anon_free(&sup->su_swaddr[newsz], ptob(npages));
	anon_unresv(ptob(npages));

	return (npages);
}
/* End XENIX Support */
