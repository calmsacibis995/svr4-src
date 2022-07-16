/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:fbio.c	1.3.1.1"

#include "sys/types.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/fbuf.h"
#include "sys/kmem.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/vfs.h"
#include "sys/cred.h"
#include "sys/vnode.h"


#include "vm/seg.h"
#include "vm/seg_kmem.h"
#include "vm/seg_map.h"

/*
 * Pseudo-bio routines which use a segmap mapping to address file data.
 */

/*
 * Variables for maintaining the free list of fbuf structures.
 */
static struct fbuf *fb_free;
static int nfb_incr = 0x10;

/*
 * Return a pointer to locked kernel virtual address for
 * the given <vp, off> for len bytes.  It is not allowed to
 * have the offset cross a MAXBSIZE boundary over len bytes.
 */
int
fbread(vp, off, len, rw, fbpp)
	struct vnode *vp;
	register off_t off;
	uint len;
	enum seg_rw rw;
	struct fbuf **fbpp;
{
	register addr_t addr;
	register u_int o;
	register struct fbuf *fbp;
	faultcode_t err;

	o = off & MAXBOFFSET;
	if (o + len > MAXBSIZE)
		cmn_err(CE_PANIC, "fbread");
	addr = segmap_getmap(segkmap, vp, off & MAXBMASK);
	err = as_fault(&kas, addr + o, len, F_SOFTLOCK, rw);
	if (err) {
		(void) segmap_release(segkmap, addr, 0);
		if (FC_CODE(err) == FC_OBJERR)
			return FC_ERRNO(err);
		else
			return EIO;
	}
	fbp = (struct fbuf *)kmem_fast_alloc((caddr_t *)&fb_free,
	  sizeof (*fb_free), nfb_incr, KM_SLEEP);
	fbp->fb_addr = addr + o;
	fbp->fb_count = len;
	*fbpp = fbp;
	return 0;
}

/*
 * Similar to fbread() but we call segmap_pagecreate instead of using
 * as_fault for SOFTLOCK to create the pages without using VOP_GETPAGE
 * and then we zero up to the length rounded to a page boundary.
 * XXX - this won't work right when bsize < PAGESIZE!!!
 */
void
fbzero(vp, off, len, fbpp)
	struct vnode *vp;
	off_t off;
	uint len;
	struct fbuf **fbpp;
{
	addr_t addr;
	register uint o, zlen;

	o = off & MAXBOFFSET;
	ASSERT(o + len <= MAXBSIZE);
	addr = segmap_getmap(segkmap, vp, off & MAXBMASK) + o;

	*fbpp = (struct fbuf *)kmem_fast_alloc((caddr_t *)&fb_free,
	  sizeof (*fb_free), nfb_incr, KM_SLEEP);
	(*fbpp)->fb_addr = addr;
	(*fbpp)->fb_count = len;

	segmap_pagecreate(segkmap, addr, len, 1);

	/*
	 * Now we zero all the memory in the mapping we are interested in.
	 */
	zlen = (addr_t)ptob(btopr(len + addr)) - addr;
	ASSERT(zlen >= len && o + zlen <= MAXBSIZE);
	bzero(addr, zlen);
}

/*
 * Release the fbp using the rw mode specified.
 */
void
fbrelse(fbp, rw)
	register struct fbuf *fbp;
	enum seg_rw rw;
{
	addr_t addr;

	(void) as_fault(&kas, fbp->fb_addr, fbp->fb_count, F_SOFTUNLOCK, rw);
	addr = (addr_t)((uint)fbp->fb_addr & MAXBMASK);
	(void) segmap_release(segkmap, addr, 0);
	kmem_fast_free((caddr_t *)&fb_free, (caddr_t)fbp);
}

/*
 * KLUDGE - variant of fbrelse() that invalidates the pages upon releaseing.
 */
void
fbrelsei(fbp, rw)
	register struct fbuf *fbp;
	enum seg_rw rw;
{
	addr_t addr;

	(void) as_fault(&kas, fbp->fb_addr, fbp->fb_count, F_SOFTUNLOCK, rw);
	addr = (addr_t)((uint)fbp->fb_addr & MAXBMASK);
	(void) segmap_release(segkmap, addr, SM_INVAL);
	kmem_fast_free((caddr_t *)&fb_free, (caddr_t)fbp);
}

/*
 * Perform a direct write using segmap_release and the mapping
 * information contained in the inode.  Upon return the fbp is
 * invalid.
 */
int
fbwrite(fbp)
	register struct fbuf *fbp;
{
	int err;
	addr_t addr;

	(void) as_fault(&kas, fbp->fb_addr, fbp->fb_count,
	  F_SOFTUNLOCK, S_WRITE);
	addr = (addr_t)((uint)fbp->fb_addr & MAXBMASK);
	err = segmap_release(segkmap, addr, SM_WRITE);
	kmem_fast_free((caddr_t *)&fb_free, (caddr_t)fbp);
	return err;
}

/*
 * Perform a async direct write using segmap_release and the mapping
 * information contained in the inode.  Upon return the fbp is
 * invalid.
 */
int
fbawrite(fbp)
	register struct fbuf *fbp;
{
	int err;
	addr_t addr;

	(void) as_fault(&kas, fbp->fb_addr, fbp->fb_count,
	  F_SOFTUNLOCK, S_WRITE);
	addr = (addr_t)((uint)fbp->fb_addr & MAXBMASK);
	err = segmap_release(segkmap, addr, SM_WRITE | SM_ASYNC);
	kmem_fast_free((caddr_t *)&fb_free, (caddr_t)fbp);
	return err;
}

/*
 * KLUDGE - variant of fbwrite() that invalidates the pages upon releasing
 */
int
fbwritei(fbp)
	register struct fbuf *fbp;
{
	int err;
	addr_t addr;

	(void) as_fault(&kas, fbp->fb_addr, fbp->fb_count,
	  F_SOFTUNLOCK, S_WRITE);
	addr = (addr_t)((uint)fbp->fb_addr & MAXBMASK);
	err = segmap_release(segkmap, addr, SM_WRITE | SM_INVAL);
	kmem_fast_free((caddr_t *)&fb_free, (caddr_t)fbp);
	return err;
}

/*
 * Perform a synchronous indirect write of the given block number
 * on the given device, using the given fbuf.  Upon return the fbp
 * is invalid.
 */
int
fbiwrite(fbp, devvp, bn, bsize)
	register struct fbuf *fbp;
	register struct vnode *devvp;
	daddr_t bn;
	int bsize;
{
	register struct buf *bp;
	int error;
	addr_t addr;

	/*
	 * Allocate a temp bp using pageio_setup, but then use it
	 * for physio to the area mapped by fbuf which is currently
	 * all locked down in place.
	 *
	 * XXX - need to have a generalized bp header facility
	 * which we build up pageio_setup on top of.  Other places
	 * (like here and in device drivers for the raw I/O case)
	 * could then use these new facilities in a more straight
	 * forward fashion instead of playing all these games.
	 */
	bp = pageio_setup((struct page *)NULL, fbp->fb_count, devvp, B_WRITE);
	bp->b_flags &= ~B_PAGEIO;		/* XXX */
	bp->b_flags |= B_PHYS;
#if 0
	bp->b_un.b_addr = (caddr_t)svirtophys(fbp->fb_addr);	/* XXX */
#else
	bp->b_un.b_addr = fbp->fb_addr;
#endif

	bp->b_blkno = bn * btod(bsize);
	bp->b_dev = cmpdev(devvp->v_rdev);	/* store in old dev format */
	bp->b_edev = devvp->v_rdev;
	bp->b_proc = NULL;			/* i.e. the kernel */

	(*bdevsw[getmajor(devvp->v_rdev)].d_strategy)(bp);

	error = biowait(bp);
	pageio_done(bp);

	(void) as_fault(&kas, fbp->fb_addr, fbp->fb_count,
	  F_SOFTUNLOCK, S_OTHER);
	addr = (addr_t)((uint)fbp->fb_addr & MAXBMASK);
	if (error == 0)
		error = segmap_release(segkmap, addr, 0);
	else
		(void) segmap_release(segkmap, addr, 0);
	kmem_fast_free((caddr_t *)&fb_free, (caddr_t)fbp);

	return error;
}
