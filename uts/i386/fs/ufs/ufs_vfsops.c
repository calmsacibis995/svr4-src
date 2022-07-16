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

#ident	"@(#)kern-fs:ufs/ufs_vfsops.c	1.3.3.10"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysmacros.h>
#include <sys/kmem.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/disp.h>
#include <sys/buf.h>
#include <sys/pathname.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/conf.h>
#include <sys/fs/ufs_fsdir.h>
#include <sys/fs/ufs_fs.h>
#include <sys/fs/ufs_inode.h>
#undef NFS
#include <sys/statvfs.h>
#include <sys/mount.h>
#include <sys/swap.h>
#include <sys/errno.h>
#include <sys/debug.h>
#include "fs/fs_subr.h"

struct vnode *makespecvp();

extern int ufsfstype;

/*
 * ufs vfs operations.
 */
STATIC int ufs_mount();
STATIC int ufs_unmount();
STATIC int ufs_root();
STATIC int ufs_statvfs();
STATIC int ufs_sync();
STATIC int ufs_vget();
STATIC int ufs_mountroot();
STATIC int ufs_swapvp();	/* XXX */
void sbupdate();

struct vfsops ufs_vfsops = {
	ufs_mount,
	ufs_unmount,
	ufs_root,
	ufs_statvfs,
	ufs_sync,
	ufs_vget,
	ufs_mountroot,
	ufs_swapvp,	/* XXX - swapvp */
	fs_nosys,	/* filler */
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
};

extern int ufsfstype;

/*
 * XXX - this appears only to be used by the VM code to handle the case where
 * UNIX is running off the mini-root.  That probably wants to be done
 * differently.
 */
struct vnode *rootvp;

STATIC int
ufs_mount(vfsp, mvp, uap, cr)
	register struct vfs *vfsp;
	struct vnode *mvp;
	struct mounta *uap;
	struct cred *cr;
{
	register dev_t dev;
	struct vnode *bvp;
	struct pathname dpn;
	register int error;
	enum whymountroot why;

	if (!suser(cr))
		return (EPERM);

	if (mvp->v_type != VDIR)
		return (ENOTDIR);
	if ((uap->flags & MS_REMOUNT) == 0 &&
		(mvp->v_count != 1 || (mvp->v_flag & VROOT)))
			return (EBUSY);

	/*
	 * Get arguments
	 */
	if (error = pn_get(uap->dir, UIO_USERSPACE, &dpn)) { 

		return (error);
	}

	/*
	 * Resolve path name of special file being mounted.
	 */
	if (error = lookupname(uap->spec, UIO_USERSPACE, FOLLOW, NULLVPP, &bvp)) {
		pn_free(&dpn);
		return (error);
	}
	if (bvp->v_type != VBLK) {
		VN_RELE(bvp);
		pn_free(&dpn);
		return (ENOTBLK);
	}
	dev = bvp->v_rdev;
	VN_RELE(bvp);
	/*
	 * Ensure that this device isn't already mounted,
	 * unless this is a REMOUNT request
	 */
	if (vfs_devsearch(dev) != NULL) {
		if (uap->flags & MS_REMOUNT)
			why = ROOT_REMOUNT;
		else {
			pn_free(&dpn);
			return EBUSY;
		}
	} else {
		why = ROOT_INIT;
	}	
	if (getmajor(dev) >= bdevcnt) {
		pn_free(&dpn);
		return (ENXIO);
	}

	/*
	 * If the device is a tape, mount it read only
	 */
	if ((*bdevsw[getmajor(dev)].d_flag & D_TAPE) == D_TAPE)
		vfsp->vfs_flag |= VFS_RDONLY;

	if (uap->flags & MS_RDONLY)
		vfsp->vfs_flag |= VFS_RDONLY;

	/*
	 * Mount the filesystem.
	 */
	error = mountfs(vfsp, why, dev, dpn.pn_path, cr, 0);
	pn_free(&dpn);
	return (error);
}

/*
 * Mount root file system.
 * "why" is ROOT_INIT on initial call, ROOT_REMOUNT if called to
 * remount the root file system, and ROOT_UNMOUNT if called to
 * unmount the root (e.g., as part of a system shutdown).
 *
 * XXX - this may be partially machine-dependent; it, along with the VFS_SWAPVP
 * operation, goes along with auto-configuration.  A mechanism should be
 * provided by which machine-INdependent code in the kernel can say "get me the
 * right root file system" and "get me the right initial swap area", and have
 * that done in what may well be a machine-dependent fashion.
 * Unfortunately, it is also file-system-type dependent (NFS gets it via
 * bootparams calls, UFS gets it from various and sundry machine-dependent
 * mechanisms, as SPECFS does for swap).
 */
STATIC int
ufs_mountroot(vfsp, why)
	struct vfs *vfsp;
	enum whymountroot why;
{
	register struct fs *fsp;
	register int error;
	static int ufsrootdone = 0;
	register struct buf *bp;
	struct vnode *vp;
	register struct ufsvfs *ufsvfsp = (struct ufsvfs *)vfsp->vfs_data;

	if (why == ROOT_INIT) {
		if (ufsrootdone++)
			return (EBUSY);
		if (rootdev == (dev_t)NODEV)
			return (ENODEV);
		vfsp->vfs_dev = rootdev;
#ifdef notneeded
		if ((boothowto & RB_WRITABLE) == 0) {
			/*
			 * We mount a ufs root file system read-only to
			 * avoid problems during fsck.   After fsck runs,
			 * we remount it read-write.
			 */
			vfsp->vfs_flag |= VFS_RDONLY;
		}
#endif
	}
	else  if (why == ROOT_REMOUNT) {
  		fsp = getfs(vfsp);
		if (fsp->fs_state == FSACTIVE)
			return EINVAL;
		vfsp->vfs_flag |= VFS_REMOUNT;
	}		
	else if (why == ROOT_UNMOUNT) {
		ufs_update();
		fsp = getfs(vfsp);
/***
		if (fsp->fs_state == FSACTIVE)
****/		{
			fsp->fs_time = hrestime.tv_sec;
			if (vfsp->vfs_flag & VFS_BADBLOCK)
				fsp->fs_state = FSBAD;
			else
				fsp->fs_state = FSOKAY - (long)fsp->fs_time;
			vp = ((struct ufsvfs *)vfsp->vfs_data)->vfs_devvp;
			sbupdate(vfsp);
			(void) VOP_CLOSE(vp, FREAD|FWRITE, 1, (off_t)0, u.u_cred);
			VN_RELE(vp);
		}
		bdwait();
		return 0;
	}		
	error = vfs_lock(vfsp);
	if (error)
		return error;
	error = mountfs(vfsp, why, rootdev, "/", u.u_cred, 1);
	/* XXX - assumes root device is not indirect, because we don't set */
	/* rootvp.  Is rootvp used for anything?  If so, make another arg */
	/* to mountfs (in S5 case too?) */
	if (error) {
		vfs_unlock(vfsp);
		if (rootvp) {
			VN_RELE(rootvp);
			rootvp = (struct vnode *)0;
		}
		return (error);
	}
	vfs_add((struct vnode *)0, vfsp,
		(vfsp->vfs_flag & VFS_RDONLY) ? MS_RDONLY : 0);
	vfs_unlock(vfsp);
	fsp = getfs(vfsp);
	clkset(fsp->fs_time);
	return (0);
}

STATIC int
mountfs(vfsp, why, dev, path, cr, isroot)
	struct vfs *vfsp;
	enum whymountroot why;
	dev_t dev;
	char *path;
	struct cred *cr;
	int isroot;
{
	struct vnode *devvp = 0;
	register struct fs *savefsp;
	register struct fs *fsp;
	register struct ufsvfs *ufsvfsp = 0;
	register struct buf *bp = 0;
	struct buf *tp = 0;
	int error;
	int blks;
	caddr_t space = 0;
	int i;
	long size;
	size_t len;
	int needclose = 0;
	struct inode *rip;
	struct vnode *rvp;
	struct buf *ngeteblk();
	
#ifdef notneeded
	/* not required since fsinit has done this */
	static int initdone = 0;
	if (!initdone) {
		ihinit();
		initdone = 1;
	}
#endif
	if (why == ROOT_INIT) {
		/*
		 * Open the device.
		 */
		devvp = makespecvp(dev, VBLK);

		/*
		 * Open block device mounted on.
		 * When bio is fixed for vnodes this can all be vnode
		 * operations.
		 */
		error = VOP_OPEN(&devvp,
		    (vfsp->vfs_flag & VFS_RDONLY) ? FREAD : FREAD|FWRITE, cr);
		if (error)
			return (error);
		needclose = 1;

		/*
		 * Refuse to go any further if this
		 * device is being used for swapping.
		 */
		if (IS_SWAPVP(devvp)) {
			error = EBUSY;
			goto out;
		}
	}
	/*
	/* check for dev already mounted on
	 */

	if (vfsp->vfs_flag & VFS_REMOUNT) {
		devvp = ((struct ufsvfs *)vfsp->vfs_data)->vfs_devvp;
	}
	ASSERT(devvp != 0);
	/*
	 * Flush back any dirty pages on the block device to
	 * try and keep the buffer cache in sync with the page
	 * cache if someone is trying to use block devices when
	 * they really should be using the raw device.
	 */
	(void) VOP_PUTPAGE(devvp, 0, 0, B_INVAL, cr);
	binval(dev);
	if (vfsp->vfs_flag & VFS_REMOUNT) {
#ifdef QUOTA
		ufs_iflush(vfsp, ((struct ufsvfs *)vfsp->vfs_data)->vfs_qinod);
#else
		ufs_iflush(vfsp);
#endif
	}		
	/*
	 * read in superblock
	 */

	tp = bread(dev, BBSIZE/SBSIZE, SBSIZE);
	if (tp->b_flags & B_ERROR) {
		goto out;
	}



	fsp = (struct fs *)tp->b_un.b_addr;
	if (fsp->fs_magic != FS_MAGIC || fsp->fs_bsize > MAXBSIZE ||
	    fsp->fs_frag > MAXFRAG ||	
	    fsp->fs_bsize < sizeof (struct fs) || fsp->fs_bsize < PAGESIZE/2) {
		error = EINVAL;	/* also needs translation */
		goto out;
	}

       if (vfsp->vfs_flag & VFS_REMOUNT) {
               bp = ((struct ufsvfs *)vfsp->vfs_data)->vfs_bufp;
               savefsp = (struct fs *)kmem_alloc(bp->b_bcount, KM_SLEEP);
               bcopy((caddr_t)bp->b_un.b_addr, (caddr_t)savefsp, (u_int)bp->b_bcount);
               goto modify_now;
       }
	/*
	 * Allocate VFS private data.
	 */
	if ((ufsvfsp =
	  (struct ufsvfs *) kmem_zalloc(sizeof(struct ufsvfs), KM_SLEEP)) == NULL)
		return EBUSY;
	vfsp->vfs_bcount = 0;
	vfsp->vfs_data = (caddr_t) ufsvfsp;
	vfsp->vfs_fstype = ufsfstype;
	vfsp->vfs_dev = dev;
	vfsp->vfs_flag |= VFS_NOTRUNC;
	vfsp->vfs_fsid.val[0] = dev;
	vfsp->vfs_fsid.val[1] = ufsfstype;
	ufsvfsp->vfs_devvp = devvp;
	/*
	 * Copy the super block into a buffer in its native size.
	 * Use ngeteblk to allocate the buffer
	 */
	bp = ngeteblk((int)fsp->fs_bsize);
	((struct ufsvfs *)vfsp->vfs_data)->vfs_bufp = bp;
	bp->b_bcount = fsp->fs_sbsize;
modify_now:
	bcopy((caddr_t)tp->b_un.b_addr, (caddr_t)bp->b_un.b_addr,
	   (u_int)fsp->fs_sbsize);
	tp->b_flags |= B_STALE | B_AGE;
	brelse(tp);
	tp = 0;
	fsp = (struct fs *)bp->b_un.b_addr;
	/*
	 * Curently we only allow a remount to change from
	 * read-only to read-write.
	 */
	if (vfsp->vfs_flag & VFS_RDONLY) {
		ASSERT((vfsp->vfs_flag & VFS_REMOUNT) == 0);
		fsp->fs_ronly = 1;
		fsp->fs_fmod = 0;	
	} else {
		if (vfsp->vfs_flag & VFS_REMOUNT) {
			if (fsp->fs_state == FSACTIVE) {
				error = EINVAL;
				goto out;
			}
		}

		if ((fsp->fs_state + (long)fsp->fs_time) == FSOKAY)
			fsp->fs_state = FSACTIVE;
		else if (isroot)
			fsp->fs_state = FSBAD;
		else {
			error = ENOSPC;
			goto out;
		}
		/* write out to disk synchronously */
		tp = ngeteblk((int)fsp->fs_bsize);
		tp->b_edev = dev;
		tp->b_dev = cmpdev(dev);
		tp->b_blkno = SBLOCK;
		tp->b_bcount = fsp->fs_sbsize;
		bcopy((char *)fsp, tp->b_un.b_addr, (u_int)fsp->fs_sbsize);
		bwrite(tp);
		tp = 0;
		fsp->fs_fmod = 1;
		fsp->fs_ronly = 0;
	}
	vfsp->vfs_bsize = fsp->fs_bsize;
	/*
	 * Read in cyl group info
	 */
	blks = howmany(fsp->fs_cssize, fsp->fs_fsize);
	space = (caddr_t)kmem_alloc((u_int)fsp->fs_cssize, KM_SLEEP);
	if (space == 0) {
		error = ENOMEM;
		goto out;
	}
	for (i = 0; i < blks; i += fsp->fs_frag) {
		size = fsp->fs_bsize;
		if (i + fsp->fs_frag > blks)
			size = (blks - i) * fsp->fs_fsize;
		tp = bread(dev, (daddr_t)fragstoblks(fsp, fsp->fs_csaddr+i),
			 fsp->fs_bsize);
		if (tp->b_flags & B_ERROR) {
			goto out;
		}
		bcopy((caddr_t)tp->b_un.b_addr, space, (u_int)size);
		fsp->fs_csp[fragstoblks(fsp, i)] = (struct csum *)space;
		space += size;
		tp->b_flags |= B_AGE;
		brelse(tp);
		tp = 0;
	}
	copystr(path, fsp->fs_fsmnt, sizeof (fsp->fs_fsmnt) - 1, &len);
	bzero(fsp->fs_fsmnt + len, sizeof (fsp->fs_fsmnt) - len);

        if (vfsp->vfs_flag & VFS_REMOUNT) {
                if (savefsp) {
                        kmem_free((caddr_t)savefsp->fs_csp[0],(u_int)savefsp->fs_cssize);
                        kmem_free(savefsp, bp->b_bcount);
		}
                return(0);
        }

	if (why == ROOT_INIT) {
		if (isroot)
			rootvp = devvp;
	}
	if (error = ufs_iget(vfsp, fsp, UFSROOTINO, &rip))
		goto out;
	rvp = ITOV(rip);
	rvp->v_flag |= VROOT;
	((struct ufsvfs *)vfsp->vfs_data)->vfs_root = rvp;
	IUNLOCK(rip);
	return (0);
out:
	if (error == 0)
		error = EIO;
        if (space)
                kmem_free((void *)space, (u_int)fsp->fs_cssize);
        if (why == ROOT_REMOUNT) {
                if (savefsp) {
                        bcopy((caddr_t)savefsp, (caddr_t)fsp, fsp->fs_sbsize);
                        kmem_free(savefsp, bp->b_bcount);
                }
                return(error);
        }

	if (bp) {
		bp->b_flags |= B_AGE;
		brelse(bp);
	}
	if (tp) {
		tp->b_flags |= B_AGE;
		brelse(tp);
	}
	if (ufsvfsp)
		kmem_free((caddr_t)ufsvfsp, sizeof(struct ufsvfs));
	if (needclose) {
		(void) VOP_CLOSE(devvp, (vfsp->vfs_flag & VFS_RDONLY) ?
		      FREAD : FREAD|FWRITE, 1, 0, cr);
		binval(dev);
	}
	VN_RELE(devvp);
	return (error);
}

/*
 * vfs operations
 */
STATIC int
ufs_unmount(vfsp, cr)
	struct vfs *vfsp;
	struct cred *cr;
{

	return (unmount1(vfsp, 0, cr));
}

STATIC int
unmount1(vfsp, forcibly, cr)
	register struct vfs *vfsp;
	int forcibly;
	struct cred *cr;
{
	dev_t dev = vfsp->vfs_dev;
	register struct fs *fs;
	register int stillopen;
	register struct ufsvfs *ufsvfsp = (struct ufsvfs *)vfsp->vfs_data;
	int flag;
	struct vnode *bvp, *rvp;
	struct inode *rip;
	struct buf *bp;
	extern int updlock;
	
	if (!suser(cr))
		return (EPERM);

#ifdef QUOTA
	if ((stillopen = ufs_iflush(vfsp, ufsvfsp->vfs_qinod)) < 0 && !forcibly)
#else
	if ((stillopen = ufs_iflush(vfsp)) < 0 && !forcibly)
#endif
		return (EBUSY);
	if (stillopen < 0)
		return (EBUSY);		/* XXX */
#ifdef QUOTA
	(void) closedq(ufsvfsp, cr);
	/*
	 * Here we have to ufs_iflush again to get rid of the quota inode.
	 * A drag, but it would be ugly to cheat, & this doesn't happen often
	 */
	(void) ufs_iflush(vfsp, (struct inode *)NULL);
#endif
	/* Flush root inode to disk */
	rvp = ufsvfsp->vfs_root;
	ASSERT(rvp != NULL);
	rip = VTOI(rvp);
	ILOCK(rip);
	ufs_iupdat(rip, 1);

	fs = getfs(vfsp);
	bp = ufsvfsp->vfs_bufp;
	bvp = ufsvfsp->vfs_devvp;
	kmem_free((caddr_t)fs->fs_csp[0], (u_int)fs->fs_cssize);
	flag = !fs->fs_ronly;
	while(updlock)
		(void)sleep((caddr_t)&updlock, PINOD);
	updlock++;		
	if (!fs->fs_ronly) {
		bflush(dev);
		fs->fs_time = hrestime.tv_sec;
		if (vfsp->vfs_flag & VFS_BADBLOCK)
			fs->fs_state = FSBAD;
		else
			fs->fs_state = FSOKAY - (long)fs->fs_time;
		bcopy((char *)fs, bp->b_un.b_addr, (long)fs->fs_sbsize);
		bp->b_edev = dev;
		bp->b_dev = cmpdev(dev);
		bp->b_bcount = fs->fs_sbsize;
		bp->b_blkno = SBLOCK;
		bwrite(bp);
	}
	else {
		bp->b_flags |= B_AGE;
		brelse(bp);
	}
	updlock = 0;
	wakeup((caddr_t)&updlock);
	(void) VOP_PUTPAGE(bvp, 0, 0, B_INVAL, cr);
	(void) VOP_CLOSE(bvp, flag, 1, 0, cr);
	VN_RELE(bvp);
	binval(dev);
	ufs_iput(rip);
	_remque(rip);
	kmem_free((caddr_t)ufsvfsp, sizeof(struct ufsvfs));
	return (0);
}
STATIC int
ufs_root(vfsp, vpp)
	struct vfs *vfsp;
	struct vnode **vpp;
{
	struct ufsvfs *ufsvfsp = (struct ufsvfs *)vfsp->vfs_data;
	struct vnode *vp = ufsvfsp->vfs_root;

	VN_HOLD(vp);
	*vpp = vp;
	return (0);
}

/*
 * Get file system statistics.
 */
STATIC int
ufs_statvfs(vfsp, sp)
	register struct vfs *vfsp;
	struct statvfs *sp;
{
	register struct fs *fsp;
	int blk, i;
	long bavail;
	
	fsp = getfs(vfsp);
	if (fsp->fs_magic != FS_MAGIC)
		return (EINVAL);
	(void)bzero((caddr_t)sp, (int)sizeof(*sp));
	sp->f_bsize = fsp->fs_bsize;
	sp->f_frsize = fsp->fs_fsize;
	sp->f_blocks = fsp->fs_dsize;
	sp->f_bfree = fsp->fs_cstotal.cs_nbfree * fsp->fs_frag +
	    fsp->fs_cstotal.cs_nffree;
	/*
	 * avail = MAX(max_avail - used, 0)
	 */
	bavail = (fsp->fs_dsize * (100 - fsp->fs_minfree) / 100) -
	    (fsp->fs_dsize - sp->f_bfree);
	sp->f_bavail = bavail < 0 ? 0 : bavail;
	/*
	 * inodes
	 */
	sp->f_files =  fsp->fs_ncg * fsp->fs_ipg;
	sp->f_ffree = sp->f_favail = fsp->fs_cstotal.cs_nifree;
	sp->f_fsid = vfsp->vfs_dev;
	(char *) strcpy(sp->f_basetype, vfssw[vfsp->vfs_fstype].vsw_name);
	sp->f_flag = vf_to_stf(vfsp->vfs_flag);
	sp->f_namemax = MAXNAMLEN;
	blk = fsp->fs_spc * fsp->fs_cpc / NSPF(fsp);
	for (i = 0; i < blk; i += fsp->fs_frag)
		/* void */;
	i -= fsp->fs_frag;
	blk = i / fsp->fs_frag;
	bcopy((char *)&(fsp->fs_rotbl[blk]), sp->f_fstr, 14);
	return (0);
}

/*
 * Flush any pending I/O to file system vfsp.
 * The ufs_update() routine will only flush *all* ufs files.
 */
/*ARGSUSED*/
STATIC int
ufs_sync(vfsp, flag, cr)
	struct vfs *vfsp;
	short flag;
	struct cred *cr;
{

	if (flag & SYNC_ATTR)
		ufs_flushi(SYNC_ATTR);
	else
		ufs_update();
	return 0;
}

void
sbupdate(vfsp)
	struct vfs *vfsp;
{
	register struct fs *fs = getfs(vfsp);
	register struct buf *bp;
	int blks;
	caddr_t space;
	int i;
	long size;
	struct buf *ngeteblk();

	bp = ngeteblk(fs->fs_bsize);
	bp->b_edev = vfsp->vfs_dev;
	bp->b_dev = cmpdev(vfsp->vfs_dev);
	bp->b_blkno = SBLOCK;
	bp->b_bcount = fs->fs_sbsize; 
	bcopy((caddr_t)fs, bp->b_un.b_addr, (uint)fs->fs_sbsize);
	bwrite(bp);
	blks = howmany(fs->fs_cssize, fs->fs_fsize);
	space = (caddr_t)fs->fs_csp[0];
	for (i = 0; i < blks; i += fs->fs_frag) {
		size = fs->fs_bsize;
		if (i + fs->fs_frag > blks)
			size = (blks -i) * fs->fs_fsize;
		bp = getblk(vfsp->vfs_dev,
			(daddr_t)(fragstoblks(fs, fs->fs_csaddr+i)),
			fs->fs_bsize);
		bcopy(space, bp->b_un.b_addr, (u_int)size);
		space += size;
		bp->b_bcount = size;	
		bwrite(bp);

	}
}

STATIC int
ufs_vget(vfsp, vpp, fidp)
	struct vfs *vfsp;
	struct vnode **vpp;
	struct fid *fidp;
{
	register struct ufid *ufid;
	register struct fs *fs = getfs(vfsp);
	struct inode *ip;

	ufid = (struct ufid *)fidp;
	if (ufs_iget(vfsp, fs, ufid->ufid_ino, &ip)) {
		*vpp = NULL;
		return (0);
	}
	if (ip->i_gen != ufid->ufid_gen) {
		idrop(ip);
		*vpp = NULL;
		return (0);
	}
	IUNLOCK(ip);
	*vpp = ITOV(ip);
	if ((ip->i_mode & ISVTX) && !(ip->i_mode & (IEXEC | IFDIR))) {
		(*vpp)->v_flag |= VISSWAP;
	}
	return (0);
}

/* ARGSUSED */
STATIC int
ufs_swapvp(vfsp, vpp, nm)
	struct vfs *vfsp;
	struct vnode **vpp;
	char *nm;
{
	return ENOSYS;
}
