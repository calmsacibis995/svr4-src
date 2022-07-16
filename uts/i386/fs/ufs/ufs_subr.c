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

#ident	"@(#)kern-fs:ufs/ufs_subr.c	1.3.1.1"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/fs/ufs_fs.h>
#include <sys/cmn_err.h>
#ifdef _KERNEL
#include <sys/systm.h>
#include <sys/sysmacros.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/debug.h>
#include <sys/fs/ufs_inode.h>
#include <sys/kmem.h>
#include <vm/hat.h>
#include <vm/as.h>
#include <vm/seg.h>
#include <vm/page.h>
#include <vm/pvn.h>
#include <vm/seg_map.h>
#include <sys/swap.h>
#include <vm/seg_kmem.h>

#ifdef SUNOS_FBUF
int syncprt = 0;
#endif
int updlock;
void ufs_flushi();

/*
 * ufs_update performs the ufs part of `sync'.  It goes through the disk
 * queues to initiate sandbagged IO; goes through the inodes to write
 * modified nodes; and it goes through the mount table to initiate
 * the writing of the modified super blocks.
 */
void
ufs_update()
{
	register struct vfs *vfsp;
	extern struct vfsops ufs_vfsops;
	struct fs *fs;

	while (updlock) {
		(void) sleep((caddr_t)&updlock, PINOD);
	}
	updlock++;
	
#ifdef SUNOS_FBUF
	if (syncprt)
		bufstats();
#endif
	/*
	 * Write back modified superblocks.
	 * Consistency check that the superblock of
	 * each file system is still in the buffer cache.
	 */
	for (vfsp = rootvfs; vfsp != NULL; vfsp = vfsp->vfs_next)
		if (vfsp->vfs_op == &ufs_vfsops) {
			fs = getfs(vfsp);
			if (fs->fs_fmod == 0)
				continue;
			if (fs->fs_ronly != 0) {
				cmn_err(CE_PANIC, "fs = %s update: ro fs mod\n",
					fs->fs_fsmnt);
			}
			fs->fs_fmod = 0;
			fs->fs_time = hrestime.tv_sec;
			sbupdate(vfsp);
		}

	ufs_flushi(0);

	/*
	 * Force stale buffer cache information to be flushed,
	 * for all devices.  This should cause any remaining control
	 * information (e.g., cg and inode info) to be flushed back.
	 */
	bflush((dev_t)NODEV);
	updlock = 0;
	wakeup((caddr_t)&updlock);
	return;
}

void
ufs_flushi(flag)
	short flag;
{
	register struct inode *ip;
	register struct vnode *vp;
	register int cheap = flag & SYNC_ATTR;

	/*
	 * Write back each (modified) inode,
	 * but don't sync back pages if vnode is
	 * part of the virtual swap device.
	 */
	for (ip = ufs_inode; ip < inodeNINODE; ip++) {
		vp = ITOV(ip);
		/*
		 * Skip locked & inactive inodes.
		 * Skip inodes w/ no pages and no inode changes.
		 * Skip inodes from read only vfs's.
		 * Skip inodes whose vnode count is 0.
		 */
		if ((ip->i_flag & (IRWLOCKED | ILOCKED)) ||
		    (ip->i_flag & IREF) == 0 ||
		    ((vp->v_pages == NULL) &&
		    ((ip->i_flag & (IMOD | IACC | IUPD | ICHG)) == 0)) ||
		    (vp->v_vfsp == NULL) ||
		    (vp->v_count == 0) ||
		    ((vp->v_vfsp->vfs_flag & VFS_RDONLY) != 0))
			continue;
		ufs_ilock(ip);
		VN_HOLD(vp);

		/*
		 * If this is an inode sync for file system hardening
		 * or this is a full sync but file is a swap file,
		 * don't sync pages but make sure the inode is up
		 * to date. In other cases, push everything out.
		 */
		if (cheap || IS_SWAPVP(vp)) {
			IUPDAT(ip, 0);
		} else {
			(void) ufs_syncip(ip, B_ASYNC);
		}
		ufs_iput(ip);
	}
	return;
}

/*
 * Flush all the pages associated with an inode using the given flags,
 * then force inode information to be written back using the given flags.
 */
int
ufs_syncip(ip, flags)
	register struct inode *ip;
	int flags;
{
	int error;
	register struct vnode *vp = ITOV(ip);

	if (ip->i_fs == NULL)
		return (0);			/* not active */
	if (vp->v_pages == NULL || vp->v_type == VCHR)
		error = 0;
	else
		error = VOP_PUTPAGE(vp, 0, 0, flags, (struct cred *)0);
	if (ip->i_flag & (IUPD |IACC | ICHG | IMOD)) {
		if ((flags & B_ASYNC) != 0) {
			IUPDAT(ip, 0);
		} else {
			IUPDAT(ip, 1);
		}
	}
	return (error);
}

/*
 * Check that a specified block number is in range.
 */
ufs_badblock(fs, bn)
	register struct fs *fs;
	daddr_t bn;
{

	if ((unsigned)bn >= fs->fs_size) {
		cmn_err(CE_WARN, "bad block %d, %s: bad block\n", bn, fs->fs_fsmnt);
		return (1);
	}
	return (0);
}

#ifdef SUNOS_FBUF

/*
 * Print out statistics on the current allocation of the buffer pool.
 * Can be enabled to print out on every ``sync'' by setting "syncprt".
 */
bufstats()
{
	int s, i, j, count;
	register struct buf *bp, *dp;
	int counts[MAXBSIZE/CLBYTES+1];
	static char *bname[BQUEUES] = { "LRU", "AGE", "EMPTY" };

	for (bp = bfreelist, i = 0; bp < &bfreelist[BQUEUES]; bp++, i++) {
		count = 0;
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			counts[j] = 0;
		s = spl6();
		for (dp = bp->av_forw; dp != bp; dp = dp->av_forw) {
			counts[dp->b_bufsize/CLBYTES]++;
			count++;
		}
		(void) splx(s);
		cmn_err(CE_NOTE, "%s: total-%d", bname[i], count);
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			if (counts[j] != 0)
				cmn_err(CE_NOTE, ", %d-%d", j * CLBYTES, counts[j]);
		printf("\n");
	}
}

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
	register u_int off;
	u_int len;
	enum seg_rw rw;
	struct fbuf **fbpp;
{
	register addr_t addr;
	register u_int o;
	register struct fbuf *fb;
	faultcode_t err;

	o = off & MAXBOFFSET;
	ASSERT(o + len <= MAXBSIZE);
	addr = segmap_getmap(segkmap, vp, off & MAXBMASK);

	err = as_fault(&kas, addr + o, len, F_SOFTLOCK, rw);
	if (err) {
		(void) segmap_release(segkmap, addr, 0);
		if (FC_CODE(err) == FC_OBJERR)
			return (FC_ERRNO(err));
		else
			return (EIO);
	}
	fb = (struct fbuf *)kmem_fast_alloc((caddr_t *)&fb_free,
	    sizeof (*fb_free), nfb_incr, KM_SLEEP);
	fb->fb_addr = addr + o;
	fb->fb_count = len;
	*fbpp = fb;
	return (0);
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
	u_int off;
	u_int len;
	struct fbuf **fbpp;
{
	addr_t addr;
	register u_int o, zlen;

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
 * Release the fb using the rw mode specified
 */
void
fbrelse(fb, rw)
	register struct fbuf *fb;
	enum seg_rw rw;
{
	addr_t addr;

	(void) as_fault(&kas, fb->fb_addr, fb->fb_count, F_SOFTUNLOCK, rw);
	addr = (addr_t)((u_int)fb->fb_addr & MAXBMASK);
	(void) segmap_release(segkmap, addr, 0);
	kmem_fast_free((caddr_t *)&fb_free, (caddr_t)fb);
}

/*
 * Perform a direct write using the segmap_release and the mapping
 * information contained in the inode.  Upon return the fb is invalid.
 */
int
fbwrite(fb)
	register struct fbuf *fb;
{
	int err;
	addr_t addr;

	(void) as_fault(&kas, fb->fb_addr, fb->fb_count, F_SOFTUNLOCK, S_WRITE);
	addr = (addr_t)((u_int)fb->fb_addr & MAXBMASK);
	err = segmap_release(segkmap, addr, SM_WRITE);
	kmem_fast_free((caddr_t *)&fb_free, (caddr_t)fb);
	return (err);
}

/*
 * Perform an indirect write using the given fbuf to the given
 * file system block number.  Upon return the fb is invalid.
 */
int
fbiwrite(fb, ip, bn)
	register struct fbuf *fb;
	register struct inode *ip;
	daddr_t bn;
{
	register struct buf *bp;
	int err;
	addr_t addr;

	/*
	 * Allocate a temp bp using pageio_setup, but then use it
	 * for physio to the area mapped by fbuf which is currently
	 * all locked down in place.
	 *
	 * XXX - need to have a generalized bp header facility
	 * which we build up pageio_setup on top of.  Other places
	 * (like here and in device drivers for the raw io case)
	 * could then use these new facilities in a more straight
	 * forward fashion instead of playing all theses games.
	 */
	bp = pageio_setup((struct page *)NULL, fb->fb_count, ip->i_devvp,
	    B_WRITE);
	bp->b_flags &= ~B_PAGEIO;		/* XXX */
	bp->b_flags |= B_PHYS;
	bp->b_un.b_addr = fb->fb_addr;
	bp->b_blkno = fsbtodb(ip->i_fs, bn);
	bp->b_edev = ip->i_dev;
	bp->b_dev = cmpdev(ip->i_dev);
	bp->b_proc = NULL;			/* i.e. the kernel */

	(*bdevsw[getmajor(ip->i_dev)].d_strategy)(bp);

	err = biowait(bp);
	pageio_done(bp);

	(void) as_fault(&kas, fb->fb_addr, fb->fb_count, F_SOFTUNLOCK, S_OTHER);
	addr = (addr_t)((u_int)fb->fb_addr & MAXBMASK);
	if (err == 0)
		err = segmap_release(segkmap, addr, 0);
	else
		(void) segmap_release(segkmap, addr, 0);
	kmem_fast_free((caddr_t *)&fb_free, (caddr_t)fb);

	return (err);
}
#endif /* SUNOS_FBUF */
#endif /* _KERNEL */

extern	int around[9];
extern	int inside[9];
extern	u_char *fragtbl[];

/*
 * Update the frsum fields to reflect addition or deletion
 * of some frags.
 */
void
fragacct(fs, fragmap, fraglist, cnt)
	struct fs *fs;
	int fragmap;
	long fraglist[];
	int cnt;
{
	int inblk;
	register int field, subfield;
	register int siz, pos;

	inblk = (int)(fragtbl[fs->fs_frag][fragmap]) << 1;
	fragmap <<= 1;
	for (siz = 1; siz < fs->fs_frag; siz++) {
		if ((inblk & (1 << (siz + (fs->fs_frag % NBBY)))) == 0)
			continue;
		field = around[siz];
		subfield = inside[siz];
		for (pos = siz; pos <= fs->fs_frag; pos++) {
			if ((fragmap & field) == subfield) {
				fraglist[siz] += cnt;
				pos += siz;
				field <<= siz;
				subfield <<= siz;
			}
			field <<= 1;
			subfield <<= 1;
		}
	}
	return;
}

/*
 * Block operations
 */

/*
 * Check if a block is available
 */

isblock(fs, cp, h)
	struct fs *fs;
	unsigned char *cp;
	daddr_t h;
{
	unsigned char mask;

	switch ((int)fs->fs_frag) {
	case 8:
		return (cp[h] == 0xff);
	case 4:
		mask = 0x0f << ((h & 0x1) << 2);
		return ((cp[h >> 1] & mask) == mask);
	case 2:
		mask = 0x03 << ((h & 0x3) << 1);
		return ((cp[h >> 2] & mask) == mask);
	case 1:
		mask = 0x01 << (h & 0x7);
		return ((cp[h >> 3] & mask) == mask);
	default:
		cmn_err(CE_PANIC, "isblock");
		return (NULL);
	}
}

/*
 * Take a block out of the map
 */
void
clrblock(fs, cp, h)
	struct fs *fs;
	u_char *cp;
	daddr_t h;
{

	switch ((int)fs->fs_frag) {
	case 8:
		cp[h] = 0;
		return;
	case 4:
		cp[h >> 1] &= ~(0x0f << ((h & 0x1) << 2));
		return;
	case 2:
		cp[h >> 2] &= ~(0x03 << ((h & 0x3) << 1));
		return;
	case 1:
		cp[h >> 3] &= ~(0x01 << (h & 0x7));
		return;
	default:
		cmn_err(CE_PANIC, "clrblock");
	}
	return;
}

/*
 * Put a block into the map
 */
void
setblock(fs, cp, h)
	struct fs *fs;
	unsigned char *cp;
	daddr_t h;
{

	switch ((int)fs->fs_frag) {

	case 8:
		cp[h] = 0xff;
		return;
	case 4:
		cp[h >> 1] |= (0x0f << ((h & 0x1) << 2));
		return;
	case 2:
		cp[h >> 2] |= (0x03 << ((h & 0x3) << 1));
		return;
	case 1:
		cp[h >> 3] |= (0x01 << (h & 0x7));
		return;
	default:
		cmn_err(CE_PANIC, "setblock");
	}
	return;
}

#if !(defined(vax) || defined(sun)) || defined(VAX630)
/*
 * C definitions of special vax instructions.
 */

scanc(size, cp, table, mask)
	u_int size;
	register u_char *cp, table[];
	register u_char mask;
{
	register u_char *end = &cp[size];

	while (cp < end && (table[*cp] & mask) == 0)
		cp++;
	return (end - cp);
}
#endif /* !(defined(vax) || defined(sun)) || defined(VAX630) */

#if !defined(vax)

skpc(c, len, cp)
	register char c;
	register u_int len;
	register char *cp;
{

	if (len == 0)
		return (0);
	while (*cp++ == c && --len)
		;
	return (len);
}

#ifdef notdef
locc(c, len, cp)
	register char c;
	register u_int len;
	register char *cp;
{

	if (len == 0)
		return (0);
	while (*cp++ != c && --len)
		;
	return (len);
}
#endif /* notdef */

#endif /* !defined(vax) */

