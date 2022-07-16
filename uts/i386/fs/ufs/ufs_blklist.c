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

#ident	"@(#)kern-fs:ufs/ufs_blklist.c	1.3.1.4"

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
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/mode.h"
#include "sys/user.h"
#include "sys/kmem.h"

#include "vm/pvn.h"

#include "sys/proc.h"	/* XXX -- needed for user-context kludge in ILOCK */
#include "sys/disp.h"	/* XXX */

#include "sys/fs/ufs_fs.h"
#include "sys/fs/ufs_inode.h"
#include "fs/fs_subr.h"


/*
 *  Allocate and build the block address map
 */

ufs_allocmap(ip)
register struct inode *ip;
{
	register int	*bnptr;
	register int	bsize;
	register int	nblks;
	register int	npblks;
	register struct vnode *vp;
	int err = 0;

	vp = ITOV(ip);
	if (ip->i_map) 
		return err;

	/*
	 * Get number of blocks to be mapped.
	 */

	ASSERT(ip->i_map == 0);
	bsize = VBSIZE(vp);
	nblks = (ip->i_size + bsize - 1)/bsize;
	if (nblks == 0)
		return 0;

	if (PAGESIZE > bsize) {
		int bpp;

		bpp = PAGESIZE/bsize;
		npblks = ((ip->i_size + PAGESIZE) >> PAGESHIFT) * bpp;
		ASSERT(npblks >= nblks);
	} else
		npblks = nblks;


	bnptr = (int *)kmem_alloc(sizeof(int)*npblks, KM_NOSLEEP);
	if (bnptr == NULL)
		return ENOMEM;

	/*
	 * Build the actual list of block numbers
	 * for the file.
	 */

	if ((err = ufs_bldblklst(bnptr, ip, nblks)) == 0) {
		/*
		 * If the size is not an integral number of
		 * pages long, then the last few block
		 * number up to the next page boundary are
		 * made zero so that no one will try to
		 * read them in.
		 */
		while (nblks < npblks)
			bnptr[nblks++] = 0;
		ip->i_map = bnptr;
#ifdef DEBUG
		ip->i_mapsz = npblks;
		ip->i_oldsz = ip->i_size;
#endif
	} else
		kmem_free(bnptr, sizeof(int) * npblks);

	return err;
}

/*	Build the list of block numbers for a file.  This is used
 *	for mapped files.
 */

ufs_bldblklst(lp, ip, nblks)
register int		*lp;
register struct inode	*ip;
register int		nblks;
{
	register int	lim;
	register int	*eptr;
	register int	i;
	register struct vnode *vp;
	int		*ufs_bldindr();
	dev_t	 dev;

	/*
	 * Get the block numbers from the direct blocks first.
	 */

	vp = ITOV(ip);
	eptr = &lp[nblks];
	if (nblks < NDADDR)
		lim = nblks;
	else
		lim = NDADDR;
	
	for (i = 0;  i < lim;  i++)
		*lp++ = ip->i_db[i];
	
	if (lp >= eptr)
		return 0;
	
	dev = vp->v_vfsp->vfs_dev;
	i = 0;
	while (lp < eptr) {
		lp = ufs_bldindr(ip, lp, eptr, dev, ip->i_ib[i], i);
		if (lp == 0)
			return 1;
		i++;
	}
	return 0;
}

int  *
ufs_bldindr(ip, lp, eptr, dev, blknbr, indlvl)
struct inode 		*ip;
register int		*lp;
register int		*eptr;
register dev_t		dev;
int			blknbr;
int			indlvl;
{
	register struct buf *bp;
	register int	*bnptr;
	int		cnt;
	struct buf 	*bread();
	int 		bsize;
	struct vnode	*vp;
	struct fs *fs;
	int sksize;

	vp = ITOV(ip);
	bsize = vp->v_vfsp->vfs_bsize;
	if (blknbr == 0) {
		sksize = 1;
		for (cnt = 0; cnt <= indlvl; cnt++)
			sksize *= (bsize/sizeof(int));
		if (eptr - lp < sksize)
			sksize = eptr - lp;
		for (cnt = 0; cnt < sksize; cnt++)
			*lp++ = 0;
		return lp;
	}
	fs = getfs(vp->v_vfsp);
	bp = bread(dev, fragstoblks(fs, blknbr), bsize);
	if (bp->b_flags & B_ERROR) {
		brelse(bp);
		return((int *) 0);
	}
	bnptr = bp->b_un.b_words;
	cnt = NINDIR(getfs(vp->v_vfsp));
	
	ASSERT(indlvl >= 0);
	while (cnt--  &&  lp < eptr) {
		if (indlvl == 0) {
			*lp++ = *bnptr++;
		} else {
			lp = ufs_bldindr(ip, lp, eptr, dev, *bnptr++, indlvl-1);
			if (lp == 0) {
				brelse(bp);
				return((int *) 0);
			}
		}
	}

	brelse(bp);
	return(lp);
}

/*	Free the block list attached to an inode.
 */

void
ufs_freemap(ip)
struct inode	*ip;
{
	register int	nblks;
	register int	npblks;
	register	bsize;
	register struct vnode *vp;
	register int	type;
	register int	*bnptr;

	vp = ITOV(ip);
	ASSERT(ip->i_flag & ILOCKED);
	
	type = ip->i_mode & IFMT;
	if (type != IFREG || ip->i_map == NULL)
		return;

	bsize = VBSIZE(vp);
	nblks = (ip->i_size + bsize - 1)/bsize;

	if (PAGESIZE > bsize) {
		int bpp;

		bpp = PAGESIZE/bsize;
		nblks = ((ip->i_size + PAGEOFFSET) >> PAGESHIFT) * bpp;
	} else
		nblks = (ip->i_size + bsize -1)/bsize;

#ifdef DEBUG
	if (nblks != ip->i_mapsz) {
		printf("ufs_freemap: i_mapsz 0x%x i_oldsz 0x%x nblks 0x%x\n",
			ip->i_mapsz, ip->i_oldsz, nblks);
		call_demon();
	}
#endif

	bnptr = ip->i_map;
	ip->i_map = NULL;
	kmem_free((caddr_t)bnptr, nblks*sizeof(int));

#ifdef DEBUG
	ip->i_mapsz = 0;
	ip->i_oldsz = 0;
#endif
	return;
}
