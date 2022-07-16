/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:s5/s5blklist.c	1.2.1.3"

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

#include "sys/fs/s5param.h"
#include "sys/fs/s5fblk.h"
#include "sys/fs/s5filsys.h"
#include "sys/fs/s5ino.h"
#include "sys/fs/s5inode.h"
#include "sys/fs/s5macros.h"

#include "fs/fs_subr.h"

STATIC int	s5bldblklst();
STATIC int	s5bldindr();

/*
 * Allocate and build the block address map.
 */
int
s5allocmap(ip)
	register struct inode *ip;
{
	register int	*bnptr;
	register int	bsize;
	register int	nblks;
	register int	npblks;
	register struct vnode *vp = ITOV(ip);
	int err = 0;

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
		npblks = nblks + bpp - 1;
		npblks /= bpp;
		npblks *= bpp;
	} else
		npblks = nblks;

	bnptr = (int *)kmem_alloc(sizeof(int)*npblks, KM_NOSLEEP);
	if (bnptr == NULL)
		return ENOMEM;

	/*
	 * Build the actual list of block numbers for the file.
	 */
	if ((err = s5bldblklst(bnptr, ip, nblks)) == 0) {
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

/*
 * Build the list of block numbers for a file.  This is used
 * for mapped files.
 */
STATIC
int
s5bldblklst(lp, ip, nblks)
	int	*lp;
	register struct inode	*ip;
	register int		nblks;
{
	register int	lim;
	register int	*eptr;
	register int	i;
	register struct vnode *vp = ITOV(ip);
	int err;
	dev_t	 dev;

	/*
	 * Get the block numbers from the direct blocks first.
	 */
	eptr = &lp[nblks];
	lim = (nblks < NADDR-3) ? nblks : NADDR-3;
	
	for (i = 0; i < lim; i++)
		*lp++ = ip->i_addr[i];
	
	if (lp >= eptr)
		return 0;
	
	dev = vp->v_vfsp->vfs_dev;
	while (lp < eptr) {
		err = s5bldindr(ip, &lp, eptr, dev, ip->i_addr[i], i-(NADDR-3));
		if (err)
			return err;
		i++;
	}
	return 0;
}

STATIC
int 
s5bldindr(ip, lp, eptr, dev, blknbr, indlvl)
	struct inode 		*ip;
	register int		**lp;
	register int		*eptr;
	register dev_t	dev;
	int			blknbr;
	int			indlvl;
{
	register struct buf *bp;
	register int	*bnptr;
	int		cnt;
	struct s5vfs	*s5vfsp;
	int err = 0;
	int 		bsize, sksize;
	struct vnode	*vp = ITOV(ip);

	bsize = VBSIZE(vp);
	if (blknbr == 0){
		sksize = 1;
		for (cnt=0; cnt < (indlvl + 1); cnt++)
			sksize *= (bsize/sizeof(int));

		if (eptr - *lp < sksize)
			sksize = eptr - *lp;

		for (cnt=0; cnt < sksize; cnt++)
			*(*lp)++ = 0;
		return 0;
	}

	bp = bread(dev, blknbr, bsize);
	if (bp->b_flags & B_ERROR) {
		brelse(bp);
		return ENXIO;
	}
	bnptr = bp->b_un.b_words;
	s5vfsp = S5VFS(vp->v_vfsp);
	cnt = s5vfsp->vfs_nindir;
	
	ASSERT(indlvl >= 0);
	while (cnt-- && *lp < eptr) {
		if (indlvl == 0)
			*(*lp)++ = *bnptr++;
		else {
			err = s5bldindr(ip, lp, eptr, dev, *bnptr++, indlvl-1);
			if (err) 
				break;
		}
	}

	brelse(bp);
	return err;
}

/*
 * Free the block list attached to an inode.
 */
s5freemap(ip)
	struct inode	*ip;
{
	register int	nblks;
	register	bsize;
	register struct vnode *vp = ITOV(ip);
	register int	*bnptr;

	ASSERT(ip->i_flag & ILOCKED);

	if (vp->v_type != VREG || ip->i_map == NULL)
		return 0;

	bsize = VBSIZE(vp);
	nblks = (ip->i_size + bsize - 1)/bsize;
	if (PAGESIZE > bsize) {
		int bpp;

		bpp = PAGESIZE/bsize;
		nblks += bpp - 1;
		nblks /= bpp;
		nblks *= bpp;
	}

#ifdef DEBUG
	if (nblks != ip->i_mapsz) {
		printf("s5freemap: i_mapsz 0x%x i_oldsz 0x%x nblks 0x%x\n",
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
	return 0;
}
