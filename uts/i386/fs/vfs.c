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

#ident	"@(#)kern-fs:vfs.c	1.3.1.1"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/uio.h"
#include "sys/fstyp.h"
#include "sys/kmem.h"
#include "sys/inline.h"
#include "sys/systm.h"
#include "sys/pathname.h"
#include "sys/proc.h"
#include "sys/mount.h"
#include "sys/vfs.h"
#include "sys/statvfs.h"
#include "sys/statfs.h"
#include "sys/cred.h"
#include "sys/vnode.h"
#include "sys/dnlc.h"
#include "sys/file.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"

/*
 * VFS global data.
 */
vnode_t *rootdir;		/* pointer to root vnode. */

STATIC struct vfs root;
struct vfs *rootvfs = &root;	/* pointer to root vfs; head of VFS list. */
int sync_active;		/* sync currently in progress, fix for
                                   Neal Nelson performanace improvement */

/*
 * System calls.
 */

/*
 * "struct mounta" defined in sys/vfs.h.
 */

/* ARGSUSED */
int
mount(uap, rvp)
	register struct mounta *uap;
	rval_t *rvp;
{
	vnode_t *vp = NULL;
	register struct vfs *vfsp;
	struct vfssw *vswp;
	struct vfsops *vfsops;
	register int error;
	int remount = 0, ovflags;
	
	/*
	 * Resolve second path name (mount point).
	 */
	if (error = lookupname(uap->dir, UIO_USERSPACE, FOLLOW, NULLVPP, &vp))
		return error;
	if (vp->v_vfsmountedhere != NULL) {
		VN_RELE(vp);
		return EBUSY;
	}
	if (vp->v_flag & VNOMOUNT) {
		VN_RELE(vp);
		return EINVAL;
	}

	/*
	 * Backward compatibility: require the user program to
	 * supply a flag indicating a new-style mount, otherwise
	 * assume the fstype of the root file system and zero
	 * values for dataptr and datalen.  MS_FSS indicates an
	 * SVR3 4-argument mount; MS_DATA is the preferred way
	 * and indicates a 6-argument mount.
	 */
	if (uap->flags & (MS_DATA|MS_FSS)) {
		u_int n, fstype;
		char fsname[FSTYPSZ];

		/*
		 * Even funnier: we want a user-supplied fstype name here,
		 * but for backward compatibility we have to accept a
		 * number if one is provided.  The heuristic used is to
		 * assume that a "pointer" with a numeric value of less
		 * than 256 is really an int.
		 */
		if ((fstype = (u_int)uap->fstype) < 256) {
			if (fstype == 0 || fstype >= nfstype) {
				VN_RELE(vp);
				return EINVAL;
			}
			vfsops = vfssw[fstype].vsw_vfsops;
		} else if (error = copyinstr(uap->fstype, fsname,
		  FSTYPSZ, &n)) {
			if (error == ENAMETOOLONG)
				error = EINVAL;
			VN_RELE(vp);
			return error;
		} else if ((vswp = vfs_getvfssw(fsname)) == NULL) {
			VN_RELE(vp);
			return EINVAL;
		} else
			vfsops = vswp->vsw_vfsops;
	} else
		vfsops = rootvfs->vfs_op;

	if ((uap->flags & MS_DATA) == 0) {
		uap->dataptr = NULL;
		uap->datalen = 0;
	}
		
	/*
	 * If this is a remount we don't want to create a new VFS.
	 * Instead we pass the existing one with a remount flag.
	 */
	if (uap->flags & MS_REMOUNT) {
		remount = 1;
		/*
		 * Confirm that the vfsp associated with the mount point
		 * has already been mounted on.
		 */
		if ((vp->v_flag & VROOT) == 0) {
			VN_RELE(vp);
			return ENOENT;
		}
		/*
		 * Disallow making file systems read-only.  Ignore other flags.
		 */
		if (uap->flags & MS_RDONLY) {
			VN_RELE(vp);
			return EINVAL;
		}
		vfsp = vp->v_vfsp;
		ovflags = vfsp->vfs_flag;
		vfsp->vfs_flag |= VFS_REMOUNT;
		vfsp->vfs_flag &= ~VFS_RDONLY;

	} else {
		if ((vfsp = (vfs_t *) kmem_alloc(sizeof(vfs_t), KM_SLEEP))
		  == NULL) {
			VN_RELE(vp);
			return EBUSY;
		}
		VFS_INIT(vfsp, vfsops, (caddr_t) NULL);
	}
	
	/*
	 * Lock the vfs so that lookuppn() will not venture into the
	 * covered vnode's subtree.
	 */
	if (error = vfs_lock(vfsp)) {
		VN_RELE(vp);
		if (!remount)
			kmem_free((caddr_t) vfsp, sizeof(struct vfs));
		return error;
	}

	dnlc_purge_vp(vp);

	if (error = VFS_MOUNT(vfsp, vp, uap, u.u_cred)) {
		vfs_unlock(vfsp);
		if (remount)
			vfsp->vfs_flag = ovflags;
		else
			kmem_free((caddr_t) vfsp, sizeof(struct vfs));
		VN_RELE(vp);
	} else {
		if (remount) {
			vfsp->vfs_flag &= ~VFS_REMOUNT;
			VN_RELE(vp);
		} else {
			vfs_add(vp, vfsp, uap->flags);
			vp->v_vfsp->vfs_nsubmounts++;
		}
		vfs_unlock(vfsp);
	}

	return error;
}

struct umounta {
	char	*pathp;
};

/* ARGSUSED */
int
umount(uap, rvp)
	struct umounta *uap;
	rval_t *rvp;
{
	vnode_t *fsrootvp;
	register struct vfs *vfsp;
	register int error;

	/*
	 * Lookup user-supplied name.
	 */
	if (error = lookupname(uap->pathp, UIO_USERSPACE, FOLLOW,
	  NULLVPP, &fsrootvp))
		return error;
	/*
	 * Find the vfs to be unmounted.  The caller may have specified
	 * either the directory mount point (preferred) or else (for a
	 * disk-based file system) the block device which was mounted.
	 * Check to see which it is; if it's the device, search the VFS
	 * list to find the associated vfs entry.
	 */
	if (fsrootvp->v_flag & VROOT)
		vfsp = fsrootvp->v_vfsp;
	else if (fsrootvp->v_type == VBLK)
		vfsp = vfs_devsearch(fsrootvp->v_rdev);
	else
		vfsp = NULL;
	VN_RELE(fsrootvp);
	if (vfsp == NULL)
		return EINVAL;
	/*
	 * Perform the unmount.
	 */
	return dounmount(vfsp, u.u_cred);
}

int
dounmount(vfsp, cr)
	register struct vfs *vfsp;
	register cred_t *cr;
{
	register vnode_t *coveredvp;
	int error;

	/*
	 * Get covered vnode.
	 */
	coveredvp = vfsp->vfs_vnodecovered;
	/*
	 * Lock vnode to maintain fs status quo during unmount.
	 */
	if (error = vfs_lock(vfsp))
		return error;

	/*
	 * Purge all dnlc entries for this vfs.
	 */
	dnlc_purge_vfsp(vfsp, 0);

	VFS_SYNC(vfsp, 0, cr);

	if (error = VFS_UNMOUNT(vfsp, cr))
		vfs_unlock(vfsp);
	else {
		--coveredvp->v_vfsp->vfs_nsubmounts;
		ASSERT(vfsp->vfs_nsubmounts == 0);
		vfs_remove(vfsp);
		kmem_free((caddr_t)vfsp, (u_int)sizeof(*vfsp));
		VN_RELE(coveredvp);
	}
	return error;
}

/*
 * Update every mounted file system.  We call the vfs_sync operation of
 * each file system type, passing it a NULL vfsp to indicate that all
 * mounted file systems of that type should be updated.
 */
void
sync()
{
	register int i;
	for (i = 1; i < nfstype; i++)
		(void) (*vfssw[i].vsw_vfsops->vfs_sync)(NULL, 0, u.u_cred);
}

/* ARGSUSED */
int
syssync(uap, rvp)
	char *uap;
	rval_t *rvp;
{
	if (sync_active == 1)
		return 0;
	sync_active = 1;
	sync();
	sync_active = 0;
	return 0;

}

/*
 * Get file system statistics (statvfs and fstatvfs).
 */

struct statvfsa {
	char	*fname;
	struct	statvfs *sbp;
};

#if defined(__STDC__)
STATIC int	cstatvfs(struct vfs *, struct statvfs *);
#else
STATIC int	cstatvfs();
#endif


/* ARGSUSED */
int
statvfs(uap, rvp)
	register struct statvfsa *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register int error;

	if (error = lookupname(uap->fname, UIO_USERSPACE,
	  FOLLOW, NULLVPP, &vp))
		return error;
	error = cstatvfs(vp->v_vfsp, uap->sbp);
	VN_RELE(vp);
	return error;
}

struct fstatvfsa {
	int	fdes;
	struct	statvfs *sbp;
};

/* ARGSUSED */
int
fstatvfs(uap, rvp)
	register struct fstatvfsa *uap;
	rval_t *rvp;
{
	struct file *fp;
	register int error;

	if (error = getf(uap->fdes, &fp))
		return error;
	return cstatvfs(fp->f_vnode->v_vfsp, uap->sbp);
}

/*
 * Common routine for statvfs and fstatvfs.
 */
STATIC int
cstatvfs(vfsp, ubp)
	register struct vfs *vfsp;
	struct statvfs *ubp;
{
	struct statvfs ds;
	register int error;

	struct_zero((caddr_t)&ds, sizeof(ds));
	if ((error = VFS_STATVFS(vfsp, &ds)) == 0
	  && copyout((caddr_t)&ds, (caddr_t)ubp, sizeof(ds)))
		error = EFAULT;
	return error;
}

/*
 * statfs(2) and fstatfs(2) have been replaced by statvfs(2) and
 * fstatvfs(2) and will be removed from the system in a near-future
 * release.
 */
struct statfsa {
	char	*fname;
	struct	statfs *sbp;
	int	len;
	int	fstyp;
};

#if defined(__STDC__)
STATIC int	cstatfs(struct vfs *, struct statfs *, int);
#else
STATIC int	cstatfs();
#endif

/* ARGSUSED */
int
statfs(uap, rvp)
	register struct statfsa *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register int error;

	if (error = lookupname(uap->fname, UIO_USERSPACE,
	  FOLLOW, NULLVPP, &vp))
		return error;
	if (uap->fstyp != 0)
		error = EINVAL;
	else
		error = cstatfs(vp->v_vfsp, uap->sbp, uap->len);
	VN_RELE(vp);
	return error;
}

struct fstatfsa {
	int	fdes;
	struct	statfs *sbp;
	int	len;
	int	fstyp;
};

/* ARGSUSED */
int
fstatfs(uap, rvp)
	register struct fstatfsa *uap;
	rval_t *rvp;
{
	struct file *fp;
	register int error;

	if (error = getf(uap->fdes, &fp))
		return error;
	if (uap->fstyp != 0)
		return EINVAL;
	return cstatfs(fp->f_vnode->v_vfsp, uap->sbp, uap->len);
}

/*
 * Common routine for fstatfs and statfs.
 */
STATIC int
cstatfs(vfsp, sbp, len)
	register struct vfs *vfsp;
	struct statfs *sbp;
	int len;
{
	struct statfs sfs;
	struct statvfs svfs;
	register int error, i;
	char *cp, *cp2;
	register struct vfssw *vswp;

	if (len < 0 || len > sizeof(struct statfs))
		return EINVAL;
	if (error = VFS_STATVFS(vfsp, &svfs))
		return error;

	/*
	 * Map statvfs fields into the old statfs structure.
	 */
	struct_zero((caddr_t)&sfs, sizeof(sfs));
	sfs.f_bsize = svfs.f_bsize;
	sfs.f_frsize = (svfs.f_frsize == svfs.f_bsize) ? 0 : svfs.f_frsize;
	sfs.f_blocks = svfs.f_blocks * (svfs.f_frsize/512);
	sfs.f_bfree = svfs.f_bfree * (svfs.f_frsize/512);
	sfs.f_files = svfs.f_files;
	sfs.f_ffree = svfs.f_ffree;

	cp = svfs.f_fstr;
	cp2 = sfs.f_fname;
	i = 0;
	while (i++ < sizeof(sfs.f_fname))
		if (*cp != '\0')
			*cp2++ = *cp++;
		else
			*cp2++ = '\0';
	while (*cp != '\0'
	  && i++ < (sizeof(svfs.f_fstr) - sizeof(sfs.f_fpack)))
		cp++;
	cp++;
	cp2 = sfs.f_fpack;
	i = 0;
	while (i++ < sizeof(sfs.f_fpack))
		if (*cp != '\0')
			*cp2++ = *cp++;
		else
			*cp2++ = '\0';
	if ((vswp = vfs_getvfssw(svfs.f_basetype)) == NULL)
		sfs.f_fstyp = 0;
	else
		sfs.f_fstyp = vswp - vfssw;

	if (copyout((caddr_t)&sfs, (caddr_t)sbp, len))
		return EFAULT;

	return 0;
}

/*
 * System call to map fstype numbers to names, and vice versa.
 */
struct fsa {
	int	opcode;
};

struct	fsinda {
	int	opcode;
	char	*fsname;
};

struct fstypa {
	int opcode;
	int index;
	char *cbuf;
};

STATIC int sysfsind(), sysfstyp();	

int
sysfs(uap, rvp)
	register struct fsa *uap;
	rval_t *rvp;
{
	int error;

	switch (uap->opcode) {
	case GETFSIND:
		error = sysfsind((struct fsinda *) uap, rvp);
		break;
	case GETFSTYP:
		error = sysfstyp((struct fstypa *) uap, rvp);
		break;
	case GETNFSTYP:
		/*
		 * Return number of fstypes configured in the system.
		 */
		rvp->r_val1 = nfstype - 1;
		error = 0;
		break;
	default:
		error = EINVAL;
	}

	return error;
}

STATIC int
sysfsind(uap, rvp)
	register struct	fsinda *uap;
	rval_t *rvp;
{
	/*
	 * Translate fs identifier to an index into the vfssw structure.
	 */
	register struct vfssw *vswp;
	char fsbuf[FSTYPSZ];
	int error;
	u_int len = 0;

	error = copyinstr(uap->fsname, fsbuf, FSTYPSZ, &len);
	if (error == ENOENT)		/* XXX */
		error = EINVAL;		/* XXX */
	if (len == 1)	/* Includes null byte */
		error = EINVAL;
	if (error)
		return error;
	/*
	 * Search the vfssw table for the fs identifier
	 * and return the index.
	 */
	for (vswp = vfssw; vswp < &vfssw[nfstype]; vswp++) {
		if (strcmp(vswp->vsw_name, fsbuf) == 0) {
			rvp->r_val1 = vswp - vfssw;
			return 0;
		}
	}
	return EINVAL;
}

/* ARGSUSED */
STATIC int
sysfstyp(uap, rvp)
	register struct fstypa *uap;
	rval_t *rvp;
{
	/*
	 * Translate fstype index into an fs identifier.
	 */
	register char *src;
	register int index;
	register struct vfssw *vswp;
	char *osrc;

	if ((index = uap->index) <= 0 || index >= nfstype)
		return EINVAL;
	vswp = &vfssw[index];
	src = vswp->vsw_name ? vswp->vsw_name : "";
	for (osrc = src; *src++; )
		;
	if (copyout(osrc, uap->cbuf, src - osrc))
		return EFAULT;
	return 0;
}
/*
 * External routines.
 */

/*
 * vfs_mountroot is called by main() to mount the root filesystem.
 */
void
vfs_mountroot()
{
	register int error;
	register int i;
	struct vfssw *vsw;

	/*
	 * "rootfstype" will ordinarily have been initialized to
	 * contain the name of the fstype of the root file system
	 * (this is user-configurable). Use the name to find the
	 * vfssw table entry.
	 */
	if (vsw = vfs_getvfssw(rootfstype)) {
		VFS_INIT(rootvfs, vsw->vsw_vfsops, (caddr_t)0);
		error = VFS_MOUNTROOT(rootvfs, ROOT_INIT);
	} else {
		/*
		 * If rootfstype is not set or not found, step through
		 * all the fstypes until we find one that will accept
		 * a mountroot() request.
		 */
		for (i = 1; i < nfstype; i++) {
			if (vfssw[i].vsw_vfsops) {
				VFS_INIT(rootvfs, vfssw[i].vsw_vfsops,
				  (caddr_t)0);
				if ((error =
				  VFS_MOUNTROOT(rootvfs, ROOT_INIT)) == 0)
					break;
			}
		}
	}
	if (error)
		cmn_err(CE_PANIC, "vfs_mountroot: cannot mount root");
	/*
	 * Get vnode for '/'.  Set up rootdir, u.u_rdir and u.u_cdir
	 * to point to it.  These are used by lookuppn() so that it
	 * knows where to start from ('/' or '.').
	 */
	if (VFS_ROOT(rootvfs, &rootdir))
		cmn_err(CE_PANIC, "vfs_mountroot: no root vnode");
	u.u_cdir = rootdir;
	VN_HOLD(u.u_cdir);
	u.u_rdir = NULL;
}

/*
 * vfs_add is called by a specific filesystem's mount routine to add
 * the new vfs into the vfs list and to cover the mounted-on vnode.
 * The vfs should already have been locked by the caller.
 *
 * coveredvp is zero if this is the root.
 */
void
vfs_add(coveredvp, vfsp, mflag)
	register vnode_t *coveredvp;
	register struct vfs *vfsp;
	int mflag;
{
	if (coveredvp != NULL) {
		vfsp->vfs_next = rootvfs->vfs_next;
		rootvfs->vfs_next = vfsp;
		coveredvp->v_vfsmountedhere = vfsp;
	} else {
		/*
		 * This is the root of the whole world.
		 */
		rootvfs = vfsp;
		vfsp->vfs_next = NULL;
	}
	vfsp->vfs_vnodecovered = coveredvp;

	if (mflag & MS_RDONLY)
		vfsp->vfs_flag |= VFS_RDONLY;
	else
		vfsp->vfs_flag &= ~VFS_RDONLY;

	if (mflag & MS_NOSUID)
		vfsp->vfs_flag |= VFS_NOSUID;
	else
		vfsp->vfs_flag &= ~VFS_NOSUID;
}

/*
 * Remove a vfs from the vfs list, and destroy pointers to it.
 * Should be called by filesystem "unmount" code after it determines
 * that an unmount is legal but before it destroys the vfs.
 */
void
vfs_remove(vfsp)
	register struct vfs *vfsp;
{
	register struct vfs *tvfsp;
	register vnode_t *vp;
	
	ASSERT(vfsp->vfs_flag & VFS_MLOCK);

	/*
	 * Can't unmount root.  Should never happen because fs will
	 * be busy.
	 */
	ASSERT(vfsp != rootvfs);

	for (tvfsp = rootvfs; tvfsp != NULL; tvfsp = tvfsp->vfs_next) {
		if (tvfsp->vfs_next == vfsp) {
			/*
			 * Remove vfs from list, unmount covered vp.
			 */
			tvfsp->vfs_next = vfsp->vfs_next;
			vp = vfsp->vfs_vnodecovered;
			vp->v_vfsmountedhere = NULL;
			/*
			 * Release lock and wakeup anybody waiting.
			 */
			vfs_unlock(vfsp);
			return;
		}
	}
	/*
	 * Can't find vfs to remove.
	 */
	cmn_err(CE_PANIC, "vfs_remove: vfs not found");
}

/*
 * Lock a filesystem to prevent access to it while mounting
 * and unmounting.  Returns error if already locked.
 */
int
vfs_lock(vfsp)
	register struct vfs *vfsp;
{
	if (vfsp->vfs_flag & VFS_MLOCK)
		return EBUSY;
	vfsp->vfs_flag |= VFS_MLOCK;
	return 0;
}

/*
 * Unlock a locked filesystem.
 */
void
vfs_unlock(vfsp)
	register struct vfs *vfsp;
{
	ASSERT(vfsp->vfs_flag & VFS_MLOCK);

	vfsp->vfs_flag &= ~VFS_MLOCK;
	/*
	 * Wake anybody (most likely lookuppn()) waiting for the
	 * lock to clear.
	 */
	if (vfsp->vfs_flag & VFS_MWAIT) {
		vfsp->vfs_flag &= ~VFS_MWAIT;
		wakeprocs((caddr_t)vfsp, PRMPT);
	}
}

struct vfs *
getvfs(fsid)
	fsid_t *fsid;
{
	register struct vfs *vfsp;

	for (vfsp = rootvfs; vfsp; vfsp = vfsp->vfs_next) {
		if (vfsp->vfs_fsid.val[0] == fsid->val[0]
		  && vfsp->vfs_fsid.val[1] == fsid->val[1]) {
			break;
		}
	}
	return vfsp;
}

/*
 * Search the vfs list for a specified device.  Returns a pointer to it
 * or NULL if no suitable entry is found.
 */
struct vfs *
vfs_devsearch(dev)
	dev_t dev;
{
	register struct vfs *vfsp;

	for (vfsp = rootvfs; vfsp != NULL; vfsp = vfsp->vfs_next)
		if (vfsp->vfs_dev == dev)
			return vfsp;
	return NULL;
}

/*
 * Find a vfssw entry given a file system type name.
 */
struct vfssw *
vfs_getvfssw(type)
	register char *type;
{
	register int i;

	if (type == NULL || *type == '\0')
		return NULL;
	for (i = 1; i < nfstype; i++)
		if (strcmp(type, vfssw[i].vsw_name) == 0)
			return &vfssw[i];
	return NULL;
}

/*
 * Map VFS flags to statvfs flags.  These shouldn't really be separate
 * flags at all.
 */
u_long
vf_to_stf(vf)
	u_long vf;
{
	u_long stf = 0;

	if (vf & VFS_RDONLY)
		stf |= ST_RDONLY;
	if (vf & VFS_NOSUID)
		stf |= ST_NOSUID;
	if (vf & VFS_NOTRUNC)
		stf |= ST_NOTRUNC;
	
	return stf;
}

/*
 * Entries for (illegal) fstype 0.
 */

STATIC int vfsstray();

STATIC struct vfsops vfs_strayops = {
	vfsstray,
	vfsstray,
	vfsstray,
	vfsstray,
	vfsstray,
	vfsstray,
	vfsstray,
	vfsstray,
	vfsstray,
	vfsstray,
	vfsstray,
	vfsstray,
};

STATIC int
vfsstray()
{
	cmn_err(CE_PANIC, "stray vfs operation");
	/* NOTREACHED */
}

void
vfsinit()
{
	register int i;

	/*
	 * fstype 0 is (arbitrarily) invalid.
	 */
	vfssw[0].vsw_vfsops = &vfs_strayops;
	vfssw[0].vsw_name = "BADVFS";

	/*
	 * Call all the init routines.
	 */
	for (i = 1; i < nfstype; i++)
		(*vfssw[i].vsw_init)(&vfssw[i], i);
}

/*
 * fsnull() and fsstray() are references needed by old lboot; remove when
 * lboot is fixed.
 */
void
fsnull()
{
}

void
fsstray()
{
}
