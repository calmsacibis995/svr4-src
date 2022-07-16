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

#ident	"@(#)kern-fs:ufs/quota_ufs.c	1.3"

/*
 * Routines used in checking limits on file system usage.
 */

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
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/fs/ufs_quota.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>
#include <sys/errno.h>

/*
 * Find the dquot structure that should
 * be used in checking i/o on inode ip.
 */
struct dquot *
getinoquota(ip)
	register struct inode *ip;
{
	register struct dquot *dqp;
	register struct ufsvfs *ufsvfsp;
	struct dquot *xdqp;

	ufsvfsp = (struct ufsvfs *)(ip->i_vnode.v_vfsp->vfs_data);
	/*
	 * Check for quotas enabled.
	 */
	if ((ufsvfsp->vfs_qflags & MQ_ENABLED) == 0)
		return (NULL);
	/*
	 * Check for someone doing I/O to quota file.
	 */
	if (ip == ufsvfsp->vfs_qinod)
		return (NULL);
	if (getdiskquota((uid_t)ip->i_uid, ufsvfsp, 0, &xdqp))
		return (NULL);
	dqp = xdqp;
	if (dqp->dq_fhardlimit == 0 && dqp->dq_fsoftlimit == 0 &&
	    dqp->dq_bhardlimit == 0 && dqp->dq_bsoftlimit == 0) {
		dqput(dqp);
		dqp = NULL;
	} else {
		DQUNLOCK(dqp);
	}
	return (dqp);
}

/*
 * Update disk usage, and take corrective action.
 */
int
chkdq(ip, change, force)
	struct inode *ip;
	long change;
	int force;
{
	register struct dquot *dqp;
	register u_long ncurblocks;
	int error = 0;

	if (change == 0)
		return (0);
	dqp = ip->i_dquot;
	if (dqp == NULL)
		return (0);
	dqp->dq_flags |= DQ_MOD;
	if (change < 0) {
		if ((int)dqp->dq_curblocks + change >= 0)
			dqp->dq_curblocks += change;
		else
			dqp->dq_curblocks = 0;
		if (dqp->dq_curblocks < dqp->dq_bsoftlimit)
			dqp->dq_btimelimit = 0;
		dqp->dq_flags &= ~DQ_BLKS;
		return (0);
	}

	ncurblocks = dqp->dq_curblocks + change;
	/*
	 * Allocation. Check hard and soft limits.
	 * Skip checks for super user.
	 */
	if (u.u_cred->cr_uid == 0)
		goto out;
	/*
	 * Dissallow allocation if it would bring the current usage over
	 * the hard limit or if the user is over his soft limit and his time
	 * has run out.
	 */
	if (ncurblocks >= dqp->dq_bhardlimit && dqp->dq_bhardlimit && !force) {
		if ((dqp->dq_flags & DQ_BLKS) == 0 && ip->i_uid == u.u_cred->cr_ruid)
			dqp->dq_flags |= DQ_BLKS;
		error = ENOSPC;
	}
	if (ncurblocks >= dqp->dq_bsoftlimit && dqp->dq_bsoftlimit) {
		if (dqp->dq_curblocks < dqp->dq_bsoftlimit ||
		    dqp->dq_btimelimit == 0) {
			dqp->dq_btimelimit =
			    hrestime.tv_sec +
			    ((struct ufsvfs *)ip->i_vnode.v_vfsp->vfs_data)->vfs_btimelimit;
		} else if (hrestime.tv_sec > dqp->dq_btimelimit && !force) {
			if ((dqp->dq_flags & DQ_BLKS) == 0 &&
			    ip->i_uid == u.u_cred->cr_ruid)
				dqp->dq_flags |= DQ_BLKS;
			error = ENOSPC;
		}
	}
out:
	if (error == 0)
		dqp->dq_curblocks = ncurblocks;
	return (error);
}

/*
 * Check the inode limit, applying corrective action.
 */
int
chkiq(ufsvfsp, ip, uid, force)
	struct ufsvfs *ufsvfsp;
	struct inode *ip;
	uid_t uid;
	int force;
{
	register struct dquot *dqp;
	register u_long ncurfiles;
	struct dquot *xdqp;
	int error = 0;

	/*
	 * Free.
	 */
	if (ip != NULL) {
		dqp = ip->i_dquot;
		if (dqp == NULL)
			return (0);
		dqp->dq_flags |= DQ_MOD;
		if (dqp->dq_curfiles)
			dqp->dq_curfiles--;
		if (dqp->dq_curfiles < dqp->dq_fsoftlimit)
			dqp->dq_ftimelimit = 0;
		dqp->dq_flags &= ~DQ_FILES;
		return (0);
	}

	/*
	 * Allocation. Get dquot for for uid, fs.
	 * Check for quotas enabled.
	 */
	if ((ufsvfsp->vfs_qflags & MQ_ENABLED) == 0)
		return (0);
	if (getdiskquota(uid, ufsvfsp, 0, &xdqp))
		return (0);
	dqp = xdqp;
	if (dqp->dq_fsoftlimit == 0 && dqp->dq_fhardlimit == 0) {
		dqput(dqp);
		return (0);
	}
	dqp->dq_flags |= DQ_MOD;
	/*
	 * Skip checks for super user.
	 */
	if (u.u_cred->cr_uid == 0)
		goto out;
	ncurfiles = dqp->dq_curfiles + 1;
	/*
	 * Dissallow allocation if it would bring the current usage over
	 * the hard limit or if the user is over his soft limit and his time
	 * has run out.
	 */
	if (ncurfiles >= dqp->dq_fhardlimit && dqp->dq_fhardlimit && !force) {
		if ((dqp->dq_flags & DQ_FILES) == 0 && uid == u.u_cred->cr_ruid)
			dqp->dq_flags |= DQ_FILES;
		error = ENOSPC;
	} else if (ncurfiles >= dqp->dq_fsoftlimit && dqp->dq_fsoftlimit) {
		if (ncurfiles == dqp->dq_fsoftlimit || dqp->dq_ftimelimit==0) {
			dqp->dq_ftimelimit = hrestime.tv_sec + ufsvfsp->vfs_ftimelimit;
		} else if (hrestime.tv_sec > dqp->dq_ftimelimit && !force) {
			if ((dqp->dq_flags&DQ_FILES) == 0 && uid == u.u_cred->cr_ruid)
				dqp->dq_flags |= DQ_FILES;
			error = ENOSPC;
		}
	}
out:
	if (error == 0)
		dqp->dq_curfiles++;
	dqput(dqp);
	return (error);
}

/*
 * Release a dquot.
 */
void
dqrele(dqp)
	register struct dquot *dqp;
{

	if (dqp != NULL) {
		DQLOCK(dqp);
		if (dqp->dq_cnt == 1 && dqp->dq_flags & DQ_MOD)
			dqupdate(dqp);
		dqput(dqp);
	}
}
