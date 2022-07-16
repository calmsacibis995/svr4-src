/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)xx:io/xxrdwri.c	1.2.2.4"

#include "sys/types.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/param.h"
#include "sys/swap.h"
#include "sys/sysmacros.h"
#include "sys/resource.h"
#include "sys/systm.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"

#include "sys/proc.h"	/* XXX -- needed for user-context kludge in ILOCK */
#include "sys/disp.h"

#include "sys/fs/s5param.h"
#include "sys/fs/s5inode.h"
#include "sys/fs/s5macros.h"

#include "vm/seg_kmem.h"
#include "vm/seg_map.h"
#include "vm/seg.h"
#include "vm/page.h"

#include "sys/cmn_err.h"
#include "sys/kmem.h"

/*
 * Package the arguments into a uio structure and invoke readi()
 * or xxwritei(), as appropriate.
 */
int
xxrdwri(rw, ip, base, len, offset, seg, ioflag, aresid)
	enum uio_rw rw;
	struct inode *ip;
	caddr_t base;
	int len;
	off_t offset;
	enum uio_seg seg;
	int ioflag;
	int *aresid;
{
	struct uio auio;
	struct iovec aiov;
	int error;

	aiov.iov_base = base;
	auio.uio_resid = aiov.iov_len = len;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = offset;
	auio.uio_segflg = (short)seg;
	auio.uio_limit = offset + NBPSCTR;
	if (rw == UIO_WRITE) {
		auio.uio_fmode = FWRITE;
		error = xxwritei(ip, &auio, ioflag);
	} else {
		auio.uio_fmode = FREAD;
		error = xxreadi(ip, &auio, ioflag);
	}
	if (aresid)
		*aresid = auio.uio_resid;
	return error;
}

/*
 * Read the file corresponding to the supplied inode.
 */
/* ARGSUSED */
int
xxreadi(ip, uiop, ioflag)
	register struct inode *ip;
	register struct uio *uiop;
	int ioflag;
{
	register unsigned int on, n;
	int mode, flags, error = 0;
	off_t off;
	caddr_t base;
	struct vnode *vp = ITOV(ip);

	mode = ip->i_mode;
	if (MANDLOCK(vp, mode)
	  && (error = chklock(vp, FREAD,
	    uiop->uio_offset, uiop->uio_resid, uiop->uio_fmode)))
		return error;
	if (uiop->uio_resid == 0)
		return 0;
	if (uiop->uio_offset < 0)
		return EINVAL;
	do {
		/*
		 * Prepare to map in a MAXBSIZE chunk of the file for I/O.
		 * Compute n, the number of bytes which can be read from
		 * this mapping.
		 */
		off = uiop->uio_offset & MAXBMASK;
		on = uiop->uio_offset & MAXBOFFSET;
		n = MIN(MAXBSIZE-on, uiop->uio_resid);
		n = (ip->i_size < uiop->uio_offset) ?
		  0 : MIN(n, ip->i_size - uiop->uio_offset);
		if (n == 0)
			break;
		base = segmap_getmap(segkmap, vp, off);
		flags = 0;
		if ((error = uiomove(base+on, n, UIO_READ, uiop)) == 0) {
			/*
			 * If we read to the end of the mapping or to
			 * EOF, we won't need these pages again soon.
			 */
			if (n + on == MAXBSIZE
			  || uiop->uio_offset == ip->i_size)
				flags |= SM_DONTNEED;
			error = segmap_release(segkmap, base, flags);
			ip->i_flag |= IACC;
		} else
			(void) segmap_release(segkmap, base, 0);
	} while (error == 0 && uiop->uio_resid > 0);

	return error;
}

/*
 * Write the file corresponding to the specified inode.
 */
int
xxwritei(ip, uiop, ioflag)
	register struct inode *ip;
	register struct uio *uiop;
	int ioflag;
{
	register struct vnode *vp = ITOV(ip);
	register struct s5vfs *s5vfsp = S5VFS(vp->v_vfsp);
	register unsigned int n, on;
	off_t off;
	daddr_t firstlbn, lastlbn;
	caddr_t base, addr;
	unsigned long int oresid = uiop->uio_resid;
	rlim_t limit = uiop->uio_limit;
	int mode = ip->i_mode, error = 0, flags, pagecreate;
	int bsize = VBSIZE(vp);
	int i;
	off_t osize;
	int part_write = 0;
	daddr_t dblist[MAXBSIZE/NBPSCTR];
	page_t *iolpl[MAXBSIZE/PAGESIZE + 2];
	page_t **ppp;

	if (MANDLOCK(vp, mode)
	  && (error = chklock(vp, FWRITE,
	    uiop->uio_offset, uiop->uio_resid, uiop->uio_fmode)))
		return error;
	if (uiop->uio_offset < 0)
		return EINVAL;

	ip->i_flag |= INOACC;	/* Don't update access time in getpage() */
	if (ioflag & IO_SYNC)
		ip->i_flag |= ISYNC;

	while (error == 0 && uiop->uio_resid > 0) {
		if (part_write)
			goto err;
		off = uiop->uio_offset & MAXBMASK;
		on = uiop->uio_offset & MAXBOFFSET;
		n = MIN(MAXBSIZE-on, uiop->uio_resid);
		if (vp->v_type == VREG && uiop->uio_offset + n >= limit) {
			if (uiop->uio_offset >= limit) {
				error = EFBIG;
				goto err;
			}
			n = limit - uiop->uio_offset;
		}

		osize = ip->i_size;

		/*
		 * as_iolock will determine if we are properly
		 * page-aligned to do the pagecreate case, and if so,
		 * will hold the "from" pages until after the uiomove
		 * to avoid deadlocking and to catch the case of
		 * writing a file to itself.
		 */
		n = as_iolock(uiop, iolpl, n, vp, osize, &pagecreate);
		if (n == 0) {
			error = EFAULT;
			goto err;
		}

		/*
		 * We must ensure that any file blocks are allocated before
		 * we perform the I/O.
		 */
		firstlbn = uiop->uio_offset >> s5vfsp->vfs_bshift;
		lastlbn = (uiop->uio_offset + n - 1) >> s5vfsp->vfs_bshift;
		for (i = 0; i < (MAXBSIZE/NBPSCTR); i++)
			dblist[i] = 0;
		error = xxbmapalloc(ip, firstlbn, lastlbn, pagecreate, &dblist[0]);
		if (error) {
			if (error != ENOSPC || dblist[0] == 0) {
				for (ppp = iolpl; *ppp; ppp++ )
					PAGE_RELE(*ppp);
				goto err;
			} else {
				/*
				 * There are not enough free blocks. We can
				 * only accomodate a partial write and need
				 * to calculate how much we can really write.
				 */
				part_write++;
				n = ((firstlbn+1) << s5vfsp->vfs_bshift)
							- uiop->uio_offset;
				i = 1;
				while (i <= MAXBSIZE/NBPSCTR
				  && dblist[i++] != 0)
					n += bsize;
				if (pagecreate) {
					/*
					 * Since we can only do a partial write,
					 * we might not be able to page-create
					 * anymore.
					 */
					pagecreate = ((n & PAGEOFFSET) == 0 ||
						uiop->uio_offset + n >= osize);
				}
			}
		}

		if (uiop->uio_offset + n > osize)
			ip->i_size = uiop->uio_offset + n;

		if (bsize < PAGESIZE && uiop->uio_offset > osize) {
			/*
			 * If we are leaving a hole in the
			 * page at old EOF and the page
			 * is in the page pool, we need
			 * to mark the page as read-only
			 * so that any attempt to store to
			 * the page will cause a write-fault
			 * to force the holes to be filled.
			 */
			off_t nboff = (osize + bsize - 1) & ~s5vfsp->vfs_bmask;
			if ((nboff & PAGEOFFSET) != 0
			  && nboff + bsize <= uiop->uio_offset) {
				/*
				 * There are holes at end of page
				 * and we skip at least one.
				 */
				page_t *pp = page_lookup(vp, nboff & PAGEMASK);
				if (pp)
					page_rdonly(pp);
			}
		}

		base = segmap_getmap(segkmap, vp, off);

		if (pagecreate)
			segmap_pagecreate(segkmap, base+on, (u_int)n, 0);

		error = uiomove(base+on, n, UIO_WRITE, uiop);

		/* Now release any pages held by as_iolock. */
		for (ppp = iolpl; *ppp; ppp++ )
			PAGE_RELE(*ppp);

		PREEMPT();

		if (pagecreate 
		  && uiop->uio_offset < roundup(off + on + n, PAGESIZE)) {
			/*
			 * We created pages without initializing them
			 * completely, thus we need to zero the part
			 * that wasn't set up.  This happens on most
			 * EOF write cases and if we had some sort of
			 * error during the uiomove.
			 */
			int nzero, nmoved;

			nmoved = uiop->uio_offset - (off + on);
			ASSERT(nmoved >= 0 && nmoved <= n);
			nzero = roundup(n, PAGESIZE) - nmoved;
			ASSERT(nzero > 0 && on + nmoved + nzero <= MAXBSIZE);
			(void) kzero(base + on + nmoved, (u_int)nzero);
		}
		if (error) {
			/*
			 * We may have already allocated file blocks as well
			 * as pages.  It's hard to undo the block allocation,
			 * but we must be sure to invalidate any pages that
			 * may have been allocated.
			 */
			(void) segmap_release(segkmap, base, SM_INVAL);
			ILOCK(ip);
			ip->i_size = osize;
			IUNLOCK(ip);
		} else {
			flags = 0;
			if (ioflag & IO_SYNC) {
				if (IS_SWAPVP(vp))
					flags = SM_WRITE|SM_FREE|SM_DONTNEED;
				else {
					ip->i_flag |= ISYN;
					flags = SM_WRITE;
				}
			} else if (n + on == MAXBSIZE || IS_SWAPVP(vp))
				/*
				 * Have written a whole chunk.  Start an
				 * asynchronous write and mark the buffer
				 * to indicate that it won't be needed
				 * again soon.
				 */
				flags = SM_WRITE|SM_ASYNC|SM_DONTNEED;
			error = segmap_release(segkmap, base, flags);
		}
		if (error == 0)
			ip->i_flag |= IUPD|ICHG;
	}

	ip->i_flag &= ~(ISYNC | INOACC);	
	return error;

err:
	/*
	 * If we've already done a partial write, terminate
	 * the write but return no error.
	 */
	if (oresid != uiop->uio_resid)
		error = 0;
	ip->i_flag &= ~(ISYNC|INOACC);	
	return error;

}
