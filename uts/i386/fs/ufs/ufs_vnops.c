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

#ident  "@(#)kern-fs:ufs/ufs_vnops.c	1.3.5.15"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/systm.h>
#include <sys/sysmacros.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/cred.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/disp.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/flock.h>
#include <sys/kmem.h>
#include <sys/uio.h>
#include <sys/conf.h>
#include <sys/mman.h>
#include <sys/pathname.h>
#include <sys/debug.h>
#include <sys/vmmeter.h>
#include <sys/cmn_err.h>

#include <sys/fs/ufs_fs.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fsdir.h>
#ifdef QUOTA
#include <sys/fs/ufs_quota.h>
#endif
#include <sys/dirent.h>		/* must be AFTER <sys/fs/fsdir.h>! */
#include <sys/errno.h>
#include <sys/sysinfo.h>

#include <vm/hat.h>
#include <vm/page.h>
#include <vm/pvn.h>
#include <vm/as.h>
#include <vm/seg.h>
#include <vm/seg_map.h>
#include <vm/seg_vn.h>
#include <vm/rm.h>
#include <sys/swap.h>

#include "fs/fs_subr.h"

/*#include <sys/fs/specfifo.h>	/* this defines PIPE_BUF for ufs_getattr() */

#define ISVDEV(t)	((t) == VCHR || (t) == VBLK || (t) == VFIFO \
				|| (t) == VXNAM)

STATIC	int ufs_open();
STATIC	int ufs_close();
STATIC	int ufs_read();
STATIC	int ufs_write();
STATIC	int ufs_ioctl();
STATIC	int ufs_getattr();
STATIC	int ufs_setattr();
STATIC	int ufs_access();
STATIC	int ufs_lookup();
STATIC	int ufs_create();
STATIC	int ufs_remove();
STATIC	int ufs_link();
STATIC	int ufs_rename();
STATIC	int ufs_mkdir();
STATIC	int ufs_rmdir();
STATIC	int ufs_readdir();
STATIC	int ufs_symlink();
STATIC	int ufs_readlink();
STATIC	int ufs_fsync();
STATIC	void ufs_inactive();
STATIC	int ufs_fid();
STATIC	void ufs_rwlock();
STATIC	void ufs_rwunlock();
STATIC	int ufs_seek();
STATIC	int ufs_frlock();
STATIC  int ufs_space();

STATIC	int ufs_getpage();
STATIC	int ufs_putpage();
STATIC	int ufs_map();
STATIC	int ufs_addmap();
STATIC	int ufs_delmap();
STATIC	int ufs_allocstore();

struct vnodeops ufs_vnodeops = {
	ufs_open,
	ufs_close,
	ufs_read,
	ufs_write,
	ufs_ioctl,
	fs_setfl,
	ufs_getattr,
	ufs_setattr,
	ufs_access,
	ufs_lookup,
	ufs_create,
	ufs_remove,
	ufs_link,
	ufs_rename,
	ufs_mkdir,
	ufs_rmdir,
	ufs_readdir,
	ufs_symlink,
	ufs_readlink,
	ufs_fsync,
	ufs_inactive,
	ufs_fid,
	ufs_rwlock,
	ufs_rwunlock,
	ufs_seek,
	fs_cmp,
	ufs_frlock,
	ufs_space,
	fs_nosys,	/* realvp */
	ufs_getpage,
	ufs_putpage,
	ufs_map,
	ufs_addmap,
	ufs_delmap,
	fs_poll,
	fs_nosys,	/* dump */
	fs_pathconf,
	ufs_allocstore,
	fs_nosys,	/* filler */
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
};

/*
 * No special action required for ordinary files.  (Devices are handled
 * through the device file system.)
 */
/* ARGSUSED */
STATIC int
ufs_open(vpp, flag, cr)
	struct vnode **vpp;
	int flag;
	struct cred *cr;
{

#ifdef DEBUG
	if ((*vpp)->v_type == VNON || (*vpp)->v_type == VBAD)
                return (EACCES);
#endif
	return 0;
}

/*ARGSUSED*/
STATIC int
ufs_close(vp, flag, count, offset, cr)
	struct vnode *vp;
	int flag;
	int count;
	off_t offset;
	struct cred *cr;
{
	register struct inode *ip = VTOI(vp);

	ILOCK(ip);
	ITIMES(ip);
	cleanlocks(vp, u.u_procp->p_epid, u.u_procp->p_sysid);
	IUNLOCK(ip);
	return 0;
}

/*ARGSUSED*/
STATIC int
ufs_read(vp, uiop, ioflag, cr)
	struct vnode *vp;
	struct uio *uiop;
	int ioflag;
	struct cred *cr;
{
	struct inode *ip = VTOI(vp);
	int error;

	ASSERT(ip->i_flag & IRWLOCKED);

	error = rwip(ip, uiop, UIO_READ, ioflag);
	ITIMES(ip);
	return error;
}

/*ARGSUSED*/
STATIC int
ufs_write(vp, uiop, ioflag, cr)
	struct vnode *vp;
	struct uio *uiop;
	int ioflag;
	struct cred *cr;
{
	int error;
	struct inode *ip;

	/*
	 * NOTE:  this assertion is consistent with the agreed on
	 * vnode interface provisions for preserving atomicity of
	 * reads and writes, but it necessarily implies that the
	 * ufs_ilock() call in ufs_rdwr is recursive.
	 */
	ip = VTOI(vp);
	ASSERT((ip->i_flag & IRWLOCKED)
	  || (vp->v_type != VREG && vp->v_type != VDIR));

	if (vp->v_type == VREG &&
	  (error = fs_vcode(vp, &ip->i_vcode)))
		return error;

	if ((ioflag & IO_APPEND) != 0 && (ip->i_mode & IFMT) == IFREG) {
		/*
		 * In append mode start at end of file.
		 */
		uiop->uio_offset = ip->i_size;
	}
	error = rwip(ip, uiop, UIO_WRITE, ioflag);
	ITIMES(ip);
	return (error);
}

#ifdef notneeded
/*
 * read or write a vnode
 */
/*ARGSUSED*/
STATIC int
ufs_rdwr(vp, uiop, rw, ioflag, cr)
	struct vnode *vp;
	struct uio *uiop;
	enum uio_rw rw;
	int ioflag;
	struct cred *cr;
{
	register struct inode *ip;
	int error;
	int didlock;

	ip = VTOI(vp);
	if ((ioflag & IO_APPEND) != 0 && (rw == UIO_WRITE) &&
	    (ip->i_mode & IFMT) == IFREG) {
		/*
		 * In append mode start at end of file after locking it.
		 */
		didlock = 1;
		ufs_ilock(ip);
		uiop->uio_offset = ip->i_size;
	} else
		didlock = 0;

	error = rwip(ip, uiop, rw, ioflag);
	ITIMES(ip);

	if (didlock)
		ufs_iunlock(ip);

	return (error);
}
#endif

/*
 * Don't cache write blocks to files with the sticky bit set.
 * Used to keep swap files from blowing the page cache on a server.
 */
int stickyhack = 1;

/*
 * rwip does the real work of read or write requests for ufs.
 */
STATIC int
rwip(ip, uio, rw, ioflag)
	register struct inode *ip;
	register struct uio *uio;
	enum uio_rw rw;
	int ioflag;
{
	register u_int off;
	register addr_t base;
	register int n, on, mapon;
	register struct fs *fs;
	struct vnode *vp;
	int type, error, pagecreate;
	rlim_t limit = uio->uio_limit;
	u_int flags;
	int iupdat_flag;
	long old_blocks;
	long oresid = uio->uio_resid;
	page_t *iolpl[MAXBSIZE/PAGESIZE + 2];
	page_t **ppp;

	/*
	 * ip->i_size is incremented before the uiomove
	 * is done on a write.  If the move fails (bad user
	 * address) reset ip->i_size.
	 * The better way would be to increment ip->i_size
	 * only if the uiomove succeeds.
	 */
	int i_size_changed = 0;
	int old_i_size;

	ASSERT(rw == UIO_READ || rw == UIO_WRITE);
	type = ip->i_mode & IFMT;
	ASSERT(type == IFREG || type == IFDIR || type == IFLNK);
	vp = ITOV(ip);
	if (MANDLOCK(vp, ip->i_mode)
	  && (error = chklock(vp, rw == UIO_READ ? FREAD : FWRITE,
	    uio->uio_offset, uio->uio_resid, uio->uio_fmode)))
		return (error);
	if (uio->uio_offset < 0 || (uio->uio_offset + uio->uio_resid) < 0)
		return (EINVAL);
	if (uio->uio_resid == 0)
		return (0);

	if (rw == UIO_WRITE) {
		ip->i_flag |= INOACC;	/* don't update ref time in getpage */
	} else {
		ip->i_flag |= IACC;
	}

	if (ioflag & IO_SYNC) {
		ip->i_flag |= ISYNC;
		old_blocks = ip->i_blocks;
		iupdat_flag = 0;
	}
	fs = ip->i_fs;
	do {
		off = uio->uio_offset & MAXBMASK;
		mapon = uio->uio_offset & MAXBOFFSET;
		on = blkoff(fs, uio->uio_offset);
		n = MIN(fs->fs_bsize - on, uio->uio_resid);

		old_i_size = ip->i_size;

		if (rw == UIO_READ) {
			int diff = old_i_size - uio->uio_offset;

			if (diff <= 0) {
				error = 0;
				goto out;
			}
			if (diff < n)
				n = diff;
		}

		if (rw == UIO_WRITE) {
			if (type == IFREG && uio->uio_offset + n >= limit) {
				if (uio->uio_offset >= limit) {
					error = EFBIG;
					goto out;
				}
				n = limit - uio->uio_offset;
			}

			/*
			 * as_iolock will determine if we are properly
			 * page-aligned to do the pagecreate case, and if so,
			 * will hold the "from" pages until after the uiomove
			 * to avoid deadlocking and to catch the case of
			 * writing a file to itself.
			 */
			n = as_iolock(uio, iolpl, n, vp, old_i_size,
					&pagecreate);
			if (n == 0) {
				error = EFAULT;
				break;
			}

			if (uio->uio_offset + n > old_i_size) {
				i_size_changed = 1;
				iupdat_flag = 1;
			}
	
			/*
			 * ufs_bmap is used so that we are sure that
			 * if we need to allocate new blocks, that it
			 * is done here before we up the file size.
			 */
			ILOCK(ip);
			error = ufs_bmap(ip,
			    (daddr_t)lblkno(fs, uio->uio_offset),
			    (daddr_t *)NULL, (daddr_t *)NULL,
			    (int)(on + n), S_WRITE, pagecreate);
			IUNLOCK(ip);
			if (error) {
				for (ppp = iolpl; *ppp; ppp++)
					PAGE_RELE(*ppp);
				break;
			}

			if (i_size_changed)
				ip->i_size = uio->uio_offset + n;

			base = segmap_getmap(segkmap, vp, off);

			if (pagecreate)
				segmap_pagecreate(segkmap, base, (u_int)n, 0);

		} else /* rw == UIO_READ */ {
			base = segmap_getmap(segkmap, vp, off);
			pagecreate = 0;
			iolpl[0] = NULL;
		}

		error = uiomove(base + mapon, (long)n, rw, uio);

		/* Now release any pages held by as_iolock */
		for (ppp = iolpl; *ppp; ppp++)
			PAGE_RELE(*ppp);

		if (pagecreate && uio->uio_offset <
		    roundup(off + mapon + n, PAGESIZE)) {
			/*
			 * We created pages w/o initializing them completely,
			 * thus we need to zero the part that wasn't set up.
			 * This happens on most EOF write cases and if
			 * we had some sort of error during the uiomove.
			 */
			int nzero, nmoved;

			nmoved = uio->uio_offset - (off + mapon);
			ASSERT(nmoved >= 0 && nmoved <= n);
			nzero = roundup(n, PAGESIZE) - nmoved;
			ASSERT(nzero > 0 && mapon + nmoved + nzero <= MAXBSIZE);
			(void) kzero(base + mapon + nmoved, (u_int)nzero);
		}

		if (error) {
			/*
			 * If we failed on a write, we may have already allocated
			 * file blocks as well as pages.  It's hard to undo the
			 * block allocation, but we must be sure to invalidate
			 * any pages that may have been allocated.
			 */
			(void) segmap_release(segkmap, base,
			  rw == UIO_WRITE ? SM_INVAL : 0);
			/*
			 * The move failed, fix up i_size,
			 * but after segmap_release.
			 */
			if (i_size_changed) {
				ILOCK(ip);
				ip->i_size = old_i_size;
				IUNLOCK(ip);
			}
		} else {
			flags = 0;
			if (rw == UIO_WRITE) {
				/*
				 * Force write back for synchronous write cases.
				 */
				if ((ioflag & IO_SYNC) || type == IFDIR) {
					/*
					 * If the sticky bit is set but the
					 * execute bit is not set, we do a
					 * synchronous write back and free
					 * the page when done.  We set up swap
					 * files to be handled this way to
					 * prevent servers from keeping around
					 * the client's swap pages too long.
					 * XXX - there ought to be a better way.
					 */
					if (IS_SWAPVP(vp)) {
						flags = SM_WRITE | SM_FREE |
						    SM_DONTNEED;
					} else {
						iupdat_flag = 1;
						flags = SM_WRITE;
					}
				} else if (n + on == MAXBSIZE ||
				    IS_SWAPVP(vp)) {
					/*
					 * Have written a whole block.
					 * Start an asynchronous write and
					 * mark the buffer to indicate that
					 * it won't be needed again soon.
					 */
					flags = SM_WRITE | SM_ASYNC |
					    SM_DONTNEED;
				}
				ip->i_flag |= IUPD | ICHG;
  				if ((u.u_cred->cr_uid != 0) &&
				    (ip->i_mode & (VSGID|(VEXEC>>3))) ==
				     (VSGID|(VEXEC>>3)))
					ip->i_mode &= ~(ISUID | ISGID);
			} else if (rw == UIO_READ) {
				/*
				 * If read a whole block, or read to eof,
				 * won't need this buffer again soon.
				 */
				if (n + on == MAXBSIZE ||
				    uio->uio_offset == ip->i_size)
					flags = SM_DONTNEED;
			}
			error = segmap_release(segkmap, base, flags);
		}

	} while (error == 0 && uio->uio_resid > 0 && n != 0);

	/*
	 * If we are doing synchronous write the only time we should
	 * not be sync'ing the ip here is if we have the stickyhack
	 * activated, the file is marked with the sticky bit and
	 * no exec bit, the file length has not been changed and
	 * no new blocks have been allocated during this write.
	 */
	if ((ioflag & IO_SYNC) != 0 && rw == UIO_WRITE &&
	    (iupdat_flag != 0 || old_blocks != ip->i_blocks)) {
		ufs_iupdat(ip, 1);
	}

out:
    	/*
         * If we've already done a partial-write, terminate
	 * the write but return no error.
	 */
	if (oresid != uio->uio_resid)
		error = 0;

	ip->i_flag &= ~(ISYNC | INOACC);
	return (error);
}

/* ARGSUSED */
STATIC int
ufs_ioctl(vp, cmd, arg, flag, cr, rvalp)
	struct vnode *vp;
	int cmd;
	int arg;
	int flag;
	struct cred *cr;
	int *rvalp;
{
#ifdef QUOTA
	if (cmd == Q_QUOTACTL)
		return (quotactl(vp, arg, cr));
	else
#endif /* QUOTA */
	return ENOTTY;
}

/* ARGSUSED */
STATIC int
ufs_getattr(vp, vap, flags, cr)
	struct vnode *vp;
	register struct vattr *vap;
	int flags;
	struct cred *cr;
{
	register struct inode *ip = VTOI(vp);
	struct fs *fsp = getfs(vp->v_vfsp);

	/*
	 * Return all the attributes.  This should be refined so
	 * that it only returns what's asked for.
	 */
	ITIMES(ip);	/* mark correct time in inode */

	/*
	 * Copy from inode table.
	 */
	vap->va_type = vp->v_type;
	vap->va_mode = ip->i_mode & MODEMASK;
	vap->va_uid = ip->i_uid;
	vap->va_gid = ip->i_gid;
	vap->va_fsid = ip->i_dev;
	vap->va_nodeid = ip->i_number;
	vap->va_nlink = ip->i_nlink;
	vap->va_size = ip->i_size;
	vap->va_vcode = ip->i_vcode;
	if (vp->v_type == VCHR || vp->v_type == VBLK || vp->v_type == VXNAM)
		vap->va_rdev = ip->i_rdev;
	else
		vap->va_rdev = 0;	/* not a b/c spec. */
	vap->va_atime.tv_sec = ip->i_atime.tv_sec;
	vap->va_atime.tv_nsec = ip->i_atime.tv_usec*1000;
	vap->va_mtime.tv_sec = ip->i_mtime.tv_sec;
	vap->va_mtime.tv_nsec = ip->i_mtime.tv_usec*1000;
	vap->va_ctime.tv_sec = ip->i_ctime.tv_sec;
	vap->va_ctime.tv_nsec = ip->i_ctime.tv_usec*1000;

	switch (ip->i_mode & IFMT) {

	case IFBLK:
		vap->va_blksize = MAXBSIZE;		/* was BLKDEV_IOSIZE */
		break;

	case IFCHR:
		vap->va_blksize = MAXBSIZE;
		break;

	default:
		vap->va_blksize = fsp->fs_fsize;
		break;
	}
	vap->va_nblocks = ip->i_blocks;
	return (0);
}

STATIC int
ufs_setattr(vp, vap, flags, cr)
	register struct vnode *vp;
	register struct vattr *vap;
	int flags;
	struct cred *cr;
{
	int error = 0;
	register long int mask = vap->va_mask;
	register struct inode *ip;
	int issync = 0;
#ifdef QUOTA
	register long change;
#endif

	/*
	 * Cannot set these attributes.
	 */
	if (mask & AT_NOSET)
		return EINVAL;

	ip = VTOI(vp);
	IRWLOCK(ip);
	ILOCK(ip);

	/*
	 * Change file access modes.  Must be owner or super-user.
	 */
	if (mask & AT_MODE) {
		if (cr->cr_uid != ip->i_uid && !suser(cr)) {
			error = EPERM;
			goto out;
		}
		ip->i_mode &= IFMT;
		ip->i_mode |= vap->va_mode & ~IFMT;
		if (cr->cr_uid != 0) {
			/*
			 * A non-privileged user can set the sticky bit
			 * on a directory.
			 */
			if (vp->v_type != VDIR)
				ip->i_mode &= ~ISVTX;
			if (!groupmember((uid_t)ip->i_gid, cr))
				ip->i_mode &= ~ISGID;
		}
		ip->i_flag |= ICHG;
	}
	if (mask & (AT_UID|AT_GID)) {
		int checksu = 0;

		/*
		 * To change file ownership, a process not running as
		 * super-user must be running as the owner of the file.
		 */
		if (cr->cr_uid != ip->i_uid)
			checksu = 1;
		else {
			if (rstchown) {
				/*
				 * "chown" is restricted.  A process not
				 * running as super-user cannot change the
				 * owner, and can only change the group to a
				 * group of which it's currently a member.
				 */
				if (((mask & AT_UID) && vap->va_uid != ip->i_uid)
				    || ((mask & AT_GID) && !groupmember(vap->va_gid, cr)))
					checksu = 1;
			}
		}

		if (checksu && !suser(cr)) {
			error = EPERM;
			goto out;
		}

  		if (cr->cr_uid != 0) {
			if ((ip->i_mode & (VSGID|(VEXEC>>3))) == 
			    (VSGID|(VEXEC>>3)))
				ip->i_mode &= ~(ISGID);
			ip->i_mode &= ~(ISUID);
		}

		if (mask & AT_UID) {
#ifdef QUOTA
			/*
			 * Remove the blocks, and the file, from the old user's
			 * quota.
			 * Checking whether the UID is really changing or not
			 * just speeds things a little.
			 */
			if (ip->i_uid == vap->va_uid)
				change = 0;
			else
				change = ip->i_blocks;
			(void) chkdq(ip, -change, 1);
			(void) chkiq((struct ufsvfs *)(ip->i_vnode.v_vfsp->vfs_data), ip,
			    (uid_t)ip->i_uid, 1);
			dqrele(ip->i_dquot);
#endif
			ip->i_uid = vap->va_uid;
		}
		if (mask & AT_GID)
			ip->i_gid = vap->va_gid;
		ip->i_flag |= ICHG;
#ifdef QUOTA
		if (mask & AT_UID) {
			/*
			 * Add the blocks, and the file, to the old user's
			 * quota.
			 * XXX - could this be done before setting ICHG?  It
			 * wasn't in the old code; was this necessary or was it
			 * just an accident?
			 */
			ip->i_dquot = getinoquota(ip);
			(void) chkdq(ip, change, 1);
			(void) chkiq((struct ufsvfs *)(ip->i_vnode.v_vfsp->vfs_data),
			    (struct inode *)NULL, (uid_t)ip->i_uid, 1);
		}
#endif
	}
	/*
	 * Truncate file.  Must have write permission and not be a directory.
	 */
	if (mask & AT_SIZE) {
		if (vp->v_type == VDIR) {
			error = EISDIR;
			goto out;
		}
		if (error = ufs_iaccess(ip, IWRITE, cr))
			goto out;
		if (vp->v_type == VREG &&
		  (error = fs_vcode(vp, &ip->i_vcode)))
			goto out;
		if (error = ufs_itrunc(ip, vap->va_size))
			goto out;
		issync++;
	}
	/*
	 * Change file access or modified times.
	 */
	if (mask & (AT_ATIME|AT_MTIME)) {
		if (cr->cr_uid != ip->i_uid && cr->cr_uid != 0) {
			if (flags & ATTR_UTIME)
				error = EPERM;
			else
				error = ufs_iaccess(ip, IWRITE, cr);
			if (error)
				goto out;
		}
		if (mask & AT_ATIME) {
			ip->i_atime.tv_sec = vap->va_atime.tv_sec;
			ip->i_atime.tv_usec = vap->va_atime.tv_nsec/1000;
			ip->i_flag &= ~IACC;
		}
		if (mask & AT_MTIME) {
			ip->i_mtime.tv_sec = vap->va_mtime.tv_sec;
			ip->i_mtime.tv_usec = vap->va_mtime.tv_nsec/1000;
			ip->i_ctime.tv_sec = hrestime.tv_sec;
			ip->i_ctime.tv_usec = hrestime.tv_nsec/1000;
			ip->i_flag &= ~(IUPD|ICHG);
			ip->i_flag |= IMODTIME;
		}
		ip->i_flag |= IMOD;
	}
out:
	if (!(flags & ATTR_EXEC))
		ufs_iupdat(ip, issync);

	IUNLOCK(ip);
	IRWUNLOCK(ip);
	return (error);
}

/*ARGSUSED*/
STATIC int
ufs_access(vp, mode, flags, cr)
	struct vnode *vp;
	int mode;
	int flags;
	struct cred *cr;
{
	register struct inode *ip = VTOI(vp);
	int error;

	ILOCK(ip);
	error = ufs_iaccess(ip, mode, cr);
	ufs_iunlock(ip);
	return (error);
}

/* ARGSUSED */
STATIC int
ufs_readlink(vp, uiop, cr)
	struct vnode *vp;
	struct uio *uiop;
	struct cred *cr;
{
	register struct inode *ip;
	register int error;

	if (vp->v_type != VLNK)
		return EINVAL;
	ip = VTOI(vp);
	error = rwip(ip, uiop, UIO_READ, 0);
	ITIMES(ip);
	return (error);
}

/* ARGSUSED */
STATIC int
ufs_fsync(vp, cr)
	struct vnode *vp;
	struct cred *cr;
{
	register struct inode *ip = VTOI(vp);
	int error;

	ILOCK(ip);
	error = ufs_syncip(ip, 0);	/* Do synchronous writes */
	ITIMES(ip);			/* XXX: is this necessary ??? */
	ufs_iunlock(ip);
	return (error);
}

/*ARGSUSED*/
STATIC void
ufs_inactive(vp, cr)
	struct vnode *vp;
	struct cred *cr;
{

	ufs_iinactive(VTOI(vp));
}

/*
 * Unix file system operations having to do with directory manipulation.
 */
/* ARGSUSED */
STATIC int
ufs_lookup(dvp, nm, vpp, pnp, flags, rdir, cr)
	struct vnode *dvp;
	char *nm;
	struct vnode **vpp;
	struct pathname *pnp;
	int flags;
	struct vnode *rdir;
	struct cred *cr;
{
	register struct inode *ip;
	struct inode *xip;
	register int error;

	/*
	 * Null component name is a synonym for directory being searched.
	 */
	if (*nm == '\0') {
		VN_HOLD(dvp);
		*vpp = dvp;
		return 0;
	}

	ip = VTOI(dvp);
	error = ufs_dirlook(ip, nm, &xip, cr);
	ITIMES(ip);
	if (error == 0) {
		ip = xip;
		*vpp = ITOV(ip);
		if ((ip->i_mode & ISVTX) && !(ip->i_mode & (IEXEC | IFDIR)) &&
		    stickyhack) {
			(*vpp)->v_flag |= VISSWAP;
		}
		ITIMES(ip);
		ufs_iunlock(ip);
		/*
		 * If vnode is a device return special vnode instead.
		 */
		if (ISVDEV((*vpp)->v_type)) {
			struct vnode *newvp;

			newvp = specvp(*vpp, (*vpp)->v_rdev, (*vpp)->v_type,
			    cr);
			VN_RELE(*vpp);
			if (newvp == NULL)
				error = ENOSYS;
			else
				*vpp = newvp;
		}
	}
	return error;
}

STATIC int
ufs_create(dvp, name, vap, excl, mode, vpp, cr)
	struct vnode *dvp;
	char *name;
	struct vattr *vap;
	enum vcexcl excl;
	int mode;
	struct vnode **vpp;
	struct cred *cr;
{
	register int error;
	register struct inode *ip = VTOI(dvp);
	struct inode *xip;

	/* must be super-user to set sticky bit */
	if (cr->cr_uid != 0)
		vap->va_mode &= ~VSVTX;

	if (*name == '\0') {
		/*
		 * Null component name refers to the directory itself.
		 */
		VN_HOLD(dvp);
		ILOCK(ip);
		ITIMES(ip);
		error = EEXIST;
	} else {
		xip = NULL;
		error = ufs_direnter(ip, name, DE_CREATE, (struct inode *) 0,
		  (struct inode *) 0, vap, &xip, cr);
		ITIMES(ip);
		ip = xip;
	}

	/*
	 * If the file already exists and this is a non-exclusive create,
	 * check permissions and allow access for non-directories.
	 * Read-only create of an existing directory is also allowed.
	 * We fail an exclusive create of anything which already exists.
	 */
	if (error == EEXIST) {
		if (excl == NONEXCL) {
			if (((ip->i_mode & IFMT) == IFDIR) && (mode & IWRITE))
				error = EISDIR;
			else if (mode)
				error = ufs_iaccess(ip, mode, cr);
			else
				error = 0;
		}
		if (error)
			ufs_iput(ip);
		else if (((ip->i_mode & IFMT) == IFREG)
		    && (vap->va_mask & AT_SIZE) && vap->va_size == 0) {
			/*
			 * Truncate regular files, if requested by caller.
			 */
			(void) ufs_itrunc(ip, (u_long)0);
		}
	}
	if (error)
		return error;
	*vpp = ITOV(ip);
	ITIMES(ip);
	if (((ip->i_mode & IFMT) == IFREG) && (vap->va_mask & AT_SIZE))
		error = fs_vcode(ITOV(ip), &ip->i_vcode);
	ufs_iunlock(ip);
	/*
	 * If vnode is a device return special vnode instead.
	 */
	if (!error && ISVDEV((*vpp)->v_type)) {
		struct vnode *newvp;

		newvp = specvp(*vpp, (*vpp)->v_rdev, (*vpp)->v_type, cr);
		VN_RELE(*vpp);
		if (newvp == NULL)
			return ENOSYS;
		*vpp = newvp;
	}

	return (error);
}

/*ARGSUSED*/
STATIC int
ufs_remove(vp, nm, cr)
	struct vnode *vp;
	char *nm;
	struct cred *cr;
{
	register struct inode *ip = VTOI(vp);
	register int error;

	error = ufs_dirremove(ip, nm, (struct inode *)0, (struct vnode *)0,
	    DR_REMOVE, cr);
	ITIMES(ip);
	return error;
}

/*
 * Link a file or a directory.  Only the super-user is allowed to make a
 * link to a directory.
 */
STATIC int
ufs_link(tdvp, svp, tnm, cr)
	register struct vnode *tdvp;
	struct vnode *svp;
	char *tnm;
	struct cred *cr;
{
	register struct inode *sip;
	register struct inode *tdp;
	register int error;
	struct vnode *realvp;

	if (VOP_REALVP(svp, &realvp) == 0)
		svp = realvp;
	if (svp->v_type == VDIR && !suser(cr))
		return EPERM;
	sip = VTOI(svp);
	tdp = VTOI(tdvp);
	error = ufs_direnter(tdp, tnm, DE_LINK, (struct inode *) 0,
	    sip, (struct vattr *)0, (struct inode **)0, cr);
	ITIMES(sip);
	ITIMES(tdp);
	return error;
}

/*
 * Rename a file or directory.
 * We are given the vnode and entry string of the source and the
 * vnode and entry string of the place we want to move the source
 * to (the target). The essential operation is:
 *	unlink(target);
 *	link(source, target);
 *	unlink(source);
 * but "atomically".  Can't do full commit without saving state in
 * the inode on disk, which isn't feasible at this time.  Best we
 * can do is always guarantee that the TARGET exists.
 */
/*ARGSUSED*/
STATIC int
ufs_rename(sdvp, snm, tdvp, tnm, cr)
	struct vnode *sdvp;		/* old (source) parent vnode */
	char *snm;			/* old (source) entry name */
	struct vnode *tdvp;		/* new (target) parent vnode */
	char *tnm;			/* new (target) entry name */
	struct cred *cr;
{
	struct inode *sip;		/* source inode */
	register struct inode *sdp;	/* old (source) parent inode */
	register struct inode *tdp;	/* new (target) parent inode */
	register int error;

	sdp = VTOI(sdvp);
	tdp = VTOI(tdvp);
	/*
	 * Look up inode of file we're supposed to rename.
	 */
	if (error = ufs_dirlook(sdp, snm, &sip, cr))
		return error;
	ufs_iunlock(sip);	/* unlock inode (it's held) */
	/*
	 * Make sure we can delete the source entry.  This requires
	 * write permission on the containing directory.  If that
	 * directory is "sticky" it further requires (except for the
	 * super-user) that the user own the directory or the source 
	 * entry, or else have permission to write the source entry.
	 */
	if ((error = ufs_iaccess(sdp, IWRITE, cr)) != 0
	    || ((sdp->i_mode & ISVTX) && cr->cr_uid != 0
	    && cr->cr_uid != sdp->i_uid && cr->cr_uid != sip->i_uid
	    && (error = ufs_iaccess(sip, IWRITE, cr)) != 0)) {
		irele(sip);
		return error;
	}

	/*
	 * Check for renaming '.' or '..' or alias of '.'
	 */
	if (strcmp(snm, ".") == 0 || strcmp(snm, "..") == 0 || sdp == sip) {
		error = EINVAL;
		goto out;
	}

	/*
	 * Link source to the target.
	 */
	if (error = ufs_direnter(tdp, tnm, DE_RENAME, sdp, sip,
	    (struct vattr *)0, (struct inode **)0, cr)) {
		/*
		 * ESAME isn't really an error; it indicates that the
		 * operation should not be done because the source and target
		 * are the same file, but that no error should be reported.
		 */
		if (error == ESAME)
			error = 0;
		goto out;
	}

	/*
	 * Unlink the source.
	 * Remove the source entry.  ufs_dirremove() checks that the entry
	 * still reflects sip, and returns an error if it doesn't.
	 * If the entry has changed just forget about it.  Release
	 * the source inode.
	 */
	if ((error = ufs_dirremove(sdp, snm, sip, (struct vnode *)0,
	    DR_RENAME, cr)) == ENOENT)
		error = 0;

out:
	ITIMES(sdp);
	ITIMES(tdp);
	irele(sip);
	return (error);
}

/*ARGSUSED*/
STATIC int
ufs_mkdir(dvp, dirname, vap, vpp, cr)
	struct vnode *dvp;
	char *dirname;
	register struct vattr *vap;
	struct vnode **vpp;
	struct cred *cr;
{
	register struct inode *ip;
	struct inode *xip;
	register int error;

	ASSERT((vap->va_mask & (AT_TYPE|AT_MODE)) == AT_TYPE|AT_MODE);

	ip = VTOI(dvp);
	error = ufs_direnter(ip, dirname, DE_MKDIR, (struct inode *) 0,
	    (struct inode *) 0, vap, &xip, cr);
	ITIMES(ip);
	if (error == 0) {
		ip = xip;
		*vpp = ITOV(ip);
		ITIMES(ip);
		ufs_iunlock(ip);
	} else if (error == EEXIST)
		ufs_iput(xip);
	return (error);
}

/*ARGSUSED*/
STATIC int
ufs_rmdir(vp, nm, cdir, cr)
	struct vnode *vp;
	char *nm;
	struct vnode *cdir;
	struct cred *cr;
{
	register struct inode *ip = VTOI(vp);
	register int error;

	error = ufs_dirremove(ip, nm, (struct inode *)0, cdir, DR_RMDIR, cr);
	ITIMES(ip);
	return error;
}

/* ARGSUSED */
STATIC int
ufs_readdir(vp, uiop, cr, eofp)
	struct vnode *vp;
	struct uio *uiop;
	struct cred *cr;
	int *eofp;
{
	register struct iovec *iovp;
	register struct inode *ip;
	register struct direct *idp;
	register struct dirent *odp;
	register u_int offset;
	register int incount = 0;
	register int outcount = 0;
	register u_int bytes_wanted, total_bytes_wanted;
	caddr_t outbuf;
	size_t bufsize;
	int error = 0;
	struct fbuf *fbp;
	int fastalloc;
	static caddr_t dirbufp;
	int direntsz;

	ip = VTOI(vp);
	ASSERT((ip)->i_flag & IRWLOCKED);

	iovp = uiop->uio_iov;
	if ((total_bytes_wanted = iovp->iov_len) == 0)
		return (0);

	/* Force offset to be valid (to guard against bogus lseek() values) */
	offset = uiop->uio_offset & ~(DIRBLKSIZ - 1);

	/* Quit if at end of file */
	if (offset >= ip->i_size) {
		if (eofp)
			*eofp = 1;
		return (0);
	}

	/*
	 * Get space to change directory entries into fs independent format.
	 * Do fast alloc for the most commonly used-request size (filesystem
	 * block size).
	 */
	fastalloc = (total_bytes_wanted == MAXBSIZE);
	bufsize = total_bytes_wanted + sizeof (struct dirent);
	if (fastalloc)
		outbuf = (caddr_t) kmem_fast_alloc(&dirbufp, bufsize, 1 ,
			KM_SLEEP);
	else
		outbuf = (caddr_t) kmem_alloc(bufsize, KM_SLEEP);
	odp = (struct dirent *)outbuf;
	direntsz = (char *) odp->d_name - (char *) odp;

	ILOCK(ip);
nextblk:
	bytes_wanted = total_bytes_wanted;

	/* Truncate request to file size */
	if (offset + bytes_wanted > ip->i_size)
		bytes_wanted = ip->i_size - offset;

	/* Comply with MAXBSIZE boundary restrictions of fbread() */
	if ((offset & MAXBOFFSET) + bytes_wanted > MAXBSIZE)
		bytes_wanted = MAXBSIZE - (offset & MAXBOFFSET);

	/* Read in the next chunk */
	if (error = fbread(vp, (long)offset, bytes_wanted, S_OTHER, &fbp))
		goto out;
	ip->i_flag |= IACC;
	incount = 0;
	idp = (struct direct *)fbp->fb_addr;
	if (idp->d_ino == 0 && idp->d_reclen == 0 &&
		idp->d_namlen == 0) {
		cmn_err(CE_WARN, "ufs_readir: bad dir, inumber = %d\n", ip->i_number);
		fbrelse(fbp, S_OTHER);
		error = ENXIO;
		goto out;
	}	
	/* Transform to file-system independent format */
	while (incount < bytes_wanted) {
		/* Skip to requested offset and skip empty entries */
		if (idp->d_ino != 0 && offset >= uiop->uio_offset) {
			odp->d_ino = idp->d_ino;
			odp->d_reclen = (direntsz + idp->d_namlen + 1 + (NBPW-1)) & ~(NBPW-1);
			odp->d_off = offset + idp->d_reclen;
			strcpy(odp->d_name, idp->d_name);
			outcount += odp->d_reclen;
			/* Got as many bytes as requested, quit */
			if (outcount > total_bytes_wanted) {
				outcount -= odp->d_reclen;
				break;
			}
			odp = (struct dirent *)((int)odp + odp->d_reclen);
		}
		incount += idp->d_reclen;
		offset += idp->d_reclen;
		idp = (struct direct *)((int)idp + idp->d_reclen);
	}
	/* Release the chunk */
	fbrelse(fbp, S_OTHER);

	/* Read whole block, but got no entries, read another if not eof */
	if (offset < ip->i_size && !outcount)
		goto nextblk;

	/* Copy out the entry data */
	if (error = uiomove(outbuf, (long)outcount, UIO_READ, uiop))
		goto out;

	uiop->uio_offset = offset;
out:
	ITIMES(ip);
	IUNLOCK(ip);
	if (fastalloc)
		kmem_fast_free(&dirbufp, outbuf);
	else
		kmem_free((void *)outbuf, bufsize);
	if (eofp && error == 0)
		*eofp = (uiop->uio_offset >= ip->i_size);
	return (error);
}

/*ARGSUSED*/
STATIC int
ufs_symlink(dvp, linkname, vap, target, cr)
	register struct vnode *dvp;	/* ptr to parent dir vnode */
	char *linkname;			/* name of symbolic link */
	struct vattr *vap;		/* attributes */
	char *target;			/* target path */
	struct cred *cr;		/* user credentials */
{
	struct inode *ip, *dip = VTOI(dvp);
	register int error;

	ip = (struct inode *)0;
	vap->va_type = VLNK;
	vap->va_rdev = 0;
	if ((error = ufs_direnter(dip, linkname, DE_CREATE,
	    (struct inode *)0, (struct inode *)0, vap, &ip, cr)) == 0) {
		error = ufs_rdwri(UIO_WRITE, ip, target, (int)strlen(target),
		    (off_t)0, UIO_SYSSPACE, (int *)0);
		ufs_iput(ip);
	} else if (error == EEXIST)
		ufs_iput(ip);
	ITIMES(VTOI(dvp));
	return (error);
}

/*
 * Ufs specific routine used to do ufs io.
 */
int
ufs_rdwri(rw, ip, base, len, offset, seg, aresid)
	enum uio_rw rw;
	struct inode *ip;
	caddr_t base;
	int len;
	off_t offset;
	enum uio_seg seg;
	int *aresid;
{
	struct uio auio;
	struct iovec aiov;
	register int error;

	aiov.iov_base = base;
	aiov.iov_len = len;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = offset;
	auio.uio_segflg = (short)seg;
	auio.uio_resid = len;
	error = rwip(ip, &auio, rw, 0);
	if (rw == UIO_WRITE)
		auio.uio_fmode = FWRITE;
	else
		auio.uio_fmode = FREAD;

	if (aresid) {
		*aresid = auio.uio_resid;
	} else if (auio.uio_resid) {
		error = EIO;
	}
	return (error);
}

#ifdef notneeded
/*
 * Record-locking requests are passed to the local Lock-Manager daemon.
 */
STATIC int
ufs_lockctl(vp, ld, cmd, cred, clid)
	struct vnode *vp;
	struct flock *ld;
	int cmd;
	struct cred *cred;
	int clid;
{
	lockhandle_t lh;
	struct fid *fidp;

	/* Convert vnode into lockhandle-id. This is awfully like makefh(). */
	if (VOP_FID(vp, &fidp) || fidp == NULL) {
		return (EINVAL);
	}
	bzero((caddr_t)&lh.lh_id, sizeof (lh.lh_id));	/* clear extra bytes */
	lh.lh_fsid.val[0] = vp->v_vfsp->vfs_fsid.val[0];
	lh.lh_fsid.val[1] = vp->v_vfsp->vfs_fsid.val[1];
	lh.lh_fid.fid_len = fidp->fid_len;
	bcopy(fidp->fid_data, lh.lh_fid.fid_data, fidp->fid_len);
	freefid(fidp);

	/* Add in vnode and server and call to common code */
	lh.lh_vp = vp;
	lh.lh_servername = hostname;
	return (klm_lockctl(&lh, ld, cmd, cred, clid));
}
#endif

STATIC int
ufs_fid(vp, fidpp)
	struct vnode *vp;
	struct fid **fidpp;
{
	register struct ufid *ufid;

	ufid = (struct ufid *)kmem_zalloc(sizeof (struct ufid), KM_SLEEP);
	ufid->ufid_len = sizeof (struct ufid) - sizeof (ushort);
	ufid->ufid_ino = VTOI(vp)->i_number;
	ufid->ufid_gen = VTOI(vp)->i_gen;
	*fidpp = (struct fid *)ufid;
	return 0;
}

STATIC void
ufs_rwlock(vp)
	struct vnode *vp;
{
	IRWLOCK(VTOI(vp));
}

STATIC void
ufs_rwunlock(vp)
	struct vnode *vp;
{
	IRWUNLOCK(VTOI(vp));
}
			
/* ARGSUSED */
STATIC int
ufs_seek(vp, ooff, noffp)
	struct vnode *vp;
	off_t ooff;
	off_t *noffp;
{
	return *noffp < 0 ? EINVAL : 0;
}

/* ARGSUSED */
STATIC int
ufs_frlock(vp, cmd, bfp, flag, offset, cr)
	register struct vnode *vp;
	int cmd;
	struct flock *bfp;
	int flag;
	off_t offset;
	cred_t *cr;
 {
	register struct inode *ip = VTOI(vp);
 
	/*
	 * If file is being mapped, disallow frlock.
	 */
	if (ip->i_mapcnt > 0 && MANDLOCK(vp, ip->i_mode))
		return EAGAIN;
 
	return fs_frlock(vp, cmd, bfp, flag, offset, cr);
}

/* ARGSUSED */
STATIC int
ufs_space(vp, cmd, bfp, flag, offset, cr)
	struct vnode *vp;
	int cmd;
	struct flock *bfp;
	int flag;
	off_t offset;
	struct cred *cr;
{
	int error;

	if (cmd != F_FREESP)
		return EINVAL;
	if ((error = convoff(vp, bfp, 0, offset)) == 0)
		error = ufs_freesp(vp, bfp, flag);
	return error;
}

int ufs_ra = 1;
int ufs_lostpage;	/* number of times we lost original page */

/*
 * Called from pvn_getpages or ufs_getpage to get a particular page.
 * When we are called the inode is already locked.
 *
 *	If rw == S_WRITE and block is not allocated, need to alloc block.
 *	If ppp == NULL, async I/O is requested.
 *
 * bsize is either 4k or 8k.  To handle the case of 4k bsize and 8k pages
 * we will do two reads to get the data and don't bother with read ahead.
 * Thus having 4k file systems on a Sun-3 works, but it is not recommended.
 * N.B. 4k file system not tested yet!
 */
/* ARGSUSED */
STATIC int
ufs_getapage(vp, off, protp, pl, plsz, seg, addr, rw, cr)
	struct vnode *vp;
	register u_int off;
	u_int *protp;
	struct page *pl[];
	u_int plsz;
	struct seg *seg;
	addr_t addr;
	enum seg_rw rw;
	struct cred *cr;
{
	register struct inode *ip;
	register struct fs *fs;
	register int bsize;
	u_int xlen;
	struct buf *bp, *bp2;
	struct vnode *devvp;
	struct page *pp, *pp2, **ppp, *pagefound;
	daddr_t lbn, bn, bn2;
	u_int io_off, io_len;
	u_int lbnoff, blksz;
	int err, nio, do2ndread, pgoff;
	dev_t dev;
	int pgaddr;
	
	ip = VTOI(vp);
	fs = ip->i_fs;
	bsize = fs->fs_bsize;
	devvp = ip->i_devvp;
	dev = devvp->v_rdev;

reread:
	err = 0;
	bp = NULL;
	bp2 = NULL;
	pagefound = NULL;
	pgoff = 0;
	lbn = lblkno(fs, off);
	lbnoff = off & fs->fs_bmask;
	if (pl != NULL)
		pl[0] = NULL;

/*
	pagefound = page_find(vp, off);
	if (pagefound != NULL)
		goto out;
*/

	err = ufs_bmap(ip, lbn, &bn, &bn2, (int)blksize(fs, ip, lbn), rw, 0);

	if (err)
		goto out;

	if (bn == UFS_HOLE && protp != NULL)
		*protp &= ~PROT_WRITE;

	if (PAGESIZE > bsize) {
		if (bsize == PAGESIZE / 2) {
			nio = 2;
			do2ndread = 1;
		} else
			cmn_err(CE_PANIC, "ufs_getapage bad bsize");
	} else {
		nio = 1;
		if (ufs_ra && ip->i_nextr == off && bn2 != UFS_HOLE)
			do2ndread = 1;
		else
			do2ndread = 0;
	}

	if ((pagefound = page_find(vp, off)) == NULL) {
		/*
		 * Page doesn't exist, need to create it.
		 * First compute size we really want to get.
		 */
		if (lbn < NDADDR) {
			/*
			 * Direct block, use standard blksize macro.
			 */
			blksz = blksize(fs, ip, lbn);
		} else {
			/*
			 * Indirect block, round up to smaller of
			 * page boundary or file system block size.
			 */
			blksz = MIN(roundup(ip->i_size, PAGESIZE) - lbnoff,
			    bsize);
		}
		if (bn == UFS_HOLE || off >= lbnoff + blksz) {
			/*
			 * Block for this page is not allocated or the offset
			 * is beyond the current allocation size (from
			 * ufs_bmap) and the page was not found.  If we need
			 * a page, allocate and return a zero page.
			 */
			if (pl != NULL) {
				pp = rm_allocpage(seg, addr, PAGESIZE, 
								P_CANWAIT);
				if (page_enter(pp, vp, off)) {
 					cmn_err(CE_PANIC,
 					    "ufs_getapage page_enter");

				}
				pagezero(pp, 0, PAGESIZE);
				page_unlock(pp);
				pp->p_nio = nio;
				if (nio > 1) {
					pp->p_intrans = pp->p_pagein= 1;
					PAGE_HOLD(pp);
				}
				pl[0] = pp;
				pl[1] = NULL;
			}
			if (protp != NULL)
				*protp &= ~PROT_WRITE;
		} else {
			/*
			 * Need to really do disk I/O to get the page(s).
			 */
			pp = pvn_kluster(vp, off, seg, addr, &io_off, &io_len,
			    lbnoff, blksz, 0);

 			ASSERT(pp != NULL);

			if (pl != NULL) {
				register int sz;

				if (plsz >= io_len) {
					/*
					 * Everything fits, set up to load
					 * up and hold all the pages.
					 */
					pp2 = pp;
					sz = io_len;
				} else {
					/*
					 * Set up to load plsz worth
					 * starting at the needed page.
					 */
					for (pp2 = pp; pp2->p_offset != off;
					    pp2 = pp2->p_next) {
						ASSERT(pp2->p_next->p_offset !=
						    pp->p_offset);
					}
					sz = plsz;
				}

				ppp = pl;
				do {
					PAGE_HOLD(pp2);
					*ppp++ = pp2;
					pp2 = pp2->p_next;
					sz -= PAGESIZE;
				} while (sz > 0);
				*ppp = NULL;		/* terminate list */
			}

			if (nio == 2)
				pp->p_nio = 2;

			bp = pageio_setup(pp, io_len, devvp, pl == NULL ?
			    (B_ASYNC | B_READ) : B_READ);

			bp->b_edev = dev;
			bp->b_dev = cmpdev(dev);
			bp->b_blkno = fsbtodb(fs, bn) +
			    btodb(blkoff(fs, io_off));

			/*
			 * Zero part of page which we are not
			 * going to be reading from disk now.
			 */
			xlen = io_len & PAGEOFFSET;
			if (xlen != 0)
				pagezero(pp->p_prev, xlen, PAGESIZE - xlen);

			(*bdevsw[getmajor(dev)].d_strategy)(bp);

			ip->i_nextr = io_off + io_len;
#ifdef notneeded
			u.u_ru.ru_majflt++;
			if (seg == segkmap)
				u.u_ru.ru_inblock++;	/* count as `read' */
#endif
			vminfo.v_pgin++;
			vminfo.v_pgpgin += btopr(io_len);
		}
	}

	lbn++;
	lbnoff += fs->fs_bsize;

	if (do2ndread && lbnoff < ip->i_size) {
		addr_t addr2;

		addr2 = addr + (lbnoff - off);

		if (lbn < NDADDR) {
			/*
			 * Direct block, use standard blksize macro.
			 */
			blksz = blksize(fs, ip, lbn);
		} else {
			/*
			 * Indirect block, round up to smaller of
			 * page boundary or file system block size.
			 */
			blksz = MIN(roundup(ip->i_size, PAGESIZE) - lbnoff,
			    bsize);
		}

		/*
		 * Either doing read-ahead (nio == 1)
		 * or second read for page (nio == 2).
		 */
		if (nio == 1) {
			/*
			 * If addr is now in a different seg,
			 * don't bother with read-ahead.
			 */
			if (addr2 >= seg->s_base + seg->s_size)
				pp2 = NULL;
			else {
				pp2 = pvn_kluster(vp, lbnoff, seg, addr2,
				    &io_off, &io_len, lbnoff, blksz, 1);
			}
			pgoff = 0;
		} else {
			if (bn2 == UFS_HOLE || rw == S_WRITE) {
				/*
				 * Read-ahead number didn't suffice,
				 * retry as primary block now.  Note
				 * that if we are doing allocation
				 * with nio == 2, we will force both
				 * logical blocks to be allocated.
				 */
				err = ufs_bmap(ip, lbn, &bn2, (daddr_t *)0,
				    (int)blksize(fs, ip, lbn), rw, 0);
				if (err)
					goto out;
				if (bn2 == UFS_HOLE) {
					if (protp != NULL)
						*protp &= ~PROT_WRITE;
					goto out;
				}
			}
			pp2 = pp;
			io_len = blksz;
			io_off = off;
			pgoff = bsize;
		}

		if (pp2 != NULL) {
			bp2 = pageio_setup(pp2, io_len, devvp,
			    (pl == NULL || nio == 1) ?
			    (B_ASYNC | B_READ) : B_READ);

			bp2->b_edev = dev;
			bp2->b_dev = cmpdev(dev);
			bp2->b_blkno = fsbtodb(fs, bn2) +
			    btodb(blkoff(fs, io_off));
			bp2->b_un.b_addr = (caddr_t)pgoff;

			/*
			 * Zero part of page which we are not
			 * going to be reading from disk now.
			 */
			xlen = (io_len + pgoff) & PAGEOFFSET;
			if (xlen != 0)
				pagezero(pp2->p_prev, xlen, PAGESIZE - xlen);

			(*bdevsw[getmajor(dev)].d_strategy)(bp2);
			vminfo.v_pgin++;
			vminfo.v_pgpgin += btopr(io_len);
		}
	}

out:
	if (bp != NULL && pl != NULL) {
		if (err == 0)
			err = biowait(bp);
		else
			(void) biowait(bp);
		pageio_done(bp);
		if (nio == 2 && bp2 != NULL) {
			if (err == 0)
				err = biowait(bp2);
			else
				(void) biowait(bp2);
			pageio_done(bp2);
		}
	} else if (pagefound != NULL) {
		int s;

		/*
		 * We need to be careful here because if the page was
		 * previously on the free list, we might have already
		 * lost it at interrupt level.
		 */
		s = splvm();
		if (pagefound->p_vnode == vp && pagefound->p_offset == off) {
			/*
			 * If the page is still intransit or if
			 * it is on the free list call page_lookup
			 * to try and wait for / reclaim the page.
			 */
			if (pagefound->p_intrans || pagefound->p_free)
				pagefound = page_lookup(vp, off);
		}
		(void) splx(s);
		if (pagefound == NULL || pagefound->p_offset != off ||
		    pagefound->p_vnode != vp || pagefound->p_gone) {
			ufs_lostpage++;
			goto reread;
		}
		if (pl != NULL) {
			PAGE_HOLD(pagefound);
			pl[0] = pagefound;
			pl[1] = NULL;
			ip->i_nextr = off + PAGESIZE;
		}
	}

	if (err && pl != NULL) {
		for (ppp = pl; *ppp != NULL; *ppp++ = NULL)
			PAGE_RELE(*ppp);
	}

	return (err);
}

/*
 * Return all the pages from [off..off+len) in given file
 */
STATIC int
ufs_getpage(vp, off, len, protp, pl, plsz, seg, addr, rw, cr)
	struct vnode *vp;
	u_int off;
	u_int len;
	u_int *protp;
	struct page *pl[];
	u_int plsz;
	struct seg *seg;
	addr_t addr;
	enum seg_rw rw;
	struct cred *cr;
{
	struct inode *ip = VTOI(vp);
	int err;

	if (vp->v_flag & VNOMAP)
		return (ENOSYS);

	ILOCK(ip);

	/*
	 * This check for beyond EOF allows the request to extend up to
	 * the page boundary following the EOF.	 Strictly speaking,
	 * it should be (off + len > (ip->i_size + PAGEOFFSET) % PAGESIZE),
	 * but in practice, this is equivalent and faster.
	 *
	 * Also, since we may be called as a side effect of a bmap or
	 * dirsearch() using fbread() when the blocks might be being
	 * allocated and the size has not yet been up'ed.  In this case
	 * we disable the check and always allow the getpage to go through
	 * if the segment is seg_map, since we want to be able to return
	 * zeroed pages if bmap indicates a hole in the non-write case.
	 * For ufs, we also might have to read some frags from the disk
	 * into a page if we are extending the number of frags for a given
	 * lbn in ufs_bmap().
	 */
	if (off + len > ip->i_size + PAGEOFFSET &&
			!(seg == segkmap && rw == S_OTHER)) {
		IUNLOCK(ip);
		return (EFAULT);	/* beyond EOF */
	}

	if (protp != NULL)
		*protp = PROT_ALL;

	if (len <= PAGESIZE)
		err = ufs_getapage(vp, off, protp, pl, plsz, seg, addr,
		    rw, cr);
	else
		err = pvn_getpages(ufs_getapage, vp, off, len, protp, pl, plsz,
		    seg, addr, rw, cr);

	/*
	 * If the inode is not already marked for IACC (in rwip() for read)
	 * and the inode is not marked for no access time update (in rwip()
	 * for write) then update the inode access time and mod time now.
	 */

	if ((ip->i_flag & (IACC | INOACC)) == 0) {
		if (rw != S_OTHER)
			ip->i_flag |= IACC;
		if (rw == S_WRITE)
			ip->i_flag |= IUPD;
		ITIMES(ip);
	}

	IUNLOCK(ip);

	return (err);
}

/*
 * Flags are composed of {B_ASYNC, B_INVAL, B_FREE, B_DONTNEED}
 */
STATIC int
ufs_writelbn(ip, bn, pp, len, pgoff, flags)
	register struct inode *ip;
	daddr_t bn;
	struct page *pp;
	u_int len;
	u_int pgoff;
	int flags;
{
	struct buf *bp;
	int err;

	if ((bp = pageio_setup(pp, len, ip->i_devvp, B_WRITE | flags))
	    == NULL) {
		pvn_fail(pp, B_WRITE | flags);
		return (ENOMEM);
	}

	bp->b_edev = ip->i_dev;
	bp->b_dev = cmpdev(ip->i_dev);
	bp->b_blkno = bn;
	bp->b_blkno = bn;
	bp->b_un.b_addr = (addr_t)pgoff;

	(*bdevsw[getmajor(ip->i_dev)].d_strategy)(bp);

	/*
	 * If async, assume that pvn_done will handle the pages
	 * when I/O is done.
	 */
	if (flags & B_ASYNC)
		return (0);

	err = biowait(bp);
	pageio_done(bp);

	return (err);
}

/*
 * Flags are composed of {B_ASYNC, B_INVAL, B_FREE, B_DONTNEED, B_FORCE}
 * If len == 0, do from off to EOF.
 *
 * The normal cases should be len == 0 & off == 0 (entire vp list),
 * len == MAXBSIZE (from segmap_release actions), and len == PAGESIZE
 * (from pageout).
 *
 * Note that for ufs it is possible to have dirty pages beyond
 * roundup(ip->i_size, PAGESIZE).  This can happen the file
 * length is long enough to involve indirect blocks (which are
 * always fs->fs_bsize'd) and PAGESIZE < bsize while the length
 * is such that roundup(blkoff(fs, ip->i_size), PAGESIZE) < bsize.
 */
/* ARGSUSED */
STATIC int
ufs_putpage(vp, off, len, flags, cr)
	register struct vnode *vp;
	u_int off, len;
	int flags;
	struct cred *cr;
{
	register struct inode *ip;
	register struct page *pp;
	register struct fs *fs;
	struct page *dirty, *io_list;
	register u_int io_off, io_len;
	daddr_t lbn, bn;
	u_int lbn_off;
	int bsize;
	int vpcount;
	int err = 0;

	ip = VTOI(vp);

	if (vp->v_flag & VNOMAP)
		return (ENOSYS);

	/*
	 * The following check is just for performance
	 * and therefore doesn't need to be foolproof.
	 * The subsequent code will gracefully do nothing
	 * in any case.
	 */
	if (vp->v_pages == NULL || off >= ip->i_size)
		return (0);

	/*
	 * Return if an iinactive is already in progress on this
	 * inode since the page will be written out by that process.
	 */
	if ((ip->i_flag & ILOCKED) && (ip->i_owner != curproc->p_slot) &&
	    (ip->i_flag & IINACTIVE))
		return (EAGAIN);

	vpcount = vp->v_count;
	VN_HOLD(vp);
	fs = ip->i_fs;

again:
	if (len == 0) {
		/*
		 * Search the entire vp list for pages >= off
		 */
		pvn_vplist_dirty(vp, off, flags);
		/*
		 * We have just sync'ed back all the pages
		 * on the inode; turn off the IMODTIME flag.
		 */
		if (off == 0)
			ip->i_flag &= ~IMODTIME;

		goto vcount_chk;
	} else {
		/*
		 * Do a range from [off...off + len) via page_find.
		 * We set limits so that we kluster to bsize boundaries.
		 */
		if (off >= ip->i_size)
			dirty = NULL;
		else {
			u_int fsize, eoff;

			/*
			 * Use MAXBSIZE rounding to get indirect block pages
			 * which might beyond roundup(ip->i_size, PAGESIZE);
			 */
			fsize = (ip->i_size + MAXBOFFSET) & MAXBMASK;
			eoff = MIN(off + len, fsize);
			dirty = pvn_range_dirty(vp, off, eoff,
			    (u_int)(off & fs->fs_bmask),
			    (u_int)((eoff + fs->fs_bsize - 1) & fs->fs_bmask),
			    flags);
		}
	}

	/*
	 * Now pp will have the list of kept dirty pages marked for
	 * write back.  All the pages on the pp list need to still
	 * be dealt with here.  Verify that we can really can do the
	 * write back to the filesystem and if not and we have some
	 * dirty pages, return an error condition.
	 */
	if (fs->fs_ronly && dirty != NULL)
		err = EROFS;
	else
		err = 0;

	if (dirty != NULL) {
		/*
		 * Destroy the read ahead value now since we are
		 * really going to write.
		 */
		ip->i_nextr = 0;

		/*
		 * This is an attempt to clean up loose ends left by
		 * applications that store into mapped files.  It's
		 * insufficient, strictly speaking, for ill-behaved
		 * applications, but about the best we can do.
		 */
		if ((ip->i_flag & IMODTIME) == 0 || (flags & B_FORCE)) {
			ip->i_flag |= IUPD;
			ITIMES(ip);
			if (vp->v_type == VREG)
				err = fs_vcode(vp, &ip->i_vcode);
		}
	}

	/*
	 * Handle all the dirty pages not yet dealt with.
	 */
	while (err == 0 && (pp = dirty) != NULL) {
		/*
		 * Pull off a contiguous chunk that fixes in one lbn
		 */
		io_off = pp->p_offset;
		lbn = lblkno(fs, io_off);
		bsize = blksize(fs, ip, lbn);

		/*
		 * Since the blocks should already be allocated for
		 * any dirty pages, we only need to use S_OTHER
		 * here and we should not get back a bn == UFS_HOLE.
		 */
		if (err = ufs_bmap(ip, lbn, &bn, (daddr_t *)0, bsize, S_OTHER,
		    0))
			break;
		ASSERT(bn != UFS_HOLE);	/* only true since bsize >= PAGESIZE */
		page_sub(&dirty, pp);
		io_list = pp;
		io_len = PAGESIZE;
		lbn_off = lbn << fs->fs_bshift;

		while (dirty != NULL && dirty->p_offset < lbn_off + bsize &&
		    dirty->p_offset == io_off + io_len) {
			pp = dirty;
			page_sub(&dirty, pp);
			page_sortadd(&io_list, pp);
			io_len += PAGESIZE;
		}

		/*
		 * Check for page length rounding problems.
		 */
		if (io_off + io_len > lbn_off + bsize) {
			ASSERT((io_off+io_len) - (lbn_off+bsize) < PAGESIZE);
			io_len = lbn_off + bsize - io_off;
		}

		/*
		 * I/O may be asynch, so need to set nio first.
		 */
		if (fs->fs_bsize < PAGESIZE)
			pp->p_nio = PAGESIZE / fs->fs_bsize;

		/*
		 * XXX - should zero any bytes beyond EOF.
		 */

		bn = fsbtodb(fs, bn) + btodb(io_off - lbn_off);
		err = ufs_writelbn(ip, bn, io_list, io_len, 0, flags);

		/*
		 * See if we need to do a 2nd write of the same page.
		 * This is needed if the file system block size is
		 * less than the pagesize and we are not at EOF yet.
		 */
		if (fs->fs_bsize < PAGESIZE && ip->i_size > lbn_off + bsize) {
			if (err) {
				pvn_fail(pp, B_WRITE | flags);
			} else {
				bsize = blksize(fs, ip, ++lbn);
				err = ufs_bmap(ip, lbn, &bn, (daddr_t *)0,
				    bsize, S_OTHER, 0);
				if (err) {
					pvn_fail(pp, B_WRITE | flags);
					break;
				}
				ASSERT(bn != UFS_HOLE);
				err = ufs_writelbn(ip, fsbtodb(fs, bn), pp,
				    (u_int)bsize, (u_int)fs->fs_bsize, flags);
			}
		}
	}

	if (err) {
		if (dirty != NULL)
			pvn_fail(dirty, B_WRITE | flags);
	} else if (off == 0 && len >= ip->i_size) {
		/*
		 * If doing "synchronous invalidation" make
		 * sure that all the pages are actually gone.
		 */
		 if ((flags & (B_INVAL | B_ASYNC)) == B_INVAL
		   && !pvn_vpempty(vp))
		goto again;

		/*
		 * If we have just sync'ed back all the pages on
		 * the inode, we can turn off the IMODTIME flag.
		 */
		ip->i_flag &= ~IMODTIME;

	}
	/*
	 * Instead of using VN_RELE here we are careful to
	 * call the inactive routine only if the vnode
	 * reference count is now zero but wasn't zero
	 * coming into putpage.  This is to prevent calling
	 * the inactive routine on a vnode that is already
	 * considered to be in the "inactive" state.
	 * XXX -- inactive is a relative term here (sigh).
	 */
vcount_chk:
	if (--vp->v_count == 0 && vpcount > 0)
		ufs_iinactive(ip);
	return (err);
}

/* ARGSUSED */
STATIC int
ufs_map(vp, off, as, addrp, len, prot, maxprot, flags, cr)
	struct vnode *vp;
	uint off;
	struct as *as;
	addr_t *addrp;
	uint len;
	u_char prot, maxprot;
	uint flags;
	struct cred *cr;
{
	struct segvn_crargs vn_a;
	int error = 0;

	if (vp->v_flag & VNOMAP)
		return (ENOSYS);

	if ((int)off < 0 || (int)(off + len) < 0)
		return (EINVAL);

	if (vp->v_type != VREG)
		return (ENODEV);

 
	/*
	 * If file is being locked, disallow mapping.
	 */
	if (vp->v_filocks != NULL && MANDLOCK(vp, VTOI(vp)->i_mode))
		return EAGAIN;

	if ((flags & MAP_FIXED) == 0) {
		map_addr(addrp, len, (off_t)off, 1);
		if (*addrp == NULL)
			return (ENOMEM);
	} else {
		/*
		 * User specified address - blow away any previous mappings
		 */
		(void) as_unmap(as, *addrp, len);
	}       

	ILOCK(VTOI(vp));
	(void) ufs_allocmap(VTOI(vp));

	vn_a.vp = vp;
	vn_a.offset = off;
	vn_a.type = flags & MAP_TYPE;
	vn_a.prot = prot;
	vn_a.maxprot = maxprot;
	vn_a.cred = cr;
	vn_a.amp = NULL;

	error = as_map(as, *addrp, len, segvn_create, (caddr_t)&vn_a);
	IUNLOCK(VTOI(vp));
	return error;	
}

/* ARGSUSED */
STATIC int
ufs_addmap(vp, off, as, addr, len, prot, maxprot, flags, cr)
	struct vnode *vp;
	uint off;
	struct as *as;
	addr_t addr;
	uint len;
	u_char prot, maxprot;
	uint flags;
	struct cred *cr;
{
	if (vp->v_flag & VNOMAP)
		return (ENOSYS);

	VTOI(vp)->i_mapcnt += btopr(len);
	return 0;
}

/*ARGSUSED*/
STATIC int
ufs_delmap(vp, off, as, addr, len, prot, maxprot, flags, cr)
	struct vnode *vp;
	u_int off;
	struct as *as;
	addr_t addr;
	u_int len;
	u_int prot, maxprot;
	u_int flags;
	struct cred *cr;
{
	struct inode *ip;

	if (vp->v_flag & VNOMAP)
		return (ENOSYS);

	ip = VTOI(vp);
	ip->i_mapcnt -= btopr(len); 	/* Count released mappings */
	ASSERT(ip->i_mapcnt >= 0);
	return (0);
}

STATIC int
ufs_allocstore(vp, off, len, cred)
	struct vnode	*vp;
	u_int		off;
	u_int		len;
	struct cred	*cred;
{
	struct inode	*ip = VTOI(vp);
	struct fs	*fs = ip->i_fs;
	daddr_t lbn, llbn;
	u_int	bsize;
	int	err = 0;

	ASSERT(off + len <= ip->i_size);

	ILOCK(ip);

	lbn = lblkno(fs, off);
	llbn = lblkno(fs, off + len - 1);

	while (lbn <= llbn && err == 0) {
		bsize = blksize(fs, ip, lbn);
		err = ufs_bmap(ip, lbn++, NULL, NULL, bsize, S_WRITE, 1);
	}

	IUNLOCK(ip);

	return err;
}
