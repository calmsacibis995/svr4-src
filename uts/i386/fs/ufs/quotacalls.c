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

#ident	"@(#)kern-fs:ufs/quotacalls.c	1.3"

/*
 * Quota system calls.
 */
#ifdef QUOTA
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/cred.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/uio.h>
#include <sys/fs/ufs_quota.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>
#include <sys/errno.h>
#include <sys/debug.h>
#include <sys/cmn_err.h>

/*
 * Sys call to allow users to find out
 * their current position wrt quota's
 * and to allow super users to alter it.
 */


/*ARGSUSED*/
int
quotactl(vp, arg, cr)
	struct vnode *vp;
	int arg;
	struct cred *cr;
{
	struct quotctl quot;
	register struct vfs *vfsp;
	extern struct vfsops ufs_vfsops;
	struct ufsvfs *ufsvfsp;
	register int error = 0;

	if (copyin((caddr_t)arg, (caddr_t)&quot, sizeof(struct quotctl)))
		return EFAULT;
	if (quot.uid < 0)
		quot.uid = cr->cr_ruid;
	if (quot.op == Q_SYNC && vp == NULL) {
		ufsvfsp = NULL;
	} else if (quot.op != Q_ALLSYNC) {
		ufsvfsp = (struct ufsvfs *)(vp->v_vfsp->vfs_data);
	}
	switch (quot.op) {

	case Q_QUOTAON:
		error = opendq(ufsvfsp, quot.addr, cr);
		break;

	case Q_QUOTAOFF:
		error = closedq(ufsvfsp, cr);
		break;

	case Q_SETQUOTA:
	case Q_SETQLIM:
		error = setquota(quot.op, (uid_t)quot.uid, ufsvfsp, quot.addr, cr);
		break;

	case Q_GETQUOTA:
		error = getquota((uid_t)quot.uid, ufsvfsp, quot.addr, cr);
		break;

	case Q_SYNC:
		error = qsync(ufsvfsp);
		break;

	case Q_ALLSYNC:
		(void)qsync(NULL);
		break;

	default:
		error = EINVAL;
		break;
	}
	return error;
}

/*
 * Set the quota file up for a particular file system.
 * Called as the result of a setquota system call.
 */
STATIC int
opendq(ufsvfsp, addr, cr)
	register struct ufsvfs *ufsvfsp;
	caddr_t addr;			/* quota file */
	struct cred *cr;
{
	struct vnode *vp;
	struct dquot *dqp;
	int error;

	if (!suser(cr))
		return (EPERM);
	error =
	    lookupname(addr, UIO_USERSPACE, FOLLOW, (struct vnode **)NULL,
	    &vp);
	if (error)
		return (error);
	if ((struct ufsvfs *)(vp->v_vfsp->vfs_data) != ufsvfsp || vp->v_type != VREG) {
		VN_RELE(vp);
		return (EACCES);
	}
	if (ufsvfsp->vfs_qflags & MQ_ENABLED)
		(void) closedq(ufsvfsp, cr);
	if (ufsvfsp->vfs_qinod != NULL) {	/* open/close in progress */
		VN_RELE(vp);
		return (EBUSY);
	}
	ufsvfsp->vfs_qinod = VTOI(vp);
	/*
	 * The file system time limits are in the super user dquot.
	 * The time limits set the relative time the other users
	 * can be over quota for this file system.
	 * If it is zero a default is used (see quota.h).
	 */
	error = getdiskquota((uid_t)0, ufsvfsp, 1, &dqp);
	if (error == 0) {
		ufsvfsp->vfs_btimelimit =
		    (dqp->dq_btimelimit? dqp->dq_btimelimit: DQ_BTIMELIMIT);
		ufsvfsp->vfs_ftimelimit =
		    (dqp->dq_ftimelimit? dqp->dq_ftimelimit: DQ_FTIMELIMIT);
		dqput(dqp);
		ufsvfsp->vfs_qflags = MQ_ENABLED;	/* enable quotas */
	} else {
		/*
		 * Some sort of I/O error on the quota file.
		 */
		irele(ufsvfsp->vfs_qinod);
		ufsvfsp->vfs_qinod = NULL;
	}
	return (error);
}

/*
 * Close off disk quotas for a file system.
 */
int
closedq(ufsvfsp, cr)
	register struct ufsvfs *ufsvfsp;
	register struct cred *cr;
{
	register struct dquot *dqp;
	register struct inode *ip;
	register struct inode *qip;

	if (!suser(cr))
		return (EPERM);
	if ((ufsvfsp->vfs_qflags & MQ_ENABLED) == 0)
		return (0);
	qip = ufsvfsp->vfs_qinod;
	ASSERT(qip != NULL);
	ufsvfsp->vfs_qflags = 0;	/* disable quotas */
loop:
	/*
	 * Run down the inode table and release all dquots assciated with
	 * inodes on this filesystem.
	 */
	for (ip = ufs_inode; ip < inodeNINODE; ip++) {
		dqp = ip->i_dquot;
		if (dqp != NULL && dqp->dq_ufsvfsp == ufsvfsp) {
			if (dqp->dq_flags & DQ_LOCKED) {
				dqp->dq_flags |= DQ_WANT;
				(void) sleep((caddr_t)dqp, PINOD+2);
				goto loop;
			}
			dqp->dq_flags |= DQ_LOCKED;
			dqput(dqp);
			ip->i_dquot = NULL;
		}
	}
	/*
	 * Run down the dquot table and clean and invalidate the
	 * dquots for this file system.
	 */
	for (dqp = dquot; dqp < dquotNDQUOT; dqp++) {
		if (dqp->dq_ufsvfsp == ufsvfsp) {
			if (dqp->dq_flags & DQ_LOCKED) {
				dqp->dq_flags |= DQ_WANT;
				(void) sleep((caddr_t)dqp, PINOD+2);
				goto loop;
			}
			dqp->dq_flags |= DQ_LOCKED;
			if (dqp->dq_flags & DQ_MOD)
				dqupdate(dqp);
			dqinval(dqp);
		}
	}
	/*
	 * Sync and release the quota file inode.
	 */
	ufs_ilock(qip);
	(void) ufs_syncip(qip, 0);
	ufs_iput(qip);
	ufsvfsp->vfs_qinod = NULL;
	return (0);
}

/*
 * Set various feilds of the dqblk according to the command.
 * Q_SETQUOTA - assign an entire dqblk structure.
 * Q_SETQLIM - assign a dqblk structure except for the usage.
 */
STATIC int
setquota(cmd, uid, ufsvfsp, addr, cr)
	int cmd;
	uid_t uid;
	struct ufsvfs *ufsvfsp;
	caddr_t addr;
	struct cred *cr;
{
	register struct dquot *dqp;
	struct dquot *xdqp;
	struct dqblk newlim;
	int error;

	if (!suser(cr))
		return (EPERM);
	if ((ufsvfsp->vfs_qflags & MQ_ENABLED) == 0)
		return (ESRCH);
	error = copyin(addr, (caddr_t)&newlim, sizeof (struct dqblk));
	if (error)
		return (error);
	error = getdiskquota(uid, ufsvfsp, 0, &xdqp);
	if (error)
		return (error);
	dqp = xdqp;
	/*
	 * Don't change disk usage on Q_SETQLIM
	 */
	if (cmd == Q_SETQLIM) {
		newlim.dqb_curblocks = dqp->dq_curblocks;
		newlim.dqb_curfiles = dqp->dq_curfiles;
	}
	dqp->dq_dqb = newlim;
	if (uid == 0) {
		/*
		 * Timelimits for the super user set the relative time
		 * the other users can be over quota for this file system.
		 * If it is zero a default is used (see quota.h).
		 */
		ufsvfsp->vfs_btimelimit =
		    newlim.dqb_btimelimit? newlim.dqb_btimelimit: DQ_BTIMELIMIT;
		ufsvfsp->vfs_ftimelimit =
		    newlim.dqb_ftimelimit? newlim.dqb_ftimelimit: DQ_FTIMELIMIT;
	} else {
		/*
		 * If the user is now over quota, start the timelimit.
		 * The user will not be warned.
		 */
		if (dqp->dq_curblocks >= dqp->dq_bsoftlimit &&
		    dqp->dq_bsoftlimit && dqp->dq_btimelimit == 0)
			dqp->dq_btimelimit = hrestime.tv_sec + ufsvfsp->vfs_btimelimit;
		else
			dqp->dq_btimelimit = 0;
		if (dqp->dq_curfiles >= dqp->dq_fsoftlimit &&
		    dqp->dq_fsoftlimit && dqp->dq_ftimelimit == 0)
			dqp->dq_ftimelimit = hrestime.tv_sec + ufsvfsp->vfs_ftimelimit;
		else
			dqp->dq_ftimelimit = 0;
		dqp->dq_flags &= ~(DQ_BLKS|DQ_FILES);
	}
	dqp->dq_flags |= DQ_MOD;
	dqupdate(dqp);
	dqput(dqp);
	return (0);
}

/*
 * Q_GETQUOTA - return current values in a dqblk structure.
 */
STATIC int
getquota(uid, ufsvfsp, addr,cr)
	uid_t uid;
	struct ufsvfs *ufsvfsp;
	caddr_t addr;
	struct cred *cr;
{
	register struct dquot *dqp;
	struct dquot *xdqp;
	int error;

	if (uid != cr->cr_ruid && !suser(cr))
		return (EPERM);
	if ((ufsvfsp->vfs_qflags & MQ_ENABLED) == 0)
		return (ESRCH);
	error = getdiskquota(uid, ufsvfsp, 0, &xdqp);
	if (error)
		return (error);
	dqp = xdqp;
	if (dqp->dq_fhardlimit == 0 && dqp->dq_fsoftlimit == 0 &&
	    dqp->dq_bhardlimit == 0 && dqp->dq_bsoftlimit == 0) {
		error = ESRCH;
	} else {
		error =
		    copyout((caddr_t)&dqp->dq_dqb, addr, sizeof (struct dqblk));
	}
	dqput(dqp);
	return (error);
}

/*
 * Q_SYNC - sync quota files to disk.
 */
STATIC int
qsync(ufsvfsp)
	register struct ufsvfs *ufsvfsp;
{
	register struct dquot *dqp;

	if (ufsvfsp != NULL && (ufsvfsp->vfs_qflags & MQ_ENABLED) == 0)
		return (ESRCH);
	for (dqp = dquot; dqp < dquotNDQUOT; dqp++) {
		if ((dqp->dq_flags & DQ_MOD) &&
		    (ufsvfsp == NULL || dqp->dq_ufsvfsp == ufsvfsp) &&
		    (dqp->dq_ufsvfsp->vfs_qflags & MQ_ENABLED) &&
		    (dqp->dq_flags & DQ_LOCKED) == 0) {
			dqp->dq_flags |= DQ_LOCKED;
			dqupdate(dqp);
			DQUNLOCK(dqp);
		}
	}
	return (0);
}

#ifdef notneeded
STATIC int
fdevtoufsvfsp(fdev, ufsvfspp)
	char *fdev;
	struct ufsvfs **ufsvfspp;
{
	struct vnode *vp;
	dev_t dev;
	int error;

	error =
	    lookupname(fdev, UIO_USERSPACE, FOLLOW, (struct vnode **)NULL,
	    &vp);
	if (error)
		return (error);
	if (vp->v_type != VBLK) {
		VN_RELE(vp);
		return (ENOTBLK);
	}
	dev = vp->v_rdev;
	VN_RELE(vp);
	*mpp = getmp(dev);
	if (*mpp == NULL)
		return (ENODEV);
	else
		return (0);
}
#endif
#endif /* QUOTA */
