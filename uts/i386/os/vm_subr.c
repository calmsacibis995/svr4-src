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

#ident	"@(#)kern-os:vm_subr.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/sysmacros.h"
#include "sys/inline.h"
#include "sys/buf.h"
#include "sys/uio.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/mman.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/vm.h"

#include "vm/hat.h"
#include "vm/as.h"
#include "vm/seg.h"
#include "vm/page.h"
#ifdef AT386	/* 16 MB support */
#include "sys/dmaable.h"
#include "sys/conf.h"
#endif	/* 16 MB support */

#include "vm/seg_vn.h"
#include "vm/seg_kmem.h"
#include "vm/seg_u.h"

int	buscheck();
int	useracc();

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *	The device number
 *	Read/write flag
 * Essentially all the work is computing physical addresses and
 * validating them.
 * If the user has the proper access privileges, the process is
 * marked 'delayed unlock' and the pages involved in the I/O are
 * faulted and locked. After the completion of the I/O, the above pages
 * are unlocked.
 */

/*
 * This routine has been replaced by uiophysio() and is retained here
 * only for backward compatibility with old device drivers.
 */

void
physio(strat, bp, dev, rw)
	void (*strat)();
	register struct buf *bp;
	dev_t dev;
	int rw;
{
	int	uiophysio();
	struct	iovec iovec;
	struct	uio uio;
	int	error;

	iovec.iov_base = u.u_base;
	iovec.iov_len = u.u_count;
	uio.uio_iov = &iovec;
	uio.uio_iovcnt = 1;
	uio.uio_offset = u.u_offset;
	uio.uio_segflg = (u.u_segflg == 0) ? UIO_USERSPACE : UIO_SYSSPACE;
	uio.uio_fmode = (rw == B_READ) ? FREAD : FWRITE;	/* XXX */
	uio.uio_limit = u.u_rlimit[RLIMIT_FSIZE].rlim_cur;
	uio.uio_resid = u.u_count;				/* XXX */

		/* old driver interface so expand the dev */
	if ((error = uiophysio(strat, bp, expdev(dev), rw, &uio)) != 0)
		u.u_error = (char) error;

	u.u_count = uio.uio_resid;
	u.u_offset = uio.uio_offset;
}

int
uiophysio(strat, bp, dev, rw, uio)
	void (*strat)();
	struct buf *bp;
	dev_t dev;
	int rw;
	struct uio *uio;
{
	register struct iovec *iov;
	register int c;
	faultcode_t fault_err;
	struct proc *procp;
	struct as *asp;
	char *a;
	int hpf, s, error = 0;

	ASSERT(syswait.physio >= 0);
	syswait.physio++;
	if (rw)
		sysinfo.phread++;
	else
		sysinfo.phwrite++;

	hpf = (bp == NULL);
	if (hpf) {
		/*
		 * Get a buffer header off the free list.
		 */
		s = spl6();
		while (pfreecnt == 0)
			sleep((caddr_t)&pfreelist, PRIBIO);
		ASSERT(pfreecnt);
		ASSERT(pfreelist.av_forw);
		pfreecnt--;
		bp = pfreelist.av_forw;
		pfreelist.av_forw = bp->av_forw;
		splx(s);
	}

	if (uio->uio_segflg == UIO_USERSPACE) {
		procp = u.u_procp;
		asp = procp->p_as;
	} else {
		procp = NULL;
		asp = &kas;
	}

	while(uio->uio_iovcnt > 0) {
		iov = uio->uio_iov;
		if ((uio->uio_segflg == UIO_USERSPACE) &&
		    ((error = useracc(iov->iov_base, (uint)iov->iov_len,
			      rw == B_READ ? B_WRITE : B_READ) == NULL)))
			break;

		s = spl6();
		while (bp->b_flags & B_BUSY) {
			bp->b_flags |= B_WANTED;
			(void) sleep((caddr_t)bp, PRIBIO+1);
		}
		(void) splx(s);

		bp->b_oerror = 0;		/* old error field */
		bp->b_error = 0;
		bp->b_proc = procp;

		while (iov->iov_len > 0) {
			if (uio->uio_resid == 0)
				break;
			bp->b_flags = B_KERNBUF | B_BUSY | B_PHYS | rw;
			bp->b_edev = dev;
			bp->b_dev = cmpdev(dev);
			bp->b_blkno = btodt(uio->uio_offset);
			/*
			 * Don't count on b_addr remaining untouched by the
			 * code below (it may be reset because someone does
			 * a bp_mapin on the buffer) -- reset from the iov
			 * each time through, updating the iov's base address
			 * instead.
			 */
			a = bp->b_un.b_addr = iov->iov_base;
			c = bp->b_bcount = MIN(iov->iov_len, uio->uio_resid);
			fault_err = as_fault(asp, a, (uint)c, F_SOFTLOCK,
					   rw == B_READ? S_WRITE : S_READ);
			if (fault_err != 0) {
				/*
				 * Even though the range of addresses were
				 * valid and had the correct permissions,
				 * we couldn't lock down all the pages for
				 * the access we needed. (e.g. we needed to
				 * allocate filesystem blocks for
				 * rw == B_READ but the file system was full).
				 */
				if (FC_CODE(fault_err) == FC_OBJERR)
					error = FC_ERRNO(fault_err);
				else
					error = EFAULT;
				bp->b_flags |= B_ERROR;
				bp->b_error = error;
				(void) spl6();
				if (bp->b_flags & B_WANTED)
					wakeprocs((caddr_t)bp, PRMPT);
				(void) splx(s);
				bp->b_flags &= ~(B_BUSY|B_WANTED|B_PHYS);
				break;
			}
			if (buscheck(bp) < 0) {
				/*
				 * The io was not requested across legal pages.
				 */
				bp->b_flags |= B_ERROR;
				bp->b_error = error = EFAULT;
			} else {
#ifdef AT386	/* 16 MB support */
				if (dma_check_on) {
					register unsigned int	maj;

					maj = getmajor(dev);
					ASSERT(maj >= cdevcnt || cdevsw[maj].d_flag);
					if (maj >= cdevcnt ||
						*cdevsw[maj].d_flag & D_OLD ||
						*cdevsw[maj].d_flag & D_DMA)
						dmaable_rawio(strat, bp, rw);
					else	(*strat)(bp);
				} else
#endif	/* 16 MB support */
					(*strat)(bp);
				s = spl6();
				while ((bp->b_flags & B_DONE) == 0)
					sleep((caddr_t)bp, PRIBIO);
				splx(s);
				if (bp->b_flags & B_ERROR)
					 if (!(error = bp->b_error) &&
					    !(error = bp->b_oerror)) 
						error = EIO; 
			}
			if (as_fault(asp, a, (uint)c, F_SOFTUNLOCK,
			             rw == B_READ? S_WRITE : S_READ) != 0)
				cmn_err(CE_PANIC, "physio unlock");

			(void) spl6();
			if (bp->b_flags & B_WANTED)
				wakeprocs((caddr_t)bp, PRMPT);
			(void) splx(s);

			c -= bp->b_resid;
			iov->iov_base += c;
			iov->iov_len -= c;
			uio->uio_resid -= c;
			uio->uio_offset += c;
			/* bp->b_resid - temp kludge for tape drives */
			if (bp->b_resid || error)
				break;
		}
		bp->b_flags &= ~(B_BUSY|B_WANTED|B_PHYS);
		/* bp->b_resid - temp kludge for tape drives */
		if (bp->b_resid || error)
			break;
		uio->uio_iov++;
		uio->uio_iovcnt--;
	}

	if (hpf) {
		s = spl6();
		bp->av_forw = pfreelist.av_forw;
		pfreelist.av_forw = bp;
		pfreecnt++;
		wakeprocs((caddr_t)&pfreelist, PRMPT);
		splx(s);
	}

	ASSERT(syswait.physio);
	syswait.physio--;
	return(error);
}

int
useracc(addr, count, access)
	register caddr_t addr;
	register uint count;
	register int access;
{
	uint prot;

	prot = PROT_USER | ((access == B_READ) ? PROT_READ : PROT_WRITE);
	return(as_checkprot(u.u_procp->p_as, (addr_t)addr, count, prot) ==
	    0);
}

int
kernacc(addr, count, access)
	register caddr_t addr;
	register uint count;
	register int access;
{
	uint prot;

	prot = ((access == B_READ) ? PROT_READ : PROT_WRITE);
	return(as_checkprot(&kas, (addr_t)addr, count, prot) == 0);
}

/* Add pages to page free list.
 * NOTE: For 386 memory can be in physical discontiguous chunks.
 *
 *
 */

void
memialloc()
{
	register struct page *p;


	for (p = pages; p < epages; p++) {


		page_free(p, 1);
		/* mem_init(p); */

	}
}


/*
 * Obsolete; this check is performed as necessary by individual device
 * drivers.
 */
int
physck(nblocks, rw)
	daddr_t nblocks;
	int rw;
{
	register unsigned over;
	register off_t upper, limit;
	struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;

	limit = nblocks << SCTRSHFT;
	if (u.u_offset >= limit) {
		if (u.u_offset > limit || rw == B_WRITE)
			u.u_error = ENXIO;
		return(0);
	}
	upper = u.u_offset + u.u_count;
	if (upper > limit) {
		over = upper - limit;
		u.u_count -= over;
		uap = (struct a *)u.u_ap;
		uap->count -= over;
	}
	return(1);
}

/*
 * as_shmlookup - Try to determine whether a given user address is mapped
 * to a shared memory segment.  Return the anon_map structure ptr or NULL.
 *
 * XXX - This routine is a (hopefully) temporary kludge pending some
 *       vm-system semantics for doing this.  It could fail to disqualify
 *       an invalid shared segment if it looks an awful lot like the
 *       segment set up by shmat().
 */
struct anon_map *
as_shmlookup(as, addr)
	struct as *as;
	addr_t addr;
{
	register struct seg *seg;
	register struct segvn_data *segvn;

	/* Find an address segment that contains this address */
	if ((seg = as_segat(as, addr)) == NULL)
		return (NULL);

	/* Make sure it is a segvn segment */
	if (seg->s_ops != &segvn_ops)
		return (NULL);

	/* Get the private data and make sure the map index is zero */
	segvn = (struct segvn_data *)seg->s_data;
	if (segvn->amp == NULL)
		return (NULL);

	/* Check that the entire anon region is mapped by the segment */
	if ((segvn->amp->size != seg->s_size) || (seg->s_base != addr))
		return (NULL);

	/* Check other flags, etc., to try to verify validity */
	if ((segvn->vp != NULL) ||
	    (segvn->type != MAP_SHARED) ||
	    (segvn->maxprot != PROT_ALL))
		return (NULL);

	return (segvn->amp);
}

/* This routine is used to find a (seg_vn) segment in the address space
 * of process *prp, whose anon map is *amp.
 * Used by events to find a particular shared memory segment.
 */

struct seg  *
amtoseg(prp, amp)
register proc_t	*prp;
register struct anon_map	*amp;
{

	/*	Find the segment for the process "prp" which refers to
	**	the anon_map "amp".
	*/

	register struct as *as;
	register struct seg *seg, *sseg;
	register struct segvn_data *segvn;

        as = prp->p_as;

        sseg = seg = as->a_segs;
	if (seg != NULL) {
		do {

			/* Make sure it is a segvn segment */
			if (seg->s_ops != &segvn_ops)
				continue;

			segvn = (struct segvn_data *)seg->s_data;
			if (segvn->amp == amp)
				return(seg);

		} while ((seg = seg->s_next) != sseg);
	}
	return(NULL);
}

/* XXX - OBSOLETE ROUTINES MAINTAINED FOR COMPATIBILITY */

/*
 * Soft-lock address range; presumably before user physio.
 */
int
userdma(base, count, rw)
	register caddr_t base;
	register uint count;
	register int rw;
{
	register res;

	if (res = (useracc(base, count, rw == B_READ ? S_WRITE:S_READ) == 1)) {
		if (as_fault(u.u_procp->p_as, base, count, F_SOFTLOCK,
		                rw == B_READ ? S_WRITE:S_READ))
			res = 0;
	}

	return(res);
}

/*
 * Soft-unlock address range; presumably after user physio.
 */
void
undma(base, count, rw)
	caddr_t base;
	uint count;
	int rw;
{
	if (as_fault(u.u_procp->p_as, base, count, F_SOFTUNLOCK,
	             rw == B_READ ? S_WRITE : S_READ) != 0)
		cmn_err(CE_PANIC, "userdma unlock");
}
