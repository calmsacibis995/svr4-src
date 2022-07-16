/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)xx:io/xxalloc.c	1.2.2.4"

#include "sys/types.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"
#include "sys/cred.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/fcntl.h"
#include "sys/file.h"
#include "sys/flock.h"
#include "sys/param.h"
#include "sys/stat.h"
#include "sys/swap.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/mode.h"
#include "sys/user.h"

#include "vm/pvn.h"
#include "vm/page.h"

#include "sys/proc.h"
#include "sys/disp.h"

#include "sys/fs/s5param.h"
#include "sys/fs/xxfblk.h"
#include "sys/fs/xxfilsys.h"
#include "sys/fs/s5ino.h"
#include "sys/fs/s5inode.h"
#include "sys/fs/s5macros.h"

#include "fs/fs_subr.h"

typedef	struct fblk *FBLKP;

void	xxblkfree(), xxifree(), xxindirtrunc();

/*
 * xxblkalloc will obtain the next available free disk block from
 * the free list of the specified device.  The super-block has
 * up to NICFREE remembered free blocks; the last of these is
 * read to obtain NICFREE more.
 *
 * no space on dev x/y -- when the free list is exhausted.
 */
int
xxblkalloc(vfsp, bnp)
	register struct vfs *vfsp;
	daddr_t *bnp;
{
	register dev_t dev;
	register daddr_t bno;
	register struct filsys *fp;
	register struct buf *bp;
	int bsize;

	dev = vfsp->vfs_dev;
	bsize = vfsp->vfs_bsize;
	fp = getfs(vfsp);
	while (fp->s_flock)
		sleep((caddr_t)&fp->s_flock, PINOD);
	do {
		if (fp->s_nfree <= 0)
			goto nospace;
		if ((bno = fp->s_free[--fp->s_nfree]) == 0)
			goto nospace;
	} while (xxbadblock(fp, bno, dev));


	if (fp->s_nfree <= 0) {
		fp->s_flock++;
		bp = bread(dev, bno, bsize);
		if ((bp->b_flags & B_ERROR) == 0) {
			fp->s_nfree = ((FBLKP)(bp->b_un.b_addr))->df_nfree;
			bcopy((caddr_t)((FBLKP)(bp->b_un.b_addr))->df_free,
			    (caddr_t)fp->s_free, sizeof(fp->s_free));
		}
		bp->b_flags &= ~B_DELWRI;
		bp->b_flags |= B_STALE|B_AGE;
		brelse(bp);
		fp->s_flock = 0;
		wakeprocs((caddr_t)&fp->s_flock, PRMPT);
	}
	if (fp->s_nfree <= 0 || fp->s_nfree > NICFREE) {
		prdev("Bad free count", dev);
		goto nospace;
	}
	if (fp->s_tfree)
		fp->s_tfree--;
	fp->s_fmod = 1;
	*bnp = bno;
	return 0;

nospace:
	fp->s_nfree = 0;
	fp->s_tfree = 0;
	delay(5*HZ);
	prdev("no space", dev);
	return ENOSPC;
}

/*
 * Place the specified disk block back on the free list of the
 * specified device.
 */
void
xxblkfree(vfsp, bno)
	register struct vfs *vfsp;
	register daddr_t bno;
{
	register dev_t dev = vfsp->vfs_dev;
	int bsize = vfsp->vfs_bsize;
	register struct filsys *fp = getfs(vfsp);
	register struct buf *bp;

	fp->s_fmod = 1;
	while (fp->s_flock)
		sleep((caddr_t)&fp->s_flock, PINOD);
	if (xxbadblock(fp, bno, dev))
		return;
	if (fp->s_nfree <= 0) {
		fp->s_nfree = 1;
		fp->s_free[0] = 0;
	}
	if (fp->s_nfree >= NICFREE) {
		fp->s_flock++;
		bp = getblk(dev, bno, vfsp->vfs_bsize);
		((FBLKP)(bp->b_un.b_addr))->df_nfree = fp->s_nfree;
		bcopy((caddr_t)fp->s_free,
		  (caddr_t)((FBLKP)(bp->b_un.b_addr))->df_free,
		  sizeof(fp->s_free));
		fp->s_nfree = 0;
		bdwrite(bp);
		fp->s_flock = 0;
		wakeprocs((caddr_t)&fp->s_flock, PRMPT);
	} else if (incore(dev, bno, bsize)) {
		/*
		 * There may be a leftover in-core buffer for this block;
		 * if so, make sure it's marked invalid and turn off
		 * B_DELWRI so that it will not subsequently be written
		 * to disk.  Otherwise, if the block is subsequently
		 * allocated as file data, the stale data in the buffer
		 * will be aliasing the data in the page cache.
		 */
		bp = getblk(dev, bno, bsize);
		bp->b_flags &= ~B_DELWRI;
		bp->b_flags |= B_STALE|B_AGE;
		brelse(bp);
	}

	fp->s_free[fp->s_nfree++] = bno;
	fp->s_tfree++;
	fp->s_fmod = 1;
}

/*
 * Check that a block number is in the range between the I list
 * and the size of the device.  This is used mainly to check that
 * a garbage file system has not been mounted.
 *
 * bad block on dev x/y -- not in range
 */
int
xxbadblock(fp, bn, dev)
	register struct filsys *fp;
	daddr_t bn;
	dev_t dev;
{
	if (bn < (daddr_t)fp->s_isize || bn >= (daddr_t)fp->s_fsize) {
		prdev("bad block", dev);
		return 1;
	}
	return 0;
}

/*
 * Allocate an unused inode on the specified device.  Used with
 * file creation.  The algorithm keeps up to NICINOD spare inodes
 * in the super-block.  When this runs out, a linear search through
 * the i-list is instituted to pick up NICINOD more.
 */
int
xxialloc(vfsp, mode, nlink, rdev, uid, gid, ipp)
	struct vfs *vfsp;
	u_short mode;
	int nlink;
	dev_t rdev;
	int uid;
	int gid;
	struct inode **ipp;
{
	dev_t dev = vfsp->vfs_dev;
	int bsize = vfsp->vfs_bsize;
	struct s5vfs *s5vfsp = S5VFS(vfsp);
	register struct filsys *fp;
	register struct vnode *vp;
	struct inode *ip = NULL;
	register int i;
	struct buf *bp;
	register struct dinode *dp;
	register u_short ino;
	daddr_t adr;
	int error;
	o_dev_t oldrdev;

	fp = getfs(vfsp);
loop:
	while (fp->s_ilock)
		sleep((caddr_t)&fp->s_ilock, PINOD);
	fp->s_ilock = 1;
loop1:
	if (fp->s_ninode > 0 && (ino = fp->s_inode[--fp->s_ninode])) {
		if (error = xxiget(vfsp, ino, &ip)) {
			fp->s_ilock = 0;
			wakeprocs(&fp->s_ilock, PRMPT);
			return error;
		}
		vp = ITOV(ip);
		if (ip->i_mode == 0) {
			/* Found inode: update now to avoid races */
			enum vtype type;

			vp->v_type = type = IFTOVT((int)mode);
			ip->i_flag |= IACC|IUPD|ICHG|ISYN;
			ip->i_mode = mode;
			ip->i_nlink = (o_nlink_t)nlink;
			ip->i_uid = (o_uid_t)uid;
			ip->i_gid = (o_gid_t)gid;
			ip->i_size = 0;
			for (i = 0; i < NADDR; i++)
				ip->i_addr[i] = 0;
			/*
			 * Must set rdev after address fields are
			 * zeroed because rdev is defined to be the
			 * first address field (inode.h).
			 */
			if (type == VCHR || type == VBLK) { 
				ip->i_rdev = rdev;
				/* update i_addr components */
				ip->i_major = getemajor(rdev);
				ip->i_minor = geteminor(rdev);
				ip->i_bcflag |= NDEVFORMAT;
				/*
				 * To preserve backward compatibility we store
				 * dev in old format if it fits, otherwise
				 * NODEV is assigned.
				 */
				if ((oldrdev = cmpdev(rdev)) != (o_dev_t) NODEV)
					ip->i_oldrdev = (daddr_t)oldrdev;
				else
					ip->i_oldrdev = (daddr_t)NODEV;

			} else if (type == VXNAM) {
				/*
				 * Believe it or not. XENIX stores 
				 * semaphore info in rdev.
				 */
				ip->i_rdev = rdev;
				ip->i_oldrdev = rdev; /* need this for xxiupdat */
			}

			vp->v_rdev = ip->i_rdev;
			if (fp->s_tinode)
				fp->s_tinode--;
			fp->s_fmod = 1;
			xxiupdat(ip);
			fp->s_ilock = 0;
			wakeprocs(&fp->s_ilock, PRMPT);
			*ipp = ip;
			return 0;
		}
		/*
		 * Inode was allocated after all.  Look some more.
		 */
		cmn_err(CE_NOTE, "xxialloc: inode was already allocated\n");
		xxiupdat(ip);
		fp->s_ilock = 0;
		wakeprocs(&fp->s_ilock, PRMPT);
		xxiput(ip);
		goto loop;
	}
	/*
	 * Only try to rebuild freelist if there are free inodes.
	 */
	if (fp->s_tinode > 0) {
		fp->s_ninode = NICINOD;
		ino = FsINOS(s5vfsp, fp->s_inode[0]);
		for (adr = FsITOD(s5vfsp, ino); adr < (daddr_t)fp->s_isize;
		  adr++) {
			bp = bread(dev, adr, bsize);
			if (bp->b_flags & B_ERROR) {
				brelse(bp);
				ino += s5vfsp->vfs_inopb;
				continue;
			}
			dp = (struct dinode *) bp->b_un.b_addr;
			for (i = 0; i < s5vfsp->vfs_inopb; i++, ino++, dp++) {
				if (fp->s_ninode <= 0)
					break;
				if (dp->di_mode == 0)
					fp->s_inode[--fp->s_ninode] = ino;
			}
			brelse(bp);
			if (fp->s_ninode <= 0)
				break;
		}
		if (fp->s_ninode > 0) {
			fp->s_inode[fp->s_ninode-1] = 0;
			fp->s_inode[0] = 0;
		}
		if (fp->s_ninode != NICINOD) {
			fp->s_ninode = NICINOD;
			goto loop1;
		}
	}

	fp->s_ninode = 0;
	fp->s_tinode = 0;
	fp->s_ilock = 0;
	wakeprocs((caddr_t)&fp->s_ilock, PRMPT);
	prdev("Out of inodes", dev);
	return ENOSPC;
}

/*
 * Free the specified inode on the specified device.
 * The algorithm stores up to NICINOD inodes in the
 * super-block and throws away any more.
 */
void
xxifree(ip)
	register struct inode *ip;
{
	register struct filsys *fp;
	register u_short ino;
	register struct vnode *vp = ITOV(ip);

	ASSERT(ip->i_flag & ILOCKED);
	/*
	 * Don't put an already free inode on the free list.
	 */
	if (ip->i_mode == 0)
		return;
	ino = ip->i_number;
	fp = getfs(vp->v_vfsp);
	while (fp->s_ilock)
		sleep((caddr_t)&fp->s_ilock, PINOD);
	fp->s_ilock = 1;
	ip->i_mode = 0;		/* zero means inode not allocated */
	/*
	 * Update disk inode from incore slot before putting it on
	 * the freelist; this eliminates a race in the simplex code
	 * which did an xxifree() and then an xxiupdat() in xxiput().
	 */
	xxiupdat(ip);
	fp->s_tinode++;
	fp->s_fmod = 1;
	if (fp->s_ninode >= NICINOD || fp->s_ninode == 0) {
		if (ino < fp->s_inode[0])
			fp->s_inode[0] = ino;
	} else
		fp->s_inode[fp->s_ninode++] = ino;
	fp->s_ilock--;
	wakeprocs((caddr_t)&fp->s_ilock, PRMPT);
}

#define NIADDR	3		/* number of indirect block pointers */
#define NDADDR	(NADDR-NIADDR)	/* number of direct block pointers */
#define IB(i)	(NDADDR + (i))	/* index of i'th indirect block ptr */
#define SINGLE	0		/* single indirect block ptr */
#define DOUBLE	1		/* double indirect block ptr */
#define TRIPLE	2		/* triple indirect block ptr */

/*
 * Free storage space associated with the specified inode.  The portion
 * to be freed is specified by lp->l_start and lp->l_len (already
 * normalized to a "whence" of 0).

 * This is an experimental facility whose continued existence is not
 * guaranteed.  Currently, we only support the special case of
 * l_len == 0, meaning free to end of file.
 *
 * Blocks are freed in reverse order.  This FILO algorithm will tend to
 * maintain a contiguous free list much longer than FIFO.  See also
 * xxitrunc() in xxinode.c.
 *
 * Bug: unused bytes in the last retained block are not cleared.  This
 * may result in a "hole" in the file that does not read as zeroes.
 */
int
xxfreesp(vp, lp, flag)
	register struct vnode *vp;
	register struct flock *lp;
	int flag;
{
	register int i;
	register struct inode *ip = VTOI(vp);
	register struct vfs *vfsp;
	register struct s5vfs *s5vfsp;
	register daddr_t bn;
	register daddr_t lastblock;
	register long bsize;
	long nindir;
	int error;
	daddr_t lastiblock[NIADDR];
	daddr_t save[NADDR];

	ASSERT(vp->v_type == VREG);
	ASSERT(lp->l_start >= 0);	/* checked by convoff */

	if (lp->l_len != 0)
		return EINVAL;
	if (ip->i_size == lp->l_start)
		return 0;

	/*
	 * If file is currently in use for swap, disallow truncate-down.
	 */
	if (ip->i_size > lp->l_start && IS_SWAPVP(vp))
		return EBUSY;

	vfsp = vp->v_vfsp;
	bsize = vfsp->vfs_bsize;
	s5vfsp = S5VFS(vfsp);
	nindir = s5vfsp->vfs_nindir;

	/*
	 * Check if there is any active mandatory lock on the
	 * range that will be truncated/expanded.
	 */
	if (MANDLOCK(vp, ip->i_mode)) {
		int save_start;

		save_start = lp->l_start;

		if (ip->i_size < lp->l_start) {
			/*
			 * "Truncate up" case: need to make sure there
			 * is no lock beyond current end-of-file. To
			 * do so, we need to set l_start to the size
			 * of the file temporarily.
			 */
			lp->l_start = ip->i_size;
		}
		lp->l_type = F_WRLCK;
		lp->l_sysid = u.u_procp->p_sysid;
		lp->l_pid = u.u_procp->p_epid;
		i = (flag & (FNDELAY|FNONBLOCK)) ? 0 : SLPFLCK;
		if ((i = reclock(vp, lp, i, 0, lp->l_start)) != 0
		  || lp->l_type != F_UNLCK)
			return i ? i : EAGAIN;

		lp->l_start = save_start;
	}

	if (ip->i_size < lp->l_start) {
		unsigned long nboff, pgoff;
		page_t *pp;

		/*
		 * "Truncate up" case: the file is grown to the size
		 * indicated by l_start.  Only the last block is
		 * actually allocated (any blocks in between will be
		 * holes).
		 */
		lastblock = (lp->l_start - 1) >> s5vfsp->vfs_bshift;
		ILOCK(ip);
		if (ip->i_map)
			xxfreemap(ip);

		nboff = (ip->i_size + bsize-1) & ~s5vfsp->vfs_bmask;
		pgoff = (ip->i_size + PAGEOFFSET) & PAGEMASK;

		if (bsize < PAGESIZE && (ip->i_size & PAGEOFFSET)) {

			if (nboff < pgoff && nboff + bsize <= lp->l_start) {
				/*
				 * There are holes in the page at old EOF.
				 */
				page_t *pp;

				pp = page_lookup(vp, ip->i_size & PAGEMASK);
				if (pp)
					page_rdonly(pp);

			}
		}
		if (error = xxbmapalloc(ip, lastblock, lastblock, 0, 0)) {
			IUNLOCK(ip);
			return error;
		}
		ip->i_size = lp->l_start;
		ip->i_flag |= IUPD|ICHG;
		ITIMES(ip);
		IUNLOCK(ip);
		return 0;
	}

	ILOCK(ip);

	if (ip->i_map)
		xxfreemap(ip);

	if (error = fs_vcode(vp, &ip->i_vcode)) {
		IUNLOCK(ip);
		return error;
	}

	/*
	 * Update the pages of the file.  If the file is not being
	 * truncated to a block boundary, the contents of the
	 * pages following the end of the file must be zeroed
	 * in case they ever become accessible again because
	 * of subsequent file growth.
	 */
	pvn_vptrunc(vp, lp->l_start,
	  (u_int)(bsize - (lp->l_start & s5vfsp->vfs_bmask)));

	/*
	 * Calculate index into inode's block list of last block
	 * we want to keep.  Lastblock is -1 when the file is
	 * truncated to 0.
	 *
	 * Think of the file as consisting of four separate lists
	 * of data blocks:  one list of up to NDADDR blocks pointed
	 * to directly from the inode, and three lists of up to
	 * nindir, nindir**2 and nindir**3 blocks, respectively
	 * headed by the SINGLE, DOUBLE and TRIPLE indirect block
	 * pointers.  For each of the four lists, we're calculating
	 * the index within the list of the last block we want to
	 * keep.  If the index for list i is negative, it means
	 * that said block is not in list i (but perhaps in list i-1),
	 * hence all blocks in list i are to be discarded; if the
	 * index is beyond the end of list i, it means that the
	 * block is not in list i (but perhaps in list i+1), hence
	 * all blocks in list i are to be kept.
	 */

	nindir = s5vfsp->vfs_nindir;
	lastblock =
	 (long)(((u_long)lp->l_start + bsize-1) >> s5vfsp->vfs_bshift) - 1;
	lastiblock[SINGLE] = lastblock - NDADDR;
	lastiblock[DOUBLE] = lastiblock[SINGLE] - nindir;
	lastiblock[TRIPLE] = lastiblock[DOUBLE] - nindir*nindir;

	/*
	 * Update file size and block pointers in
	 * disk inode before we start freeing blocks.
	 * Also normalize lastiblock values to -1
	 * for calls to xxindirtrunc below.
	 */
	for (i = NADDR-1; i >= 0; i--)
		save[i] = ip->i_addr[i];

	for (i = TRIPLE; i >= SINGLE; i--)
		if (lastiblock[i] < 0) {
			ip->i_addr[IB(i)] = 0;
			lastiblock[i] = -1;
		}

	for (i = NDADDR-1; i > lastblock; i--)
		ip->i_addr[i] = 0;

	ip->i_size = lp->l_start;
	ip->i_flag |= IUPD|ICHG|ISYN;
	xxiupdat(ip);

	/*
	 * Indirect blocks first.
	 */
	for (i = TRIPLE; i >= SINGLE; i--) {
		if ((bn = save[IB(i)]) != 0) {
			xxindirtrunc(vfsp, bn, lastiblock[i], i);
			if (lastiblock[i] < 0)
				xxblkfree(vfsp, bn);
		}
		if (lastiblock[i] >= 0) {
			IUNLOCK(ip);
			return 0;
		}
	}

	/*
	 * Direct blocks.
	 */

	for (i = NDADDR-1; i > lastblock; i--) {
		if ((bn = save[i]) != 0)
			xxblkfree(vfsp, bn);
	}
	IUNLOCK(ip);
	return 0;
}

void
xxindirtrunc(vfsp, bn, lastbn, level)
	register struct vfs *vfsp;
	daddr_t bn, lastbn;
	int level;
{
	register int i;
	register struct buf *bp, *copy;
	register daddr_t *bap;
	long bsize, factor;
	struct s5vfs *s5vfsp = S5VFS(vfsp);
	long nindir;
	daddr_t last;

	/*
	 * Calculate index in current block of last block (pointer) to be kept.
	 * A lastbn of -1 indicates that the entire block is going away, so we
	 * need not calculate the index.
	 */

	bsize = vfsp->vfs_bsize;
	nindir = s5vfsp->vfs_nindir;
	factor = 1;
	for (i = SINGLE; i < level; i++)
		factor *= nindir;
	last = lastbn;
	if (lastbn > 0)
		last /= factor;

	/*
	 * Get buffer of block pointers, zero those entries corresponding to
	 * blocks to be freed, and update on-disk copy first.  (If the entire
	 * block is to be discarded, there's no need to zero it out and
	 * rewrite it, since there are no longer any pointers to it, and it
	 * will be freed shortly by the caller anyway.)
	 * Note potential deadlock if we run out of buffers.  One way to
	 * avoid this might be to use statically-allocated memory instead;
	 * you'd have to make sure that only one process at a time got at it.
	 */

	copy = ngeteblk(bsize);
	bp = bread(vfsp->vfs_dev, bn, bsize);
	if (bp->b_flags & B_ERROR) {
		brelse(copy);
		brelse(bp);
		return;
	}
	bap = bp->b_un.b_daddr;
	bcopy((caddr_t)bap, copy->b_un.b_addr, bsize);
	if (last < 0)
		brelse(bp);
	else {
		bzero((caddr_t)&bap[last+1],
			(int)(nindir - (last+1)) * sizeof(daddr_t));
		bwrite(bp);
	}
	bap = copy->b_un.b_daddr;

	/*
	 * Recursively free totally unused blocks.
	 */

	for (i = nindir-1; i > last; i--)
		if ((bn = bap[i]) != 0) {
			if (level > SINGLE)
				xxindirtrunc(vfsp, bn, (daddr_t)-1, level-1);
			xxblkfree(vfsp, bn);
		}

	/*
	 * Recursively free last partial block.
	 */

	if (level > SINGLE && lastbn >= 0) {
		last = lastbn % factor;
		if ((bn = bap[i]) != 0)
			xxindirtrunc(vfsp, bn, last, level-1);
	}

	brelse(copy);
}
