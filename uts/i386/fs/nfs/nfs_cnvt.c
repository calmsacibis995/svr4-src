/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:nfs/nfs_cnvt.c	1.3"
/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 *	nfs_cnvt.c:	given a file handle and a mode, return an open file
 *			descriptor of that corresponding file.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/proc.h"
#include "sys/debug.h"

#include "rpc/types.h"
#define NFSSERVER
#include "nfs/nfs.h"
#include "nfs/nfssys.h"

int
nfs_cnvt(arg, rvp)
	register struct nfs_cnvt_args *arg;
	rval_t *rvp;
{
	int			fd;
	struct file		*fp;
	struct vnode		*vp;
	int			error;
	struct nfs_cnvt_args	a;
	register struct nfs_cnvt_args *ap;
	fhandle_t		tfh;
	register int		filemode;
	int			mode;
	char			flags;
	struct vnode		*myfhtovp();
	extern struct fileops	vnodefops;
	
	/*
	 *      NFS_CNVT:  given a pointer to an fhandle_t and a mode, open
	 *      the file corresponding to the fhandle_t with the given mode and
	 *      return a file descriptor.
	 */
	if (!suser(u.u_cred))
		return (EPERM);

	if (copyin((caddr_t) arg, (caddr_t) &a, sizeof(a)))
		return (EFAULT);
	else
		ap = &a;
	if (copyin((caddr_t) ap->fh, (caddr_t) &tfh, sizeof(tfh)))
		return (EFAULT);

	filemode = ap->filemode - FOPEN;
	if (filemode & FCREAT)
		return (EINVAL);

	mode = 0;
	if (filemode & FREAD)
		mode |= VREAD;
	if (filemode & (FWRITE | FTRUNC))
		mode |= VWRITE;

	/*
	 *      Adapted from copen:
	 */
	error = falloc((struct vnode *)NULL, filemode & FMASK,
		&fp, &fd);
	if (error)
		return error;

	/*
	 *      This takes the place of lookupname in copen.  Note that
	 *      we can't use the normal fhtovp function because we want
	 *      this to work on files that may not have been exported.
	 */
	if ((vp = myfhtovp(&tfh)) == (struct vnode *) NULL) {
		error = ESTALE;
		goto out;
	}

	/*
	 *      Adapted from vn_open:
	 */
	if (filemode & (FWRITE | FTRUNC)) {
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
			if ((error = VOP_GETATTR(vp, &vattr, 0,
			  u.u_cred)) == 0
			  && MANDLOCK(vp, vattr.va_mode))
				error = EAGAIN;
		}
		if (error)
			goto out;
	}
	/*
	 * Check permissions.
	 * Must have read and write permission to open a file for
	 * private access.
	 */
	if (error = VOP_ACCESS(vp, mode, 0, u.u_cred)) {
		goto out;
	}
	error = VOP_OPEN(&vp, filemode, u.u_cred);
	if ((error == 0) && (filemode & FTRUNC)) {
		struct vattr vattr;

		vattr.va_size = 0;
		vattr.va_mask = AT_SIZE;
		if ((error = VOP_SETATTR(vp, &vattr, 0, 
		  u.u_cred)) != 0) {
			(void)VOP_CLOSE(vp, filemode, 1, 0, u.u_cred);
		}
	}
	if (error)
		goto out;

	/*
	 *      Adapted from copen:
	 */
	fp->f_vnode = vp;
	if (copyout((caddr_t) &fd, (caddr_t) ap->fd, sizeof(fd)))
		error = EFAULT;
	else
		return (0);
out:
	crfree(fp->f_cred);
	fp->f_count = 0;
	if (vp)
		VN_RELE(vp);
	return (error);
}

/*
 * We require a version of fhtovp that simply converts an fhandle_t to
 * a vnode without any ancillary checking (e.g., whether it's exported).
 */
STATIC struct vnode *
myfhtovp(fh)
	fhandle_t *fh;
{
	int error;
	struct vnode *vp;
	register struct vfs *vfsp;

	vfsp = getvfs(&fh->fh_fsid);
	if (vfsp == (struct vfs *) NULL) {
		return ((struct vnode *) NULL);
	}
	error = VFS_VGET(vfsp, &vp, (struct fid *)&(fh->fh_len));
	if (error || vp == (struct vnode *) NULL) {
		return ((struct vnode *) NULL);
	}
	return (vp);
}
