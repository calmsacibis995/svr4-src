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

#ident  "@(#)kern-fs:ufs/ufs_alloc.c	1.3.2.4"

#include <sys/types.h>
#include <sys/debug.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/cred.h>
#include <sys/proc.h>
#include <sys/disp.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/fs/ufs_fs.h>
#include <sys/fs/ufs_inode.h>
#ifdef QUOTA
#include <sys/fs/ufs_quota.h>
#endif
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/sysmacros.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/flock.h>
#include <fs/fs_subr.h>
#include <sys/cmn_err.h>

STATIC ino_t	hashalloc();
STATIC daddr_t	fragextend();
STATIC daddr_t	alloccg();
STATIC daddr_t	alloccgblk();
STATIC ino_t	ialloccg();
STATIC daddr_t	mapsearch();

extern int	inside[], around[];
extern u_char	*fragtbl[];
void free();
void ufs_ifree();

/*
 * Allocate a block in the file system.
 *
 * The size of the requested block is given, which must be some
 * multiple of fs_fsize and <= fs_bsize.
 * A preference may be optionally specified. If a preference is given
 * the following hierarchy is used to allocate a block:
 *   1) allocate the requested block.
 *   2) allocate a rotationally optimal block in the same cylinder.
 *   3) allocate a block in the same cylinder group.
 *   4) quadradically rehash into other cylinder groups, until an
 *      available block is located.
 * If no block preference is given the following heirarchy is used
 * to allocate a block:
 *   1) allocate a block in the cylinder group that contains the
 *      inode for the file.
 *   2) quadradically rehash into other cylinder groups, until an
 *      available block is located.
 */
alloc(ip, bpref, size, bnp)
	register struct inode *ip;
	daddr_t bpref;
	int size;
	daddr_t *bnp;
{
	register struct fs *fs;
	daddr_t bno;
	int cg;
#ifdef QUOTA
	int err;
#endif /* QUOTA */

	if (size == 0)
		return(EINVAL);

	fs = ip->i_fs;
	if ((unsigned)size > fs->fs_bsize || fragoff(fs, size) != 0) {
		cmn_err(CE_PANIC,
		"alloc: bad size, dev = 0x%x, bsize = %d, size = %d, fs = %s\n",
		    ip->i_dev, fs->fs_bsize, size, fs->fs_fsmnt);
		/* NOTREACHED */
	}
	if (size == fs->fs_bsize && fs->fs_cstotal.cs_nbfree == 0)
		goto nospace;
	if (u.u_cred->cr_uid != 0 && freespace(fs, fs->fs_minfree) <= 0)
		goto nospace;
#ifdef QUOTA
	err = chkdq(ip, (long)btodb(size), 0);
	if (err)
		return (err);
#endif /* QUOTA */
	if (bpref >= fs->fs_size)
		bpref = 0;
	if (bpref == 0)
		cg = (int)itog(fs, ip->i_number);
	else
		cg = dtog(fs, bpref);
	bno = (daddr_t)hashalloc(ip, cg, (long)bpref, size,
	    (u_long (*)())alloccg);
	if (bno > 0) {
		*bnp = bno;
		return (0);
	}
nospace:
	delay(5*HZ);
	cmn_err(CE_NOTE, "%s: file system full\n", fs->fs_fsmnt);
	return (ENOSPC);
}

/*
 * Reallocate a fragment to a bigger size
 *
 * The number and size of the old block is given, and a preference
 * and new size is also specified.  The allocator attempts to extend
 * the original block.  Failing that, the regular block allocator is
 * invoked to get an appropriate block.
 */
realloccg(ip, bprev, bpref, osize, nsize, bnp)
	register struct inode *ip;
	daddr_t bprev, bpref;
	int osize, nsize;
	daddr_t *bnp;
{
	daddr_t bno;
	register struct fs *fs;
	int cg, request;
#ifdef QUOTA
	int err;
#endif /* QUOTA */

	fs = ip->i_fs;
	if ((unsigned)osize > fs->fs_bsize || fragoff(fs, osize) != 0 ||
	    (unsigned)nsize > fs->fs_bsize || fragoff(fs, nsize) != 0) {
		cmn_err(CE_PANIC,
		"realloccg: bad size, dev = 0x%x, bsize = %d, osize = %d, nsize = %d, fs = %s\n",
		    ip->i_dev, fs->fs_bsize, osize, nsize, fs->fs_fsmnt);
		/* NOTREACHED */
	}
	if (u.u_cred->cr_uid != 0 && freespace(fs, fs->fs_minfree) <= 0)
		goto nospace;
	if (bprev == 0) {
		cmn_err(CE_PANIC,
		"realloccg: bad bprev, dev = 0x%x, bsize = %d, bprev = %d, fs = %s\n",
		    ip->i_dev, fs->fs_bsize, bprev, fs->fs_fsmnt);
		/* NOTREACHED */
	}
#ifdef QUOTA
	err = chkdq(ip, (long)btodb(nsize - osize), 0);
	if (err)
		return (err);
#endif /* QUOTA */
	cg = dtog(fs, bprev);
	bno = fragextend(ip, cg, (long)bprev, osize, nsize);
	if (bno != 0) {
		*bnp = bno;
		return (0);
	}
	if (bpref >= fs->fs_size)
		bpref = 0;
	switch ((int)fs->fs_optim) {
	case FS_OPTSPACE:
		/*
		 * Allocate an exact sized fragment. Although this makes 
		 * best use of space, we will waste time relocating it if 
		 * the file continues to grow. If the fragmentation is
		 * less than half of the minimum free reserve, we choose
		 * to begin optimizing for time.
		 */
		request = nsize;
		if (fs->fs_minfree < 5 ||
		    fs->fs_cstotal.cs_nffree >
		    fs->fs_dsize * fs->fs_minfree / (2 * 100))
			break;
		cmn_err(CE_NOTE, "%s: optimization changed from SPACE to TIME\n",
			fs->fs_fsmnt);
		fs->fs_optim = FS_OPTTIME;
		break;
	default:
		/*
		 * Old file systems.
		 */
		cmn_err(CE_NOTE, "%s: bad optimization, defaulting to TIME\n",
			fs->fs_fsmnt);
		fs->fs_optim = FS_OPTTIME;
		/* fall through */
	case FS_OPTTIME:
		/*
		 * At this point we have discovered a file that is trying
		 * to grow a small fragment to a larger fragment. To save
		 * time, we allocate a full sized block, then free the 
		 * unused portion. If the file continues to grow, the 
		 * `fragextend' call above will be able to grow it in place
		 * without further copying. If aberrant programs cause
		 * disk fragmentation to grow within 2% of the free reserve,
		 * we choose to begin optimizing for space.
		 */
		request = fs->fs_bsize;
		if (fs->fs_cstotal.cs_nffree <
		    fs->fs_dsize * (fs->fs_minfree - 2) / 100)
			break;
		cmn_err(CE_NOTE, "%s: optimization changed from TIME to SPACE\n",
			fs->fs_fsmnt);
		fs->fs_optim = FS_OPTSPACE;
		break;
	}
	bno = (daddr_t)hashalloc(ip, cg, (long)bpref, request,
		(u_long (*)())alloccg);
	if (bno > 0) {
		*bnp = bno;
		if (nsize < request)
			(void)free(ip, bno + numfrags(fs, nsize),
				(off_t)(request - nsize));
		return (0);
	}
nospace:
	delay(5*HZ);
	cmn_err(CE_NOTE, "%s: file system full\n", fs->fs_fsmnt);
	return (ENOSPC);
}

/*
 * Allocate an inode in the file system.
 * 
 * A preference may be optionally specified. If a preference is given
 * the following hierarchy is used to allocate an inode:
 *   1) allocate the requested inode.
 *   2) allocate an inode in the same cylinder group.
 *   3) quadradically rehash into other cylinder groups, until an
 *      available inode is located.
 * If no inode preference is given the following heirarchy is used
 * to allocate an inode:
 *   1) allocate an inode in cylinder group 0.
 *   2) quadradically rehash into other cylinder groups, until an
 *      available inode is located.
 */
ufs_ialloc(pip, ipref, mode, ipp)
	register struct inode *pip;
	ino_t ipref;
	mode_t mode;
	struct inode **ipp;
{
	register struct inode *ip;
	register struct fs *fs;
	int cg;
	ino_t ino;
	int err;

	fs = pip->i_fs;
loop:
	if (fs->fs_cstotal.cs_nifree == 0)
		goto noinodes;
#ifdef QUOTA
	err = chkiq((struct ufsvfs *)pip->i_vnode.v_vfsp->vfs_data, (struct inode *)NULL,
	    u.u_cred->cr_uid, 0);
	if (err)
		return (err);
#endif
	if (ipref >= (u_long)(fs->fs_ncg * fs->fs_ipg))
		ipref = 0;
	cg = itog(fs, ipref);
	ino = (ino_t)hashalloc(pip, cg, (long)ipref, mode, (u_long (*)())ialloccg);
	if (ino == 0)
		goto noinodes;
	err = ufs_iget(pip->i_vnode.v_vfsp, fs, ino, ipp);
	if (err) {
		ufs_ifree(pip, ino, 0);
		return (err);
	}
	ip = *ipp;
	if (ip->i_mode) {
#ifdef DEBUG
		cmn_err(CE_NOTE, "mode = 0%o, inum = %d, fs = %s\n",
		    ip->i_mode, ip->i_number, fs->fs_fsmnt);
#endif
		ufs_iupdat(ip, 1);
		ufs_iput(ip);
		goto loop;			
	}
	if (ip->i_blocks) {				/* XXX */
/*		cmn_err(CE_NOTE, "free inode %s/%d had %d blocks\n",
		    fs->fs_fsmnt, ino, ip->i_blocks);
*/
		ip->i_blocks = 0;
	}
	return (0);
noinodes:
	cmn_err(CE_NOTE, "%s: out of inodes\n", fs->fs_fsmnt);
	return (ENOSPC);
}

/*
 * Find a cylinder to place a directory.
 *
 * The policy implemented by this algorithm is to select from
 * among those cylinder groups with above the average number of
 * free inodes, the one with the smallest number of directories.
 */
ino_t
dirpref(fs)
	register struct fs *fs;
{
	int cg, minndir, mincg, avgifree;

	avgifree = fs->fs_cstotal.cs_nifree / fs->fs_ncg;
	minndir = fs->fs_ipg;
	mincg = 0;
	for (cg = 0; cg < fs->fs_ncg; cg++)
		if (fs->fs_cs(fs, cg).cs_ndir < minndir &&
		    fs->fs_cs(fs, cg).cs_nifree >= avgifree) {
			mincg = cg;
			minndir = fs->fs_cs(fs, cg).cs_ndir;
		}
	return ((ino_t)(fs->fs_ipg * mincg));
}

/*
 * Select the desired position for the next block in a file.  The file is
 * logically divided into sections. The first section is composed of the
 * direct blocks. Each additional section contains fs_maxbpg blocks.
 * 
 * If no blocks have been allocated in the first section, the policy is to
 * request a block in the same cylinder group as the inode that describes
 * the file. If no blocks have been allocated in any other section, the
 * policy is to place the section in a cylinder group with a greater than
 * average number of free blocks.  An appropriate cylinder group is found
 * by using a rotor that sweeps the cylinder groups. When a new group of
 * blocks is needed, the sweep begins in the cylinder group following the
 * cylinder group from which the previous allocation was made. The sweep
 * continues until a cylinder group with greater than the average number
 * of free blocks is found. If the allocation is for the first block in an
 * indirect block, the information on the previous allocation is unavailable;
 * here a best guess is made based upon the logical block number being
 * allocated.
 * 
 * If a section is already partially allocated, the policy is to
 * contiguously allocate fs_maxcontig blocks.  The end of one of these
 * contiguous blocks and the beginning of the next is physically separated
 * so that the disk head will be in transit between them for at least
 * fs_rotdelay milliseconds.  This is to allow time for the processor to
 * schedule another I/O transfer.
 */
daddr_t
blkpref(ip, lbn, indx, bap)
	struct inode *ip;
	daddr_t lbn;
	int indx;
	daddr_t *bap;
{
	register struct fs *fs;
	register int cg;
	int avgbfree, startcg;
	daddr_t nextblk;

	fs = ip->i_fs;
	if (indx % fs->fs_maxbpg == 0 || bap[indx - 1] == 0) {
		if (lbn < NDADDR) {
			cg = itog(fs, ip->i_number);
			return (fs->fs_fpg * cg + fs->fs_frag);
		}
		/*
		 * Find a cylinder with greater than average
		 * number of unused data blocks.
		 */
		if (indx == 0 || bap[indx - 1] == 0)
			startcg = itog(fs, ip->i_number) + lbn / fs->fs_maxbpg;
		else
			startcg = dtog(fs, bap[indx - 1]) + 1;
		startcg %= fs->fs_ncg;
		avgbfree = fs->fs_cstotal.cs_nbfree / fs->fs_ncg;
		for (cg = startcg; cg < fs->fs_ncg; cg++)
			if (fs->fs_cs(fs, cg).cs_nbfree >= avgbfree) {
				fs->fs_cgrotor = cg;
				return (fs->fs_fpg * cg + fs->fs_frag);
			}
		for (cg = 0; cg <= startcg; cg++)
			if (fs->fs_cs(fs, cg).cs_nbfree >= avgbfree) {
				fs->fs_cgrotor = cg;
				return (fs->fs_fpg * cg + fs->fs_frag);
			}
		return (NULL);
	}
	/*
	 * One or more previous blocks have been laid out. If less
	 * than fs_maxcontig previous blocks are contiguous, the
	 * next block is requested contiguously, otherwise it is
	 * requested rotationally delayed by fs_rotdelay milliseconds.
	 */
	nextblk = bap[indx - 1] + fs->fs_frag;
	if (indx > fs->fs_maxcontig &&
	    bap[indx - fs->fs_maxcontig] + blkstofrags(fs, fs->fs_maxcontig)
	    != nextblk)
		return (nextblk);
	if (fs->fs_rotdelay != 0)
		/*
		 * Here we convert ms of delay to frags as:
		 * (frags) = (ms) * (rev/sec) * (sect/rev) /
		 *	((sect/frag) * (ms/sec))
		 * then round up to the next block.
		 */
		nextblk += roundup(fs->fs_rotdelay * fs->fs_rps * fs->fs_nsect /
		    (NSPF(fs) * 1000), fs->fs_frag);
	return (nextblk);
}

/*
 * Free a block or fragment.
 *
 * The specified block or fragment is placed back in the
 * free map. If a fragment is deallocated, a possible 
 * block reassembly is checked.
 */
void
free(ip, bno, size)
	register struct inode *ip;
	daddr_t bno;
	off_t size;
{
	register struct fs *fs;
	register struct cg *cgp;
	register struct buf *bp;
	int cg, blk, frags, bbase;
	register int i;

	fs = ip->i_fs;
	if ((unsigned)size > fs->fs_bsize || fragoff(fs, size) != 0) {
		cmn_err(CE_PANIC, 
		    "free: bad size, dev = 0x%x, bsize = %d, size = %d, fs = %s\n",
		    ip->i_dev, fs->fs_bsize, size, fs->fs_fsmnt);
		/* NOTREACHED */
	}
	cg = dtog(fs, bno);
	if (ufs_badblock(fs, bno)) {
		cmn_err(CE_NOTE, "bad block %d, ino %d\n", bno, ip->i_number);
		return;
	}
	bp = bread(ip->i_dev, (daddr_t)fragstoblks(fs, cgtod(fs, cg)),
		    (int)fs->fs_bsize);

	cgp = (struct cg *)bp->b_un.b_addr;
	if (bp->b_flags & B_ERROR || cgp->cg_magic != CG_MAGIC) {
		brelse(bp);
		return;
	}
	cgp->cg_time = hrestime.tv_sec;
	bno = dtogd(fs, bno);
	if (size == fs->fs_bsize) {
		if (isblock(fs, cgp->cg_free, (daddr_t)fragstoblks(fs, bno))) {
			brelse(bp);
			cmn_err(CE_PANIC, "free: freeing free block,dev = 0x%x, block = %d, fs = %s\n",
			    ip->i_dev, bno, fs->fs_fsmnt);
			/* NOTREACHED */
		}
		setblock(fs, cgp->cg_free, (daddr_t)fragstoblks(fs, bno));
		cgp->cg_cs.cs_nbfree++;
		fs->fs_cstotal.cs_nbfree++;
		fs->fs_cs(fs, cg).cs_nbfree++;
		i = cbtocylno(fs, bno);
		cgp->cg_b[i][cbtorpos(fs, bno)]++;
		cgp->cg_btot[i]++;
	} else {
		bbase = bno - fragnum(fs, bno);
		/*
		 * Decrement the counts associated with the old frags
		 */
		blk = blkmap(fs, cgp->cg_free, bbase);
		fragacct(fs, blk, cgp->cg_frsum, -1);
		/*
		 * Deallocate the fragment
		 */
		frags = numfrags(fs, size);
		for (i = 0; i < frags; i++) {
			if (isset(cgp->cg_free, bno + i)) {
				cmn_err(CE_PANIC,
				"free: freeing free frag, dev = 0x%x, block = %d, fs = %s\n",
				    ip->i_dev, bno + i, fs->fs_fsmnt);
				/* NOTREACHED */
			}
			setbit(cgp->cg_free, bno + i);
		}
		cgp->cg_cs.cs_nffree += i;
		fs->fs_cstotal.cs_nffree += i;
		fs->fs_cs(fs, cg).cs_nffree += i;
		/*
		 * Add back in counts associated with the new frags
		 */
		blk = blkmap(fs, cgp->cg_free, bbase);
		fragacct(fs, blk, cgp->cg_frsum, 1);
		/*
		 * If a complete block has been reassembled, account for it
		 */
		if (isblock(fs, cgp->cg_free, (daddr_t)fragstoblks(fs,bbase))) {
			cgp->cg_cs.cs_nffree -= fs->fs_frag;
			fs->fs_cstotal.cs_nffree -= fs->fs_frag;
			fs->fs_cs(fs, cg).cs_nffree -= fs->fs_frag;
			cgp->cg_cs.cs_nbfree++;
			fs->fs_cstotal.cs_nbfree++;
			fs->fs_cs(fs, cg).cs_nbfree++;
			i = cbtocylno(fs, bbase);
			cgp->cg_b[i][cbtorpos(fs, bbase)]++;
			cgp->cg_btot[i]++;
		}
	}
	fs->fs_fmod++;
	bdwrite(bp);
}

/*
 * Free an inode.
 *
 * The specified inode is placed back in the free map.
 */
void
ufs_ifree(ip, ino, mode)
	struct inode *ip;
	ino_t ino;
	mode_t mode;
{
	register struct fs *fs;
	register struct cg *cgp;
	register struct buf *bp;
	ino_t inot;
	int cg;

	fs = ip->i_fs;
	if ((unsigned)ino >= fs->fs_ipg*fs->fs_ncg) {
		cmn_err(CE_PANIC, "ifree: range, dev = 0x%x, ino = %d, fs = %s\n",
		    ip->i_dev, ino, fs->fs_fsmnt);
		/* NOTREACHED */
	}
	cg = itog(fs, ino);
	bp = bread(ip->i_dev, (daddr_t)fragstoblks(fs, cgtod(fs, cg)),
		    (int)fs->fs_bsize);

	cgp = (struct cg *)bp->b_un.b_addr;
	if (bp->b_flags & B_ERROR || cgp->cg_magic != CG_MAGIC) {
		brelse(bp);
		return;
	}
	cgp->cg_time = hrestime.tv_sec;
	inot = ino % (u_long)fs->fs_ipg;
	if (isclr(cgp->cg_iused, inot)) {
		brelse(bp);
		cmn_err(CE_NOTE, "mode= %o, ino = %d, fs = %s\n",
		    ip->i_mode, ino, fs->fs_fsmnt);
		return;
	}
	clrbit(cgp->cg_iused, inot);
	if (inot < (u_long)cgp->cg_irotor)
		cgp->cg_irotor = inot;
	cgp->cg_cs.cs_nifree++;
	fs->fs_cstotal.cs_nifree++;
	fs->fs_cs(fs, cg).cs_nifree++;
	if ((mode & IFMT) == IFDIR) {
		cgp->cg_cs.cs_ndir--;
		fs->fs_cstotal.cs_ndir--;
		fs->fs_cs(fs, cg).cs_ndir--;
	}
	fs->fs_fmod++;
	bdwrite(bp);
}

#ifdef notneeded
/*
 * Fserr prints the name of a file system with an error diagnostic.
 * 
 * The form of the error message is:
 *	fs: error message
 */
fserr(fs, cp)
	struct fs *fs;
	char *cp;
{

	log(LOG_ERR, "%s: %s\n", fs->fs_fsmnt, cp);
}
#endif

/*
 * Implement the cylinder overflow algorithm.
 *
 * The policy implemented by this algorithm is:
 *   1) allocate the block in its requested cylinder group.
 *   2) quadradically rehash on the cylinder group number.
 *   3) brute force search for a free block.
 */
STATIC ino_t
hashalloc(ip, cg, pref, size, allocator)
	struct inode *ip;
	int cg;
	long pref;
	int size;	/* size for data blocks, mode for inodes */
	u_long (*allocator)();
{
	register struct fs *fs;
	register int i;
	long result;
	int icg = cg;

	fs = ip->i_fs;
	/*
	 * 1: preferred cylinder group
	 */
	result = (*allocator)(ip, cg, pref, size);
	if (result)
		return (result);
	/*
	 * 2: quadratic rehash
	 */
	for (i = 1; i < fs->fs_ncg; i *= 2) {
		cg += i;
		if (cg >= fs->fs_ncg)
			cg -= fs->fs_ncg;
		result = (*allocator)(ip, cg, 0, size);
		if (result)
			return (result);
	}
	/*
	 * 3: brute force search
	 * Note that we start at i == 2, since 0 was checked initially,
	 * and 1 is always checked in the quadratic rehash.
	 */
	cg = (icg + 2) % fs->fs_ncg;
	for (i = 2; i < fs->fs_ncg; i++) {
		result = (*allocator)(ip, cg, 0, size);
		if (result)
			return (result);
		cg++;
		if (cg == fs->fs_ncg)
			cg = 0;
	}
	return (NULL);
}

/*
 * Determine whether a fragment can be extended.
 *
 * Check to see if the necessary fragments are available, and 
 * if they are, allocate them.
 */
STATIC daddr_t
fragextend(ip, cg, bprev, osize, nsize)
	struct inode *ip;
	int cg;
	long bprev;
	int osize, nsize;
{
	register struct fs *fs;
	register struct buf *bp;
	register struct cg *cgp;
	long bno;
	int frags, bbase;
	int i;

	fs = ip->i_fs;
	if (fs->fs_cs(fs, cg).cs_nffree < numfrags(fs, nsize - osize))
		return (NULL);
	frags = numfrags(fs, nsize);
	bbase = fragnum(fs, bprev);
	if (bbase > fragnum(fs, (bprev + frags - 1))) {
		/* cannot extend across a block boundary */
		return (NULL);
	}

	bp = bread(ip->i_dev, (daddr_t)fragstoblks(fs, cgtod(fs, cg)),
		    (int)fs->fs_bsize);

	cgp = (struct cg *)bp->b_un.b_addr;
	if (bp->b_flags & B_ERROR || cgp->cg_magic != CG_MAGIC) {
		brelse(bp);
		return (NULL);
	}
	cgp->cg_time = hrestime.tv_sec;
	bno = dtogd(fs, bprev);
	for (i = numfrags(fs, osize); i < frags; i++)
		if (isclr(cgp->cg_free, bno + i)) {
			brelse(bp);
			return (NULL);
		}
	/*
	 * The current fragment can be extended,
	 * deduct the count on fragment being extended into
	 * increase the count on the remaining fragment (if any)
	 * allocate the extended piece.
	 */
	for (i = frags; i < fs->fs_frag - bbase; i++)
		if (isclr(cgp->cg_free, bno + i))
			break;
	cgp->cg_frsum[i - numfrags(fs, osize)]--;
	if (i != frags)
		cgp->cg_frsum[i - frags]++;
	for (i = numfrags(fs, osize); i < frags; i++) {
		clrbit(cgp->cg_free, bno + i);
		cgp->cg_cs.cs_nffree--;
		fs->fs_cs(fs, cg).cs_nffree--;
		fs->fs_cstotal.cs_nffree--;
	}
	fs->fs_fmod++;
	bdwrite(bp);
	return (bprev);
}

/*
 * Determine whether a block can be allocated.
 *
 * Check to see if a block of the apprpriate size
 * is available, and if it is, allocate it.
 */
STATIC daddr_t
alloccg(ip, cg, bpref, size)
	struct inode *ip;
	int cg;
	daddr_t bpref;
	int size;
{
	register struct fs *fs;
	register struct buf *bp;
	register struct cg *cgp;
	int bno, frags;
	int allocsiz;
	register int i;

	fs = ip->i_fs;
	if (fs->fs_cs(fs, cg).cs_nbfree == 0 && size == fs->fs_bsize)
		return (NULL);
	bp = bread(ip->i_dev, (daddr_t)fragstoblks(fs, cgtod(fs, cg)),
		    (int)fs->fs_bsize);
	cgp = (struct cg *)bp->b_un.b_addr;
	if (bp->b_flags & B_ERROR || cgp->cg_magic != CG_MAGIC ||
	    (cgp->cg_cs.cs_nbfree == 0 && size == fs->fs_bsize)) {
		brelse(bp);
		return (NULL);
	}
	cgp->cg_time = hrestime.tv_sec;
	if (size == fs->fs_bsize) {
		bno = alloccgblk(fs, cgp, bpref);
		bdwrite(bp);
		return (bno);
	}
	/*
	 * Check to see if any fragments are already available
	 * allocsiz is the size which will be allocated, hacking
	 * it down to a smaller size if necessary.
	 */
	frags = numfrags(fs, size);
	for (allocsiz = frags; allocsiz < fs->fs_frag; allocsiz++)
		if (cgp->cg_frsum[allocsiz] != 0)
			break;
	if (allocsiz == fs->fs_frag) {
		/*
		 * No fragments were available, so a block
		 * will be allocated and hacked up.
		 */
		if (cgp->cg_cs.cs_nbfree == 0) {
			brelse(bp);
			return (NULL);
		}
		bno = alloccgblk(fs, cgp, bpref);
		bpref = dtogd(fs, bno);
		for (i = frags; i < fs->fs_frag; i++)
			setbit(cgp->cg_free, bpref + i);
		i = fs->fs_frag - frags;
		cgp->cg_cs.cs_nffree += i;
		fs->fs_cstotal.cs_nffree += i;
		fs->fs_cs(fs, cg).cs_nffree += i;
		fs->fs_fmod++;
		cgp->cg_frsum[i]++;
		bdwrite(bp);
		return (bno);
	}
	bno = mapsearch(fs, cgp, bpref, allocsiz);
	if (bno < 0) {
		brelse(bp);
		return (NULL);
	}
	for (i = 0; i < frags; i++)
		clrbit(cgp->cg_free, bno + i);
	cgp->cg_cs.cs_nffree -= frags;
	fs->fs_cstotal.cs_nffree -= frags;
	fs->fs_cs(fs, cg).cs_nffree -= frags;
	fs->fs_fmod++;
	cgp->cg_frsum[allocsiz]--;
	if (frags != allocsiz)
		cgp->cg_frsum[allocsiz - frags]++;
	bdwrite(bp);
	return (cg * fs->fs_fpg + bno);
}

/*
 * Allocate a block in a cylinder group.
 *
 * This algorithm implements the following policy:
 *   1) allocate the requested block.
 *   2) allocate a rotationally optimal block in the same cylinder.
 *   3) allocate the next available block on the block rotor for the
 *      specified cylinder group.
 * Note that this routine only allocates fs_bsize blocks; these
 * blocks may be fragmented by the routine that allocates them.
 */
STATIC daddr_t
alloccgblk(fs, cgp, bpref)
	register struct fs *fs;
	register struct cg *cgp;
	daddr_t bpref;
{
	daddr_t bno;
	int cylno, pos, delta;
	short *cylbp;
	register int i;

	if (bpref == 0) {
		bpref = cgp->cg_rotor;
		goto norot;
	}
	bpref = blknum(fs, bpref);
	bpref = dtogd(fs, bpref);
	/*
	 * If the requested block is available, use it.
	 */
	if (isblock(fs, cgp->cg_free, (daddr_t)fragstoblks(fs, bpref))) {
		bno = bpref;
		goto gotit;
	}
	/*
	 * Check for a block available on the same cylinder.
	 */
	cylno = cbtocylno(fs, bpref);
	if (cgp->cg_btot[cylno] == 0)
		goto norot;
	if (fs->fs_cpc == 0) {
		/*
		 * Block layout info is not available, so just
		 * have to take any block in this cylinder.
		 */
		bpref = howmany(fs->fs_spc * cylno, NSPF(fs));
		goto norot;
	}
	/*
	 * Check the summary information to see if a block is 
	 * available in the requested cylinder starting at the
	 * requested rotational position and proceeding around.
	 */
	cylbp = cgp->cg_b[cylno];
	pos = cbtorpos(fs, bpref);
	for (i = pos; i < NRPOS; i++)
		if (cylbp[i] > 0)
			break;
	if (i == NRPOS)
		for (i = 0; i < pos; i++)
			if (cylbp[i] > 0)
				break;
	if (cylbp[i] > 0) {
		/*
		 * Found a rotational position, now find the actual
		 * block.  A panic if none is actually there.
		 */
		pos = cylno % fs->fs_cpc;
		bno = (cylno - pos) * fs->fs_spc / NSPB(fs);
		if (fs->fs_postbl[pos][i] == -1) {
			cmn_err(CE_PANIC,"alloccgblk: cyl groups corrupted,pos = %d, i = %d, fs = %s\n",
			    pos, i, fs->fs_fsmnt);
			
			/* NOTREACHED */
		}
		for (i = fs->fs_postbl[pos][i];; ) {
			if (isblock(fs, cgp->cg_free, (daddr_t)(bno + i))) {
				bno = blkstofrags(fs, (bno + i));
				goto gotit;
			}
			delta = fs->fs_rotbl[i];
			if (delta <= 0 || delta > MAXBPC - i)
				break;
			i += delta;
		}
		cmn_err(CE_PANIC, "alloccgblk: can't find blk in cyl, pos = %d, i = %d, fs = %s\n", pos, i, fs->fs_fsmnt);
		/* NOTREACHED */
	}
norot:
	/*
	 * No blocks in the requested cylinder, so take
	 * next available one in this cylinder group.
	 */
	bno = mapsearch(fs, cgp, bpref, (int)fs->fs_frag);
	if (bno < 0)
		return (NULL);
	cgp->cg_rotor = bno;
gotit:
	clrblock(fs, cgp->cg_free, (long)fragstoblks(fs, bno));
	cgp->cg_cs.cs_nbfree--;
	fs->fs_cstotal.cs_nbfree--;
	fs->fs_cs(fs, cgp->cg_cgx).cs_nbfree--;
	cylno = cbtocylno(fs, bno);
	cgp->cg_b[cylno][cbtorpos(fs, bno)]--;
	cgp->cg_btot[cylno]--;
	fs->fs_fmod++;
	return (cgp->cg_cgx * fs->fs_fpg + bno);
}

/*
 * Determine whether an inode can be allocated.
 *
 * Check to see if an inode is available, and if it is,
 * allocate it using the following policy:
 *   1) allocate the requested inode.
 *   2) allocate the next available inode after the requested
 *      inode in the specified cylinder group.
 */
STATIC ino_t
ialloccg(ip, cg, ipref, mode)
	struct inode *ip;
	int cg;
	daddr_t ipref;
	int mode;
{
	register struct fs *fs;
	register struct cg *cgp;
	struct buf *bp;
	int start, len, loc, map, i;

	fs = ip->i_fs;
	if (fs->fs_cs(fs, cg).cs_nifree == 0)
		return (NULL);
	bp = bread(ip->i_dev, (daddr_t)fragstoblks(fs, cgtod(fs, cg)),
		    (int)fs->fs_bsize);

	cgp = (struct cg *)bp->b_un.b_addr;
	if (bp->b_flags & B_ERROR || cgp->cg_magic != CG_MAGIC ||
	    cgp->cg_cs.cs_nifree == 0) {
		brelse(bp);
		return (NULL);
	}
	cgp->cg_time = hrestime.tv_sec;
	if (ipref) {
		ipref %= fs->fs_ipg;
		if (isclr(cgp->cg_iused, ipref))
			goto gotit;
	}
	start = cgp->cg_irotor / NBBY;
	len = howmany(fs->fs_ipg - cgp->cg_irotor, NBBY);
	loc = skpc(0xff, (u_int)len, &cgp->cg_iused[start]);
	if (loc == 0) {
		len = start + 1;
		start = 0;
		loc = skpc(0xff, (u_int)len, &cgp->cg_iused[0]);
		if (loc == 0) {
			brelse(bp);
			cmn_err(CE_PANIC, "ialloccg: map corrupted, cg = %s, irotor = %d, fs = %s\n",
			    cg, cgp->cg_irotor, fs->fs_fsmnt);
			/* NOTREACHED */
		}
	}
	i = start + len - loc;
	map = cgp->cg_iused[i];
	ipref = i * NBBY;
	for (i = 1; i < (1 << NBBY); i <<= 1, ipref++) {
		if ((map & i) == 0) {
			cgp->cg_irotor = ipref;
			goto gotit;
		}
	}

	cmn_err(CE_PANIC, "ialloccg: block not in mapfs = %s\n", fs->fs_fsmnt);
	/* NOTREACHED */
gotit:
	setbit(cgp->cg_iused, ipref);
	cgp->cg_cs.cs_nifree--;
	fs->fs_cstotal.cs_nifree--;
	fs->fs_cs(fs, cg).cs_nifree--;
	fs->fs_fmod++;
	if ((mode & IFMT) == IFDIR) {
		cgp->cg_cs.cs_ndir++;
		fs->fs_cstotal.cs_ndir++;
		fs->fs_cs(fs, cg).cs_ndir++;
	}
	bdwrite(bp);
	return (cg * fs->fs_ipg + ipref);
}

/*
 * Find a block of the specified size in the specified cylinder group.
 *
 * It is a panic if a request is made to find a block if none are
 * available.
 */
STATIC daddr_t
mapsearch(fs, cgp, bpref, allocsiz)
	register struct fs *fs;
	register struct cg *cgp;
	daddr_t bpref;
	int allocsiz;
{
	daddr_t bno;
	int start, len, loc, i;
	int blk, field, subfield, pos;

	/*
	 * Find the fragment by searching through the
	 * free block map for an appropriate bit pattern.
	 */
	if (bpref)
		start = dtogd(fs, bpref) / NBBY;
	else
		start = cgp->cg_frotor / NBBY;
	len = howmany(fs->fs_fpg, NBBY) - start;
	loc = scanc((unsigned)len, (u_char *)&cgp->cg_free[start],
	    (u_char *)fragtbl[fs->fs_frag],
	    (u_char)(1 << (allocsiz - 1 + (fs->fs_frag % NBBY))));
	if (loc == 0) {
		len = start + 1;
		start = 0;
		loc = scanc((unsigned)len, (u_char *)&cgp->cg_free[0],
		    (u_char *)fragtbl[fs->fs_frag],
		    (u_char)(1 << (allocsiz - 1 + (fs->fs_frag % NBBY))));
		if (loc == 0) {
			cmn_err(CE_PANIC, "mapsearch: map corrupted, start = %d, len = %d, fs = %s\n",
			    start, len, fs->fs_fsmnt);

			/* NOTREACHED */
		}
	}
	bno = (start + len - loc) * NBBY;
	cgp->cg_frotor = bno;
	/*
	 * Found the byte in the map, sift
	 * through the bits to find the selected frag
	 */
	for (i = bno + NBBY; bno < i; bno += fs->fs_frag) {
		blk = blkmap(fs, cgp->cg_free, bno);
		blk <<= 1;
		field = around[allocsiz];
		subfield = inside[allocsiz];
		for (pos = 0; pos <= fs->fs_frag - allocsiz; pos++) {
			if ((blk & field) == subfield)
				return (bno + pos);
			field <<= 1;
			subfield <<= 1;
		}
	}
	cmn_err(CE_PANIC, "mapsearch: block not in map, bno = %d, fs = %s\n", bno, fs->fs_fsmnt);

	/* NOTREACHED */
}

#define UFSNADDR (NDADDR + NIADDR)/* NADDR applies to S5 only */
#define IB(i)	(NDADDR + (i))	/* index of i'th indirect block ptr */
#define SINGLE	0		/* single indirect block ptr */
#define DOUBLE	1		/* double indirect block ptr */
#define TRIPLE	2		/* triple indirect block ptr */

/*
 * Free storage space associated with the specified inode.  The portion
 * to be freed is specified by lp->l_start and lp->l_len (already
 * normalized to a "whence" of 0).

 * This is an experimental facility whose continued existence is not
 * guaranteed.  Currently, we only support the special case
 * of l_len == 0, meaning free to end of file.
 *
 * Blocks are freed in reverse order.  This FILO algorithm will tend to
 * maintain a contiguous free list much longer than FIFO.
 * See also ufs_itrunc() in ufs_inode.c.
 *
 * Bug: unused bytes in the last retained block are not cleared.
 * This may result in a "hole" in the file that does not read as zeroes.
 */
/* ARGSUSED */
int
ufs_freesp(vp, lp, flag)
	register struct vnode *vp;
	register struct flock *lp;
	int flag;
{
	register int i;
	register struct inode *ip = VTOI(vp);
	int error;
	
	ASSERT(vp->v_type == VREG);
	ASSERT(lp->l_start >= 0);	/* checked by convoff */

	if (lp->l_len != 0)
		return EINVAL;
	if (ip->i_size == lp->l_start)
		return 0;

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

	ILOCK(ip);

 	if (vp->v_type == VREG && (error = fs_vcode(vp, &ip->i_vcode))) {
 		IUNLOCK(ip);
 		return error;
 	}

	error = ufs_itrunc(ip, lp->l_start);
	IUNLOCK(ip);
	return error;
}

STATIC void
ufsindirtrunc(vfsp, fs, bn, lastbn, level, ip)
	register struct vfs *vfsp;
	struct fs *fs;
	daddr_t bn, lastbn;
	int level;
	struct inode *ip;
{
	register int i;
	register struct buf *bp, *copy;
	register daddr_t *bap;
	long bsize, factor;
	long nindir;
	daddr_t last;

	/*
	 * Calculate index in current block of last block (pointer) to be kept.
	 * A lastbn of -1 indicates that the entire block is going away, so we
	 * need not calculate the index.
	 */

	bsize = vfsp->vfs_bsize;
	nindir = NINDIR(fs);
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
	bp = bread(vfsp->vfs_dev, (daddr_t)fragstoblks(fs, bn), bsize);

	if (bp->b_flags & B_ERROR) {
		brelse(copy);
		brelse(bp);
		return;
	}
	bap = bp->b_un.b_daddr;
	bcopy((caddr_t)bap, copy->b_un.b_addr, fs->fs_bsize);
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
				ufsindirtrunc(vfsp, fs, bn, (daddr_t)-1, level-1, ip);
			(void)free(ip, bn, bsize);
		}

	/*
	 * Recursively free last partial block.
	 */

	if (level > SINGLE && lastbn >= 0) {
		last = lastbn % factor;
		if ((bn = bap[i]) != 0)
			ufsindirtrunc(vfsp, fs, bn, last, level-1, ip);
	}

	brelse(copy);
	return;
}
