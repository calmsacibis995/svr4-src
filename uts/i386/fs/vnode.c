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

#ident  "@(#)kern-fs:vnode.c	1.3.1.4"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/uio.h"
#include "sys/file.h"
#include "sys/pathname.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/stat.h"
#include "sys/mode.h"
#include "sys/conf.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/sysmacros.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"

#include "sys/disp.h"
#include "sys/systm.h"

/*
 * Convert stat(2) formats to vnode types and vice versa.  (Knows about
 * numerical order of S_IFMT and vnode types.)
 */
enum vtype iftovt_tab[] = {
	VNON, VFIFO, VCHR, VNON, VDIR, VXNAM, VBLK, VNON,
	VREG, VNON, VLNK, VNON, VNON, VNON, VNON, VNON
};

u_short vttoif_tab[] = {
	0, S_IFREG, S_IFDIR, S_IFBLK, S_IFCHR, S_IFLNK, S_IFIFO, S_IFNAM, 0
};

/*
 * Read or write a vnode.  Called from kernel code.
 */
int
vn_rdwr(rw, vp, base, len, offset, seg, ioflag, ulimit, cr, residp)
	enum uio_rw rw;
	struct vnode *vp;
	caddr_t base;
	int len;
	off_t offset;
	enum uio_seg seg;
	int ioflag;
	long ulimit;		/* meaningful only if rw is UIO_WRITE */
	cred_t *cr;
	int *residp;
{
	struct uio uio;
	struct iovec iov;
	int error;

	if (rw == UIO_WRITE && (vp->v_vfsp->vfs_flag & VFS_RDONLY))
		return EROFS;

	iov.iov_base = base;
	iov.iov_len = len;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = offset;
	uio.uio_segflg = (short)seg;
	uio.uio_resid = len;
	uio.uio_limit = ulimit;
	VOP_RWLOCK(vp);
	if (rw == UIO_WRITE) {
		uio.uio_fmode = FWRITE;
		error = VOP_WRITE(vp, &uio, ioflag, cr);
	} else {
		uio.uio_fmode = FREAD;
		error = VOP_READ(vp, &uio, ioflag, cr);
	}
	VOP_RWUNLOCK(vp);
	if (residp)
		*residp = uio.uio_resid;
	else if (uio.uio_resid)
		error = EIO;
	return error;
}

/*
 * Release a vnode.  Decrements reference count and calls
 * VOP_INACTIVE on last reference.
 */
void
vn_rele(vp)
	register struct vnode *vp;
{
#ifdef DEBUG
	if( vp->v_count > 32000 )
		call_demon();
#endif

	ASSERT(vp->v_count != 0);
	if (--vp->v_count == 0)
		VOP_INACTIVE(vp, u.u_cred);
}

/*
 * Open/create a vnode.
 * This may be callable by the kernel, the only known use
 * of user context being that the current user credentials
 * are used for permissions.  crwhy is defined iff filemode & FCREAT.
 */
int
vn_open(pnamep, seg, filemode, createmode, vpp, crwhy)
	char *pnamep;
	enum uio_seg seg;
	register int filemode;
	int createmode;
	struct vnode **vpp;
	enum create crwhy;
{
	struct vnode *vp;
	register int mode;
	register int error;

	mode = 0;
	if (filemode & FREAD)
		mode |= VREAD;
	if (filemode & (FWRITE|FTRUNC))
		mode |= VWRITE;
 
	if (filemode & FCREAT) {
		struct vattr vattr;
		enum vcexcl excl;

		/*
		 * Wish to create a file.
		 */
		vattr.va_type = VREG;
		vattr.va_mode = createmode;
		vattr.va_mask = AT_TYPE|AT_MODE;
		if (filemode & FTRUNC) {
			vattr.va_size = 0;
			vattr.va_mask |= AT_SIZE;
		}
		if (filemode & FEXCL)
			excl = EXCL;
		else
			excl = NONEXCL;
		filemode &= ~(FTRUNC|FEXCL);
		
		/* 
		 * vn_create can take a while, so preempt.
		 */
		PREEMPT();
		if (error =
		  vn_create(pnamep, seg, &vattr, excl, mode, &vp, crwhy))
			return error;
		PREEMPT();
	} else {
		/*
		 * Wish to open a file.  Just look it up.
		 */
		if (error = lookupname(pnamep, seg, FOLLOW, NULLVPP, &vp))
			return error;
		/*
		 * Can't write directories, active texts, or
		 * read-only filesystems.  Can't truncate files
		 * on which mandatory locking is in effect.
		 */
		if (filemode & (FWRITE|FTRUNC)) {
			struct vattr vattr;

			if (vp->v_type == VDIR) {
				error = EISDIR;
				goto out;
			}
			if (vp->v_vfsp->vfs_flag & VFS_RDONLY) {
				error = EROFS;
				goto out;
			}
			/*
			 * Can't truncate files on which mandatory locking
			 * is in effect.
			 */
			if ((filemode & FTRUNC) && vp->v_filocks != NULL) {
				vattr.va_mask = AT_MODE;
				if ((error =
				    VOP_GETATTR(vp, &vattr, 0, u.u_cred)) == 0
				  && MANDLOCK(vp, vattr.va_mode))
					error = EAGAIN;
			}
			if (error)
				goto out;
		}
		/*
		 * Check permissions.
		 */
		if (error = VOP_ACCESS(vp, mode, 0, u.u_cred))
			goto out;
	}
	/*
	 * Do opening protocol.
	 */
	error = VOP_OPEN(&vp, filemode, u.u_cred);
	/*
	 * Truncate if required.
	 */
	if (error == 0 && (filemode & FTRUNC)) {
		struct vattr vattr;

		vattr.va_size = 0;
		vattr.va_mask = AT_SIZE;
		if (error = VOP_SETATTR(vp, &vattr, 0, u.u_cred))
			(void) VOP_CLOSE(vp, filemode, 1, 0, u.u_cred);
	}
out:
	if (error) {
		VN_RELE(vp);
	} else
		*vpp = vp;
	return error;
}

/*
 * Create a vnode (makenode).
 */
int
vn_create(pnamep, seg, vap, excl, mode, vpp, why)
	char *pnamep;
	enum uio_seg seg;
	struct vattr *vap;
	enum vcexcl excl;
	int mode;
	struct vnode **vpp;
	enum create why;
{
	struct vnode *dvp;	/* ptr to parent dir vnode */
	struct pathname pn;
	register int error;

	ASSERT((vap->va_mask & (AT_TYPE|AT_MODE)) == (AT_TYPE|AT_MODE));

	/*
	 * Lookup directory.
	 * If new object is a file, call lower level to create it.
	 * Note that it is up to the lower level to enforce exclusive
	 * creation, if the file is already there.
	 * This allows the lower level to do whatever
	 * locking or protocol that is needed to prevent races.
	 * If the new object is directory call lower level to make
	 * the new directory, with "." and "..".
	 */
	if (error = pn_get(pnamep, seg, &pn))
		return error;
	dvp = NULL;
	*vpp = NULL;
	/*
	 * lookup will find the parent directory for the vnode.
	 * When it is done the pn holds the name of the entry
	 * in the directory.
	 * If this is a non-exclusive create we also find the node itself.
	 */
	if (excl == EXCL) 
		error = lookuppn(&pn, NO_FOLLOW, &dvp, NULLVPP); 
	else 
		error = lookuppn(&pn, FOLLOW, &dvp, vpp); 
	if (error) {
		pn_free(&pn);
		if (why == CRMKDIR && error == EINVAL)
			error = EEXIST;		/* SVID */
		return error;
	}

	if (why != CRMKNOD)
		vap->va_mode &= ~VSVTX;

	/*
	 * Make sure filesystem is writeable.
	 */
	if (dvp->v_vfsp->vfs_flag & VFS_RDONLY) {
		if (*vpp)
			VN_RELE(*vpp);
		error = EROFS;
	} else if (excl == NONEXCL && *vpp != NULL) {
		register struct vnode *vp = *vpp;

		/*
		 * File already exists.  If a mandatory lock has been
		 * applied, return EAGAIN.
		 */
		if (vp->v_filocks != NULL) {
			struct vattr vattr;

			if (error = VOP_GETATTR(vp, &vattr, 0, u.u_cred)) {
				VN_RELE(vp);
				goto out;
			}
			if (MANDLOCK(vp, vattr.va_mode)) {
				error = EAGAIN;
				VN_RELE(vp);
				goto out;
			}
		}

		/*
		 * If the file is the root of a VFS, we've crossed a
		 * mount point and the "containing" directory that we
		 * acquired above (dvp) is irrelevant because it's in
		 * a different file system.  We apply VOP_CREATE to the
		 * target itself instead of to the containing directory
		 * and supply a null path name to indicate (conventionally)
		 * the node itself as the "component" of interest.
		 *
		 * The intercession of the file system is necessary to
		 * ensure that the appropriate permission checks are
		 * done.
		 */
		if (vp->v_flag & VROOT) {
			ASSERT(why != CRMKDIR);
			error =
			  VOP_CREATE(vp, "", vap, excl, mode, vpp, u.u_cred);
			/*
			 * If the create succeeded, it will have created
			 * a new reference to the vnode.  Give up the
			 * original reference.
			 */
			VN_RELE(vp);
			goto out;
		}

		/*
		 * We throw the vnode away to let VOP_CREATE
		 * truncate the file in a non-racy manner.
		 */
		VN_RELE(vp);
	}

	if (error == 0) {
		/*
		 * Call mkdir() if specified, otherwise create().
		 */
		if (why == CRMKDIR)
			error =
			  VOP_MKDIR(dvp, pn.pn_path, vap, vpp, u.u_cred);
		else
			error = VOP_CREATE(dvp, pn.pn_path, vap,
			  excl, mode, vpp, u.u_cred);
	}
out:
	pn_free(&pn);
	VN_RELE(dvp);
	return error;
}

/*
 * Link.
 */
int
vn_link(from, to, seg)
	char *from;
	char *to;
	enum uio_seg seg;
{
	struct vnode *fvp;		/* from vnode ptr */
	struct vnode *tdvp;		/* to directory vnode ptr */
	struct pathname pn;
	register int error;
	struct vattr vattr;
	long fsid;

	fvp = tdvp = NULL;
	if (error = pn_get(to, seg, &pn))
		return error;
	if (error = lookupname(from, seg, NO_FOLLOW, NULLVPP, &fvp))
		goto out;

	if (error = lookuppn(&pn, NO_FOLLOW, &tdvp, NULLVPP))
		goto out;

	/*
	 * Make sure both source vnode and target directory vnode are
	 * in the same vfs and that it is writeable.
	 */
	if (error = VOP_GETATTR(fvp, &vattr, 0, u.u_cred))
		goto out;
	fsid = vattr.va_fsid;
	if (error = VOP_GETATTR(tdvp, &vattr, 0, u.u_cred))
		goto out;
	if (fsid != vattr.va_fsid) {
		error = EXDEV;
		goto out;
	}
	if (tdvp->v_vfsp->vfs_flag & VFS_RDONLY) {
		error = EROFS;
		goto out;
	}
	/*
	 * Do the link.
	 */
	error = VOP_LINK(tdvp, fvp, pn.pn_path, u.u_cred);
out:
	pn_free(&pn);
	if (fvp)
		VN_RELE(fvp);
	if (tdvp)
		VN_RELE(tdvp);
	return error;
}

/*
 * Rename.
 */
int
vn_rename(from, to, seg)
	char *from;
	char *to;
	int seg;
{
	struct vnode *fdvp;		/* from directory vnode ptr */
	struct vnode *fvp;		/* from vnode ptr */
	struct vnode *tdvp;		/* to directory vnode ptr */
	struct pathname fpn;		/* from pathname */
	struct pathname tpn;		/* to pathname */
	register int error;

	fdvp = tdvp = fvp = NULL;
	/*
	 * Get to and from pathnames.
	 */
	if (error = pn_get(from, seg, &fpn))
		return error;
	if (error = pn_get(to, seg, &tpn)) {
		pn_free(&fpn);
		return error;
	}
	/*
	 * Lookup to and from directories.
	 */
	if (error = lookuppn(&fpn, NO_FOLLOW, &fdvp, &fvp))
		goto out;
	/*
	 * Make sure there is an entry.
	 */
	if (fvp == NULL) {
		error = ENOENT;
		goto out;
	}
	if (error = lookuppn(&tpn, NO_FOLLOW, &tdvp, NULLVPP))
		goto out;
	/*
	 * Make sure both the from vnode and the to directory are
	 * in the same vfs and that it is writable.
	 */
	if ((fvp->v_vfsp != tdvp->v_vfsp) ||
	    (fdvp->v_vfsp != tdvp->v_vfsp)) {
		error = EXDEV;
		goto out;
	}
	if (tdvp->v_vfsp->vfs_flag & VFS_RDONLY) {
		error = EROFS;
		goto out;
	}
	/*
	 * Do the rename.
	 */
	error = VOP_RENAME(fdvp, fpn.pn_path, tdvp, tpn.pn_path, u.u_cred);
out:
	pn_free(&fpn);
	pn_free(&tpn);
	if (fvp)
		VN_RELE(fvp);
	if (fdvp)
		VN_RELE(fdvp);
	if (tdvp)
		VN_RELE(tdvp);
	return error;
}

/*
 * Remove a file or directory.
 */
int
vn_remove(fnamep, seg, dirflag)
	char *fnamep;
	enum uio_seg seg;
	enum rm dirflag;
{
	struct vnode *vp;		/* entry vnode */
	struct vnode *dvp;		/* ptr to parent dir vnode */
	struct vnode *coveredvp;
	struct pathname pn;		/* name of entry */
	enum vtype vtype;
	register int error;
	register struct vfs *vfsp;

	if (error = pn_get(fnamep, seg, &pn))
		return error;
	vp = NULL;
	if (error = lookuppn(&pn, NO_FOLLOW, &dvp, &vp)) {
		pn_free(&pn);
		return error;
	}

	/*
	 * Make sure there is an entry.
	 */
	if (vp == NULL) {
		error = ENOENT;
		goto out;
	}

	vfsp = vp->v_vfsp;

	/*
	 * If the named file is the root of a mounted filesystem, fail,
	 * unless it's marked unlinkable.  In that case, unmount the
	 * filesystem and proceed with the covered vnode.
	 */
	if (vp->v_flag & VROOT) {
		if (vfsp->vfs_flag & VFS_UNLINKABLE) {
			coveredvp = vfsp->vfs_vnodecovered;
			VN_HOLD(coveredvp);
			VN_RELE(vp);
			if ((error = dounmount(vfsp, u.u_cred)) == 0) {
				vp = coveredvp;
				vfsp = vp->v_vfsp;
			} else {
				vp = NULL;
				VN_RELE(coveredvp);
				goto out;
			}
		} else {
			error = EBUSY;
			goto out;
		}
	}

	/*
	 * Make sure filesystem is writeable.
	 */
	if (vfsp && vfsp->vfs_flag & VFS_RDONLY) {
		error = EROFS;
		goto out;
	}

	/*
	 * Release vnode before removing.
	 */
	vtype = vp->v_type;
	VN_RELE(vp);
	vp = NULL;
	if (dirflag == RMDIRECTORY) {
		/*
		 * Caller is using rmdir(2), which can only be applied to
		 * directories.
		 */
		if (vtype != VDIR)
			error = ENOTDIR;
		else
			error = VOP_RMDIR(dvp, pn.pn_path, u.u_cdir, u.u_cred);
	} else {
		/*
		 * Unlink(2) can be applied to anything.
		 */
		error = VOP_REMOVE(dvp, pn.pn_path, u.u_cred);
	}
		
out:
	pn_free(&pn);
	if (vp != NULL)
		VN_RELE(vp);
	VN_RELE(dvp);
	return error;
}
