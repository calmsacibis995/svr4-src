/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *		PROPRIETARY NOTICE (Combined)
 *
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 *
 *
 *
 *		Copyright Notice
 *
 * Notice of copyright on this source code product does not indicate
 * publication.
 *
 *	(c) 1986,1987,1988,1989	 Sun Microsystems, Inc
 *	(c) 1983,1984,1985,1986,1987,1988,1989	AT&T.
 *		  All rights reserved.
 *
 */

#ident	"@(#)kern-fs:vncalls.c	1.3.2.8"

/*
 * System call routines for operations on files.  These manipulate
 * the global and per-process file table entries which refer to
 * vnodes, the system generic file abstraction.
 *
 * Many operations take a path name.  After preparing arguments, a
 * typical operation may proceed with:
 *
 *	error = lookupname(name, seg, followlink, &dvp, &vp);
 *
 * where "name" is the path name operated on, "seg" is UIO_USERSPACE
 * or UIO_SYSSPACE to indicate the address space in which the path
 * name resides, "followlink" specifies whether to follow symbolic
 * links, "dvp" is a pointer to a vnode for the directory containing
 * "name", and "vp" is a pointer to a vnode for "name".  (Both "dvp"
 * and "vp" are filled in by lookupname()).  "error" is zero for a
 * successful lookup, or a non-zero errno (from <sys/errno.h>) if an
 * error occurred.  This paradigm, in which routines return error
 * numbers to their callers and other information is returned via
 * reference parameters, now appears in many places in the kernel.
 *
 * lookupname() fetches the path name string into an internal buffer
 * using pn_get() (pathname.c) and extracts each component of the path
 * by iterative application of the file system-specific VOP_LOOKUP
 * operation until the final vnode and/or its parent are found.
 * (There is provision for multiple-component lookup as well.)  If
 * either of the addresses for dvp or vp are NULL, lookupname() assumes
 * that the caller is not interested in that vnode.  Once a vnode has
 * been found, a vnode operation (e.g. VOP_OPEN, VOP_READ) may be
 * applied to it.
 *
 * With few exceptions (made only for reasons of backward compatibility)
 * operations on vnodes are atomic, so that in general vnodes are not
 * locked at this level, and vnode locking occurs at lower levels (either
 * locally, or, perhaps, on a remote machine.  (The exceptions make use
 * of the VOP_RWLOCK and VOP_RWUNLOCK operations, and include VOP_READ,
 * VOP_WRITE, and VOP_READDIR).  In addition permission checking is
 * generally done by the specific filesystem, via its VOP_ACCESS
 * operation.  The upper (vnode) layer performs checks involving file
 * types (e.g. VREG, VDIR), since the type is static over the life of
 * the vnode.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/fcntl.h"
#include "sys/pathname.h"
#include "sys/stat.h"
#include "sys/ttold.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/mode.h"
#include "sys/proc.h"
#include "sys/sysinfo.h"
#include "sys/inline.h"
#include "sys/uio.h"
#include "sys/debug.h"
#include "sys/poll.h"
#include "sys/kmem.h"
#include "sys/filio.h"
#include "sys/locking.h"	/* XENIX Support */
#include "sys/disp.h"
#include "sys/mkdev.h"
#include "sys/time.h"
#include "sys/unistd.h"
#include "sys/conf.h"

/* XENIX Support */

#define XF_RDLCK	3	/* XENIX F_RDLCK */
#define XF_WRLCK	1	/* XENIX F_WRLCK */
#define XF_UNLCK	0	/* XENIX F_UNLCK */

	/* Maps XENIX 386 fcntl lock type value onto UNIX lock type value */
#define XMAP_TO_LTYPE(t) (t==XF_UNLCK?F_UNLCK:(t==XF_RDLCK?F_RDLCK:\
				(t==XF_WRLCK?F_WRLCK:t)))

	/* Maps UNIX fcntl lock type value onto XENIX 386 lock type value */
#define XMAP_FROM_LTYPE(t) (t==F_UNLCK?XF_UNLCK:(t==F_RDLCK?XF_RDLCK:\
				(t==F_WRLCK?XF_WRLCK:t)))
/* End XENIX Support */

/*
 * Open a file.
 */
struct opena {
	char	*fname;
	int	fmode;
	int	cmode;
};

#if defined(__STDC__)
STATIC int	copen(char *, int, int, rval_t *);
#else
STATIC int	copen();
#endif

int
open(uap, rvp)
	register struct opena *uap;
	rval_t *rvp;
{
	return copen(uap->fname, (int)(uap->fmode-FOPEN), uap->cmode, rvp);
}

/*
 * Create a file.
 */
struct creata {
	char	*fname;
	int	cmode;
};

int
creat(uap, rvp)
	register struct creata *uap;
	rval_t *rvp;
{
	return copen(uap->fname, FWRITE|FCREAT|FTRUNC, uap->cmode, rvp);
}

/*
 * Common code for open() and creat().  Check permissions, allocate
 * an open file structure, and call the device open routine (if any).
 */
STATIC int
copen(fname, filemode, createmode, rvp)
	char *fname;
	int filemode;
	int createmode;
	rval_t *rvp;
{
	vnode_t *vp;
	file_t *fp;
	register int error;
	int fd, dupfd;
	enum vtype type;
	enum create crwhy;

	if ((filemode & (FREAD|FWRITE)) == 0)
		return EINVAL;

	if ((filemode & (FNONBLOCK|FNDELAY)) == (FNONBLOCK|FNDELAY))
		filemode &= ~FNDELAY;

	if (error = falloc((vnode_t *)NULL, filemode & FMASK, &fp, &fd))
		return error;

	/*
	 * Last arg is a don't-care term if !(filemode & FCREAT).
	 */
	error = vn_open(fname, UIO_USERSPACE, filemode,
	  (int)((createmode & MODEMASK) & ~u.u_cmask), &vp, CRCREAT);

	if (error) {
		setf(fd, NULLFP);
		unfalloc(fp);
	} else if (vp->v_flag & VDUP) {
		/*
		 * Special handling for /dev/fd.  Give up the file pointer
		 * and dup the indicated file descriptor (in v_rdev).  This
		 * is ugly, but I've seen worse.
		 */
		setf(fd, NULLFP);
		unfalloc(fp);
		dupfd = getminor(vp->v_rdev);
		type = vp->v_type;
		vp->v_flag &= ~VDUP;
		VN_RELE(vp);
		if (type != VCHR)
			return EINVAL;
		if (error = getf(dupfd, &fp))
			return error;
		setf(fd, fp);
		fp->f_count++;
		rvp->r_val1 = fd;
	} else {
		fp->f_vnode = vp;
		rvp->r_val1 = fd;
	}

	return error;
}

/*
 * Close a file.
 */
struct closea {
	int	fdes;
};

/* ARGSUSED */
int
close(uap, rvp)
	register struct closea *uap;
	rval_t *rvp;
{
	file_t *fp;
	register int error;

	if (error = getf(uap->fdes, &fp))
		return error;
	error = closef(fp);
	setf(uap->fdes, NULLFP);
	return error;
}

/*
 * Read and write.
 */
struct rwa {
	int fdes;
	char *cbuf;
	unsigned count;
};

/*
 * Readv and writev.
 */
struct rwva {
	int fdes;
	struct iovec *iovp;
	int iovcnt;
};

#if defined(__STDC__)

STATIC int	rw(struct rwa *, rval_t *, int);
STATIC int	rwv(struct rwva *, rval_t *, int);
STATIC int	rdwr(file_t *, uio_t *, rval_t *, int);
#else

STATIC int	rw();
STATIC int	rwv();
STATIC int	rdwr();

#endif

int
read(uap, rvp)
	struct rwa *uap;
	rval_t *rvp;
{
	return rw(uap, rvp, FREAD);
}

int
write(uap, rvp)
	struct rwa *uap;
	rval_t *rvp;
{
	return rw(uap, rvp, FWRITE);
}

int
readv(uap, rvp)
	struct rwva *uap;
	rval_t *rvp;
{
	return rwv(uap, rvp, FREAD);
}

int
writev(uap, rvp)
	struct rwva *uap;
	rval_t *rvp;
{
	return rwv(uap, rvp, FWRITE);
}

STATIC int
rw(uap, rvp, mode)
	struct rwa *uap;
	rval_t *rvp;
	register int mode;
{
	struct uio auio;
	struct iovec aiov;
	file_t *fp;
	int error;

	if (error = getf(uap->fdes, &fp))
		return error;
	if ((fp->f_flag & mode) == 0)
		return EBADF;
	if (mode == FREAD)
		sysinfo.sysread++;
	else
		sysinfo.syswrite++;
	aiov.iov_base = (caddr_t)uap->cbuf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	return rdwr(fp, &auio, rvp, mode);
}

STATIC int
rwv(uap, rvp, mode)
	struct rwva *uap;
	rval_t *rvp;
	register int mode;
{
	file_t *fp;
	int error;
	struct uio auio;
	struct iovec aiov[16];		/* TO DO - don't hard-code size */

	if (error = getf(uap->fdes, &fp))
		return error;
	if ((fp->f_flag & mode) == 0)
		return EBADF;
	if (mode == FREAD)
		sysinfo.sysread++;
	else
		sysinfo.syswrite++;
	if (uap->iovcnt <= 0 || uap->iovcnt > sizeof(aiov)/sizeof(aiov[0]))
		return EINVAL;
	auio.uio_iov = aiov;
	auio.uio_iovcnt = uap->iovcnt;
	if (copyin((caddr_t)uap->iovp, (caddr_t)aiov,
	  (unsigned)(uap->iovcnt * sizeof(struct iovec))))
		return EFAULT;
	return rdwr(fp, &auio, rvp, mode);
}

/*
 * Common code for read and write calls: check permissions, set base,
 * count, and offset, and switch out to VOP_READ or VOP_WRITE code.
 */
STATIC int
rdwr(fp, uio, rvp, mode)
	file_t *fp;
	register struct uio *uio;
	rval_t *rvp;
	register int mode;
{
	register vnode_t	*vp;
	enum vtype		type;
	iovec_t			*iovp;
	int			ioflag;
	int			count;
	int			i;
	register int		error;

	uio->uio_resid = 0;
	iovp = uio->uio_iov;
	for (i = 0; i < uio->uio_iovcnt; i++) {
		if (iovp->iov_len < 0)
			return EINVAL;
		uio->uio_resid += iovp->iov_len;
		if (uio->uio_resid < 0)
			return EINVAL;
		iovp++;
	}
	vp = fp->f_vnode;
	type = vp->v_type;
	count = uio->uio_resid;
	uio->uio_offset = fp->f_offset;
	uio->uio_segflg = UIO_USERSPACE;
	uio->uio_fmode = fp->f_flag;
	uio->uio_limit = u.u_rlimit[RLIMIT_FSIZE].rlim_cur;
	ioflag = 0;
	if (fp->f_flag & FAPPEND)
		ioflag |= IO_APPEND;
	if (fp->f_flag & FSYNC)
		ioflag |= IO_SYNC;

	VOP_RWLOCK(vp);
	if (setjmp(&u.u_qsav))
		error = EINTR;
	else if (mode == FREAD)
		error = VOP_READ(vp, uio, ioflag, fp->f_cred);
	else
		error = VOP_WRITE(vp, uio, ioflag, fp->f_cred);
	VOP_RWUNLOCK(vp);

	if (error == EINTR && uio->uio_resid != count)
		error = 0;

	rvp->r_val1 = count - uio->uio_resid;
	u.u_ioch += (unsigned)rvp->r_val1;
	if (type == VFIFO)	/* Backward compatibility */
		fp->f_offset = rvp->r_val1;
	else
		fp->f_offset = uio->uio_offset;
	if (mode == FREAD) {
		sysinfo.readch += (unsigned)rvp->r_val1;
		if (vp->v_vfsp != NULL)
			vp->v_vfsp->vfs_bcount += 
			  rvp->r_val1 / vp->v_vfsp->vfs_bsize;

	} else {
		sysinfo.writech += (unsigned)rvp->r_val1;
		if (vp->v_vfsp != NULL)
			vp->v_vfsp->vfs_bcount += 
			  rvp->r_val1 / vp->v_vfsp->vfs_bsize;
	}
	return error;
}

/*
 * Change current working directory (".").
 */
struct chdira {
	char *fname;
};

#if defined(__STDC__)
STATIC int	chdirec(vnode_t *, vnode_t **);
#else
STATIC int	chdirec();
#endif

/* ARGSUSED */
int
chdir(uap, rvp)
	struct chdira *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register int  error;

	if (error = lookupname(uap->fname, UIO_USERSPACE, 
	    FOLLOW, NULLVPP, &vp))
		return error;

	return chdirec(vp, &u.u_cdir);
}

/*
 * File-descriptor based version of 'chdir'.
 */
struct fchdira {
	int  fd; 
};

/* ARGSUSED */
int
fchdir(uap, rvp)
	struct fchdira *uap;
	rval_t *rvp;
{
	file_t *fp;
	vnode_t *vp;
	register int error;
	
	if (error = getf(uap->fd, &fp))
		return error;
	vp = fp->f_vnode;
	VN_HOLD(vp);

	return chdirec(vp, &u.u_cdir);
}

/*
 * Change notion of root ("/") directory.
 */
/* ARGSUSED */
int
chroot(uap, rvp)
	struct chdira *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register int 	error;

	if (!suser(u.u_cred))
		return EPERM;
	if (error = lookupname(uap->fname, UIO_USERSPACE, 
	    FOLLOW, NULLVPP, &vp))
		return error;

	return chdirec(vp, &u.u_rdir);
}

/*
 * Chdirec() takes as an argument a vnode pointer and a vpp as an
 * out parameter.  If the vnode passed in corresponds to a 
 * directory for which the user has execute permission, then
 * vpp, if it is non-NULL, is updated to point to the vnode
 * passed in.  
 */
STATIC int
chdirec(vp, vpp)
	vnode_t *vp;
	vnode_t **vpp;
{
	register int error;

	if (vp->v_type != VDIR) {
		error = ENOTDIR;
		goto bad;
	}
	PREEMPT();
	if (error = VOP_ACCESS(vp, VEXEC, 0, u.u_cred))
		goto bad;
	if (*vpp)
		VN_RELE(*vpp);
	*vpp = vp;
	return 0;

bad:
	VN_RELE(vp);
	return error;
}

/*
 * Create a special file, a regular file, or a FIFO.
 */

/* SVR3 mknod arg */
struct mknoda {
	char	*fname;		/* pathname passed by user */
	mode_t	fmode;		/* mode of pathname */
	dev_t	dev;		/* device number - b/c specials only */
};

#if defined(__STDC__)
STATIC int	cmknod(int, char *, mode_t, dev_t, rval_t *);
#else
STATIC int	cmknod();
#endif

/* SVR3 mknod */
int
mknod(uap, rvp)
	register struct mknoda *uap;
	rval_t *rvp;
{
	return cmknod(_R3_MKNOD_VER, uap->fname, uap->fmode, uap->dev, rvp);
}

struct xmknoda {
	int	version;	/* version of this syscall */
	char	*fname;		/* pathname passed by user */
	mode_t	fmode;		/* mode of pathname */
	dev_t	dev;		/* device number - b/c specials only */
};

/*
 * Expanded mknod.
 */
xmknod(uap, rvp)
	register struct xmknoda *uap;
	rval_t *rvp;
{
	return cmknod(uap->version, uap->fname, uap->fmode, uap->dev, rvp);
}

/* ARGSUSED */
STATIC int
cmknod(version, fname, fmode, dev, rvp)
	int version;
	char *fname;
	mode_t fmode;
	dev_t dev;
	rval_t *rvp;
{
	vnode_t *vp;
	struct vattr vattr;
	int error;
	extern u_int maxminor;
	major_t major = getemajor(dev);
	minor_t highminor = maxminor;	/* `maxminor' is a tunable */

	/*
	 * Zero type is equivalent to a regular file.
	 */
	if ((fmode & S_IFMT) == 0)
		fmode |= S_IFREG;

	/*
	 * Must be the super-user unless making a FIFO node.
	 */
	if (((fmode & S_IFMT) != S_IFIFO) 
	/* XENIX Support */
	  && ((fmode & S_IFMT) != S_IFNAM)
	/* End XENIX Support */
	  && !suser(u.u_cred))
		return EPERM;
	/*
	 * Set up desired attributes and vn_create the file.
	 */
	vattr.va_type = IFTOVT(fmode);
	vattr.va_mode = (fmode & MODEMASK) & ~u.u_cmask;
	vattr.va_mask = AT_TYPE|AT_MODE;
	if (vattr.va_type == VCHR || vattr.va_type == VBLK
	  || vattr.va_type == VXNAM) {
		if (version == _MKNOD_VER && vattr.va_type != VXNAM) {
			if (dev == (dev_t)NODEV
			  || getemajor(dev) == (dev_t)NODEV)
				return EINVAL;
			else
				vattr.va_rdev = dev;
		} else {
			/* dev is in old format */
			if ((emajor(dev)) == (dev_t)NODEV
			  || dev == (dev_t)NODEV)
				return EINVAL;
			else
				vattr.va_rdev = expdev(dev);
		}

		/*
		 * Check minor number range
		 */
		if (vattr.va_type == VCHR) {
			if (major < cdevcnt &&
			    *cdevsw[major].d_flag & D_OLD &&
			    (cdevsw[major].d_str != (struct streamtab *)0 ||
			     shadowcsw[major].d_open != nodev))
			   	highminor = OMAXMIN; /* old-style driver */
		} else if (vattr.va_type == VBLK) {
			if (major < bdevcnt &&
			    *bdevsw[major].d_flag & D_OLD &&
			    shadowbsw[major].d_open != nodev)
			   	highminor = OMAXMIN; /* old-style driver */
		} /* VXNAM devices are limited to `maxminor' */

		/*
		 * If the driver is not installed, or is new-style, `highminor'
		 * will be equal to the tunable, `maxminor', otherwise it
		 * will be equal to `OMAXMIN'.
		 */
		if (geteminor(vattr.va_rdev) > highminor)
			return EINVAL;

		vattr.va_mask |= AT_RDEV;
	}
	if ((error = vn_create(fname, UIO_USERSPACE,
	  &vattr, EXCL, 0, &vp, CRMKNOD)) == 0)
		VN_RELE(vp);
	return error;
}

/*
 * Make a directory.
 */
struct mkdira {
	char *dname;
	int dmode;
};

/* ARGSUSED */
int
mkdir(uap, rvp)
	register struct mkdira *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	struct vattr vattr;
	int error;

	vattr.va_type = VDIR;
	vattr.va_mode = (uap->dmode & PERMMASK) & ~u.u_cmask;
	vattr.va_mask = AT_TYPE|AT_MODE;
	if ((error = vn_create(uap->dname, UIO_USERSPACE, &vattr,
	  EXCL, 0, &vp, CRMKDIR)) == 0)
		VN_RELE(vp);
	return error;
}

/*
 * Make a hard link.
 */
struct linka {
	char	*from;
	char	*to;
};

/* ARGSUSED */
int
link(uap, rvp)
	register struct linka *uap;
	rval_t *rvp;
{
	return vn_link(uap->from, uap->to, UIO_USERSPACE);
}

/*
 * Rename or move an existing file.
 */
struct renamea {
	char	*from;
	char	*to;
};

/* ARGSUSED */
int
rename(uap, rvp)
	struct renamea *uap;
	rval_t *rvp;
{
	return vn_rename(uap->from, uap->to, UIO_USERSPACE);
}

/*
 * Create a symbolic link.  Similar to link or rename except target
 * name is passed as string argument, not converted to vnode reference.
 */
struct symlinka {
	char	*target;
	char	*linkname;
};

/* ARGSUSED */
int
symlink(uap, rvp)
	register struct symlinka *uap;
	rval_t *rvp;
{
	vnode_t *dvp;
	struct vattr vattr;
	struct pathname tpn;
	struct pathname lpn;
	int error;

	if (error = pn_get(uap->linkname, UIO_USERSPACE, &lpn))
		return error;
	if (error = lookuppn(&lpn, NO_FOLLOW, &dvp, NULLVPP)) {
		pn_free(&lpn);
		return error;
	}
	if (dvp->v_vfsp->vfs_flag & VFS_RDONLY) {
		error = EROFS;
		goto out;
	}
	if ((error = pn_get(uap->target, UIO_USERSPACE, &tpn)) == 0) {
		vattr.va_type = VLNK;
		vattr.va_mode = 0777;
		vattr.va_mask = AT_TYPE|AT_MODE;
		error = VOP_SYMLINK(dvp, lpn.pn_path, &vattr,
		  tpn.pn_path, u.u_cred);
		pn_free(&tpn);
	}
out:
	pn_free(&lpn);
	VN_RELE(dvp);
	return error;
}

/*
 * Unlink (i.e. delete) a file.
 */
struct unlinka {
	char	*fname;
};

/* ARGSUSED */
int
unlink(uap, rvp)
	struct unlinka *uap;
	rval_t *rvp;
{
	return vn_remove(uap->fname, UIO_USERSPACE, RMFILE);
}

/*
 * Remove a directory.
 */
struct rmdira {
	char *dname;
};

/* ARGSUSED */
int
rmdir(uap, rvp)
	struct rmdira *uap;
	rval_t *rvp;
{
	return vn_remove(uap->dname, UIO_USERSPACE, RMDIRECTORY);
}

/*
 * Get directory entries in a file system-independent format.
 */
struct getdentsa {
	int fd;
	char *buf;
	int count;
};

int
getdents(uap, rvp)
	struct getdentsa *uap;
	rval_t *rvp;
{
	register vnode_t *vp;
	file_t *fp;
	struct uio auio;
	struct iovec aiov;
	register int error;
	int sink;

	if (error = getf(uap->fd, &fp))
		return error;
	vp = fp->f_vnode;
	if (vp->v_type != VDIR)
		return ENOTDIR;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = fp->f_offset;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_resid = uap->count;
	VOP_RWLOCK(vp);
	error = VOP_READDIR(vp, &auio, fp->f_cred, &sink);
	VOP_RWUNLOCK(vp);
	if (error)
		return error;
	rvp->r_val1 = uap->count - auio.uio_resid;
	fp->f_offset = auio.uio_offset;
	return 0;
}

/*
 * Seek on file.
 */
struct lseeka {
	int	fdes;
	off_t	off;
	int	sbase;
};

int
lseek(uap, rvp)
	register struct lseeka *uap;
	rval_t *rvp;
{
	file_t *fp;
	register vnode_t *vp;
	struct vattr vattr;
	register int error;

	if (error = getf(uap->fdes, &fp))
		return error;
	vp = fp->f_vnode;
	if (uap->sbase == 1)
		uap->off += fp->f_offset;
	else if (uap->sbase == 2) {
		vattr.va_mask = AT_SIZE;
		if (error = VOP_GETATTR(vp, &vattr, 0, fp->f_cred))
			return error;
		uap->off += vattr.va_size;
	} else if (uap->sbase != 0) {
	/* XENIX Support */
		/* For XENIX compatibility, don't send SIGSYS */
		if(!VIRTUAL_XOUT)	
	/* End XENIX Support */
		psignal(u.u_procp, SIGSYS);
		return EINVAL;
	}
	if ((error = VOP_SEEK(vp, fp->f_offset, &uap->off)) == 0)
		rvp->r_off = fp->f_offset = uap->off;
	return error;
}

/*
 * Determine accessibility of file.
 */
struct accessa {
	char	*fname;
	int	fmode;
};

#define E_OK	010	/* use effective ids */
#define R_OK	004
#define W_OK	002
#define X_OK	001

/* ARGSUSED */
int
access(uap, rvp)
	register struct accessa *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register cred_t *tmpcr;
	register int error, mode, eok;

	if (uap->fmode & ~(E_OK|R_OK|W_OK|X_OK))
		return EINVAL;

	mode = ((uap->fmode & (R_OK|W_OK|X_OK)) << 6);
	eok = (uap->fmode & E_OK);

	if (eok)
		tmpcr = u.u_cred;
	else {
		tmpcr = crdup(u.u_cred);
		tmpcr->cr_uid = u.u_cred->cr_ruid;
		tmpcr->cr_gid = u.u_cred->cr_rgid;
		tmpcr->cr_ruid = u.u_cred->cr_uid;
		tmpcr->cr_rgid = u.u_cred->cr_gid;
	}

	if (error = lookupname(uap->fname, UIO_USERSPACE,
	  FOLLOW, NULLVPP, &vp)) {
		if (!eok)
			crfree(tmpcr);
		return error;
	}

	if (mode)
		error = VOP_ACCESS(vp, mode, 0, tmpcr);

	if (!eok)
		crfree(tmpcr);
	VN_RELE(vp);
	return error;
}

/*
 * Get file attribute information through a file name or a file descriptor.
 */
struct stata {
	char	*fname;
	struct stat *sb;
};

#if defined(__STDC__)
STATIC int	cstat(vnode_t *, struct stat *, struct cred *);
#else
STATIC int	cstat();
#endif

/* ARGSUSED */
int
stat(uap, rvp)
	register struct stata *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register int error;

	if (error = lookupname(uap->fname, UIO_USERSPACE,
	  FOLLOW, NULLVPP, &vp))
		return error;
	error = cstat(vp, uap->sb, u.u_cred);
	VN_RELE(vp);
	return error;
}

struct xstatarg {
	int version;
	char *fname;
	struct xstat *sb;
};

#if defined(__STDC__)
STATIC int	xcstat(vnode_t *, struct xstat *, struct cred *);
#else
STATIC int	xcstat();
#endif

/* ARGSUSED */
int
xstat(uap, rvp)
	register struct xstatarg *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register int error;

	if (error = lookupname(uap->fname, UIO_USERSPACE,
	  FOLLOW, NULLVPP, &vp))
		return error;

	/*
	 * Check version.
	 */
	switch (uap->version) {

	case _STAT_VER:
		/* SVR4 stat */
		error = xcstat(vp, uap->sb, u.u_cred);
		break;

	case _R3_STAT_VER:
		/* SVR3 stat */
		error = cstat(vp, (struct stat *)uap->sb, u.u_cred);
		break;

	default:
		error = EINVAL;
	}

	VN_RELE(vp);
	return error;
}

struct lstata {
	char	*fname;
	struct stat *sb;
};

/* ARGSUSED */
int
lstat(uap, rvp)
	register struct lstata *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register int error;

	if (error = lookupname(uap->fname, UIO_USERSPACE,
	  NO_FOLLOW, NULLVPP, &vp))
		return error;
	error = cstat(vp, uap->sb, u.u_cred);
	VN_RELE(vp);
	return error;
}

/* ARGSUSED */
int
lxstat(uap, rvp)
	register struct xstatarg *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register int error;

	if (error = lookupname(uap->fname, UIO_USERSPACE,
	  NO_FOLLOW, NULLVPP, &vp))
		return error;

	/*
	 * Check version.
	 */
	switch (uap->version) {

	case _STAT_VER:
		/* SVR4 stat */
		error = xcstat(vp, uap->sb, u.u_cred);
		break;

	case _R3_STAT_VER:
		/* SVR3 stat */
		error = cstat(vp, (struct stat *) uap->sb, u.u_cred);
		break;

	default:
		error = EINVAL;
	}

	VN_RELE(vp);
	return error;
}

struct fstata {
	int	fdes;
	struct stat *sb;
};

/* ARGSUSED */
int
fstat(uap, rvp)
	register struct fstata *uap;
	rval_t *rvp;
{
	file_t *fp;
	register int error;

	if (error = getf(uap->fdes, &fp))
		return error;
	return cstat(fp->f_vnode, uap->sb, fp->f_cred);
}

struct fxstatarg {
	int	version;
	int	fdes;
	struct xstat *sb;
};

/* ARGSUSED */
int
fxstat(uap, rvp)
	register struct fxstatarg *uap;
	rval_t *rvp;
{
	file_t *fp;
	register int error;

	/*
	 * Check version number.
	 */
	switch (uap->version) {
	case _STAT_VER:
		break;
	default:
		return EINVAL;
	}

	if (error = getf(uap->fdes, &fp))
		return error;

	switch (uap->version) {
	case _STAT_VER:
		/* SVR4 stat */
		error = xcstat(fp->f_vnode, uap->sb, fp->f_cred);
		break;

	case _R3_STAT_VER:
		/* SVR3 stat */
		error = cstat(fp->f_vnode, (struct stat *) uap->sb, fp->f_cred);
		break;

	default:
		error = EINVAL;
	}

	return error;
}

/*
 * Common code for stat(), lstat(), and fstat().
 */
STATIC int
cstat(vp, ubp, cr)
	register vnode_t *vp;
	struct stat *ubp;
	struct cred *cr;
{
	struct stat sb;
	struct vattr vattr;
	register int error;

	vattr.va_mask = AT_STAT;
	if (error = VOP_GETATTR(vp, &vattr, 0, cr))
		return error;
	sb.st_mode = (o_mode_t) (VTTOIF(vattr.va_type) | vattr.va_mode);
	/*
	 * Check for large values.
	 */
	if (vattr.va_uid > USHRT_MAX || vattr.va_gid > USHRT_MAX
	  || vattr.va_nodeid > USHRT_MAX || vattr.va_nlink > SHRT_MAX )
		return EOVERFLOW;
	sb.st_uid = (o_uid_t) vattr.va_uid;
	sb.st_gid = (o_gid_t) vattr.va_gid;
	/*
	 * Need to convert expanded dev to old dev format.
	 */
	if (vattr.va_fsid & 0x8000)
		sb.st_dev = (o_dev_t) vattr.va_fsid;
	else
		sb.st_dev = (o_dev_t) cmpdev(vattr.va_fsid);
	sb.st_ino = (o_ino_t) vattr.va_nodeid;
	sb.st_nlink = (o_nlink_t) vattr.va_nlink;
	sb.st_size = vattr.va_size;
	sb.st_atime = vattr.va_atime.tv_sec;
	sb.st_mtime = vattr.va_mtime.tv_sec;
	sb.st_ctime = vattr.va_ctime.tv_sec;
	sb.st_rdev = (o_dev_t)cmpdev(vattr.va_rdev);

	PREEMPT();

	if (copyout((caddr_t)&sb, (caddr_t)ubp, sizeof(sb)))
		error = EFAULT;
	return error;
}

/*
 * Common code for xstat(), lxstat(), and fxstat().
 */
STATIC int
xcstat(vp, ubp, cr)
	register vnode_t *vp;
	struct xstat *ubp;
	struct cred *cr;
{
	struct xstat sb;
	struct vattr vattr;
	register int error;
	register struct vfssw *vswp;

	vattr.va_mask = AT_STAT|AT_NBLOCKS|AT_BLKSIZE;
	if (error = VOP_GETATTR(vp, &vattr, 0, cr))
		return error;

	struct_zero((caddr_t)&sb, sizeof(sb));

	sb.st_mode = VTTOIF(vattr.va_type) | vattr.va_mode;
	sb.st_uid = vattr.va_uid;
	sb.st_gid = vattr.va_gid;
	sb.st_dev = vattr.va_fsid;
	sb.st_ino = vattr.va_nodeid;
	sb.st_nlink = vattr.va_nlink;
	sb.st_size = vattr.va_size;
	sb.st_atime = vattr.va_atime;
	sb.st_mtime = vattr.va_mtime;
	sb.st_ctime = vattr.va_ctime;
	sb.st_rdev = vattr.va_rdev;
	sb.st_blksize = vattr.va_blksize;
	sb.st_blocks = vattr.va_nblocks;
	if (vp->v_vfsp) {
		vswp = &vfssw[vp->v_vfsp->vfs_fstype];
		if (vswp->vsw_name && *vswp->vsw_name)
			strcpy(sb.st_fstype, vswp->vsw_name);

	}
	if (copyout((caddr_t)&sb, (caddr_t)ubp, sizeof(sb)))
		error = EFAULT;
	return error;
}

#if defined(__STDC__)
STATIC int	cpathconf(vnode_t *, int, rval_t *, struct cred *);
#else
STATIC int	cpathconf();
#endif

/* fpathconf/pathconf interfaces */

struct fpathconfa {
	int	fdes;
	int	name;
};

/* ARGSUSED */
int
fpathconf(uap, rvp)
	register struct fpathconfa *uap;
	rval_t *rvp;
{
	file_t *fp;
	register int error;

	if (error = getf(uap->fdes, &fp))
		return error;
	return cpathconf(fp->f_vnode, uap->name, rvp, fp->f_cred);
}

struct pathconfa {
	char	*fname;
	int	name;
};

/* ARGSUSED */
int
pathconf(uap, rvp)
	register struct pathconfa *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	register int error;

	if (error = lookupname(uap->fname, UIO_USERSPACE,
	  FOLLOW, NULLVPP, &vp))
		return error;
	error = cpathconf(vp, uap->name, rvp, u.u_cred);
	VN_RELE(vp);
	return error;
}
/*
 * Common code for pathconf(), fpathconf() system calls
 */
STATIC int
cpathconf(vp, cmd, rvp, cr)
	register vnode_t *vp;
	int cmd;
	rval_t *rvp;
	struct cred *cr;
{
	register int error;
	u_long val;

	if ((error = VOP_PATHCONF(vp, cmd, &val, cr)) == 0)
		rvp->r_val1 = val;

	return error;
}

/*
 * Read the contents of a symbolic link.
 */
struct readlinka {
	char	*name;
	char	*buf;
	int	count;
};

int
readlink(uap, rvp)
	register struct readlinka *uap;
	rval_t *rvp;
{
	vnode_t *vp;
	struct iovec aiov;
	struct uio auio;
	int error;

	if (error = lookupname(uap->name, UIO_USERSPACE,
	  NO_FOLLOW, NULLVPP, &vp))
		return error;

	if (vp->v_type != VLNK) {
		error = EINVAL;
		goto out;
	}
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = 0;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_resid = uap->count;
	error = VOP_READLINK(vp, &auio, u.u_cred);
out:
	VN_RELE(vp);
	rvp->r_val1 = uap->count - auio.uio_resid;
	return error;
}

#if defined(__STDC__)
int	namesetattr(char *, enum symfollow, vattr_t *, int);
int	fdsetattr(int, vattr_t *);
#else
int	namesetattr();
int	fdsetattr();
#endif

/*
 * Change mode of file given path name.
 */
struct chmoda {
	char	*fname;
	int	fmode;
};

/* ARGSUSED */
int
chmod(uap, rvp)
	register struct chmoda *uap;
	rval_t *rvp;
{
	struct vattr vattr;

	vattr.va_mode = uap->fmode & MODEMASK;
	vattr.va_mask = AT_MODE;
	return namesetattr(uap->fname, FOLLOW, &vattr, 0);
}

/*
 * Change mode of file given file descriptor.
 */
struct fchmoda {
	int	fd;
	int	fmode;
};

/* ARGSUSED */
int
fchmod(uap, rvp)
	register struct fchmoda *uap;
	rval_t *rvp;
{
	struct vattr vattr;

	vattr.va_mode = uap->fmode & MODEMASK;
	vattr.va_mask = AT_MODE;
	return fdsetattr(uap->fd, &vattr);
}

/*
 * Change ownership of file given file name.
 */
struct chowna {
	char	*fname;
	int	uid;
	int	gid;
};

/* ARGSUSED */
int
chown(uap, rvp)
	register struct chowna *uap;
	rval_t *rvp;
{
	struct vattr vattr;

	if (uap->uid < (uid_t)-1 || uap->uid > MAXUID
	  || uap->gid < (gid_t)-1 || uap->gid > MAXUID)
		return EINVAL;
	vattr.va_uid = uap->uid;
	vattr.va_gid = uap->gid;
	vattr.va_mask = 0;
	if (vattr.va_uid != (uid_t)-1)
		vattr.va_mask |= AT_UID;
	if (vattr.va_gid != (gid_t)-1)
		vattr.va_mask |= AT_GID;
	return namesetattr(uap->fname, FOLLOW, &vattr, 0);
}

/* ARGSUSED */
int
lchown(uap, rvp)
	register struct chowna *uap;
	rval_t *rvp;
{
	struct vattr vattr;

	if (uap->uid < (uid_t)-1 || uap->uid > MAXUID
	  || uap->gid < (gid_t)-1 || uap->gid > MAXUID)
		return EINVAL;
	vattr.va_uid = uap->uid;
	vattr.va_gid = uap->gid;
	vattr.va_mask = 0;
	if (vattr.va_uid != (uid_t)-1)
		vattr.va_mask |= AT_UID;
	if (vattr.va_gid != (gid_t)-1)
		vattr.va_mask |= AT_GID;
	return namesetattr(uap->fname, NO_FOLLOW, &vattr, 0);
}

/*
 * Change ownership of file given file descriptor.
 */
struct fchowna {
	int	fd;
	int	uid;
	int	gid;
};

/* ARGSUSED */
int
fchown(uap, rvp)
	register struct fchowna *uap;
	rval_t *rvp;
{
	struct vattr vattr;

	if (uap->uid < (uid_t)-1 || uap->uid > MAXUID
	  || uap->gid < (gid_t)-1 || uap->gid > MAXUID)
		return EINVAL;
	vattr.va_uid = uap->uid;
	vattr.va_gid = uap->gid;
	vattr.va_mask = 0;
	if (vattr.va_uid != (uid_t)-1)
		vattr.va_mask |= AT_UID;
	if (vattr.va_gid != (gid_t)-1)
		vattr.va_mask |= AT_GID;
	return fdsetattr(uap->fd, &vattr);
}

/* XENIX Support */
/* 
 * Change file size.
 */
struct chsizea {
	int fdes;
	int size;
};

/* ARGSUSED */
chsize(uap, rvp)
	register struct chsizea *uap;
	rval_t *rvp;
{
	register vnode_t *vp;
	int error;
	file_t *fp;
	struct flock bf;

	if (uap->size < 0L || uap->size > u.u_rlimit[RLIMIT_FSIZE].rlim_cur)
		return EFBIG;
	if (error = getf(uap->fdes, &fp))
		return error;
	if ((fp->f_flag & FWRITE) == 0)
		return EBADF;
	vp = fp->f_vnode;
	if (vp->v_type != VREG)
		return EINVAL;         /* could have better error */
	bf.l_whence = 0;
	bf.l_start = uap->size;
	bf.l_type = F_WRLCK;
	bf.l_len = 0;
	if (error =  VOP_SPACE(vp, F_FREESP, &bf, fp->f_flag, fp->f_offset,
	  	fp->f_cred))
		if (BADVISE_PRE_SV && (error == EAGAIN))
			error = EACCES;
	return error;
}

/* 
 * Read check.
 */
struct rdchka {
	int fdes;
};

/* ARGSUSED */
rdchk(uap, rvp)
	register struct rdchka *uap;
	rval_t *rvp;
{
	register vnode_t *vp;
	file_t *fp;
	vattr_t vattr;
	register int error;

	if (error = getf(uap->fdes, &fp))
		return error;
	if ((fp->f_flag & FREAD) == 0)
		return EBADF;
	vp = fp->f_vnode;
	if (vp->v_type == VCHR)
		error = spec_rdchk(vp, fp->f_cred, &rvp->r_val1);
	else if (vp->v_type == VFIFO) {
		vattr.va_mask = AT_SIZE;
		if (error = VOP_GETATTR(vp, &vattr, 0, fp->f_cred))
			return error;
		if (vattr.va_size > 0 || fifo_rdchk(vp) <= 0
		  || fp->f_flag & (FNDELAY|FNONBLOCK))
			rvp->r_val1 = 1;
		else
			rvp->r_val1 = 0;
	} else
		rvp->r_val1 = 1;

	return error;
}

/*
 * XENIX locking() system call.  Locking() is a system call subtype called
 * through the cxenix sysent entry.
 *
 * The following is a summary of how locking() calls map onto fcntl():
 *
 *	locking() 	new fcntl()	acts like fcntl()	with flock 
 *	 'mode'		  'cmd'		     'cmd'		 'l_type'
 *	---------	-----------     -----------------	-------------
 *
 *	LK_UNLCK	F_LK_UNLCK	F_SETLK			F_UNLCK
 *	LK_LOCK		F_LK_LOCK	F_SETLKW		F_WRLCK
 *	LK_NBLCK	F_LK_NBLCK	F_SETLK			F_WRLCK
 *	LK_RLCK		F_LK_RLCK	F_SETLKW		F_RDLCK
 *	LK_NBRLCK	F_LK_NBRLCK	F_SETLW			F_RDLCK
 *
 */
struct lockinga {
	int  fdes;
	int  mode;
	long arg;
};

/* ARGSUSED */
int
locking(uap, rvp)
	struct lockinga *uap;
	rval_t *rvp;
{
	file_t *fp;
	struct flock bf, obf;
	register int error, cmd, scolk;

	scolk=0;
	if (error = getf(uap->fdes, &fp))
		return error;

	/*
	 * Map the locking() mode onto the fcntl() cmd.
	 */
	switch (uap->mode) {
	case LK_UNLCK:
		cmd = F_SETLK;
		bf.l_type = F_UNLCK;
		break;
	case LK_LOCK:
		cmd = F_SETLKW;
		bf.l_type = F_WRLCK;
		break;
	case LK_NBLCK:
		cmd = F_SETLK;
		bf.l_type = F_WRLCK;
		break;
	case LK_RLCK:
		cmd = F_SETLKW;
		bf.l_type = F_RDLCK;
		break;
	case LK_NBRLCK:
		cmd = F_SETLK;
		bf.l_type = F_RDLCK;
		break;
	/* XENIX Support */
	case F_O_GETLK:
	case F_SETLK:
	case F_SETLKW:
		/*
		 * Kludge to some SCO fcntl/lockf
		 * x.outs (they map onto locking, instead of
		 * onto fcntl...).
		 */
		if (isXOUT) {
			cmd = uap->mode;
			scolk++;
			break;
		}
		else
			return EINVAL;
	/* End XENIX Support */
	default:
		return EINVAL;
	}

	if(scolk==0){
		bf.l_whence = 1;
		if (uap->arg < 0) {
			bf.l_start = uap->arg;
			bf.l_len = -(uap->arg);
		} else {
			bf.l_start = 0L;
			bf.l_len = uap->arg;
		}
	}
	else{				/* SCO fcntl/lockf */
		if (copyin((caddr_t)uap->arg, (caddr_t)&bf, sizeof(struct o_flock)))
			return EFAULT;
		else
			bf.l_type = XMAP_TO_LTYPE(bf.l_type);	
	}


	if ((error = VOP_FRLOCK(fp->f_vnode, cmd, &bf, fp->f_flag,
	  fp->f_offset, fp->f_cred)) != 0) {
		if (BADVISE_PRE_SV && (error == EAGAIN))
			error = EACCES;
	}

	else {
	 	if (error == 0 && (uap->mode == F_SETLK || uap->mode == F_SETLKW)) 
			fp->f_vnode->v_flag |= VXLOCKED;
		if (uap->mode != F_SETLKW) {
			bf.l_type = XMAP_FROM_LTYPE(bf.l_type);
		}

		
		if(cmd == F_O_GETLK) {
			obf.l_type = bf.l_type;
			obf.l_whence = bf.l_whence;
			obf.l_start = bf.l_start;
			obf.l_len = bf.l_len;
			if(bf.l_sysid > SHRT_MAX || bf.l_pid > SHRT_MAX) 
					return EOVERFLOW;
			obf.l_sysid = (short) bf.l_sysid;
			obf.l_pid = (o_pid_t) bf.l_pid;
			if (copyout((caddr_t)&obf, (caddr_t)uap->arg, sizeof obf))
				return EFAULT;
		}
	}
	return error;
}
/* End XENIX Support */

/*
 * Set access/modify times on named file.
 */
struct utimea {
	char	*fname;
	time_t	*tptr;
};

/* ARGSUSED */
int
utime(uap, rvp)
	register struct utimea *uap;
	rval_t *rvp;
{
	time_t tv[2];
	struct vattr vattr;
	int flags = 0;

	if (uap->tptr != NULL) {
		if (copyin((caddr_t)uap->tptr,(caddr_t)tv, sizeof(tv)))
			return EFAULT;
		flags |= ATTR_UTIME;
	} else {
		tv[0] = hrestime.tv_sec;
		tv[1] = hrestime.tv_sec;
	}
	vattr.va_atime.tv_sec = tv[0];
	vattr.va_atime.tv_nsec = 0;
	vattr.va_mtime.tv_sec = tv[1];
	vattr.va_mtime.tv_nsec = 0;
	vattr.va_mask = AT_ATIME|AT_MTIME;
	return namesetattr(uap->fname, FOLLOW, &vattr, flags);
}

/*
 * Common routine for modifying attributes of named files.
 */
int
namesetattr(fnamep, followlink, vap, flags)
	char *fnamep;
	enum symfollow followlink;
	struct vattr *vap;
	int flags;
{
	vnode_t *vp;
	register int error;

	if (error = lookupname(fnamep, UIO_USERSPACE, followlink,
	  NULLVPP, &vp))
		return error;	
	if (vp->v_vfsp->vfs_flag & VFS_RDONLY)
		error = EROFS;
	else
		error = VOP_SETATTR(vp, vap, flags, u.u_cred);
	VN_RELE(vp);
	return error;
}

/*
 * Common routine for modifying attributes of files referenced
 * by descriptor.
 */
int
fdsetattr(fd, vap)
	int fd;
	struct vattr *vap;
{
	file_t *fp;
	register vnode_t *vp;
	register int error;

	if ((error = getf(fd, &fp)) == 0) {
		vp = fp->f_vnode;
		if (vp->v_vfsp->vfs_flag & VFS_RDONLY)
			return EROFS;
		error = VOP_SETATTR(vp, vap, 0, fp->f_cred);
	}
	return error;
}

/*
 * Flush output pending for file.
 */
struct fsynca {
	int fd;
};

/* ARGSUSED */
int
fsync(uap, rvp)
	struct fsynca *uap;
	rval_t *rvp;
{
	file_t *fp;
	register int error;

	if ((error = getf(uap->fd, &fp)) == 0)
		error = VOP_FSYNC(fp->f_vnode, fp->f_cred);
	return error;
}

/*
 * File control.
 */

struct fcntla {
	int fdes;
	int cmd;
	int arg;
};

int
fcntl(uap, rvp)
	register struct fcntla *uap;
	rval_t *rvp;
{
	file_t *fp;
	register int i, error;
	vnode_t *vp;
	off_t offset;
	int flag, fd;
	struct flock bf;
	struct o_flock obf;
	char flags;
	extern struct fileops vnodefops;
	/* XENIX Support */
	register unsigned virt_xout = 0;
	/* End XENIX Support */
	
	if (error = getf(uap->fdes, &fp))
		return error;
	vp = fp->f_vnode;
	flag = fp->f_flag;
	offset = fp->f_offset;

	switch (uap->cmd) {

	case F_DUPFD:
		if ((i = uap->arg) < 0 
		  || i >= u.u_rlimit[RLIMIT_NOFILE].rlim_cur)
			error = EINVAL;
		else if ((error = ufalloc(i, &fd)) == 0) {
			setf(fd, fp);
			flags = getpof(fd);
			flags = flags & ~FCLOSEXEC;
			setpof(fd, flags);
			fp->f_count++;
			rvp->r_val1 = fd;
			break;
		}
		break;

	case F_GETFD:
		rvp->r_val1 = getpof(uap->fdes);
		break;

	case F_SETFD:
		(void) setpof(uap->fdes, (char)uap->arg);
		break;

	case F_GETFL:
		rvp->r_val1 = fp->f_flag+FOPEN;
		break;

	case F_SETFL:
		if ((uap->arg & (FNONBLOCK|FNDELAY)) == (FNONBLOCK|FNDELAY))
			uap->arg &= ~FNDELAY;
 		/*
		 * FRAIOSIG is a new flag added for the raw
		 * disk async io feature. This only applies
 		 * to character special files. But in case
		 */

		if ((error = VOP_SETFL(vp, flag, uap->arg, fp->f_cred)) == 0) {
			uap->arg &= FMASK;
			fp->f_flag &= (FREAD|FWRITE);
			fp->f_flag |= (uap->arg-FOPEN) & ~(FREAD|FWRITE);
		}
		break;

	case F_GETLK:
	case F_O_GETLK:
	case F_SETLK:
	case F_SETLKW:
		/*
		 * Copy in input fields only.
		 */
		if (copyin((caddr_t)uap->arg, (caddr_t)&bf, sizeof obf)) {
			error = EFAULT;
			break;
		}
		/* XENIX Support */
		else {
			virt_xout = VIRTUAL_XOUT;
			/* Map lock type for XENIX binaries */
			if (virt_xout) 
				bf.l_type = XMAP_TO_LTYPE(bf.l_type);	
			/* Indicate to VOP_FRLOCK() that it was called by
			 * fcntl(), instead of from chklock(), etc.
			 * This info is needed to support XENIX behavior
			 * in VOP_FRLOCK().
			 */
			u.u_renv |= UB_FCNTL; 
		}
		/* End XENIX Support */
		if ((uap->cmd == F_SETLK || uap->cmd == F_SETLKW) &&
			bf.l_type != F_UNLCK) {
			setpof(uap->fdes, getpof(uap->fdes)|UF_FDLOCK);
		}
		if (error =
		  VOP_FRLOCK(vp, uap->cmd, &bf, flag, offset, fp->f_cred)) {
			/*
			 * Translation for backward compatibility.
			 */
			/* XENIX Support */
			if ((error == EAGAIN) && !(virt_xout))
			/* End XENIX Support */
				error = EACCES;
			break;
		}
		/* XENIX Support */
		else {
			if (virt_xout) {
				/* Turn on lock enforcement bit */
				if (uap->cmd == F_SETLK || uap->cmd == F_SETLKW)
					vp->v_flag |= VXLOCKED;
				/* Map lock type for XENIX binaries */
				if (uap->cmd != F_SETLKW)
					bf.l_type = XMAP_FROM_LTYPE(bf.l_type);
			}
		}
		/* End XENIX Support */

		/*
		 * If command is GETLK and no lock is found, only
		 * the type field is changed.
		 */
		if ((uap->cmd == F_O_GETLK || uap->cmd == F_GETLK)
		  && bf.l_type == F_UNLCK) {
			if (copyout((caddr_t)&bf.l_type,
			  (caddr_t)&((struct flock *)uap->arg)->l_type,
			  sizeof(bf.l_type)))
				error = EFAULT;
			break;
		}

		if (uap->cmd == F_O_GETLK) {
			/*
			 * Return an SVR3 flock structure to the user.
			 */
			obf.l_type = bf.l_type;
			obf.l_whence = bf.l_whence;
			obf.l_start = bf.l_start;
			obf.l_len = bf.l_len;
			if (bf.l_sysid > SHRT_MAX || bf.l_pid > SHRT_MAX) {
				/*
				 * One or both values for the above fields
				 * is too large to store in an SVR3 flock
				 * structure.
				 */
				error = EOVERFLOW;
				break;
			}
			obf.l_sysid = (short) bf.l_sysid;
			obf.l_pid = (o_pid_t) bf.l_pid;
			if (copyout((caddr_t)&obf, (caddr_t)uap->arg,
			  sizeof obf))
				error = EFAULT;
		} else if (uap->cmd == F_GETLK) {
			/*
			 * Copy out SVR4 flock.
			 */
			int i;

			for (i = 0; i < 4; i++)
				bf.pad[i] = 0;
		    	if (copyout((caddr_t)&bf, (caddr_t)uap->arg, sizeof bf))
			  	error = EFAULT;
		}
		/* XENIX Support */
		if (virt_xout)
			u.u_renv &= ~UB_FCNTL;
		/* End XENIX Support */
		break;

	case F_RGETLK:
	case F_RSETLK:
	case F_RSETLKW:
		/*
		 * EFT only interface, applications cannot use
		 * this interface when _STYPES is defined.
		 * This interface supports an expanded
		 * flock struct--see fcntl.h.
		 */
		if (copyin((caddr_t)uap->arg, (caddr_t)&bf, sizeof bf)) {
			error = EFAULT;
			break;
		}
		/* XENIX Support */
		else {
			virt_xout = VIRTUAL_XOUT;
			/* Map lock type for XENIX binaries */
			if (virt_xout) 
				bf.l_type = XMAP_TO_LTYPE(bf.l_type);	
			/* Indicate to VOP_FRLOCK() that it was called by
			 * fcntl(), instead of from chklock(), etc.
			 * This info is needed to support XENIX behavior
			 * in VOP_FRLOCK().
			 */
			u.u_renv |= UB_FCNTL; 
		}
		/* End XENIX Support */
		if (error =
		  VOP_FRLOCK(vp, uap->cmd, &bf, flag, offset, fp->f_cred)) {
			/*
			 * Translation for backward compatibility.
			 */
		/* XENIX Support */
			if ((error == EAGAIN) && !(virt_xout))
		/* End XENIX Support */
				error = EACCES;
			break;
		}
		/* XENIX Support */
		else {
			if (virt_xout) {
				/* Turn on lock enforcement bit */
				if (uap->cmd == F_RSETLK || uap->cmd == F_RSETLKW)
					vp->v_flag |= VXLOCKED;
				/* Map lock type for XENIX binaries */
				if (uap->cmd != F_RSETLKW)
					bf.l_type = XMAP_FROM_LTYPE(bf.l_type);
			}
		}
		/* End XENIX Support */
		if (uap->cmd == F_RGETLK
		  && copyout((caddr_t)&bf, (caddr_t)uap->arg, sizeof bf))
			  error = EFAULT;
		/* XENIX Support */
		if (virt_xout)
			u.u_renv &= ~UB_FCNTL;
		/* End XENIX Support */
		break;

	case F_CHKFL:
		/*
		 * This is for internal use only, to allow the vnode layer
		 * to validate a flags setting before applying it.  User
		 * programs can't issue it.
		 */
		error = EINVAL;
		break;

	case F_ALLOCSP:
	case F_FREESP:
		if ((flag & FWRITE) == 0)
			error = EBADF;
		else if (vp->v_type != VREG)
			error = EINVAL;
		/*
		 * For compatibility we overlay an SVR3 flock on an SVR4
		 * flock.  This works because the input field offsets 
		 * in "struct flock" were preserved.
		 */
		else if (copyin((caddr_t)uap->arg, (caddr_t)&bf, sizeof obf))
			error = EFAULT;
		else
			error =
			 VOP_SPACE(vp, uap->cmd, &bf, flag, offset, fp->f_cred);
		break;

	default:
		error = EINVAL;
		break;
	}

	return error;
}

/*
 * Duplicate a file descriptor.
 */
struct dupa {
	int	fdes;
};

int
dup(uap, rvp)
	register struct dupa *uap;
	rval_t *rvp;
{
	file_t *fp;
	register int error;
	int fd;

	if (error = getf(uap->fdes, &fp))
		return error;
	if (error = ufalloc(0, &fd))
		return error;
	setf(fd, fp);
	fp->f_count++;
	rvp->r_val1 = fd;
	return 0;
}

/*
 * I/O control.
 */
struct ioctla {
	int fdes;
	int cmd;
	int arg;
};

int
ioctl(uap, rvp)
	register struct ioctla *uap;
	rval_t *rvp;
{
	file_t *fp;
	register int error;
	register vnode_t *vp;
	struct vattr vattr;
	off_t offset;
	int flag;

	if (error = getf(uap->fdes, &fp))
		return error;
	vp = fp->f_vnode;

	if (vp->v_type == VREG || vp->v_type == VDIR) {
		/*
		 * Handle these two ioctls for regular files and
		 * directories.  All others will usually be failed
		 * with ENOTTY by the VFS-dependent code.  System V
		 * always failed all ioctls on regular files, but SunOS
		 * supported these.
		 */
		switch (uap->cmd) {
		case FIONREAD:
			if (error = VOP_GETATTR(vp, &vattr, 0, fp->f_cred))
				return error;
			offset = vattr.va_size - fp->f_offset;
			if (copyout((caddr_t)uap->arg, (caddr_t)&offset,
			  sizeof(offset)))
				return EFAULT;
			return 0;

		case FIONBIO:
			if (copyin((caddr_t)uap->arg, (caddr_t)&flag, 
			  sizeof(int)))
				return EFAULT;
			if (flag)
				fp->f_flag |= FNDELAY;
			else
				fp->f_flag &= ~FNDELAY;
			return 0;

		default:
			break;
		}
	}
	error = VOP_IOCTL(fp->f_vnode, uap->cmd, uap->arg,
	    fp->f_flag, fp->f_cred, &rvp->r_val1);
	if (error == 0) {
		switch (uap->cmd) {
		case FIONBIO:
			if (copyin((caddr_t)uap->arg, (caddr_t)&flag,
			  sizeof(int)))
				return EFAULT;		/* XXX */
			if (flag)
				fp->f_flag |= FNDELAY;
			else
				fp->f_flag &= ~FNDELAY;
			break;

		default:
			break;
	    }
	}
	return error;
}

/*
 * Old stty and gtty.  (Still.)
 */
struct sgttya {
	int	fdes;
	int	arg;
};

int
stty(uap, rvp)
	register struct sgttya *uap;
	rval_t *rvp;
{
	struct ioctla na;

	na.fdes = uap->fdes;
	na.cmd = TIOCSETP;
	na.arg = uap->arg;
	return ioctl(&na, rvp);
}

int
gtty(uap, rvp)
	register struct sgttya *uap;
	rval_t *rvp;
{
	struct ioctla na;

	na.fdes = uap->fdes;
	na.cmd = TIOCGETP;
	na.arg = uap->arg;
	return ioctl(&na, rvp);
}

/*
 * Poll file descriptors for interesting events.
 */
int pollwait;

struct polla {
	struct pollfd *fdp;
	unsigned long nfds;
	long	timo;
};

int
poll(uap, rvp)
	register struct polla *uap;
	rval_t *rvp;
{
	register int i, s;
	register fdcnt = 0;
	struct pollfd *pollp = NULL;
	struct pollfd parray[NFPCHUNK];
	time_t t;
	int lastd;
	int rem;
	int id;
	int psize;
	int dsize;
	file_t *fp;
	struct pollhead *php;
	struct pollhead *savehp = NULL;
	struct polldat *darray;
	struct polldat *curdat;
	int error = 0;
	proc_t *p = u.u_procp;
	extern time_t lbolt;

	if (uap->nfds < 0 || uap->nfds > u.u_rlimit[RLIMIT_NOFILE].rlim_cur)
		return EINVAL;
	t = lbolt;

	/*
	 * Allocate space for the pollfd array and space for the
	 * polldat structures used by polladd().  Then copy in
	 * the pollfd array from user space.
	 */
	if (uap->nfds != 0) {
		psize = uap->nfds * sizeof(struct pollfd);
		if (uap->nfds <= NFPCHUNK)
			pollp = parray;
		else if ((pollp = (struct pollfd *) kmem_alloc(psize, KM_NOSLEEP)) == NULL)
				return EAGAIN;
		dsize = uap->nfds * sizeof(struct polldat);
		if ((darray = (struct polldat *) kmem_zalloc(dsize, KM_NOSLEEP)) == NULL) {
			if (pollp != parray)
				kmem_free((caddr_t)pollp, psize);
			return EAGAIN;
		}
		if (copyin((caddr_t)uap->fdp, (caddr_t)pollp, psize)) {
			error = EFAULT;
			goto pollout;
		}

		/*
		 * Chain the polldat array together.
		 */
		lastd = uap->nfds - 1;
		if (lastd > 0) {
			darray[lastd].pd_chain = darray;
			for (i = 0; i < lastd; i++) {
				darray[i].pd_chain = &darray[i+1];
			}
		} else {
			darray[0].pd_chain = darray;
		}
		curdat = darray;
	}

	/*
	 * Retry scan of fds until an event is found or until the
	 * timeout is reached.
	 */
retry:		

	/*
	 * Polling the fds is a relatively long process.  Set up the
	 * SINPOLL flag so that we can see if something happened
	 * to an fd after we checked it but before we go to sleep.
	 */
	p->p_pollflag = SINPOLL;
	if (savehp) {			/* clean up from last iteration */
		polldel(savehp, --curdat);
		savehp = NULL;
	}
	curdat = darray;
	php = NULL;
	for (i = 0; i < uap->nfds; i++) {
		s = splhi();
		if (pollp[i].fd < 0) 
			pollp[i].revents = 0;
		else if (pollp[i].fd >= u.u_nofiles || getf(pollp[i].fd, &fp))
			pollp[i].revents = POLLNVAL;
		else {
			php = NULL;
			error = VOP_POLL(fp->f_vnode, pollp[i].events, fdcnt,
			    &pollp[i].revents, &php);
			if (error) {
				splx(s);
				goto pollout;
			}
		}
		if (pollp[i].revents)
			fdcnt++;
		else if (fdcnt == 0 && php) {
			polladd(php, pollp[i].events, pollrun,
			  (long)p, curdat++);
			savehp = php;
		}
		splx(s);
	}
	if (fdcnt) 
		goto pollout;

	/*
	 * If you get here, the poll of fds was unsuccessful.
	 * First make sure your timeout hasn't been reached.
	 * If not then sleep and wait until some fd becomes
	 * readable, writeable, or gets an exception.
	 */
	rem = uap->timo < 0 ? 1 : uap->timo - ((lbolt - t)*1000)/HZ;
	if (rem <= 0)
		goto pollout;

	s = splhi();

	/*
	 * If anything has happened on an fd since it was checked, it will
	 * have turned off SINPOLL.  Check this and rescan if so.
	 */
	if (!(p->p_pollflag & SINPOLL)) {
		splx(s);
		goto retry;
	}
	p->p_pollflag &= ~SINPOLL;

	if (uap->timo > 0) {
		/*
		 * Turn rem into milliseconds and round up.
		 */
		rem = ((rem/1000) * HZ) + ((((rem%1000) * HZ) + 999) / 1000);
		p->p_pollflag |= SPOLLTIME;
		id = timeout((void(*)())polltime, (caddr_t)p, rem);
	}

	/*
	 * The sleep will usually be awakened either by this poll's timeout 
	 * (which will have cleared SPOLLTIME), or by the pollwakeup function 
	 * called from either the VFS, the driver, or the stream head.
	 */
	if (sleep((caddr_t)&pollwait, (PZERO+1)|PCATCH)) {
		if (uap->timo > 0)
			untimeout(id);
		splx(s);
		error = EINTR;
		goto pollout;
	}
	splx(s);

	/*
	 * If SPOLLTIME is still set, you were awakened because an event
	 * occurred (data arrived, can write now, or exceptional condition).
	 * If so go back up and poll fds again. Otherwise, you've timed
	 * out so you will fall through and return.
	 */
	if (uap->timo > 0) {
		if (p->p_pollflag & SPOLLTIME) {
			untimeout(id);
			goto retry;
		}
	} else
		goto retry;

pollout:

	/*
	 * Poll cleanup code.
	 */
	p->p_pollflag = 0;
	if (savehp)
		polldel(savehp, --curdat);
	if (error == 0) {
		/*
		 * Copy out the events and return the fdcnt to the user.
		 */
		rvp->r_val1 = fdcnt;
		if (uap->nfds != 0)
			if (copyout((caddr_t)pollp, (caddr_t)uap->fdp, psize))
				error = EFAULT;
	}
	if (uap->nfds != 0) {
		kmem_free((caddr_t)darray, dsize);
		if (pollp != parray)
			kmem_free((caddr_t)pollp, psize);
	}
	return error;
}

/*
 * This function is placed in the callout table to time out a process
 * waiting on poll.  If the poll completes, this function is removed
 * from the table.  Its argument is a flag to the caller indicating a
 * timeout occurred.
 */
void
polltime(p)
	register proc_t *p;
{
	if (p->p_wchan == (caddr_t)&pollwait) {
		setrun(p);
		p->p_pollflag &= ~SPOLLTIME;
	}
}

/*
 * This function is called to inform a process that
 * an event being polled for has occurred.
 */
void
pollrun(p)
	register proc_t *p;
{
	register int s;

	s = splhi();
	if (p->p_wchan == (caddr_t)&pollwait) {
		if (p->p_stat == SSLEEP)
			setrun(p);
		else
			unsleep(p);
	}
	p->p_pollflag &= ~SINPOLL;
	splx(s);
}

int pollcoll = 0;	/* collision counter (temporary) */

/*
 * This function allocates a polldat structure, fills in the given
 * data, and places it on the given pollhead list.  This routine MUST
 * be called at splhi() to avoid races.
 */
void
polladd(php, events, fn, arg, pdp)
	register struct pollhead *php;
	short events;
	void (*fn)();
	long arg;
	register struct polldat *pdp;
{
	pdp->pd_events = events;
	pdp->pd_fn = fn;
	pdp->pd_arg = arg;
	if (php->ph_list) {
		pdp->pd_next = php->ph_list;
		php->ph_list->pd_prev = pdp;
		if (php->ph_events & events)
			pollcoll++;
	} else {
		pdp->pd_next = NULL;
	}
	pdp->pd_prev = (struct polldat *)php;
	pdp->pd_headp = php;
	php->ph_list = pdp;
	php->ph_events |= events;
}

/*
 * This function frees all the polldat structures related by
 * the sibling chain pointer.  These were all the polldats
 * allocated as the result of one poll system call.  Because
 * of race conditions, pdp may not be on php's list.
 */
void
polldel(php, pdp)
	register struct pollhead *php;
	register struct polldat *pdp;
{
	register struct polldat *p;
	register struct polldat *startp;
	register int s;

	s = splhi();
	for (p = php->ph_list; p; p = p->pd_next) {
		if (p == pdp) {
			startp = p;
			do {
				if (p->pd_headp != NULL) {
					if (p->pd_next)
						p->pd_next->pd_prev =
						  p->pd_prev;
					p->pd_prev->pd_next = p->pd_next;

					/*
					 * Recalculate the events on the list.
					 * The list is short - trust me.
					 * Note reuse of pdp here.
					 */
					p->pd_headp->ph_events = 0;
					for (pdp = p->pd_headp->ph_list;
					  pdp; pdp = pdp->pd_next)
						p->pd_headp->ph_events |=
						  pdp->pd_events;
					p->pd_next = NULL;
					p->pd_prev = NULL;
					p->pd_headp = NULL;
				}
				p = p->pd_chain;
			} while (p != startp);
			splx(s);
			return;
		}
	}
	splx(s);
}
