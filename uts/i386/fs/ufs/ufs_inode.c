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

#ident  "@(#)kern-fs:ufs/ufs_inode.c	1.3.2.6"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/cred.h>
#include <sys/user.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/disp.h>
#include <sys/mode.h>
#include <sys/cmn_err.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>
#ifdef QUOTA
#include <sys/fs/ufs_quota.h>
#endif
#include <vm/hat.h>
#include <vm/as.h>
#include <vm/pvn.h>
#include <vm/seg.h>
#include <sys/swap.h>
#include <sys/sysinfo.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/kmem.h>
#include <sys/debug.h>
#include <fs/fs_subr.h>

#define	INOHSZ	512
#if	((INOHSZ&(INOHSZ-1)) == 0)
#define	INOHASH(dev, ino)	(((dev)+(ino))&(INOHSZ-1))
#else
#define	INOHASH(dev, ino)	(((unsigned)((dev)+(ino)))%INOHSZ)
#endif

union ihead {				/* inode LRU cache, Chris Maltby */
	union  ihead *ih_head[2];
	struct inode *ih_chain[2];
} ihead[INOHSZ];

struct inode *ifreeh, **ifreet;
void ihinit(), idrop(), ufs_iinactive(), ufs_iupdat(), ufs_ilock();
void ufs_iunlock(), _remque(), _insque();

#ifdef notneeded
LOOK AT os/vnode.c for this stuff
/*
 * Convert inode formats to vnode types
 */
enum vtype iftovt_tab[] = {
	VFIFO, VCHR, VDIR, VBLK, VREG, VLNK, VSOCK, VBAD
};

int vttoif_tab[] = {
	0, IFREG, IFDIR, IFBLK, IFCHR, IFLNK, IFSOCK, IFMT, IFIFO
};
#endif

/*
 * Initialize the vfs structure
 */

int ufsfstype;
extern struct vnodeops ufs_vnodeops;
extern struct vfsops ufs_vfsops;
extern struct seg *segkmap;

void
ufsinit(vswp, fstype)
	struct vfssw *vswp;
	int fstype;
{
	ihinit();
	
	/*
	 * NOTE: Sun's auto-config initializes this in
	 * 	vfs_conf.c
	 * Associate vfs and vnode operations
	 */

	vswp->vsw_vfsops = &ufs_vfsops;
/*
	vswp->vsw_vnodeops = &ufs_vnodeops;
*/
	ufsfstype = fstype;
#ifdef QUOTA
	/*
	 * I think this goes here.
	 */
	quoinit();
#endif
	return;
}

/*
 * Initialize hash links for inodes
 * and build inode free list.
 */
void
ihinit()
{
	register int i;
	register struct inode *ip;
	register union  ihead *ih = ihead;

	if ((ufs_inode = (struct inode *)kmem_zalloc(ufs_ninode*sizeof(struct inode), KM_SLEEP)) == NULL)
		cmn_err(CE_PANIC, "ihinit: no memory for ufs inodes");
	inodeNINODE = ufs_inode + ufs_ninode;
	ip = ufs_inode;
	for (i = INOHSZ; --i >= 0; ih++) {
		ih->ih_head[0] = ih;
		ih->ih_head[1] = ih;
	}
	ifreeh = ip;
	ifreet = &ip->i_freef;
	ip->i_freeb = &ifreeh;
	ip->i_forw = ip;
	ip->i_back = ip;
	ip->i_vnode.v_data = (caddr_t)ip;
	ip->i_vnode.v_op = &ufs_vnodeops;
	for (i = ufs_ninode; --i > 0; ) {
		++ip;
		ip->i_forw = ip;
		ip->i_back = ip;
		*ifreet = ip;
		ip->i_freeb = ifreet;
		ifreet = &ip->i_freef;
		ip->i_vnode.v_data = (caddr_t)ip;
		ip->i_vnode.v_op = &ufs_vnodeops;
	}
	ip->i_freef = NULL;
	return;
}

/*
 * Look up an inode by device, inumber.  If it is in core (in the
 * inode structure), honor the locking protocol.  If it is not in
 * core, read it in from the specified device after freeing any pages.
 * In all cases, a pointer to a locked inode structure is returned.
 */
int
ufs_iget(vfsp, fs, ino, ipp)
	register struct vfs *vfsp;
	register struct fs *fs;
	ino_t ino;
	struct inode **ipp;
{
	register struct inode *ip;
	register union  ihead *ih;
	register struct buf *bp;
	register struct dinode *dp;
	register struct inode *iq;
	register struct vnode *vp;
	int error;

	sysinfo.iget++;
	/*
	 * Lookup inode in cache.
	 */
loop:
	ASSERT(getfs(vfsp) == fs);
	ih = &ihead[INOHASH(vfsp->vfs_dev, ino)];
	for (ip = ih->ih_chain[0]; ip != (struct inode *)ih; ip = ip->i_forw) {
		if (ino == ip->i_number && vfsp->vfs_dev == ip->i_dev) {
			/*
			 * Found it - check for locks.
			 */
			if ((ip->i_flag & (IRWLOCKED | ILOCKED)) &&
			    ip->i_owner != curproc->p_slot) {
				ip->i_flag |= IWANT;
				(void) sleep((caddr_t)ip, PINOD);
				goto loop;
			}
			/*
			 * If inode is on free list, remove it.
			 */
			if ((ip->i_flag & IREF) == 0) {
				iq = ip->i_freef;
				if (iq)
					iq->i_freeb = ip->i_freeb;
				else
					ifreet = ip->i_freeb;
				*ip->i_freeb = iq;
				ip->i_freef = NULL;
				ip->i_freeb = NULL;
			}
			/*
			 * Lock the inode and mark it referenced and return it.
			 */
			ip->i_flag |= IREF;
			ufs_ilock(ip);
			VN_HOLD(ITOV(ip));
			*ipp = ip;
			return (0);
		}
	}

	/*
	 * XXX -- If inode freelist is empty, toss out name cache entries
	 * in an attempt to reclaim some inodes.  Give up only when
	 * there are no more name cache entries to purge.
	 */
	while (ifreeh == NULL) {
	/*
	 * dnlc_purge1 might go to sleep. So we have to check the hash chain again
	 * to handle possible race condition.
	 */
		if (dnlc_purge1() == 0) {	/* XXX */
			break;
		}
	}
		;
	if ((ip = ifreeh) == NULL) {
		cmn_err(CE_WARN, "ufs_iget - inode table overflow");
		syserr.inodeovf++;
		return (ENFILE);
	}
	if (ip->i_mode != 0 && ITOV(ip)->v_pages != NULL)
		sysinfo.ufsipage++;
	else
		sysinfo.ufsinopage++;
	iq = ip->i_freef;
	if (iq)
		iq->i_freeb = &ifreeh;
	ifreeh = iq;
	ip->i_freef = NULL;
	ip->i_freeb = NULL;

/*
 * The following code checks to be sure that putpages from the page layer
 * have not activated the vnode while the inode is on the free list. If
 * we hit this case we put the inode back on the tail of the free list
 * and try again. If there are not other inodes on the free list then
 * we put the inode back and must call preempt so that some other process
 * can do work to free an inode.
 */

	if (ITOV(ip)->v_count > 0) {
		if (ifreeh == NULL) {
			/*
			 *	Put inode back on head of freelist... only 1 inode
			 *	left!
			 */
			ifreet = &ip->i_freef;
			ip->i_freeb = &ifreeh;
			ip->i_freef = ifreeh;
			ifreeh = ip;
			preempt();
		}
		else {
			/*
			 *	Put inode on end of freelist.
			 */
			*ifreet = ip;
			ip->i_freeb = ifreet;
			ip->i_freef = NULL;
			ifreet = &ip->i_freef;
		}
		goto loop;
	}

	ip->i_flag = IREF;
	ILOCK(ip);
	if (ufs_syncip(ip, B_INVAL) != 0 || (ip->i_flag & IWANT) != 0) {
		VN_HOLD(ITOV(ip));
		idrop(ip);
		goto loop;
	}
	for (iq = ih->ih_chain[0]; iq != (struct inode *)ih; iq = iq->i_forw) {
		if (ino == iq->i_number && vfsp->vfs_dev == iq->i_dev) {
			VN_HOLD(ITOV(ip));
			idrop(ip);
			goto loop;
		}
	}

	vp = ITOV(ip);
	ASSERT(vp->v_pages == NULL);
	ASSERT(vp->v_count == 0);

	/*
	 * Move the inode on the chain for its new (ino, dev) pair
	 */
	_remque(ip);
	_insque(ip, ih);

	if (ip->i_map) {
		ufs_freemap(ip);
	}

	ip->i_dev = vfsp->vfs_dev;
	ip->i_devvp = ((struct ufsvfs *)vfsp->vfs_data)->vfs_devvp;
	ip->i_number = ino;
	ip->i_diroff = 0;
	ip->i_fs = fs;
	ip->i_nextr = 0;
	ip->i_vcode = 0;
#ifdef QUOTA
	dqrele(ip->i_dquot);
	ip->i_dquot = NULL;
#endif
	bp = bread(ip->i_dev, (daddr_t)fragstoblks(fs, itod(fs, ino)),
	    (int)fs->fs_bsize);
	/*
	 * Check I/O errors and get vcode
	 */
	if (((error = (bp->b_flags & B_ERROR) != 0) ? EIO : 0) ||
	  IFTOVT(ip->i_mode) == VREG && ((error = fs_vcode(vp, &ip->i_vcode)) != 0)) {

		brelse(bp);
		/*
		 * The inode doesn't contain anything useful, so it
		 * would be misleading to leave it on its hash chain.
		 * `ufs_iput' will take care of putting it back on the
		 * free list.
		 */
		_remque(ip);
		ip->i_forw = ip;
		ip->i_back = ip;
		/*
		 * We also lose its inumber, just in case (as ufs_iput
		 * doesn't do that any more) - but as it isn't on its
		 * hash chain, I doubt if this is really necessary.
		 * (probably the two methods are interchangable)
		 */
		ip->i_number = 0;
		ufs_iunlock(ip);
		ufs_iinactive(ip);
		return (error);
	}
	dp = (struct dinode *)bp->b_un.b_addr;
	dp += itoo(fs, ino);
	ip->i_ic = dp->di_ic;			/* structure assignment */
	if (ip->i_eftflag != EFT_MAGIC) {
		register int ftype;

		ip->i_mode = ip->i_smode;
		ip->i_uid = ip->i_suid;
		ip->i_gid = ip->i_sgid;
		ftype = ip->i_mode & IFMT;
		if (ftype == IFBLK || ftype == IFCHR)
		ip->i_rdev = expdev(ip->i_oldrdev);
		ip->i_eftflag = (u_long)EFT_MAGIC;
	}
	/*
	 * Fill in the rest.
	 */
	vp->v_flag = 0;
	vp->v_count = 1;
	vp->v_vfsp = vfsp;
	vp->v_stream = NULL;
	vp->v_pages = NULL;
	vp->v_filocks = NULL;
	vp->v_type = IFTOVT(ip->i_mode);
	vp->v_rdev = ip->i_rdev;
	if (ino == (ino_t)UFSROOTINO) {
		vp->v_flag |= VROOT;
	}
	brelse(bp);
#ifdef QUOTA
	if (ip->i_mode != 0)
		ip->i_dquot = getinoquota(ip);
#endif
	*ipp = ip;
	return (0);
}

/*
 * Unlock inode and vrele associated vnode
 */
void
ufs_iput(ip)
	register struct inode *ip;
{

	ASSERT(ip->i_flag & ILOCKED);
	ufs_iunlock(ip);
	ITIMES(ip);
	VN_RELE(ITOV(ip));
	return;
}

/*
 * Check that inode is not locked and release associated vnode.
 */
void
irele(ip)
	register struct inode *ip;
{

	ASSERT(!(ip->i_flag & ILOCKED));
	ITIMES(ip);
	VN_RELE(ITOV(ip));
	return;
}

/*
 * Drop inode without going through the normal
 * chain of unlocking and releasing.
 */
void
idrop(ip)
	register struct inode *ip;
{
	register struct vnode *vp = &ip->i_vnode;

	ASSERT(ip->i_flag & ILOCKED);
	ufs_iunlock(ip);
	if (--vp->v_count == 0) {
		ip->i_flag = 0;
	/*
	 *  if inode is invalid or there is no page associated with
	 *  this inode, put the inode in the front of the free list
	 */
		if (ITOV(ip)->v_pages == NULL || ip->i_mode == 0) {
			if (ifreeh)
				ifreeh->i_freeb = &ip->i_freef;
			else
				ifreet = &ip->i_freef;
			ip->i_freeb = &ifreeh;
			ip->i_freef = ifreeh;
			ifreeh = ip;
			return;
		}		

		/*
		 * Otherwise, put the inode back on the end of the free list.
		 */
		if (ifreeh) {
			*ifreet = ip;
			ip->i_freeb = ifreet;
		} else {
			ifreeh = ip;
			ip->i_freeb = &ifreeh;
		}
		ip->i_freef = NULL;
		ifreet = &ip->i_freef;
	}
	return;
}

/*
 * Vnode is no longer referenced, write the inode out
 * and if necessary, truncate and deallocate the file.
 */
void
ufs_iinactive(ip)
	register struct inode *ip;
{
	mode_t mode;

	/* if inode has already been freed, just return */
	if (ip->i_freeb || ip->i_freef) {
		return;
	}
	/*
	 * Mark iinactive in progress.	This allow VOP_PUTPAGE to abort
	 * a concurrent attempt to flush a page due to pageout/fsflush.
	*/
	ASSERT((ip->i_flag & IINACTIVE) == 0);
	ip->i_flag |= IINACTIVE;

	if ((ip->i_flag & (IREF|ILOCKED)) != IREF)
		cmn_err(CE_PANIC, "ufs_iinactive");
	if (ip->i_fs->fs_ronly == 0) {
		ufs_ilock(ip);
		if (ip->i_nlink <= 0) {
			ip->i_gen++;
			(void) ufs_itrunc(ip, (u_long)0);
			mode = ip->i_mode;
			ip->i_mode = 0;
			ip->i_rdev = 0;
			ip->i_oldrdev = 0;
			ip->i_flag |= IUPD|ICHG;
			ip->i_eftflag = 0;
			ufs_ifree(ip, ip->i_number, mode);
#ifdef QUOTA
			(void) chkiq((struct ufsvfs *)ip->i_vnode.v_vfsp->vfs_data,
			    ip, (uid_t)ip->i_uid, 0);
			dqrele(ip->i_dquot);
			ip->i_dquot = NULL;
#endif
			IUPDAT(ip, 0)
		} else if (!IS_SWAPVP(ITOV(ip))) {
			/*
			 * Do an async write (B_ASYNC) of the pages
			 * on the vnode and put the pages on the free
			 * list when we are done (B_FREE).  This action
			 * will cause all the pages to be written back
			 * for the file now and will allow ufs_update() to
			 * skip over inodes that are on the free list.
			 */
			(void) ufs_syncip(ip, B_FREE | B_ASYNC);
		}
		ufs_iunlock(ip);
	}

        /* Clear all the flags (including IINACTIVE) */
	ip->i_flag = 0;
	/*
	 * Put the inode on the end of the free list.
	 * Possibly in some cases it would be better to
	 * put the inode at the head of the free list,
	 * (e.g.: where i_mode == 0 || i_number == 0)
	 * but I will think about that later.
	 * (i_number is rarely 0 - only after an i/o error in ufs_iget,
	 * where i_mode == 0, the inode will probably be wanted
	 * again soon for an ialloc, so possibly we should keep it)
	 */
	/*
	 *  if inode is invalid or there is no page associated with
	 *  this inode, put the inode in the front of the free list
	 */
	if (ITOV(ip)->v_pages == NULL || ip->i_mode == 0) {
		if (ifreeh)
			ifreeh->i_freeb = &ip->i_freef;
		else
			ifreet = &ip->i_freef;
		ip->i_freeb = &ifreeh;
		ip->i_freef = ifreeh;
		ifreeh = ip;
		return;
	}		
	if (ifreeh) {
		*ifreet = ip;
		ip->i_freeb = ifreet;
	} else {
		ifreeh = ip;
		ip->i_freeb = &ifreeh;
	}
	ip->i_freef = NULL;
	ifreet = &ip->i_freef;
	return;
}

/*
 * Check accessed and update flags on an inode structure.
 * If any are on, update the inode with the (unique) current time.
 * If waitfor is given, insure I/O order so wait for write to complete.
 */
void
ufs_iupdat(ip, waitfor)
	register struct inode *ip;
	int waitfor;
{
	register struct buf *bp;
	register struct fs *fp;
	struct dinode *dp;

	fp = ip->i_fs;
	if ((ip->i_flag & (IUPD|IACC|ICHG|IMOD)) != 0) {
		if (fp->fs_ronly)
			return;
		bp = bread(ip->i_dev, (daddr_t)fragstoblks(fp, itod(fp, ip->i_number)),
			    (int)fp->fs_bsize);

		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			return;
		}
		if (ip->i_flag & (IUPD|IACC|ICHG))
			IMARK(ip);
		ip->i_flag &= ~(IUPD|IACC|ICHG|IMOD);
		ip->i_smode = ip->i_mode;
		if (ip->i_uid > 0xffff)
			ip->i_suid = UID_NOBODY;
		else
			ip->i_suid = ip->i_uid;
		if (ip->i_gid > 0xffff)
			ip->i_sgid = GID_NOBODY;
		else
			ip->i_sgid = ip->i_gid;
		dp = (struct dinode *)bp->b_un.b_addr + itoo(fp, ip->i_number);
		dp->di_ic = ip->i_ic;	/* structure assignment */
		if (waitfor)
			bwrite(bp);
		else
			bdwrite(bp);
	}
	return;
}

#define	SINGLE	0	/* index of single indirect block */
#define	DOUBLE	1	/* index of double indirect block */
#define	TRIPLE	2	/* index of triple indirect block */

/*
 * Release blocks associated with the inode ip and
 * stored in the indirect block bn.  Blocks are free'd
 * in LIFO order up to (but not including) lastbn.  If
 * level is greater than SINGLE, the block is an indirect
 * block and recursive calls to indirtrunc must be used to
 * cleanse other indirect blocks.
 *
 * N.B.: triple indirect blocks are untested.
 */
STATIC long
indirtrunc(ip, bn, lastbn, level)
	register struct inode *ip;
	daddr_t bn, lastbn;
	int level;
{
	register int i;
	struct buf *bp, *copy;
	register daddr_t *bap;
	register struct fs *fs = ip->i_fs;
	daddr_t nb, last;
	long factor;
	int blocksreleased = 0, nblocks;
	struct buf *ngeteblk();

	/*
	 * Calculate index in current block of last
	 * block to be kept.  -1 indicates the entire
	 * block so we need not calculate the index.
	 */
	factor = 1;
	for (i = SINGLE; i < level; i++)
		factor *= NINDIR(fs);
	last = lastbn;
	if (lastbn > 0)
		last /= factor;
	nblocks = btodb(fs->fs_bsize);
	/*
	 * Get buffer of block pointers, zero those
	 * entries corresponding to blocks to be free'd,
	 * and update on disk copy first.
	 */
	copy = ngeteblk(fs->fs_bsize);
	bp = bread(ip->i_dev, (daddr_t)fragstoblks(fs, bn),
		(int)fs->fs_bsize);
	if (bp->b_flags & B_ERROR) {
		brelse(copy);
		brelse(bp);
		return (0);
	}
	bap = bp->b_un.b_daddr;
	bcopy((caddr_t)bap, (caddr_t)copy->b_un.b_daddr, (u_int)fs->fs_bsize);
	bzero((caddr_t)&bap[last + 1],
	  (u_int)(NINDIR(fs) - (last + 1)) * sizeof (daddr_t));
	bwrite(bp);
	bp = copy, bap = bp->b_un.b_daddr;

	/*
	 * Recursively free totally unused blocks.
	 */
	for (i = NINDIR(fs) - 1; i > last; i--) {
		nb = bap[i];
		if (nb == 0)
			continue;
		if (level > SINGLE)
			blocksreleased +=
			    indirtrunc(ip, nb, (daddr_t)-1, level - 1);
		(void)free(ip, nb, (off_t)fs->fs_bsize);
		blocksreleased += nblocks;
	}

	/*
	 * Recursively free last partial block.
	 */
	if (level > SINGLE && lastbn >= 0) {
		last = lastbn % factor;
		nb = bap[i];
		if (nb != 0)
			blocksreleased += indirtrunc(ip, nb, last, level - 1);
	}
	brelse(bp);
	return (blocksreleased);
}

/*
 * Truncate the inode ip to at most length size.
 * Free affected disk blocks -- the blocks of the
 * file are removed in reverse order.
 *
 * N.B.: triple indirect blocks are untested.
 */
ufs_itrunc(oip, length)
	register struct inode *oip;
	int length;
{
	register struct fs *fs = oip->i_fs;
	register struct inode *ip;
	register daddr_t lastblock;
	register off_t bsize;
	register int offset;
	daddr_t bn, lastiblock[NIADDR];
	int level;
	long nblocks, blocksreleased = 0;
	register int i;
	struct inode tip;
	daddr_t llbn;

	ASSERT(oip->i_flag & ILOCKED);
	/*
	 * We only allow truncation of regular files and directories
	 * to arbritary lengths here.  In addition, we allow symbolic
	 * links to be truncated only to zero length.  Other inode
	 * types cannot have their length set here disk blocks are
	 * being dealt with - especially device inodes where
	 * ip->i_rdev is actually being stored in ip->i_db[1]!
	 */
	i = oip->i_mode & IFMT;
	if (i != IFREG && i != IFDIR && i != IFLNK)
		return (0);
	else if (i == IFLNK && length != 0)
		return (EINVAL);

	if (length == oip->i_size) {
	/* update ctime and mtime to please POSIX tests */
		oip->i_flag |= ICHG |IUPD;
		return (0);
	}
	if (((oip->i_mode &IFMT) == IFREG) && oip->i_map)
		ufs_freemap(oip);
	
	offset = blkoff(fs, length);
	llbn = lblkno(fs, length - 1);

	if (length > oip->i_size) {
		int err;

		/*
		 * Trunc up case.  BMAPALLOC will insure that the right blocks
		 * are allocated.  This includes extending the old frag to a
		 * full block (if needed) in addition to doing any work
		 * needed for allocating the last block.
		 */
		if (offset == 0)
			err = BMAPALLOC(oip, llbn, (int)fs->fs_bsize);
		else
			err = BMAPALLOC(oip, llbn, offset);

		if (err == 0) {
			oip->i_size = length;
			oip->i_flag |= ICHG;
			ITIMES(oip);
		}

		return (err);
	}

	/* Truncate-down case. */

	/*
	 * If file is currently in use for swap, disallow truncate-down.
	 */
	if (IS_SWAPVP(ITOV(oip)))
		return EBUSY;

	/*
	 * Update the pages of the file.  If the file is not being
	 * truncated to a block boundary, the contents of the
	 * pages following the end of the file must be zero'ed
	 * in case it ever become accessable again because
	 * of subsequent file growth.
	 */
	if (offset == 0) {
		pvn_vptrunc(ITOV(oip), (u_int)length, (u_int)0);
	} else {
		int err;

		/*
		 * Make sure that the last block is properly allocated.
		 * We only really have to do this if the last block is
		 * actually allocated since ufs_bmap will now handle the case
		 * of an fragment which has no block allocated.  Just to
		 * be sure, we do it now independent of current allocation.
		 */
		err = BMAPALLOC(oip, llbn, offset);
		if (err)
			return (err);
		bsize = llbn >= NDADDR? fs->fs_bsize : fragroundup(fs, offset);
		pvn_vptrunc(ITOV(oip), (u_int)length, (u_int)(bsize - offset));
	}

	/*
	 * Calculate index into inode's block list of
	 * last direct and indirect blocks (if any)
	 * which we want to keep.  Lastblock is -1 when
	 * the file is truncated to 0.
	 */
	lastblock = lblkno(fs, length + fs->fs_bsize - 1) - 1;
	lastiblock[SINGLE] = lastblock - NDADDR;
	lastiblock[DOUBLE] = lastiblock[SINGLE] - NINDIR(fs);
	lastiblock[TRIPLE] = lastiblock[DOUBLE] - NINDIR(fs) * NINDIR(fs);
	nblocks = btodb(fs->fs_bsize);

	/*
	 * Update file and block pointers
	 * on disk before we start freeing blocks.
	 * If we crash before free'ing blocks below,
	 * the blocks will be returned to the free list.
	 * lastiblock values are also normalized to -1
	 * for calls to indirtrunc below.
	 */
	tip = *oip;			/* structure copy */
	ip = &tip;

	for (level = TRIPLE; level >= SINGLE; level--)
		if (lastiblock[level] < 0) {
			oip->i_ib[level] = 0;
			lastiblock[level] = -1;
		}
	for (i = NDADDR - 1; i > lastblock; i--)
		oip->i_db[i] = 0;

	oip->i_size = length;
	oip->i_flag |= ICHG|IUPD;
	ufs_iupdat(oip, 1);			/* do sync inode update */

	/*
	 * Indirect blocks first.
	 */
	for (level = TRIPLE; level >= SINGLE; level--) {
		bn = ip->i_ib[level];
		if (bn != 0) {
			blocksreleased +=
			    indirtrunc(ip, bn, lastiblock[level], level);
			if (lastiblock[level] < 0) {
				ip->i_ib[level] = 0;
				(void)free(ip, bn, (off_t)fs->fs_bsize);
				blocksreleased += nblocks;
			}
		}
		if (lastiblock[level] >= 0)
			goto done;
	}

	/*
	 * All whole direct blocks or frags.
	 */
	for (i = NDADDR - 1; i > lastblock; i--) {
		bn = ip->i_db[i];
		if (bn == 0)
			continue;
		ip->i_db[i] = 0;
		bsize = (off_t)blksize(fs, ip, i);
		(void)free(ip, bn, bsize);
		blocksreleased += btodb(bsize);
	}
	if (lastblock < 0)
		goto done;

	/*
	 * Finally, look for a change in size of the
	 * last direct block; release any frags.
	 */
	bn = ip->i_db[lastblock];
	if (bn != 0) {
		off_t oldspace, newspace;

		/*
		 * Calculate amount of space we're giving
		 * back as old block size minus new block size.
		 */
		oldspace = blksize(fs, ip, lastblock);
		ip->i_size = length;
		newspace = blksize(fs, ip, lastblock);
		ASSERT(newspace != 0);
		if (oldspace - newspace > 0) {
			/*
			 * Block number of space to be free'd is
			 * the old block # plus the number of frags
			 * required for the storage we're keeping.
			 */
			bn += numfrags(fs, newspace);
			(void)free(ip, bn, oldspace - newspace);
			blocksreleased += btodb(oldspace - newspace);
		}
	}
done:
/* BEGIN PARANOIA */
	for (level = SINGLE; level <= TRIPLE; level++)
		ASSERT(ip->i_ib[level] == oip->i_ib[level]);
	for (i = 0; i < NDADDR; i++)
		ASSERT(ip->i_db[i] == oip->i_db[i]);
/* END PARANOIA */
	oip->i_blocks -= blocksreleased;
	if (oip->i_blocks < 0) {		/* sanity */
		cmn_err(CE_NOTE, "ufs_itrunc: %s/%d new size = %d, blocks = %d\n",
		    fs->fs_fsmnt, oip->i_number, oip->i_size, oip->i_blocks);
		oip->i_blocks = 0;
	}
	oip->i_flag |= ICHG;
#ifdef QUOTA
	(void) chkdq(oip, -blocksreleased, 0);
#endif
	return (0);
}

/*
 * Remove any inodes in the inode cache belonging to dev
 *
 * There should not be any active ones, return error if any are found but
 * still invalidate others (N.B.: this is a user error, not a system error).
 *
 * Also, count the references to dev by block devices - this really
 * has nothing to do with the object of the procedure, but as we have
 * to scan the inode table here anyway, we might as well get the
 * extra benefit.
 *
 * This is called from umount1()/ufs_vfsops.c when dev is being unmounted.
 */
#ifdef QUOTA
ufs_iflush(vfsp, iq)
	struct vfs *vfsp;
	struct inode *iq;
#else
ufs_iflush(vfsp)
	struct vfs *vfsp;
#endif
{
	register struct inode *ip;
	register open = 0;
	register struct vnode *vp, *rvp;
	dev_t dev = vfsp->vfs_dev;
	
	rvp = ((struct ufsvfs *)(vfsp->vfs_data))->vfs_root;
	for (ip = ufs_inode; ip < inodeNINODE; ip++) {
#ifdef QUOTA
		if (ip != iq && ip->i_dev == dev) {
#else
		if (ip->i_dev == dev) {
#endif
			vp = ITOV(ip);
			if ((ip->i_flag & IREF) && (vp != rvp ||
				(vp == rvp && vp->v_count > 1))) {
				/*
				 * Set error indicator for return value,
				 * but continue invalidating other inodes.
				 */
				open = -1;
			} else {
				_remque(ip);
				ip->i_forw = ip;
				ip->i_back = ip;
				/*
				 * as IREF == 0, the inode was on the free
				 * list already, just leave it there, it will
				 * fall off the bottom eventually. We could
				 * perhaps move it to the head of the free
				 * list, but as umounts are done so
				 * infrequently, we would gain very little,
				 * while making the code bigger.
				 */
#ifdef QUOTA
				dqrele(ip->i_dquot);
				ip->i_dquot = NULL;
#endif
				ILOCK(ip);
				if (ip->i_map)
					ufs_freemap(ip);
				(void) ufs_syncip(ip, B_INVAL);
				IUNLOCK(ip);
			}
		}		
		else if ((ip->i_flag & IREF) && (ip->i_mode & IFMT) == IFBLK &&
		    ip->i_rdev == dev && open >= 0)
			open++;
	}
	return (open);
}

/*
 * Lock an inode.
 */
void
ufs_ilock(ip)
	register struct inode *ip;
{

	ILOCK(ip);
	return;
}

/*
 * Unlock an inode.
 */
void
ufs_iunlock(ip)
	register struct inode *ip;
{

	IUNLOCK(ip);
	return;
}

/*
 * Check mode permission on inode.  Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the read-only status of the file system
 * is checked.  The mode is shifted to select the owner/group/other
 * fields.  The super user is granted all permissions except
 * writing to read-only file systems.
 */
int
ufs_iaccess(ip, mode, cr)
	register struct inode *ip;
	register int mode;
	register struct cred *cr;
{
	register struct vnode *vp = ITOV(ip);

	if (mode & IWRITE) {
		/*
		 * Disallow write attempts on read-only
		 * file systems, unless the file is a block
		 * or character device or a FIFO.
		 */
		if (ip->i_fs->fs_ronly != 0) {
			if ((ip->i_mode & IFMT) != IFCHR &&
			    (ip->i_mode & IFMT) != IFBLK &&
			    (ip->i_mode & IFMT) != IFIFO) {
				return (EROFS);
			}
		}
	}
	/*
	 * If you're the super-user,
	 * you always get access.
	 */
	if (cr->cr_uid == 0)
		return (0);
	/*
	 * Access check is based on only
	 * one of owner, group, public.
	 * If not owner, then check group.
	 * If not a member of the group, then
	 * check public access.
	 */
	if (cr->cr_uid != ip->i_uid) {
		mode >>= 3;
		if (!groupmember((uid_t)ip->i_gid, cr))
			mode >>= 3;
	}
	if ((ip->i_mode & mode) == mode)
		return (0);
	if ((ip->i_mode&IEXEC == IEXEC) && is286EMUL)
		return 0;
	return (EACCES);
}


/*
 * Remove from old hash chain and insert into new one.
 */

void
_remque(ip)
struct inode *ip;
{
	ip->i_back->i_forw = ip->i_forw;
	ip->i_forw->i_back = ip->i_back;
	ip->i_forw = ip->i_back = ip;
	return;
}

void
_insque(ip, hip)
struct inode *ip;
union ihead *hip;
{
	hip->ih_chain[0]->i_back = ip;
	ip->i_forw = hip->ih_chain[0];
	hip->ih_chain[0] = ip;
	ip->i_back = (struct inode *) hip;
	return;
}
