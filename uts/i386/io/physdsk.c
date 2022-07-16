/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:physdsk.c	1.3.2.2"

/* 
 *		DMA break-up routine (used by integral hard and 
 *		floppy disk drivers)
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/tss.h"
#include "sys/sysmacros.h"
#include "sys/conf.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/inline.h"
#include "sys/kmem.h"
#include "vm/seg_kmem.h"
#include "vm/page.h"
#include "sys/map.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/dmaable.h"
#ifdef ASYNCIO
#include "sys/async.h"
#endif /* ASYNC IO */

/*
 *	Break up the request that came from physio into chunks of
 *	contiguous memory so we can get around the DMAC limitations
 *	We must be sure to pass at least 512 bytes (one sector) at a
 *      time (except for the last request).
 *
 *      Breaking up the initial request into many smaller request,
 *      as this routine attempts, does have some problems.
 *      If an end of medium (ENXIO) occurs on part of the request,
 *      it is necessary, to do as much of the request as possible
 *      especially for cpio.  This requires that the residual
 *      count be correct, so it must be fixed from the current
 *      request back to the original request.  Also, unfortunately,
 *      ENXIO can come from other errors besides end of medium.
 *      This code does the best it can, and I cannot see
 *      a better way to do it.
 *
 *      dma_breakup might consider doing direct dma when the
 *      request is not aligned, but it will fit in a page.
 *      Currently it does not do this.
 */

/* determine no. bytes till page boundary */

#define	pgbnd(a)	(NBPP - ((NBPP - 1) & (int)(a)))
#define dbbnd(a)	(NBPSCTR - ((NBPSCTR - 1) & (int)(a)))
#define SLEEP(v)	sleep((caddr_t)v,PRIBIO)

void
dma_breakup(strat, bp)
void (*strat)();
register struct buf *bp;
{
	register int iocount;
	register char *va;
	register int cc, rw, left;
	register int firsttime;
	char *kseg();

	rw = bp->b_flags & B_READ;

	/* Indicate that it is firsttime. ENXIO is handled specially        */
	/* depending on whether it is the first io subrequest or not.       */
	/* ENXIO on the first io subrequest is assumed to be an error.      */
	/* On subsequent subrequests, ENXIO assumes that part of the        */
	/* block went out prior to an end of medium error.                  */

	firsttime = 1;

	iocount = bp->b_bcount;
	if (dbbnd(bp->b_un.b_addr) < NBPSCTR) {
		char *iovaddr;

		iovaddr = bp->b_un.b_addr;

		/* user area is not aligned on block (1024 byte) boundary
		 * so copy data to a contiguous kernel buffer and do the
		 * dma from there
		 */

		va = kseg(1);
		if (va == NULL) {
			bp->b_flags |= B_ERROR | B_DONE;
			bp->b_error = EAGAIN;
			return;
		}
		bp->b_un.b_addr = va;
		do {
			/* compute the desired block number into b_blkno    */
			/* btod rounds up, which may or may not be wrong.   */
			/* Calculate the count of bytes to read/write for   */
			/* this iteration into b_bcount.  Left contains     */
			/* the number of bytes remaining after this         */
			/* iteration, and is necessary for fixing up        */
			/* the residual count if end of medium is           */
			/* encountered.                                     */

			bp->b_bcount = cc = min(iocount, NBPP);
			left = iocount - cc;
			bp->b_flags &= ~B_DONE;

			if (rw == B_READ) {

				/* Read the block into a kernel page.      */

				(*strat)(bp);
				spl6();
				while ((bp->b_flags & B_DONE) == 0) {
					bp->b_flags |= B_WANTED;
					SLEEP(bp);
				}
				spl0();

				/* Copy what was read into the kernel page  */
				/* into user memory.                        */

				if (bp->b_flags & B_KERNBUF)
					bcopy(va, iovaddr, bp->b_bcount - bp->b_resid);
				else
					copyout(va, iovaddr, bp->b_bcount - bp->b_resid);
				iovaddr += (bp->b_bcount - bp->b_resid);
				iocount -= (bp->b_bcount - bp->b_resid);

				/* If we have an error that is not          */
				/* end of medium (ENXIO) on 2nd, 3rd, ...   */
				/* interation, then return error to the     */
				/* user. End of medium (ENXIO) on a         */
				/* subsequent block will be handled         */
				/* in the b_resid != 0 code below.          */
				/* Should not be able to get ENXIO for end  */
				/* of medium when reading the disk, it      */
				/* should instead return a residual count   */
				/* indicating part or all of the request    */
				/* could not be read.  However, I have      */
				/* no idea how tape works, so I put the     */
				/* same check as for writing.               */

				if ((bp->b_flags & B_ERROR) &&
				  (firsttime || bp->b_error != ENXIO ||
				  bp->b_resid != bp->b_bcount)) {
					unkseg(va);
					return;
				}

				/* If the residual count is not zero, then  */
				/* part or all of the request could not be  */
				/* read in due to end of medium.  We fix up */
				/* the residual count to reflect the        */
				/* original request, prior to breaking it   */
				/* up.  We clear b_error in case ENXIO was  */
				/* set, and return to the user.             */

				if (bp->b_resid != 0) {
					bp->b_resid += left;
					bp->b_flags &= ~B_ERROR;
					bp->b_error = 0;
					unkseg(va);
					return;
				}
			} else {

				/* Copy the user area to the kernel page,   */
				/* to be written out.                       */

				if (bp->b_flags & B_KERNBUF)
					bcopy(iovaddr, va, cc);
				else
					copyin(iovaddr, va, cc);

				/* Write the block from a kernel page.      */

				(*strat)(bp);

				spl6();
				while ((bp->b_flags & B_DONE) == 0) {
					bp->b_flags |= B_WANTED;
					SLEEP(bp);
				}
				spl0();

				iovaddr += (bp->b_bcount - bp->b_resid);
				iocount -= (bp->b_bcount - bp->b_resid);
				/* If we have an error that is not          */
				/* end of medium (ENXIO) on 2nd, 3rd, ...   */
				/* interation, then return error to the     */
				/* user. End of medium (ENXIO) on a         */
				/* subsequent block will be handled         */
				/* in the b_resid != 0 code below.          */
				/* This is necessary, as a write of 2*NBPP  */
				/* could write out the first NBPP, but then */
				/* return ENXIO for the second write iter-  */
				/* ation. In this case it should return     */
				/* that NBPP bytes were successfully        */
				/* written, and not the ENXIO error.        */

				if ((bp->b_flags & B_ERROR) &&
				  (firsttime || bp->b_error != ENXIO ||
				  bp->b_resid != bp->b_bcount)) {
					unkseg(va);
					return;
				}

				/* If the residual count is not zero, then  */
				/* part or all of the request could not be  */
				/* written due to end of medium.  We fix up */
				/* u_count, u_base and u_offset which had   */
				/* been updated in iomove to only reflect   */
				/* the number of bytes written, and not the */
				/* amount that was moved into the kernel    */
				/* buffer.  We fix up the residual count to */
				/* reflect the original request, prior to   */
				/* breaking it up. We also clear b_error    */
				/* in case ENXIO was set.                   */

				if (bp->b_resid != 0) {
					bp->b_resid += left;
					bp->b_flags &= ~B_ERROR;
					bp->b_error = 0;
					unkseg(va);
					return;
				}
			}

			/* Indicate not firsttime for subsequent iterations */
			/* around the loop.  ENXIO is handled specially     */
			/* depending on whether it is the firsttime or not. */

			firsttime = 0;

			bp->b_blkno += btod(cc);
		} while (iocount);
		unkseg(va);
	} else {
		/*	The buffer is on a sector boundary
		**	but not necessarily on a page boundary.
		*/
	
		if ((bp->b_bcount = cc =
			min( iocount, pgbnd(bp->b_un.b_addr))) < NBPP) {
			left = iocount - cc;

			/*
			 *	Do the fragment of the buffer that's in the
			 *	first page
			 */

			bp->b_flags &= ~B_DONE;
			(*strat)(bp);
			spl6();
			while ((bp->b_flags & B_DONE) == 0) {
				bp->b_flags |= B_WANTED;
				SLEEP(bp);
			}
			spl0();

			/* If we have an error that is not end of medium    */
			/* (ENXIO) on 2nd, 3rd, ... interation, then return */
			/* error to the user. Since this code is only       */
			/* executed for the first time, if there is any     */
			/* error then return error to the user.

			if (bp->b_flags & B_ERROR) {
				return;
			}

			/* update u_base, u_offset, u_count for the amount  */
			/* read/written.                                    */

			bp->b_un.b_addr += (bp->b_bcount - bp->b_resid);
			iocount -= (bp->b_bcount - bp->b_resid);

			/* If the residual count is not zero, then part or  */
			/* all of the request could not be written due to   */
			/* end of medium. We fix up the residual count to   */
			/* reflect the original request, prior to breaking  */
			/* it up. We also clear b_error in case ENXIO was   */
			/* set.                                             */

			if (bp->b_resid != 0) {
				bp->b_resid += left;
				bp->b_flags &= ~B_ERROR;
				bp->b_error = 0;
				return;
			}

			/* update for the next block and core location.    */

			bp->b_blkno += btod(cc);

			/* Indicate not firsttime for subsequent iterations */
			/* around the loop.  ENXIO is handled specially     */
			/* depending on whether it is the firsttime or not. */

			firsttime = 0;
		}

		/*
		 *	Now do the DMA a page at a time
		 */

		while (iocount) {
			bp->b_bcount = cc = min(iocount, NBPP);
			left = iocount - cc;
			bp->b_flags &= ~B_DONE;
			(*strat)(bp);
			spl6();
			while ((bp->b_flags & B_DONE) == 0) {
				bp->b_flags |= B_WANTED;
				SLEEP(bp);
			}
			spl0();

			/* If we have an error that is not end of medium    */
			/* (ENXIO) on 2nd, 3rd, ... interation, then return */
			/* error to the user. End of medium (ENXIO) on a    */
			/* subsequent block will be handled in the b_resid  */
			/* != 0 code below.  This is necessary, as a write  */
			/* of 2*NBPP could write out the first NBPP, but    */
			/* then return ENXIO for the second write iteration.*/
			/* In this case it should return that NBPP bytes    */
			/* were successfully written, and not the ENXIO     */
			/* error.                                           */

			if ((bp->b_flags & B_ERROR) &&
			  (firsttime || bp->b_error != ENXIO ||
			  bp->b_resid != bp->b_bcount)) {
				return;
			}

			/* update u_base, u_offset, u_count for the amount  */
			/* read/written.                                    */

			bp->b_un.b_addr += (bp->b_bcount - bp->b_resid);
			iocount -= (bp->b_bcount - bp->b_resid);

			/* If the residual count is not zero, then part or  */
			/* all of the request could not be written due to   */
			/* end of medium. We fix up the residual count to   */
			/* reflect the original request, prior to breaking  */
			/* it up. We also clear b_error in case ENXIO was   */
			/* set.                                             */

			if (bp->b_resid != 0) {
				bp -> b_resid += left;
				bp->b_flags &= ~B_ERROR;
				bp->b_error = 0;
				return;
			}

			/* update for the next block and core location.    */

			bp->b_blkno += btod(cc);

			/* Indicate not firsttime for subsequent iterations */
			/* around the loop.  ENXIO is handled specially     */
			/* depending on whether it is the firsttime or not. */

			firsttime = 0;
		}
	}
}  /* end dma_breakup */

/*
 * New DMA breakup routine for use by "new" drivers.
 * On the 80386, just call dma_breakup().
 */

void
dma_pageio(strat, bp)
void (*strat)();
register struct buf *bp;
{
	dma_breakup(strat, bp);
}

extern struct map piomap[];

int piomapneed;
extern pte_t *piownptbl;
extern piomaxsz;
#define piopartial(X)	((X) & NBPSCTR-1)
/*
 * Calculate number of pages transfer touchs
 */
#define len(base, count)	\
	btoc(base + count) - btoct(base)

/*
 * Calulate starting user PTE address for transfer
 */
#define upt(base)	\
	(pte_t *) svtopte(base)

#define	kvtopiownptbl(X)	(&piownptbl[pgndx((uint)(X) - (uint)piosegs)])
extern char piosegs[];

caddr_t
mappio(base, npgs, flag)
register caddr_t base;
register int npgs;
int	 flag;		/* normally flag = 0,  nonzero for raw disk async io */
{
	register int	i;
	register pte_t	*pt;
	register pte_t	*wnpt;
	caddr_t wnaddr;

	if (npgs > piomaxsz || npgs <= 0) return(0);
	while ((i = malloc(piomap, npgs)) == 0) {
		/* don't sleep if raw disk async i/o */
		if (flag)	
			return((caddr_t) -1);
		piomapneed++;
		sleep((caddr_t)&piomapneed, PRIBIO);
	}
	wnaddr = (caddr_t) ctob(i);
	wnpt = kvtopiownptbl(wnaddr);
	wnaddr += PAGOFF(base);
	for (i = npgs; --i >= 0; base += ctob(1)) {
		pt = upt(base);
		*wnpt++ = *pt;
	}
	return(wnaddr);
}

unmappio(addr, npgs, base)
caddr_t addr;
caddr_t base;
{
	int i;
	register pte_t	*pt;
	register pte_t	*wnpt;
	caddr_t wnaddr;

	ASSERT(npgs > 0 && npgs <= piomaxsz);
	ASSERT(ptnum(addr) == ptnum(piosegs));
	ASSERT(ptnum(addr+ctob(npgs)-1) == ptnum(piosegs));

	/* Since we used a distinct set of page table entries,
	** the reference and modify info must be transcribed
	** back to the original entries (inclusive OR in effect).
	*/
	wnaddr = addr;
	wnpt = kvtopiownptbl(wnaddr);
	for (i = npgs; --i >= 0; base += ctob(1)) {
		pt = upt(base);
		if (wnpt->pgm.pg_ref)
			PG_SETREF(pt);
		if (wnpt->pgm.pg_mod)
			PG_SETMOD(pt);
		(wnpt++)->pg_pte = 0;
	}
	flushtlb();
	i = btoct(addr);
	mfree(piomap, npgs, i);
	if (piomapneed) {
		piomapneed = 0;
		wakeup((caddr_t)&piomapneed);
	}
}

pio_breakup(strat, bp, maxsecsz)
int (*strat)();
register struct buf *bp;
{
	unsigned bpcnt, usercnt;
	int flags;
	unsigned cnt, acnt, donecnt;
	daddr_t bno;
	buf_t *ebp;
	int rw;
	int soff;
	paddr_t jaddr, useradd;
	caddr_t addr;
	int npgs;
	int maxpgs;
	

	donecnt = 0;
	rw = (bp->b_flags & B_READ);
	flags = bp->b_flags;
	bpcnt = bp->b_bcount;
	cnt = bpcnt - donecnt;
	/*
	if (cnt >= NBPSCTR)
		cnt &= ~(NBPSCTR-1);
	*/
	cnt = dtob(btod(cnt));
	maxpgs = btoc(dtob(maxsecsz));
	if (maxpgs > piomaxsz)
		maxpgs = piomaxsz;
	useradd = paddr(bp);
	usercnt = bp->b_bcount;
#ifdef DEBUG_RAWIO
	cmn_err(CE_CONT,"pio: useraddr = %x count = %d cnt = %d  blkno = %d\n",
			useradd, bp->b_bcount, cnt, bp->b_blkno);
#endif
	while (cnt > 0) {
		acnt = cnt;
		npgs = len(paddr(bp), acnt);
		if (npgs > maxpgs) {
			jaddr = paddr(bp);
			soff = dtob(btod(PAGOFF(jaddr)));
			acnt = dtob(btodt(ctob(maxpgs)-soff));
		}
		if (btod(acnt) > maxsecsz)
			acnt = dtob(maxsecsz);
		npgs = len(paddr(bp), acnt);
#ifdef DEBUG_RAWIO
		cmn_err(CE_CONT,"pio: npgs = %d acnt = %d\n",npgs,acnt);
#endif
		addr = mappio(paddr(bp), npgs, bp->b_flags & B_RAIO);
		if (!addr) {
			bp->b_resid = bpcnt - donecnt;
			if (u.u_error == 0)
				u.u_error = EFAULT;
			bp->b_flags |= B_DONE|B_ERROR;
			bp->b_error = u.u_error;
			return;
		}
		/*
		 * This can only occur when the request
		 * is for raw disk async io. We don't need
		 * to set B_DONE flag here since biodone()
		 * has not been called yet.
		 */ 
		else if (addr == (caddr_t) -1) {
			ASSERT(bp->b_flags & B_RAIO);
			ASSERT(u.u_error == 0);
			u.u_error = EAGAIN;
			bp->b_error = EAGAIN;
			bp->b_flags |= B_ERROR;
			return;
		}
		bp->b_un.b_addr = addr;
		bp->b_bcount = acnt;
		bp->b_flags = flags;
		(*strat)(bp);

		spl6();
		while ((bp->b_flags & B_DONE) == 0) {
			SLEEP(bp);
		}
		spl0();

		geterror(bp);
		unmappio(addr, npgs, useradd);
		acnt -= bp->b_resid;
		bp->b_blkno += btod(acnt);
		donecnt += acnt;
		useradd += acnt;
		usercnt -= acnt;
		if (u.u_error || bp->b_resid) {
#ifdef DEBUG_RAWIO
		cmn_err(CE_CONT,"pio_breakup: error is set\n");
#endif
			bp->b_resid = bpcnt - donecnt;
			return;
		}
		cnt -= acnt;
		bp->b_un.b_addr = (caddr_t) useradd;
		bp->b_bcount=usercnt;
	}
	bp->b_flags |= B_DONE;
	bp->b_resid = 0;
}

#ifdef ASYNCIO
raio_breakup(strat, bp, maxsecsz)
int (*strat)();
register struct buf *bp;
{
	unsigned cnt;
	paddr_t useradd;
	caddr_t addr;
	int npgs,s;
	int maxpgs;
	int count=0;
	struct aiomap *aiop;
	

	cnt = dtob(btod(bp->b_bcount));
	maxpgs = btoc(dtob(maxsecsz));
	if (maxpgs > piomaxsz) {
		bp->b_flags |= B_DONE|B_ERROR;
		bp->b_error = EINVAL;
		return(EINVAL);
	}
	useradd = paddr(bp);
#ifdef DEBUG_RAWIO
	cmn_err(CE_CONT,"pio: useraddr = %x count = %d cnt = %d  blkno = %d\n",
			useradd, bp->b_bcount, cnt, bp->b_blkno);
#endif
	npgs = len(paddr(bp), cnt);
	if ( (npgs > maxpgs) || (btod(cnt) > maxsecsz) ) {
		bp->b_flags |= B_DONE|B_ERROR;
		bp->b_error = EINVAL;
		return(EINVAL);
	}
	addr = mappio(paddr(bp), npgs, bp->b_flags & B_RAIO);
	if (!addr) {
		bp->b_resid = cnt;
		if (u.u_error == 0)
			u.u_error = EFAULT;
		bp->b_flags |= B_DONE|B_ERROR;
		bp->b_error = u.u_error;
		return(EFAULT);
	}
	aiop = (struct aiomap *)kmem_zalloc(sizeof(struct aiomap), KM_NOSLEEP);
	if ( aiop == NULL ) {
		bp->b_resid = cnt;
		if (u.u_error == 0)
			u.u_error = EFAULT;
		bp->b_flags |= B_DONE|B_ERROR;
		bp->b_error = u.u_error;
		unmappio(addr, npgs, paddr(bp));
		return(EFAULT);
	}
	aiop->oldaddr= bp->b_un.b_addr; 
	aiop->newaddr=addr;
	aiop->npgs=npgs;
	bp->b_private = (char *)aiop;
	bp->b_un.b_addr = addr;
	(*strat)(bp);
}

raio_bkdone(bp)
struct	buf 	*bp;
{
	extern struct map	piomap[];
	extern int		piomapneed;
	extern pte_t		*piownptbl;
	extern char		piosegs[];
	extern int		piomaxsz;
	extern page_t		*pages;
	extern page_t		*epages;

	register caddr_t	addr;
	register int		npgs;
	register page_t		*pp;
	register pte_t		*kpte;
	register int		i;
	int count=0,s;
	struct aiomap	*aiop;

	aiop=(struct aiomap *)bp->b_private;
	geterror(bp);
	addr = aiop->newaddr;
	npgs = aiop->npgs;
	ASSERT(npgs > 0 && npgs <= piomaxsz);
	ASSERT(((u_int)addr >= (u_int) piosegs ) &&
		((u_int)addr) < (u_int) piosegs + (NPGPT * NBPP));
	ASSERT(ptnum(addr) == ptnum(piosegs));
	ASSERT(ptnum(addr+ctob(npgs)-1) == ptnum(piosegs));
	for (kpte = kvtopiownptbl(addr), i = npgs; --i > 0; kpte++) {
		ASSERT(PG_ISVALID(kpte));
		pp = page_numtopp(kpte->pgm.pg_pfn);
		ASSERT(pp >= pages && pp < epages);
		/* Assumption: process did not exit and pages locked across I/O */
		if (pp->p_keepcnt < 1)
			cmn_err(CE_PANIC,"raio_bkdone: pp %x not locked\n", pp);
		/* Should be O.K. now that we are page based */
		if (kpte->pgm.pg_ref)
			pp->p_ref = 1;
		if (kpte->pgm.pg_mod)
			pp->p_mod = 1;
		kpte->pg_pte = 0;
	}
	flushtlb();
	i = btoct(addr);
	mfree(piomap, npgs, i);
	if (piomapneed) {
		piomapneed = 0;
		wakeup((caddr_t) &piomapneed);
	}
	bp->b_un.b_addr = (caddr_t) aiop->oldaddr;
	kmem_free(aiop, sizeof(struct aiomap));
	bp->b_private=NULL;
	bp->b_flags |= B_DONE;
	bp->b_resid = 0;
}

/* determine no. bytes till page boundary */

#define	pgbnd(a)	(NBPP - ((NBPP - 1) & (int)(a)))
#define dbbnd(a)	(NBPSCTR - ((NBPSCTR - 1) & (int)(a)))
#define SLEEP(v)	sleep((caddr_t)v,PRIBIO)

void
dma_setup(bp)
register struct buf *bp;
{
	register int		iocount = bp->b_bcount;
	register char		*iovaddr = (char *)bp->b_un.b_addr;
	page_t			*pp;
	register struct aiomap	*map;
	int pagealign = ((PAGOFF(iovaddr) + iocount) >= NBPP) ? 1 : 0;
	int sectoralign = (dbbnd(iovaddr) < NBPSCTR) ? 1 : 0;
	int dmachecks = 0;

#ifdef AT386	/* 16 MB */
	if (dma_check_on && !(pagealign || sectoralign)) {
		ASSERT(PAGOFF(iovaddr) + iocount <= NBPP);
		dmachecks = DMA_BYTE(vtop(iovaddr, bp->b_proc)) ? 0 : 1;
	}
#endif	/* 16 MB */

	if (pagealign || sectoralign || dmachecks) {
		if (! (bp->b_flags & B_READ)) {	/* write case */
			pp = page_get(NBPP, P_NOSLEEP | P_DMA);
			if (pp == (page_t *) NULL) {
				bp->b_flags |= B_ERROR | B_DONE;
				bp->b_error = EAGAIN;
				return;
			}
			bp->b_private = (caddr_t)paddr(bp);
			bp->b_un.b_addr = (caddr_t)phystokv(ctob(page_pptonum(pp)));
			if (copyin(iovaddr, (caddr_t) paddr(bp), iocount) == -1) {
				bp->b_flags |= B_ERROR | B_DONE;
				bp->b_error = EFAULT;
				bp->b_un.b_addr = iovaddr;
				page_rele(pp);
			}
			return;
		}
		/* read case */
		pp = page_get(NBPP, P_NOSLEEP | P_DMA);
		if (pp == (page_t *) NULL) {
			bp->b_flags |= B_ERROR | B_DONE;
			bp->b_error = EAGAIN;
			return;
		}
		ASSERT(pp->p_vnode == NULL && pp->p_keepcnt == 1);
		map =(struct aiomap *) kmem_zalloc(sizeof(struct aiomap), KM_NOSLEEP);
		if ((bp->b_private = (caddr_t) map) == (caddr_t) NULL) {
			page_rele(pp);
			bp->b_flags |= B_ERROR | B_DONE;
			bp->b_error = EAGAIN;
			return;
		}

		map->phys1 = vtop(iovaddr, bp->b_proc);
		map->oldaddr = iovaddr;
		if (pagealign) {	/* crosses a page */
			map->phys2 = vtop(((u_int)(iovaddr+PAGESIZE) & ~POFFMASK),
					bp->b_proc);
			map->npgs = 2;
		}
		else	map->npgs = 1;
		
		bp->b_un.b_addr = (caddr_t)phystokv(ctob(page_pptonum(pp)));
	}
}  /* end dma_setup */
#endif /* ASYNC IO */
