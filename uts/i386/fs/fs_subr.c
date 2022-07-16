/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:fs_subr.c	1.3.2.2"

/*
 * Generic vnode operations.
 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/errno.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "sys/statvfs.h"
#include "sys/uio.h"
#include "sys/unistd.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/immu.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/cred.h"
#include "sys/poll.h"
#include "sys/cmn_err.h"
#include "sys/list.h"
#include "sys/stream.h"
#include "sys/rf_messg.h"
#include "sys/rf_adv.h"
#include "sys/rf_comm.h"
#include "sys/rf_comm.h"
#include "fs/fs_subr.h"


/*
 * The associated operation is not supported by the file system.
 */
int
fs_nosys()
{
	return ENOSYS;
}

/*
 * The file system has nothing to sync to disk.  However, the
 * VFS_SYNC operation must not fail.
 */
/* ARGSUSED */
int
fs_sync(vfsp, flag, cr)
	struct vfs *vfsp;
	short flag;
	cred_t *cr;
{
	return 0;
}

/*
 * Read/write lock/unlock.  Does nothing.
 */
/* ARGSUSED */
void
fs_rwlock(vp)
	vnode_t *vp;
{
}

/* ARGSUSED */
void
fs_rwunlock(vp)
	vnode_t *vp;
{
}

/*
 * Compare two vnodes.
 */
int
fs_cmp(vp1, vp2)
	register vnode_t *vp1, *vp2;
{
	return vp1 == vp2;
}

/*
 * File and record locking.
 */
/* ARGSUSED */
int
fs_frlock(vp, cmd, bfp, flag, offset, cr)
	register vnode_t *vp;
	int cmd;
	struct flock *bfp;
	int flag;
	off_t offset;
	cred_t *cr;
{
	int frcmd;

	switch (cmd) {

	case F_GETLK: 
	case F_O_GETLK: 
		bfp->l_pid = u.u_procp->p_epid;
		bfp->l_sysid = u.u_procp->p_sysid;
		frcmd = 0;
		break;

	case F_RGETLK: 
		frcmd = RCMDLCK;
		break;

	case F_SETLK: 
		bfp->l_pid = u.u_procp->p_epid;
		bfp->l_sysid = u.u_procp->p_sysid;
		frcmd = SETFLCK;
		break;

	case F_RSETLK: 
		frcmd = SETFLCK|RCMDLCK;
		break;

	case F_SETLKW: 
		bfp->l_pid = u.u_procp->p_epid;
		bfp->l_sysid = u.u_procp->p_sysid;
		frcmd = SETFLCK|SLPFLCK;
		break;

	case F_RSETLKW: 
		frcmd = SETFLCK|SLPFLCK|RCMDLCK;
		break;
		
	default:
		return EINVAL;
	}

	return reclock(vp, bfp, frcmd, flag, offset);
}

/*
 * Allow any flags.
 */
/* ARGSUSED */
int
fs_setfl(vp, oflags, nflags, cr)
	vnode_t *vp;
	int oflags;
	int nflags;
	cred_t *cr;
{
	return 0;
}

/*
 * Return the answer requested to poll() for non-device files.
 * Only POLLIN, POLLRDNORM, and POLLOUT are recognized.
 */
/* ARGSUSED */
int
fs_poll(vp, events, anyyet, reventsp, phpp)
	vnode_t *vp;
	register short events;
	int anyyet;
	register short *reventsp;
	struct pollhead **phpp;
{
	*reventsp = 0;
	if (events & POLLIN)
		*reventsp |= POLLIN;
	if (events & POLLRDNORM)
		*reventsp |= POLLRDNORM;
	if (events & POLLOUT)
		*reventsp |= POLLOUT;
	*phpp = (struct pollhead *)NULL;
	return 0;
}

/*
 * vcp is an in/out parameter.  Updates *vcp with a version code suitable
 * for the va_vcode attribute, possibly the value passed in.
 *
 * The va_vcode attribute is intended to support cache coherency
 * and IO atomicity for file servers that provide traditional
 * UNIX file system semantics.  The vnode of the file object
 * whose va_vcode is being updated must be held locked when
 * this function is evaluated.
 *
 * Returns 0 for success, a nonzero errno for failure.
 */
int
fs_vcode(vp, vcp)
	register vnode_t	*vp;
	u_long			*vcp;
{
	static u_long		vcode;
	register u_long		error = 0;

	/*
	 * RFS hooks here.
	 */
	extern int		rf_state;

	if (vp->v_type == VREG && (*vcp == 0 || rf_state == RF_UP)) {
		if (vcode == (u_long)~0) {
			cmn_err(CE_WARN, "fs_vcode: vcode overflow\n");
			error = ENOMEM;
		} else {
			register u_long	tvcode;
			rcvd_t		*rdp;

			*vcp = tvcode = ++vcode;
			if (rf_state == RF_UP && (rdp = VTORD(vp)) != NULL) {
				extern void	rfc_disable_msg();
	
				rfc_disable_msg(rdp, tvcode);
			}
		}
	}
	return error;
}

/*
 * POSIX pathconf() support.
 */
/* ARGSUSED */
int
fs_pathconf(vp, cmd, valp, cr)
	struct vnode *vp;
	int cmd;
	u_long *valp;
	struct cred *cr;
{
	register u_long val;
	register int error = 0;
	struct statvfs vfsbuf;

	switch (cmd) {

	case _PC_LINK_MAX:
		val = MAXLINK;
		break;

	case _PC_MAX_CANON:
		val = MAX_CANON;
		break;

	case _PC_MAX_INPUT:
		val = MAX_INPUT;
		break;

	case _PC_NAME_MAX:
		struct_zero((caddr_t)&vfsbuf, sizeof(vfsbuf));
		if (error = VFS_STATVFS(vp->v_vfsp, &vfsbuf))
			break;
		val = vfsbuf.f_namemax;
		break;

	case _PC_PATH_MAX:
		val = MAXPATHLEN;
		break;

	case _PC_PIPE_BUF:
		val = PIPE_BUF;
		break;

	case _PC_NO_TRUNC:
		if (vp->v_vfsp->vfs_flag & VFS_NOTRUNC)
			val = 1;	/* NOTRUNC is enabled for vp */
		else
			val = (u_long)-1;
		break;

	case _PC_VDISABLE:
		val = _POSIX_VDISABLE;
		break;

	case _PC_CHOWN_RESTRICTED:
		if (rstchown)
			val = rstchown;		/* chown restricted enabled */
		else
			val = (u_long)-1;
		break;

	default:
		error = EINVAL;
		break;
	}

	if (error == 0)
		*valp = val;
	return error;
}
