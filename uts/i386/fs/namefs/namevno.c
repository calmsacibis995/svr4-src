/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:namefs/namevno.c	1.3.1.1"
/*
 * This file defines the vnode operations for mounted file descriptors.
 * The routines in this file act as a layer between the NAMEFS file 
 * system and SPECFS/FIFOFS.  With the exception of nm_open(), nm_setattr(),
 * nm_getattr() and nm_access(), the routines simply apply the VOP 
 * operation to the vnode representing the file descriptor.  This switches 
 * control to the underlying file system to which the file descriptor 
 * belongs.
 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/cred.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/file.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/kmem.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "vm/seg.h"
#include "sys/fs/namenode.h"
#include "sys/stream.h"
#include "fs/fs_subr.h"
/*
 * Define the routines within this file.
 */
int	nm_open(),     nm_close(),   nm_read(),    nm_write();
int	nm_ioctl(),    nm_getattr(), nm_setattr(), nm_access();
int	nm_link(),     nm_fsync(),   nm_fid(),     nm_seek();
int	nm_realvp(),   nm_poll(),    nm_create();
void	nm_inactive(), nm_rwlock(),  nm_rwunlock();

/*
 * Define external routines.
 */
extern	u_short	nmgetid();
extern	int	cleanlocks();
extern	void	nmclearid(),   nameinsert(), nameremove();
extern	struct	namenode *namefind();

struct vnodeops nm_vnodeops = {
	nm_open,
	nm_close,
	nm_read,
	nm_write,
	nm_ioctl,
	fs_setfl,
	nm_getattr,
	nm_setattr,
	nm_access,
	fs_nosys,	/* lookup */
	nm_create,
	fs_nosys,	/* remove */
	nm_link,
	fs_nosys,	/* rename */
	fs_nosys,	/* mkdir */
	fs_nosys,	/* rmdir */
	fs_nosys,	/* readdir */
	fs_nosys,	/* symlink */
	fs_nosys,	/* readlink */
	nm_fsync,
	nm_inactive,
	nm_fid,
	nm_rwlock,
	nm_rwunlock,
	nm_seek,
	fs_cmp,
	fs_frlock,
	fs_nosys,	/* space */
	nm_realvp,
	fs_nosys,	/* getpages */
	fs_nosys,	/* putpages */
	fs_nosys,	/* map */
	fs_nosys,	/* addmap */
	fs_nosys,	/* delmap */
	nm_poll,
	fs_nosys,	/* dump */
	fs_pathconf,
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
	fs_nosys,
};
 
/*
 * Create a reference to the vnode representing the file descriptor.
 * Then, apply the VOP_OPEN operation to that vnode.
 *
 * The vnode for the file descriptor may be switched under you.
 * If it is, search the hash list for an nodep - nodep->nm_filevp
 * pair. If it exists, return that nodep to the user.
 * If it does not exist, create a new namenode to attach
 * to the nodep->nm_filevp then place the pair on the hash list.
 *
 * Newly created objects are like children/nodes in the mounted
 * file system, with the parent being the initial mount.
 */
int
nm_open(vpp, flag, crp)
	struct vnode **vpp;
	int flag;
	struct cred *crp;
{
	register struct namenode *nodep = VTONM(*vpp);
	register int error = 0;
	register struct namenode *newnamep;
	struct vnode *newvp;
	struct vnode *infilevp;
	struct vnode *outfilevp;

	ASSERT(nodep->nm_filevp);
	/*
	 * If the vnode is switched under us, the corresponding
	 * VN_RELE for this VN_HOLD will be done by the file system
	 * performing the switch. Otherwise, the corresponding
	 * VN_RELE will be done by nm_close().
	 */
	VN_HOLD(nodep->nm_filevp);
	infilevp = outfilevp = nodep->nm_filevp;

	if ((error = VOP_OPEN(&outfilevp, flag, crp)) != 0) {
		VN_RELE(nodep->nm_filevp);
		return (error);
	}
	if (infilevp != outfilevp) {
		/*
		 * See if the new filevp (outfilevp) is already associated
		 * with the mount point. If it is, then it already has a
		 * namenode associated with it.
		 */
		if ((newnamep = namefind(outfilevp, nodep->nm_mountpt)) != NULL)
			goto gotit;

		/*
		 * Create a new namenode. 
		 */
		newnamep =
			(struct namenode *)kmem_zalloc (sizeof(struct namenode),
				KM_SLEEP);

		/*
		 * Initialize the fields of the new vnode/namenode
		 * then overwrite the fields that should not be copied.
		 */
		*newnamep = *nodep;

		newvp = NMTOV(newnamep);
		newvp->v_flag &= ~VROOT;  	/* new objects are not roots */
		newvp->v_flag |= VNOMAP|VNOSWAP;
		newvp->v_count = 0;        	/* bumped down below */
		newvp->v_vfsmountedhere = NULL;
		newvp->v_vfsp = (*vpp)->v_vfsp;
		newvp->v_stream = outfilevp->v_stream;
		newvp->v_pages = NULL;
		newvp->v_type = outfilevp->v_type;
		newvp->v_rdev = outfilevp->v_rdev;
		newvp->v_data = (caddr_t) newnamep;
		newvp->v_filocks = NULL;
		newnamep->nm_vattr.va_type = outfilevp->v_type;
		newnamep->nm_vattr.va_nodeid = nmgetid(1);
		newnamep->nm_vattr.va_size = 0;
		newnamep->nm_vattr.va_rdev = outfilevp->v_rdev;
		newnamep->nm_flag = 0;
		newnamep->nm_filevp = outfilevp;
		/*
		 * VN_HOLD to match VN_RELE in nm_close.
		 */
		VN_HOLD(outfilevp);
		newnamep->nm_mountpt = nodep->nm_mountpt;
		newnamep->nm_backp = newnamep->nm_nextp = NULL;

		/*
		 * Insert the new namenode into the hash list.
		 */
		nameinsert(newnamep);
gotit:		
		/*
		 * Release the above reference to the infilevp, the reference 
		 * to the NAMEFS vnode, create a reference to the new vnode
		 * and return the new vnode to the user.
		 */
		VN_HOLD(NMTOV(newnamep));
		VN_RELE(*vpp);
		*vpp = NMTOV(newnamep);
	}
	return (0);
}

/*
 * Close a mounted file descriptor.
 * Remove any locks and apply the VOP_CLOSE operation to the vnode for
 * the file descriptor.
 */
int
nm_close(vp, flag, count, offset, crp)
	struct vnode *vp;
	int flag;
	int count;
	off_t offset;
	struct cred *crp;
{
	register struct namenode *nodep = VTONM(vp);
	register int error = 0;

	ASSERT (nodep->nm_filevp);
	cleanlocks(vp, u.u_procp->p_epid, u.u_procp->p_sysid);
	error = VOP_CLOSE(nodep->nm_filevp, flag, count, offset, crp);
	if ((unsigned) count == 1) {
		(void) nm_fsync(vp, crp);
		VN_RELE(nodep->nm_filevp);
	}
	return (error);
}

/*
 * Read from a mounted file descriptor.
 */
int
nm_read(vp, uiop, ioflag, crp)
	struct vnode *vp;
	struct uio *uiop;
	int ioflag;
	struct cred *crp;
{
	ASSERT (VTONM(vp)->nm_filevp);
	return (VOP_READ (VTONM(vp)->nm_filevp, uiop, ioflag, crp));
}

/*
 * Apply the VOP_WRITE operation on the file descriptor's vnode.
 */
int
nm_write(vp, uiop, ioflag, crp)
	struct vnode *vp;
	struct uio *uiop;
	int ioflag;
	struct cred *crp;
{
	ASSERT (VTONM(vp)->nm_filevp);
	return (VOP_WRITE(VTONM(vp)->nm_filevp, uiop, ioflag, crp));
}

/*
 * Apply the VOP_IOCTL operation on the file descriptor's vnode.
 */
int
nm_ioctl(vp, cmd, arg, mode, cr, rvalp)
	register struct vnode *vp;
	int cmd;
	int arg;
	int mode;
	struct cred *cr;
	int *rvalp;
{
	ASSERT (VTONM(vp)->nm_filevp);
	return (VOP_IOCTL(VTONM(vp)->nm_filevp, cmd, arg, mode, cr, rvalp));
}

/*
 * Return in vap the attributes that are stored in the namenode
 * structure. In addition, overwrite the va_mask field with 0;
 */
/* ARGSUSED */
int
nm_getattr(vp, vap, flags, crp)
	struct vnode *vp;
	struct vattr *vap;
	int flags;
	struct cred *crp;
{
	register struct namenode *nodep = VTONM(vp);
	struct vattr va;
	register int error;

	if (error = VOP_GETATTR(nodep->nm_filevp, &va, flags, crp))
		return (error);

	*vap = nodep->nm_vattr;
	vap->va_mask = 0;
	vap->va_size = va.va_size;
	return (0);
}

/*
 * Set the attributes of the namenode from the attributes in vap.
 */
/* ARGSUSED */
int
nm_setattr(vp, vap, flags, crp)
	struct vnode *vp;
	register struct vattr *vap;
	int flags;
	struct cred *crp;
{
	register struct namenode *nodep = VTONM(vp);
	register struct vattr *nmvap = &nodep->nm_vattr;
	register long mask = vap->va_mask;
	int error = 0;

	/*
	 * Cannot set these attributes.
	 */
	if (mask & (AT_NOSET|AT_SIZE))
		return (EINVAL);

	nm_rwlock(vp);

	/*
	 * Change ownership/group/time/access mode of mounted file
	 * descriptor.  Must be owner or super user.
	 */
	if (crp->cr_uid != nmvap->va_uid && !suser(crp)) {
		error = EPERM;
		goto out;
	}
	/*
	 * If request to change mode, copy new
	 * mode into existing attribute structure.
	 */
	if (mask & AT_MODE) {
		nmvap->va_mode = vap->va_mode & ~VSVTX;
		if (crp->cr_uid != 0 && !groupmember(nmvap->va_gid, crp))
			nmvap->va_mode &= ~VSGID;
	}
	/*
	 * If request was to change user or group, turn off suid and sgid
	 * bits.
	 * If the system was configured with the "rstchown" option, the 
	 * owner is not permitted to give away the file, and can change 
	 * the group id only to a group of which he or she is a member.
	 */
	if (mask & (AT_UID|AT_GID)) {
		int checksu = 0;

		if (rstchown) {
			if (((mask & AT_UID) && vap->va_uid != nmvap->va_uid)
			  || ((mask & AT_GID)
			    && !groupmember(vap->va_gid, crp)))
				checksu = 1;
		} else if (crp->cr_uid != nmvap->va_uid)
			checksu = 1;

		if (checksu && !suser(crp)) {
			error = EPERM;
			goto out;
		}
		if (crp->cr_uid != 0)
			nmvap->va_mode &= ~(VSUID|VSGID);
		if (mask & AT_UID)
			nmvap->va_uid = vap->va_uid;
		if (mask & AT_GID)
			nmvap->va_gid = vap->va_gid;
	}
	/*
	 * If request is to modify times, make sure user has write 
	 * permissions on the file.
	 */
	if (mask & (AT_ATIME|AT_MTIME)) {
		if (crp->cr_uid != 0 && !(nmvap->va_mode & VWRITE)) {
			error = EACCES;
			goto out;
		}
		if (mask & AT_ATIME)
			nmvap->va_atime = vap->va_atime;
		if (mask & AT_MTIME) {
			nmvap->va_mtime = vap->va_mtime;
			nmvap->va_ctime = hrestime;
		}
	}
out:
	nm_rwunlock(vp);
	return (error);
}

/*
 * Check mode permission on the namenode.  The mode is shifted to select 
 * the owner/group/other fields.  The super user is granted all permissions
 * on the namenode.  In addition an access check is performed on the
 * mounted file.
 */
/* ARGSUSED */
int
nm_access(vp, mode, flags, crp)
	struct vnode *vp;
	int mode;
	int flags;
	struct cred *crp;
{
	register struct namenode *nodep = VTONM(vp);
	int error, fmode, omode = mode;

	if (crp->cr_uid != 0) {
		if (crp->cr_uid != nodep->nm_vattr.va_uid) {
			mode >>= 3;
			if (!groupmember(nodep->nm_vattr.va_gid, crp))
				mode >>= 3;
		}
		if ((nodep->nm_vattr.va_mode & mode) != mode)
			return (EACCES);
	}
	if (error = VOP_ACCESS(nodep->nm_filevp, omode, flags, crp))
		return (error);
	/*
	 * Last stand.  Regardless of the requestor's credentials, don't
	 * grant a permission that wasn't granted at the time the mounted
	 * file was originally opened.
	 */
	fmode = nodep->nm_filep->f_flag;
	if (((omode & VWRITE) && (fmode & FWRITE) == 0)
	  || ((omode & VREAD) && (fmode & FREAD) == 0))
		return (EACCES);
	return (0);
}

/*
 * Dummy op so that creats and opens with O_CREAT
 * of mounted streams will work.
 */
/*ARGSUSED*/
int
nm_create(dvp, name, vap, excl, mode, vpp, cr)
	struct vnode *dvp;
	char *name;
	struct vattr *vap;
	enum vcexcl excl;
	int mode;
	struct vnode **vpp;
	struct cred *cr;
{
	register int error = 0;

	if (*name == '\0') {
		/*
		 * Null component name refers to the root.
		 */
		if ((error = nm_access(dvp, mode, 0, cr)) == 0) {
			VN_HOLD(dvp);
			*vpp = dvp;
		}
	} else {
		error = ENOSYS;
	}
	return (error);
}

/*
 * Links are not allowed on mounted file descriptors.
 */
/*ARGSUSED*/
int
nm_link(tdvp, vp, tnm, crp)
	register struct vnode *tdvp;
	struct vnode *vp;
	char *tnm;
	struct cred *crp;

{
	return (EXDEV);
}

/*
 * Apply the VOP_FSYNC operation on the file descriptor's vnode.
 */
int
nm_fsync(vp, crp)
	struct vnode *vp;
	struct cred *crp;
{
	ASSERT (VTONM(vp)->nm_filevp);
	return (VOP_FSYNC (VTONM(vp)->nm_filevp, crp));
}

/*
 * Inactivate a vnode/namenode by...
 * clearing its unique node id, removing it from the hash list
 * and freeing the memory allocated for it.
 */
void
nm_inactive(vp, crp)
	struct vnode *vp;
	struct cred *crp;
{
	register struct namenode *nodep = VTONM(vp);

	nmclearid(nodep);
	nameremove(nodep);
	kmem_free((caddr_t) nodep, sizeof(struct namenode));
}

/*
 * Apply the VOP_FID operation on the file descriptor's vnode.
 */
int
nm_fid(vp, fidnodep)
	struct vnode *vp;
	struct fid **fidnodep;
{
	ASSERT (VTONM(vp)->nm_filevp);
	return (VOP_FID(VTONM(vp)->nm_filevp, fidnodep));
}

/*
 * Lock the namenode associated with vp.
 */
void
nm_rwlock(vp)
	struct vnode *vp;
{
	register struct namenode *nodep = VTONM(vp);

	VOP_RWLOCK(nodep->nm_filevp);
}

/*
 * Unlock the namenode associated with vp.
 */
void
nm_rwunlock(vp)
	struct vnode *vp;
{
	register struct namenode *nodep = VTONM(vp);

	VOP_RWUNLOCK(nodep->nm_filevp);
}

/*
 * Apply the VOP_SEEK operation on the file descriptor's vnode.
 */
int
nm_seek(vp, ooff, noffp)
	struct vnode *vp;
	off_t ooff;
	off_t *noffp;
{
	ASSERT (VTONM(vp)->nm_filevp);
	return (VOP_SEEK(VTONM(vp)->nm_filevp, ooff, noffp));
}

/*
 * Return the vnode representing the file descriptor in vpp.
 */
int
nm_realvp(vp, vpp)
	register struct vnode *vp;
	register struct vnode **vpp;
{
	register struct vnode *fvp = VTONM(vp)->nm_filevp;
	struct vnode *rvp;

	ASSERT(fvp);
	vp = fvp;
	if (VOP_REALVP(vp, &rvp) == 0)
		vp = rvp;
	*vpp = vp;
	return (0);
}

/*
 * Apply VOP_POLL to the vnode representing the mounted file descriptor.
 */
int
nm_poll(vp, events, anyyet, reventsp, phpp)
	vnode_t *vp;
	register short events;
	int anyyet;
	register short *reventsp;
	struct pollhead **phpp;
{
	ASSERT (VTONM(vp)->nm_filevp);
	return (VOP_POLL(VTONM(vp)->nm_filevp, events, anyyet, reventsp, phpp));
}
