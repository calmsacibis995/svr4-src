/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:fdfs/fdops.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/dirent.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/inline.h"
#include "sys/kmem.h"
#include "sys/pathname.h"
#include "sys/resource.h"
#include "sys/statvfs.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/cred.h"
#include "sys/user.h"

#include "fs/fs_subr.h"

#define	min(a,b)	((a) <= (b) ? (a) : (b))
#define	max(a,b)	((a) >= (b) ? (a) : (b))
#define	round(r)	(((r)+sizeof(int)-1)&(~(sizeof(int)-1)))
#define	fdtoi(n)	((n)+100)

#define	FDFSDIRSIZE	14
struct fdfsdirect {
	short	d_ino;
	char	d_name[FDFSDIRSIZE];
};

#define	FDFSROOTINO	2
#define	FDFSDSIZE	sizeof(struct fdfsdirect)
#define	FDFSNSIZE	10

int			fdfsfstype = 0;
STATIC int		fdfsmounted = 0;
STATIC struct vfs	*fdfsvfs;
STATIC dev_t		fdfsdev;
STATIC int		fdfsrdev;

STATIC struct vnode fdfsvroot;

STATIC int	fdfsopen(), fdfsread(), fdfsgetattr(), fdfsaccess();
STATIC int	fdfslookup(), fdfscreate(), fdfsreaddir();
STATIC void	fdfsinactive();

STATIC struct vnodeops fdfsvnodeops = {
	fdfsopen,
	fs_nosys,	/* close */
	fdfsread,	/* read */
	fs_nosys,	/* write*/
	fs_nosys,	/* ioctl */
	fs_nosys,	/* setfl */
	fdfsgetattr,
	fs_nosys,	/* setattr */
	fdfsaccess,
	fdfslookup,
	fdfscreate,
	fs_nosys,	/* remove */
	fs_nosys,	/* link */
	fs_nosys,	/* rename */
	fs_nosys,	/* mkdir */
	fs_nosys,	/* rmdir */
	fdfsreaddir,
	fs_nosys,	/* symlink */
	fs_nosys,	/* readlink */
	fs_nosys,	/* fsync */
	fdfsinactive,
	fs_nosys,	/* fid */
	fs_rwlock,	/* rwlock */
	fs_rwunlock,	/* rwunlock */
	fs_nosys,	/* seek */
	fs_cmp,
	fs_nosys,	/* frlock */
	fs_nosys,	/* space */
	fs_nosys,	/* realvp */
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

STATIC int	fdfsget();

/* ARGSUSED */
STATIC int
fdfsopen(vpp, mode, cr)
	struct vnode **vpp;
	int mode;
	cred_t *cr;
{
	if ((*vpp)->v_type != VDIR)
		(*vpp)->v_flag |= VDUP;
	return 0;
}

/* ARGSUSED */
STATIC int
fdfsread(vp, uiop, ioflag, cr)
	register struct vnode *vp;
	register struct uio *uiop;
	int ioflag;
	struct cred *cr;
{
	static struct fdfsdirect dotbuf[] = {
		{ FDFSROOTINO, "."  },
		{ FDFSROOTINO, ".." }
	};
	struct fdfsdirect dirbuf;
	register int i, n;
	int minfd, maxfd, modoff, error = 0;
	int nentries = min(u.u_nofiles, u.u_rlimit[RLIMIT_NOFILE].rlim_cur);

	if (vp->v_type != VDIR)
		return ENOSYS;
	/*
	 * Fake up ".", "..", and the /dev/fd directory entries.
	 */
	if (uiop->uio_offset < 0
	  || uiop->uio_offset >= (nentries + 2) * FDFSDSIZE
	  || uiop->uio_resid <= 0)
		return 0;
	if (uiop->uio_offset < 2*FDFSDSIZE) {
		error = uiomove((caddr_t)dotbuf + uiop->uio_offset,
		  min(uiop->uio_resid, 2*FDFSDSIZE - uiop->uio_offset),
		  UIO_READ, uiop);
		if (uiop->uio_resid <= 0 || error)
			return error;
	}
	minfd = (uiop->uio_offset - 2*FDFSDSIZE)/FDFSDSIZE;
	maxfd = (uiop->uio_offset + uiop->uio_resid - 1)/FDFSDSIZE;
	modoff = uiop->uio_offset % FDFSDSIZE;
	for (i = 0; i < FDFSDIRSIZE; i++)
		dirbuf.d_name[i] = '\0';
	for (i = minfd; i < min(maxfd, nentries); i++) {
		n = i;
		dirbuf.d_ino = fdtoi(n);
		numtos((long)n, dirbuf.d_name);
		error = uiomove((caddr_t)&dirbuf + modoff,
		  min(uiop->uio_resid, FDFSDSIZE - modoff),
		    UIO_READ, uiop);
		if (uiop->uio_resid <= 0 || error)
			return error;
		modoff = 0;
	}

	return error;
}

/* ARGSUSED */
STATIC int
fdfsgetattr(vp, vap, flags, cr)
	register struct vnode *vp;
	register struct vattr *vap;
	int flags;
	cred_t *cr;
{
	if (vp->v_type == VDIR) {
		vap->va_nlink = 2;
		vap->va_size = (u.u_nofiles + 2) * FDFSDSIZE;
		vap->va_mode = 0555;
		vap->va_nodeid = FDFSROOTINO;
	} else {
		vap->va_nlink = 1;
		vap->va_size = 0;
		vap->va_mode = 0666;
		vap->va_nodeid = fdtoi(getminor(vp->v_rdev));
	}
	vap->va_type = vp->v_type;
	vap->va_rdev = vp->v_rdev;
	vap->va_blksize = 1024;
	vap->va_nblocks = 0;
	vap->va_atime = vap->va_mtime = vap->va_ctime = hrestime;
	vap->va_uid = 0;
	vap->va_gid = 0;
	vap->va_fsid = fdfsdev;
	vap->va_vcode = 0;
	return 0;
}

/* ARGSUSED */
STATIC int
fdfsaccess(vp, mode, cr)
	register struct vnode *vp;
	register int mode;
	register cred_t *cr;
{
	return 0;
}

/* ARGSUSED */
STATIC int
fdfslookup(dp, comp, vpp, pnp, flags, rdir, cr)
	struct vnode *dp;
	register char *comp;
	struct vnode **vpp;
	struct pathname *pnp;
	int flags;
	struct vnode *rdir;
	struct cred *cr;
{
	if (comp[0] == 0 || strcmp(comp, ".") == 0 || strcmp(comp, "..") == 0) {
		VN_HOLD(dp);
		*vpp = dp;
		return 0;
	}
	return fdfsget(comp, vpp);
}

/* ARGSUSED */
STATIC int
fdfscreate(dvp, comp, vap, excl, mode, vpp, cr)
	struct vnode *dvp;
	char *comp;
	struct vattr *vap;
	enum vcexcl excl;
	int mode;
	struct vnode **vpp;
	cred_t *cr;
{
	return fdfsget(comp, vpp);
}

/* ARGSUSED */
STATIC int
fdfsreaddir(vp, uiop, cr, eofp)
	struct vnode *vp;
	register struct uio *uiop;
	struct cred *cr;
	int *eofp;
{
	/* bp holds one dirent structure */
	char bp[round(sizeof(struct dirent)-1+FDFSNSIZE+1)];
	struct dirent *dirent = (struct dirent *)bp;
	int reclen, nentries;
	register int i, n;
	int oresid, dsize;
	off_t off;

	if (uiop->uio_offset < 0 || uiop->uio_resid <= 0
	  || (uiop->uio_offset % FDFSDSIZE) != 0)
		return ENOENT;

	dsize = (char *)dirent->d_name - (char *)dirent;
	oresid = uiop->uio_resid;
	nentries = min(u.u_nofiles, u.u_rlimit[RLIMIT_NOFILE].rlim_cur);

	for (; uiop->uio_resid > 0; uiop->uio_offset = off + FDFSDSIZE) {
		if ((off = uiop->uio_offset) == 0) {	/* "." */
			dirent->d_ino = FDFSROOTINO;
			dirent->d_name[0] = '.';
			dirent->d_name[1] = '\0';
			reclen = dsize+1+1;
		} else if (off == FDFSDSIZE) {		/* ".." */
			dirent->d_ino = FDFSROOTINO;
			dirent->d_name[0] = '.';
			dirent->d_name[1] = '.';
			dirent->d_name[2] = '\0';
			reclen = dsize+2+1;
		} else {
			/*
			 * Return entries corresponding to the allowable
			 * number of file descriptors for this process.
			 */
			if ((n = (off-2*FDFSDSIZE)/FDFSDSIZE) >= nentries)
				break;
			dirent->d_ino = fdtoi(n);
			numtos((long)n, dirent->d_name);
			reclen = dsize + strlen(dirent->d_name) + 1;
		}
		dirent->d_off = uiop->uio_offset + FDFSDSIZE;
		/*
		 * Pad to nearest word boundary (if necessary).
		 */
		for (i = reclen; i < round(reclen); i++)
			dirent->d_name[i-dsize] = '\0';
		dirent->d_reclen = reclen = round(reclen);
		if (reclen > uiop->uio_resid) {
			/*
			 * Error if no entries have been returned yet.
			 */
			if (uiop->uio_resid == oresid)
				return EINVAL;
			break;
		}
		/*
		 * uiomove() updates both resid and offset by the same
		 * amount.  But we want offset to change in increments
		 * of FDFSDSIZE, which is different from the number of bytes
		 * being returned to the user.  So we set uio_offset
		 * separately, ignoring what uiomove() does.
		 */
		if (uiomove((caddr_t) dirent, reclen, UIO_READ, uiop))
			return EFAULT;
	}
	if (eofp)
		*eofp = ((uiop->uio_offset-2*FDFSDSIZE)/FDFSDSIZE >= nentries);
	return 0;
}

/* ARGSUSED */
STATIC void
fdfsinactive(vp, cr)
	register struct vnode *vp;
	cred_t *cr;
{
	if (vp->v_type == VDIR)
		return;
	kmem_free((caddr_t)vp, sizeof(vnode_t));
}

STATIC int
fdfsget(comp, vpp)
	register char *comp;
	struct vnode **vpp;
{
	register int n = 0;
	register struct vnode *vp;

	while (*comp) {
		if (*comp < '0' || *comp > '9')
			return ENOENT;
		n = 10 * n + *comp++ - '0';
	}
	vp = (struct vnode *)kmem_zalloc(sizeof(struct vnode), KM_SLEEP);
	ASSERT(vp != NULL);
	vp->v_type = VCHR;
	vp->v_vfsp = fdfsvfs;
	vp->v_vfsmountedhere = NULL;
	vp->v_op = &fdfsvnodeops;
	vp->v_count = 1;
	vp->v_data = NULL;
	vp->v_flag = VNOMAP;
	vp->v_rdev = makedevice(fdfsrdev, n);
	*vpp = vp;
	return 0;
}

STATIC int	fdfsmount(), fdfsunmount(), fdfsroot(), fdfsstatvfs();

struct vfsops fdfsvfsops = {
	fdfsmount,
	fdfsunmount,
	fdfsroot,
	fdfsstatvfs,
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

void
fdfsinit(vswp, fstype)
	register struct vfssw *vswp;
	int fstype;
{
	register int dev, rdev;

	fdfsfstype = fstype;
	ASSERT(fdfsfstype != 0);
	/*
	 * Associate VFS ops vector with this fstype.
	 */
	vswp->vsw_vfsops = &fdfsvfsops;

	/*
	 * Assign unique "device" numbers (reported by stat(2)).
	 */
	dev = getudev();
	rdev = getudev();
	if (dev == -1 || rdev == -1) {
		cmn_err(CE_WARN, "fdfsinit: can't get unique device numbers");
		if (dev == -1)
			dev = 0;
		if (rdev == -1)
			rdev = 0;
	}
	fdfsdev = makedevice(dev, 0);
	fdfsrdev = rdev;
	fdfsmounted = 0;
}

/* ARGSUSED */
STATIC int
fdfsmount(vfsp, mvp, uap, cr)
	struct vfs *vfsp;
	struct vnode *mvp;
	struct mounta *uap;
	struct cred *cr;
{
	register struct vnode *vp;

	if (!suser(cr))
		return EPERM;
	if (mvp->v_type != VDIR)
		return ENOTDIR;
	if (mvp->v_count > 1 || (mvp->v_flag & VROOT))
		return EBUSY;
	/*
	 * Prevent duplicate mount.
	 */
	if (fdfsmounted)
		return EBUSY;
	vp = &fdfsvroot;
	vp->v_vfsp = vfsp;
	vp->v_vfsmountedhere = NULL;
	vp->v_op = &fdfsvnodeops;
	vp->v_count = 1;
	vp->v_type = VDIR;
	vp->v_data = NULL;
	vp->v_flag |= VROOT;
	vp->v_rdev = 0;
	vfsp->vfs_fstype = fdfsfstype;
	vfsp->vfs_data = NULL;
	vfsp->vfs_dev = fdfsdev;
	vfsp->vfs_fsid.val[0] = fdfsdev;
	vfsp->vfs_fsid.val[1] = fdfsfstype;
	vfsp->vfs_bsize = 1024;
	fdfsmounted = 1;
	fdfsvfs = vfsp;
	return 0;
}

/* ARGSUSED */
STATIC int
fdfsunmount(vfsp, cr)
	struct vfs *vfsp;
	struct cred *cr;
{
	if (!suser(cr))
		return EPERM;
	if (fdfsvroot.v_count > 1)
		return EBUSY;

	VN_RELE(&fdfsvroot);
	fdfsmounted = 0;
	fdfsvfs = NULL;
	return 0;
}

/* ARGSUSED */
STATIC int
fdfsroot(vfsp, vpp)
	struct vfs *vfsp;
	struct vnode **vpp;
{
	struct vnode *vp = &fdfsvroot;

	VN_HOLD(vp);
	*vpp = vp;
	return 0;
}

STATIC int
fdfsstatvfs(vfsp, sp)
	struct vfs *vfsp;
	register struct statvfs *sp;
{
	bzero((caddr_t)sp, sizeof(*sp));
	sp->f_bsize = 1024;
	sp->f_frsize = 1024;
	sp->f_blocks = 0;
	sp->f_bfree = 0;
	sp->f_bavail = 0;
	sp->f_files = min(u.u_nofiles, u.u_rlimit[RLIMIT_NOFILE].rlim_cur) + 2;
	sp->f_ffree = 0;
	sp->f_favail = 0;
	sp->f_fsid = vfsp->vfs_dev;
	strcpy(sp->f_basetype, vfssw[fdfsfstype].vsw_name);
	sp->f_flag = vf_to_stf(vfsp->vfs_flag);
	sp->f_namemax = FDFSNSIZE;
	strcpy(sp->f_fstr, "/dev/fd");
	strcpy(&sp->f_fstr[8], "/dev/fd");
	return 0;
}
