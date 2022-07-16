/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:proc/prvfsops.c	1.3"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/cred.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/inline.h"
#include "sys/proc.h"
#include "sys/stat.h"
#include "sys/statvfs.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/mode.h"

#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/procfs.h"

#include "fs/fs_subr.h"

#include "prdata.h"

int procfstype = 0;
int prmounted = 0;		/* Set to 1 if /proc is mounted. */
struct vfs *procvfs;		/* Points to /proc vfs entry. */
dev_t procdev;

/*
 * /proc VFS operations vector.
 */
STATIC int	prmount(), prunmount(), prroot(), prstatvfs();

struct vfsops prvfsops = {
	prmount,
	prunmount,
	prroot,
	prstatvfs,
	fs_sync,
	fs_nosys,	/* vget */
	fs_nosys,	/* mountroot */
	fs_nosys,	/* swapvp */
	fs_nosys,	/* filler */
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
	fs_nosys,
};

/* ARGSUSED */
STATIC int
prmount(vfsp, mvp, uap, cr)
	struct vfs *vfsp;
	struct vnode *mvp;
	struct mounta *uap;
	struct cred *cr;
{
	register struct vnode *vp;
	register struct prnode *pnp;

	if (!suser(cr))
		return EPERM;
	if (mvp->v_type != VDIR)
		return ENOTDIR;
	if (mvp->v_count > 1 || (mvp->v_flag & VROOT))
		return EBUSY;
	/*
	 * Prevent duplicate mount.
	 */
	if (prmounted)
		return EBUSY;
	pnp = &prrootnode;
	vp = &pnp->pr_vnode;
	vp->v_vfsp = vfsp;
	vp->v_vfsmountedhere = NULL;
	vp->v_op = &prvnodeops;
	vp->v_count = 1;
	vp->v_type = VDIR;
	vp->v_data = (caddr_t) pnp;
	vp->v_flag |= VROOT;
	pnp->pr_mode = 0555;	/* read and search permissions */
	pnp->pr_vnext = NULL;
	pnp->pr_free = NULL;
	pnp->pr_proc = NULL;
	pnp->pr_opens = 0;
	pnp->pr_writers = 0;
	pnp->pr_flags = 0;
	vfsp->vfs_fstype = procfstype;
	vfsp->vfs_data = NULL;
	vfsp->vfs_dev = procdev;
	vfsp->vfs_fsid.val[0] = procdev;
	vfsp->vfs_fsid.val[1] = procfstype;
	vfsp->vfs_bsize = 1024;
	prmounted = 1;
	procvfs = vfsp;
	return 0;
}

/* ARGSUSED */
STATIC int
prunmount(vfsp, cr)
	struct vfs *vfsp;
	struct cred *cr;
{
	register proc_t *p;

	if (!suser(cr))
		return EPERM;
	/*
	 * Ensure that no /proc vnodes are in use.
	 */
	if (prrootnode.pr_vnode.v_count > 1)
		return EBUSY;

	for (p = practive; p != NULL; p = p->p_next)
		if (p->p_trace != NULL)
			return EBUSY;

	VN_RELE(&prrootnode.pr_vnode);
	prmounted = 0;
	procvfs = NULL;
	return 0;
}

/* ARGSUSED */
STATIC int
prroot(vfsp, vpp)
	struct vfs *vfsp;
	struct vnode **vpp;
{
	struct vnode *vp = &prrootnode.pr_vnode;

	VN_HOLD(vp);
	*vpp = vp;
	return 0;
}

STATIC int
prstatvfs(vfsp, sp)
	struct vfs *vfsp;
	register struct statvfs *sp;
{
	register int i, n;

        for (n = v.v_proc, i = 0; i < v.v_proc; i++)
		if (pid_entry(i) != NULL)
                        n--;

	bzero((caddr_t)sp, sizeof(*sp));
	sp->f_bsize	= 1024;
	sp->f_frsize	= 1024;
	sp->f_blocks	= 0;
	sp->f_bfree	= 0;
	sp->f_bavail	= 0;
	sp->f_files	= v.v_proc + 2;
	sp->f_ffree	= n;
	sp->f_favail	= n;
	sp->f_fsid	= vfsp->vfs_dev;
	strcpy(sp->f_basetype, vfssw[procfstype].vsw_name);
	sp->f_flag = vf_to_stf(vfsp->vfs_flag);
	sp->f_namemax = PNSIZ;
	strcpy(sp->f_fstr, "/proc");
	strcpy(&sp->f_fstr[6], "/proc");
	return 0;
}
