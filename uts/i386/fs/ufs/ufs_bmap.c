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

#ident  "@(#)kern-fs:ufs/ufs_bmap.c	1.3.1.5"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/buf.h>
#include <sys/disp.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>
#include <vm/seg.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/vfs.h>
#include <sys/cmn_err.h>
#include <sys/debug.h>

/*
 * ufs_bmap defines the structure of file system storage by mapping
 * a logical block number in a file to a physical block number
 * on the device.  It should be called with a locked inode when
 * allocation is to be done.
 *
 * ufs_bmap translates logical block number lbn to a physical block
 * number and returns it in *bnp, possibly along with a read-ahead
 * block number in *rabnp.  bnp and rabnp can be NULL if the
 * information is not required.  rw specifies whether the mapping
 * is for read or write.  If for write, the block must be at least
 * size bytes and will be extended or allocated as needed.  If
 * alloc_only is set, ufs_bmap may not create any in-core pages
 * that correspond to the new disk allocation.  Otherwise, the in-core
 * pages will be created and initialized as needed.
 *
 * Returns 0 on success, or a non-zero errno if an error occurs.
 */
int
ufs_bmap(ip, lbn, bnp, rabnp, size, rw, alloc_only)
	register struct inode *ip;	/* file to be mapped */
	daddr_t lbn;		/* logical block number */
	daddr_t *bnp;		/* mapped block number */
	daddr_t *rabnp;		/* read-ahead block */
	int size;		/* block size */
	enum seg_rw rw;		/* S_READ, S_WRITE, or S_OTHER */
	int alloc_only;		/* allocate disk blocks but create no pages */
{
	register struct fs *fs;
	register struct buf *bp;
	register int i;
	struct buf *nbp;
	int j, sh;
	daddr_t ob, nb, pref, llbn, tbn, *bap;
	struct vnode *vp = ITOV(ip);
	long bsize = VBSIZE(vp);
	long osize, nsize;
	int issync, isdir;
	int err;
	dev_t dev;
	struct vfs *vfsp = vp->v_vfsp;
	struct vnode *devvp = ((struct ufsvfs *)vfsp->vfs_data)->vfs_devvp;
	struct fbuf *fbp;
	int nblks;
	int blkpp;

	ASSERT(rw != S_WRITE ||
		((ip->i_flag & ILOCKED) && ip->i_owner == curproc->p_slot));

	if (lbn < 0)
		return (EFBIG);

	blkpp = PAGESIZE >= bsize ? PAGESIZE/bsize : 0;
	if (ip->i_map) {
		if (PAGESIZE > bsize)
			nblks = ((ip->i_size + PAGEOFFSET) >> PAGESHIFT) * blkpp;
		else
			nblks = (ip->i_size + bsize - 1)/bsize;
#ifdef DEBUG
		if (nblks != ip->i_mapsz) {
		    printf("ufs_bmap: i_mapsz 0x%x i_oldsz 0x%x nblks 0x%x\n",
				ip->i_mapsz, ip->i_oldsz, nblks);
		    call_demon();
		}
#endif
		/*
		 * In the S_WRITE case, we ignore the last block in the i_map,
		 * since it may contain fragments which need to be reallocated.
		 */
		if (lbn < (rw == S_WRITE ? nblks - 1 : nblks))
			tbn = ip->i_map[lbn];
		else
			tbn = 0;
		if (tbn == 0) {
			if (rw == S_WRITE) {
				ufs_freemap(ip);
				goto lbmap;
			}
			if (bnp)
				*bnp = UFS_HOLE;
			if (rabnp)
				*rabnp = UFS_HOLE;
			return 0;
		}
		if (bnp)
			*bnp = tbn;
		if (rabnp) {
			if ((lbn + 1) >= nblks) {
				*rabnp = UFS_HOLE;
			} else {
				*rabnp = ip->i_map[lbn+1];
				if (*rabnp == 0)
					*rabnp = UFS_HOLE;
			}
		}
		return 0;
	}
lbmap:
	fs = ip->i_fs;
	llbn = lblkno(fs, ip->i_size - 1);
	isdir = ((ip->i_mode & IFMT) == IFDIR);
	issync = ((ip->i_flag & ISYNC) != 0);
	if (isdir || issync)
		alloc_only = 0;		/* make sure */

	/*
	 * If the next write will extend the file into a new block,
	 * and the file is currently composed of a fragment
	 * this fragment has to be extended to be a full block.
	 */
	if (rw == S_WRITE && llbn < NDADDR && llbn < lbn &&
	    (ob = ip->i_db[llbn]) != 0) {
		osize = blksize(fs, ip, llbn);
		if (osize < bsize && osize > 0) {
			addr_t zptr;
			/*
			 * Make sure we have all needed pages setup correctly.
			 */
			err = fbread(ITOV(ip), (u_int)(llbn << fs->fs_bshift),
			    bsize, S_OTHER, &fbp);
			if (err)
				return (err);
			zptr = fbp->fb_addr + osize;
			bzero(zptr, bsize-osize);

			pref = blkpref(ip, llbn, (int)llbn, &ip->i_db[0]);
			err = realloccg(ip, ob, pref, osize, bsize, &nb);
			if (err) {
				if(fbp)
					fbrelse(fbp, S_OTHER);
				cmn_err(CE_NOTE, "ufs_bmap: realloccg failed\n");
				return (err);
			}
			/*
			 * Don't check isdir here, directories won't do this
			 */
			if (issync)
				(void) fbiwrite(fbp, devvp, nb, fs->fs_fsize);
			else
				fbrelse(fbp, S_WRITE);

			ip->i_size = (llbn + 1) << fs->fs_bshift;
			ip->i_db[llbn] = nb;
			ip->i_flag |= IUPD | ICHG;
			ip->i_blocks += btodb(bsize - osize);

			if (nb != ob)
				free(ip, ob, (off_t)osize);
		}
	}

	/*
	 * The first NDADDR blocks are direct blocks.
	 */
	if (lbn < NDADDR) {
		nb = ip->i_db[lbn];
		if (rw != S_WRITE)
			goto gotit;

		if (nb == 0 || ip->i_size < (lbn + 1) << fs->fs_bshift) {
			if (nb != 0) {
				/* consider need to reallocate a frag */
				osize = fragroundup(fs, blkoff(fs, ip->i_size));
				nsize = fragroundup(fs, size);
				if (nsize <= osize)
					goto gotit;

				/* need to allocate a block or frag */
				ob = nb;
				pref = blkpref(ip, lbn, (int)lbn, &ip->i_db[0]);
				err = realloccg(ip, ob, pref, osize, nsize,
				    &nb);
				if (err)
					return (err);
			} else {
				/* need to allocate a block or frag */
				osize = 0;
				if (ip->i_size < (lbn + 1) << fs->fs_bshift)
					nsize = fragroundup(fs, size);
				else
					nsize = bsize;
				pref = blkpref(ip, lbn, (int)lbn, &ip->i_db[0]);
				err = alloc(ip, pref, nsize, &nb);
				if (err)
					return (err);
				ob = nb;
			}

			/*
			 * Read old/create new zero pages
			 */
			fbp = NULL;
			if (!alloc_only || osize != 0) {
				err = fbread(ITOV(ip),
				    (long)(lbn << fs->fs_bshift),
				    nsize, S_OTHER, &fbp);
				if (err) {
					if (nb != ob) {
						free(ip, nb, (off_t)nsize);
					} else {
						free(ip,
						    ob + numfrags(fs, osize),
						    (off_t)(nsize - osize));
					}
#ifdef QUOTA
					(void) chkdq(ip,
					    -(long)btodb(nsize - osize), 0);
#endif /* QUOTA */
					return (err);
				}
			}

			/*
			 * Write directory blocks synchronously so that they
			 * never appear with garbage in them on the disk.
			 */
			if (isdir)
				(void) fbiwrite(fbp, devvp, nb, fs->fs_fsize);
			else if (fbp)
				fbrelse(fbp, S_WRITE);
			ip->i_db[lbn] = nb;
			ip->i_blocks += btodb(nsize - osize);
			ip->i_flag |= IUPD | ICHG;

			if (nb != ob)
				free(ip, ob, (off_t)osize);
		}
gotit:
		if (bnp != NULL)
			*bnp = (nb == 0)? UFS_HOLE : nb;
		if (rabnp != NULL) {
			nb = ip->i_db[lbn + 1];
			*rabnp = (nb == 0 || lbn >= NDADDR - 1) ?
			  UFS_HOLE : nb;
		}
		return (0);
	}

	/*
	 * Determine how many levels of indirection.
	 */
	pref = 0;
	sh = 1;
	tbn = lbn - NDADDR;
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(fs);
		if (tbn < sh)
			break;
		tbn -= sh;
	}

	if (j == 0)
		return (EFBIG);

	/*
	 * Fetch the first indirect block.
	 */
	dev = ip->i_dev;
	nb = ip->i_ib[NIADDR - j];
	if (nb == 0) {
		if (rw != S_WRITE) {
			if (bnp != NULL)
				*bnp = UFS_HOLE;
			if (rabnp != NULL)
				*rabnp = UFS_HOLE;
			return (0);
		}
		/*
		 * Need to allocate an indirect block.
		 */
		pref = blkpref(ip, lbn, 0, (daddr_t *)0);
		err = alloc(ip, pref, bsize, &nb);
		if (err)
			return (err);
		/*
		 * Write zero block synchronously so that
		 * indirect blocks never point at garbage.
		 */
		bp = getblk(dev, fragstoblks(fs, nb), bsize);

		clrbuf(bp);
		bwrite(bp);

		ip->i_ib[NIADDR - j] = nb;
		ip->i_blocks += btodb(bsize);
		ip->i_flag |= IUPD | ICHG;

		/*
		 * In the ISYNC case, rwip will notice that the block
		 * count on the inode has changed and will be sure to
		 * ufs_iupdat the inode at the end of rwip.
		 */
	}

	/*
	 * Fetch through the indirect blocks.
	 */
	for (; j <= NIADDR; j++) {
		ob = nb;
		bp = bread(ip->i_dev, fragstoblks(fs, ob), bsize);

		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			return (EIO);
		}
		bap = bp->b_un.b_daddr;
		sh /= NINDIR(fs);
		i = (tbn / sh) % NINDIR(fs);
		nb = bap[i];
		if (nb == 0) {
			if (rw != S_WRITE) {
				brelse(bp);
				if (bnp != NULL)
					*bnp = UFS_HOLE;
				if (rabnp != NULL)
					*rabnp = UFS_HOLE;
				return (0);
			}
			if (pref == 0) {
				if (j < NIADDR) {
					/* Indirect block */
					pref = blkpref(ip, lbn, 0,
						(daddr_t *)0);
				} else {
					/* Data block */
					pref = blkpref(ip, lbn, i, &bap[0]);
				}
			}

			err = alloc(ip, pref, bsize, &nb);
			if (err) {
				brelse(bp);
				return (err);
			}

			if (j < NIADDR) {
				/*
				 * Write synchronously so indirect
				 * blocks never point at garbage.
				 */
				nbp = getblk(dev, fragstoblks(fs,nb), bsize);

				clrbuf(nbp);
				bwrite(nbp);
			} else if (!alloc_only ||
			    roundup(size, PAGESIZE) < bsize) {
				/*
				 * To avoid deadlocking if the pageout
				 * daemon decides to push a page for this
				 * inode while we are sleeping holding the
				 * bp but waiting more pages for fbread,
				 * we give up the bp now.
				 *
				 * XXX - need to avoid having the pageout
				 * daemon get in this situation to begin with!
				 */
				brelse(bp);
				err = fbread(ITOV(ip),
				    (long)(lbn << fs->fs_bshift),
				    bsize, S_OTHER, &fbp);
				if (err) {
					free(ip, nb, (off_t)bsize);
#ifdef QUOTA
					(void) chkdq(ip, -(long)bsize, 0);
#endif /* QUOTA */
					return (err);
				}

				/*
				 * Cases which we need to do a synchronous
				 * write of the zeroed data pages:
				 *
				 * 1) If we are writing a directory then we
				 * want to write synchronously so blocks in
				 * directories never contain garbage.
				 *
				 * 2) If we are filling in a hole and the
				 * indirect block is going to be synchronously
				 * written back below we need to make sure
				 * that the zeroes are written here before
				 * the indirect block is updated so that if
				 * we crash before the real data is pushed
				 * we will not end up with random data is
				 * the middle of the file.
				 *
				 * 3) If the size of the request rounded up
				 * to the system page size is smaller than
				 * the file system block size, we want to
				 * write out all the pages now so that
				 * they are not aborted before they actually
				 * make it to ufs_putpage since the length
				 * of the inode will not include the pages.
				 */
				if (isdir || (issync && lbn < llbn) ||
				    roundup(size, PAGESIZE) < bsize)
					(void) fbiwrite(fbp, devvp, nb,
						fs->fs_fsize);
				else
					fbrelse(fbp, S_WRITE);

				/*
				 * Now get the bp back
				 */
				bp = bread(ip->i_dev, fragstoblks(fs, ob),
					bsize);

				err = geterror(bp);
				if (err) {
					free(ip, nb, (off_t)bsize);
#ifdef QUOTA
					(void) chkdq(ip, -(long)btodb(bsize),
					    0);
#endif /* QUOTA */
					brelse(bp);
					return (err);
				}
				bap = bp->b_un.b_daddr;
			}

			bap[i] = nb;
			ip->i_blocks += btodb(bsize);
			ip->i_flag |= IUPD | ICHG;

			if (issync)
				bwrite(bp);
			else
				bdwrite(bp);
		} else {
			brelse(bp);
		}
	}
	if (bnp != NULL)
		*bnp = nb;
	if (rabnp != NULL) {
		if (i < NINDIR(fs) - 1) {
			nb = bap[i + 1];
			*rabnp = (nb == 0) ? UFS_HOLE : nb;
		} else {
			*rabnp = UFS_HOLE;
		}
	}
	return (0);
}
