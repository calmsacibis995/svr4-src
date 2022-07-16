/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:xnamfs/xnamvnops.c	1.3"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/file.h"
#include "sys/immu.h"
#include "sys/kmem.h"
#include "sys/mman.h"
#include "sys/open.h"
#include "sys/swap.h"
#include "sys/sysmacros.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/poll.h"
#include "sys/stream.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/session.h"

#include "sys/fs/xnamnode.h"

#include "vm/seg.h"
#include "vm/seg_map.h"
#include "vm/page.h"
#include "vm/pvn.h"
#include "vm/seg_dev.h"
#include "vm/seg_vn.h"

#include "fs/fs_subr.h"

extern int setjmp();
extern int nodev();
extern void unxsd_alloc(); 
extern void unxsem_alloc(); 
extern void xnamdelete();

STATIC int xnam_open();
STATIC int xnam_close();
STATIC int xnam_getattr();
STATIC int xnam_setattr();
STATIC int xnam_access();
STATIC int xnam_fsync();
STATIC void xnam_inactive();
STATIC int xnam_fid();
STATIC int xnam_cmp();
STATIC int xnam_realvp();

struct vnodeops xnam_vnodeops = {
	xnam_open,
	xnam_close,
	fs_nosys,	/* read */
	fs_nosys,	/* write */
	fs_nosys,	/* ioctl */
	fs_setfl,
	xnam_getattr,
	xnam_setattr,
	xnam_access,
	fs_nosys,	/* lookup */
	fs_nosys,	/* create */
	fs_nosys,	/* remove */
	fs_nosys,	/* link */
	fs_nosys,	/* rename */
	fs_nosys,	/* mkdir */
	fs_nosys,	/* rmdir */
	fs_nosys,	/* readdir */
	fs_nosys,	/* symlink */
	fs_nosys,	/* readlink */
	xnam_fsync,
	xnam_inactive,
	xnam_fid,
	fs_rwlock,
	fs_rwunlock,
	fs_nosys,	/* seek */
	xnam_cmp,
	fs_nosys,
	fs_nosys,	/* space */
	xnam_realvp,
	fs_nosys,	/* getpage */
	fs_nosys,	/* putpage */
	fs_nosys,	/* map */
	fs_nosys,	/* addmap */
	fs_nosys,	/* delmap */
	fs_nosys,	/* poll */
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
 * Semaphore and shared data files should not be opened.
 */
/*ARGSUSED*/
STATIC int
xnam_open(vpp, flag, cr)
	struct vnode **vpp;
	int flag;
	struct cred *cr;
{
	return EISNAM;
}

/* ARGSUSED */
STATIC int
xnam_close(vp, flag, count, offset, cr)
	struct vnode *vp;
	int flag;
	int count;
	off_t offset;
	struct cred *cr;
{
	register struct xnamnode *xp;

	ASSERT(count >= 1);
	if (count > 1)
		return 0;
	if (setjmp(&u.u_qsav))	/* catch half-closes */
		return EINTR;

	xp = VTOXNAM(vp);
	xp->x_count--;		/* one fewer open reference */
	return 0;
}


STATIC int
xnam_getattr(vp, vap, flags, cr)
	struct vnode *vp;
	register struct vattr *vap;
	int flags;
	struct cred *cr;
{
	int error;
	register struct xnamnode *xp = VTOXNAM(vp);

	if (error = VOP_GETATTR(xp->x_realvp, vap, flags, cr))
		return error;

	vap->va_atime.tv_sec = xp->x_atime;
	vap->va_atime.tv_nsec = 0;
	vap->va_mtime.tv_sec = xp->x_mtime;
	vap->va_mtime.tv_nsec = 0;
	vap->va_ctime.tv_sec = xp->x_ctime;
	vap->va_ctime.tv_nsec = 0;
	return 0;
}

STATIC int
xnam_setattr(vp, vap, flags, cr)
	struct vnode *vp;
	register struct vattr *vap;
	int flags;
	struct cred *cr;
{
	register struct xnamnode *xp = VTOXNAM(vp);
	int error;
	register int chtime = 0;

	error = VOP_SETATTR(xp->x_realvp, vap, flags, cr);
	if (error == 0) {
		/*
		 * If times were changed, update xnamnode.
		 */
		if (vap->va_mask & AT_ATIME) {
			xp->x_atime = vap->va_atime.tv_sec;
			chtime++;
		}
		if (vap->va_mask & AT_MTIME) {
			xp->x_mtime = vap->va_mtime.tv_sec;
			chtime++;
		}
		if (chtime) {
			xp->x_ctime = hrestime.tv_sec;
			xp->x_mode = vap->va_mode;
			xp->x_uid = vap->va_uid;
			xp->x_gid = vap->va_gid;
		}
	}
	return error;
}

STATIC int
xnam_access(vp, mode, flags, cr)
	struct vnode *vp;
	int mode;
	int flags;
	struct cred *cr;
{
	return VOP_ACCESS(VTOXNAM(vp)->x_realvp, mode, flags, cr);
}

STATIC int
xnam_fsync(vp, cr)
	struct vnode *vp;
	struct cred *cr;
{
	register struct xnamnode *xp = VTOXNAM(vp);
	struct vattr va, vatmp;

	/* If times didn't change, don't flush anything. */
	if ((xp->x_flag & (XNAMACC|XNAMUPD|XNAMCHG)) == 0)
		return 0;
	vatmp.va_mask = AT_ATIME|AT_MTIME;
	if (VOP_GETATTR(xp->x_realvp, &vatmp, 0, cr) == 0) {
		if (vatmp.va_atime.tv_sec > xp->x_atime)
			va.va_atime = vatmp.va_atime;
		else {
			va.va_atime.tv_sec = xp->x_atime;
			va.va_atime.tv_nsec = 0;
		}
		if (vatmp.va_mtime.tv_sec > xp->x_mtime)
			va.va_mtime = vatmp.va_mtime;
		else {
			va.va_mtime.tv_sec = xp->x_mtime;
			va.va_mtime.tv_nsec = 0;
		}
		va.va_mask = AT_ATIME|AT_MTIME;
		(void) VOP_SETATTR(xp->x_realvp, &va, 0, cr);
	}
	(void) VOP_FSYNC(xp->x_realvp, cr);
	return 0;
}

STATIC void
xnam_inactive(vp, cr)
	struct vnode *vp;
	struct cred *cr;
{
	register struct xnamnode *xp = VTOXNAM(vp);

	xnamdelete(xp);

	if(vp->v_type == VXNAM) {
		if(vp->v_rdev == XNAM_SEM && xp->x_sem)
			unxsem_alloc(xp);
		else if(vp->v_rdev == XNAM_SD && xp->x_sd)
			unxsd_alloc(xp);
	}
	(void) xnam_fsync(vp, cr);
	VN_RELE(xp->x_realvp);
	xp->x_realvp = NULL;

	kmem_free((caddr_t)xp, sizeof (*xp));
}

STATIC int
xnam_fid(vp, fidpp)
	struct vnode *vp;
	struct fid **fidpp;
{
	return VOP_FID(VTOXNAM(vp)->x_realvp, fidpp);
}

STATIC int
xnam_cmp(vp1, vp2)
	struct vnode *vp1, *vp2;
{
	return vp1 == vp2;
}

STATIC int
xnam_realvp(vp, vpp)
	register struct vnode *vp;
	register struct vnode **vpp;
{
	register struct xnamnode *xp = VTOXNAM(vp);
	struct vnode *rvp;

	if ((vp = xp->x_realvp) != (struct vnode *)0 && VOP_REALVP(vp, &rvp) == 0)
		vp = rvp;
	*vpp = vp;
	return 0;
}
