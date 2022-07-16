/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:bfs/bfs_vnops.c	1.3"

#include "sys/types.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"
#include "sys/cred.h"
#include "sys/dirent.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/fcntl.h"
#include "sys/file.h"
#include "sys/flock.h"
#include "sys/inline.h"
#include "sys/open.h"
#include "sys/param.h"
#include "sys/pathname.h"
#include "sys/sysinfo.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/stat.h"
#include "sys/uio.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/kmem.h"
#include "sys/tss.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/debug.h"
#include "vm/seg.h"
#include "vm/page.h"
#include "vm/pvn.h"
#include "sys/mman.h"
#include "sys/fs/bfs.h"
#include "fs/fs_subr.h"
#include "sys/proc.h"		/* needed for PREEMPT() */
#include "sys/disp.h"		/* needed for PREEMPT() */


/*
 * Boot file system operations vector.
 */
STATIC int bfs_open(), bfs_close(), bfs_read(), bfs_write();
STATIC int bfs_ioctl(), bfs_getattr(), bfs_setattr();
STATIC int bfs_access(), bfs_lookup(), bfs_create();
STATIC int bfs_remove(), bfs_rename(), bfs_readdir(), bfs_fsync();
STATIC int bfs_fid(), bfs_seek(), bfs_cmp(), bfs_freespace();

STATIC void bfs_inactive();
STATIC int bfs_link();

struct vnodeops bfsvnodeops = {
	bfs_open,
	bfs_close,
	bfs_read,
	bfs_write,
	bfs_ioctl,
	fs_setfl,
	bfs_getattr,
	bfs_setattr,
	bfs_access,
	bfs_lookup,
	bfs_create,
	bfs_remove,
	bfs_link,
	bfs_rename,
	fs_nosys,	/* mkdir */
	fs_nosys,	/* rmdir */
	bfs_readdir,
	fs_nosys,	/* symlink */
	fs_nosys,	/* readlink */
	bfs_fsync,
	bfs_inactive,
	bfs_fid,
	fs_rwlock,
	fs_rwunlock,
	bfs_seek,
	bfs_cmp,
	fs_frlock,
	bfs_freespace,
	fs_nosys,	/* realvp */
	fs_nosys,	/* getpage */
	fs_nosys,	/* putpage */
	fs_nosys,	/* mmap */
	fs_nosys,	/* addmap */
	fs_nosys,	/* delmap */
	fs_poll,
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

/* ARGSUSED */
STATIC int
bfs_open(vpp, flag, cr)
	struct vnode **vpp;
	int flag;
	struct cred *cr;
{
	register struct bsuper *bs;
	register struct vnode *vp;

	vp = *vpp;
	bs = (struct bsuper *)vp->v_vfsp->vfs_data;

	/*
	 * If the filesystem is locked, do not allow an open.
	 */
	if (bs->bsup_fslocked)
		return EBUSY;

	/*
	 * In BFS, only one file can be open for writing at a time.
	 */
	if (flag & FWRITE) {
		if (bs->bsup_writelock != NULL && bs->bsup_writelock != vp)
			return EBUSY;

		bs->bsup_writelock = vp;
	}
	return 0;
}

/* ARGSUSED */
STATIC int
bfs_close(vp, flag, count, offset, cr)
	register struct vnode *vp;
	int flag;
	int count;
	off_t offset;
	struct cred *cr;
{
	register struct bsuper *bs;

	bs = (struct bsuper *)vp->v_vfsp->vfs_data;

	/*
	 * If this file was open for writing and this is the last close, free
	 * up writing in the filesystem.
	 */
	if (count == 1 && (flag & FWRITE))
		bs->bsup_writelock = NULL;
	
	return 0;
}

/* ARGSUSED */
STATIC int
bfs_ioctl(vp, cmd, arg, flag, cr, rvalp)
	register struct vnode *vp;
	int cmd;
	int arg;
	int flag;
	struct cred *cr;
	int *rvalp;
{
	return ENOTTY;
}

/* ARGSUSED */
STATIC int
bfs_fsync(vp, cr)
	struct vnode *vp;
	struct cred *cr;
{
	return 0;
}

/*
 * This vnode has no more references.  Remove it from the linked list
 * and free up the space.
 */

/* ARGSUSED */
STATIC void
bfs_inactive(vp, cr)
	register struct vnode *vp;
	struct cred *cr;
{
	register struct bfs_core_vnode *cvp,*temp;
	register struct bsuper *bs = (struct bsuper *)vp->v_vfsp->vfs_data;

	if (vp->v_type != VDIR) {			/* Vnode is root */
		cvp = bs->bsup_incore_vlist;
		if (cvp->core_vnode->v_data == vp->v_data) {
			bs->bsup_incore_vlist = cvp->core_next;
			cvp->core_next = NULL;
			kmem_free(cvp,sizeof(struct bfs_core_vnode));
		} else {
			while (cvp->core_next->core_vnode->v_data != vp->v_data)
				cvp = cvp->core_next;
			temp = cvp->core_next;
			cvp->core_next = temp->core_next;
			temp->core_next = NULL;
			kmem_free(temp,sizeof(struct bfs_core_vnode));
		}
		kmem_free(vp, sizeof(struct vnode));
	}
}

/*
 * Convert a vnode to a BFS file id.  In BFS a fid is just the v_data info.
 */
STATIC int
bfs_fid(vp, fidpp)
	struct vnode *vp;
	struct fid **fidpp;
{
	register struct bfs_fid_overlay *overlay;

	overlay = (struct bfs_fid_overlay *)
	  kmem_zalloc(sizeof(struct bfs_fid_overlay), KM_SLEEP);
	overlay->o_len = sizeof(struct bfs_fid_overlay) - sizeof(ushort);
	overlay->o_offset = (long)vp->v_data;

	*fidpp = (struct fid *)overlay;
	return 0;
}

/* ARGSUSED */
STATIC int
bfs_getattr(vp, vap, flags, cr)
	register struct vnode *vp;
	register struct vattr *vap;
	int flags;
	struct cred *cr;
{
	register struct bfs_dirent *dir;
	register struct bsuper *bs = (struct bsuper *)vp->v_vfsp->vfs_data;

	dir = (struct bfs_dirent *)
	  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);

	BFS_IOBEGIN(bs);
	CHECK_LOCK(bs);

	BFS_INOLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	BFS_GETINODE(bs->bsup_devnode, vp->v_data, dir, cr);

	BFS_IOEND(bs);
	/*
	 * We get most of the attrs from the dirent.
	 */
	vap->va_type = dir->d_fattr.va_type;
	vap->va_mode = (mode_t)dir->d_fattr.va_mode;
	vap->va_uid = dir->d_fattr.va_uid;
	vap->va_gid = dir->d_fattr.va_gid;
	vap->va_nlink = (nlink_t)dir->d_fattr.va_nlink;
	vap->va_atime.tv_sec = dir->d_fattr.va_atime;
	vap->va_atime.tv_nsec = 0;
	vap->va_mtime.tv_sec = dir->d_fattr.va_mtime;
	vap->va_mtime.tv_nsec = 0;
	vap->va_ctime.tv_sec = dir->d_fattr.va_ctime;
	vap->va_ctime.tv_nsec = 0;
	vap->va_blksize = 512;
	vap->va_vcode = 0;

	/*
	 * If a file is not empty, we figure out va_size from a combination of
	 * the beginning block and eoffset.
	 */
	if (dir->d_sblock)
		vap->va_size = (dir->d_eoffset - (dir->d_sblock*BFS_BSIZE)) +1;
	else
		vap->va_size = 0;
	vap->va_nblocks = btod(vap->va_size);
	vap->va_nodeid =  dir->d_ino;
	vap->va_fsid = vp->v_vfsp->vfs_dev;
	vap->va_rdev = 0;
	kmem_free((caddr_t)dir, sizeof(struct bfs_dirent));
	BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	return 0;
}

STATIC int
bfs_setattr(vp, vap, flags, cr)
	register struct vnode *vp;
	register struct vattr *vap;
	int flags;
	register struct cred *cr;
{
	int error = 0;
	struct bfs_dirent *dir;
	register struct bfsvattr *battrs;
	register struct bsuper *bs = (struct bsuper *)vp->v_vfsp->vfs_data;
	register long int mask = vap->va_mask;

	dir = (struct bfs_dirent *)
	  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);
	battrs = &dir->d_fattr;

	BFS_IOBEGIN(bs);

	CHECK_LOCK(bs);

	BFS_INOLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	BFS_GETINODE(BFS_DEVNODE(vp->v_vfsp), (off_t)vp->v_data, dir, cr);

	/*
	 * Most of what follows taken from S5 (including gotos).
	 */

	/*
	 * Cannot set these attributes.
	 */
	if (mask & AT_NOSET) {
		BFS_IOEND(bs);
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
		kmem_free(dir, sizeof(struct bfs_dirent));
		return EINVAL;
	}

	/*
	 * Change file access modes.  Must be owner or super-user.
	 */
	if (mask & AT_MODE) {
		if (cr->cr_uid != battrs->va_uid && !suser(cr)) {
			error = EPERM;
			goto out;
		}

		battrs->va_mode = (o_mode_t)vap->va_mode;

		if (cr->cr_uid != 0) {
			/*
			 * A non-privileged user can set the sticky bit
			 * on a directory.
			 */
			if (vp->v_type != VDIR)
				battrs->va_mode &= ~VSVTX;
			if (!groupmember(battrs->va_gid, cr))
				battrs->va_mode &= ~VSGID;
		}
	}
	/*
	 * Change file ownership; must be the owner of the file
	 * or the super-user.  If the system was configured with
	 * the "rstchown" option, the owner is not permitted to
	 * give away the file, and can change the group id only
	 * to a group of which he or she is a member.
	 */
	if (mask & (AT_UID|AT_GID)) {
		int checksu = 0;

		if (rstchown) {
		       if (((mask & AT_UID) && vap->va_uid != battrs->va_uid) ||
			   ((mask & AT_GID) && !groupmember(vap->va_gid, cr)))
				checksu = 1;
		} else if (cr->cr_uid != battrs->va_uid)
			checksu = 1;

		if (checksu && !suser(cr)) {
			error = EPERM;
			goto out;
		}

		if (cr->cr_uid != 0)
			battrs->va_mode &= ~(VSUID|VSGID);
		if (mask & AT_UID)
			battrs->va_uid = vap->va_uid;
		if (mask & AT_GID)
			battrs->va_gid = vap->va_gid;
	}

	/*
	 * Truncate file.  Must have write permission and not be a directory.
	 */
	if (mask & AT_SIZE) {
		if (vap->va_size != 0) {
			error = EINVAL;
			goto out;
		}

		if (vp->v_type == VDIR) {
			error = EISDIR;
			goto out;
		}

		if (error = bfs_iaccess(vp, VWRITE, cr))
			goto out;

		bs->bsup_freeblocks += (dir->d_eblock - dir->d_sblock) +1;

		if (bs->bsup_lastfile == (off_t)vp->v_data) {
			off_t l;
			off_t lastoff;
			daddr_t lastblock;
			register struct bfs_dirent *tdir;

			tdir = (struct bfs_dirent *)
			  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);

			lastoff = 0;
			lastblock = 0;

			for (l = BFS_DIRSTART; l < bs->bsup_start;
			     l += sizeof(struct bfs_dirent)) {
				BFS_GETINODE(bs->bsup_devnode, l, tdir, cr);
				if (l != (off_t)vp->v_data &&
				    tdir->d_eblock > lastblock &&
				    tdir->d_ino != 0) {
					lastblock = tdir->d_eblock;
					lastoff = l;
				}
			}
			bs->bsup_lastfile = lastoff;
			kmem_free(tdir, sizeof(struct bfs_dirent));
		}
		dir->d_sblock = 0;
		dir->d_eblock = 0;
		dir->d_eoffset = 0;
	}

	/*
	 * Change file access or modified times.
	 */
	if (mask & (AT_ATIME|AT_MTIME)) {
		if (cr->cr_uid != battrs->va_uid && cr->cr_uid != 0) {
			if (flags & ATTR_UTIME)
				error = EPERM;
			else
				error = bfs_iaccess(vp, VWRITE, cr);
			if (error)
				goto out;
		}
		if (mask & AT_ATIME)
			battrs->va_atime = vap->va_atime.tv_sec;
		if (mask & AT_MTIME) {
			battrs->va_mtime = vap->va_mtime.tv_sec;
			battrs->va_ctime = hrestime.tv_sec;
		}
	}
out:
	BFS_PUTINODE(BFS_DEVNODE(vp->v_vfsp), vp->v_data, dir, cr);
	BFS_IOEND(bs);
	BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	kmem_free((caddr_t)dir, sizeof(struct bfs_dirent));
	return error;
}

/* ARGSUSED */
STATIC int
bfs_access(vp, mode, flags, cr)
	struct vnode *vp;
	int mode;
	int flags;
	struct cred *cr;
{
	register struct bsuper *bs = (struct bsuper *)vp->v_vfsp->vfs_data;
	int error;

	BFS_INOLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	error = bfs_iaccess(vp, mode, cr);
	BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	return error;
}

/* ARGSUSED */
STATIC int
bfs_seek(vp, ooff, noffp)
	struct vnode *vp;
	off_t ooff;
	off_t *noffp;
{
	return *noffp < 0 ? EINVAL : 0;
}


STATIC int
bfs_cmp(vp1, vp2)
	struct vnode *vp1, *vp2;
{
	return vp1 == vp2;
}


#define RECBUFSIZE	1048

STATIC int
bfs_readdir(vp, uiop, cr, eofp)
	struct vnode *vp;
	register struct uio *uiop;
	struct cred *cr;
	int *eofp;
{
	register struct bfs_dirent *dir;
	register struct bfs_ldirs *ld;
	off_t  diroff = uiop->uio_offset;
	register struct bsuper *bs = (struct bsuper *)vp->v_vfsp->vfs_data;
	register struct dirent *drp;
	char *drent;
	char *buf;
	off_t offset;
	off_t i;
	int error = 0;
	int buflen = 0;
	int len, chunksize;
	int reclen, namesz;
	int indrent = 0;
	int fixsz;

	/*
	 * Check for a valid offset into the "directory".
	 */
	if (diroff % sizeof(struct bfs_ldirs) != 0)
		return ENOENT;

	if (diroff < 0)
		return EINVAL;

	BFS_IOBEGIN(bs);
	CHECK_LOCK(bs);

	dir = (struct bfs_dirent *)
	  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);
	BFS_INOLOCK(bs, BFSROOTINO);
	/*
	 * Get the ROOT inode.
	 */
	error = BFS_GETINODE(bs->bsup_devnode, BFS_INO2OFF(BFSROOTINO), dir,cr);
	if (error) {
		kmem_free(dir, sizeof(struct bfs_dirent));
		BFS_INOUNLOCK(bs, BFSROOTINO);
		BFS_IOEND(bs);
		return(error);
	}
	
	len = dir->d_eoffset - (dir->d_sblock  * BFS_BSIZE) + 1 - diroff;
	if (len < 0) {
		kmem_free(dir, sizeof(struct bfs_dirent));
		BFS_INOUNLOCK(bs, BFSROOTINO);
		BFS_IOEND(bs);
		return ENOENT;
	}
	chunksize = MIN(len, DIRBUFSIZE);
	buf = kmem_alloc(chunksize, KM_SLEEP);

	drent = kmem_alloc(RECBUFSIZE, KM_SLEEP);
	drp = (struct dirent *)drent;
	fixsz = (char *)drp->d_name - (char *)drp;

	for (offset = diroff + (dir->d_sblock * BFS_BSIZE);
	     uiop->uio_resid > 0 && len > 0; len -= buflen, offset += buflen) {	

		buflen = MIN(chunksize, len);
		/*
		 * Get the list of entries stored in the ROOT directory
		 */
		error = BFS_GETDIRLIST(bs->bsup_devnode, offset, buf,buflen,cr);
		if (error) {
			kmem_free(dir, sizeof(struct bfs_dirent));
			kmem_free(buf, chunksize);
			kmem_free(drent, RECBUFSIZE);
			BFS_INOUNLOCK(bs, BFSROOTINO);
			BFS_IOEND(bs);
			return(error);
		}

		for (i = 0; i < buflen && uiop->uio_resid >= indrent;
		     i += sizeof(struct bfs_ldirs)) {
			ld = (struct bfs_ldirs *) (buf + i);
			if (ld->l_ino == 0 )
				continue;
			namesz = (ld->l_name[BFS_MAXFNLEN -1] == '\0') ?
				 strlen(ld->l_name) : BFS_MAXFNLEN;
			reclen = (fixsz + namesz + 1 + (NBPW -1)) & ~(NBPW -1);

			if (RECBUFSIZE - indrent < reclen || 
			    uiop->uio_resid < indrent + reclen ) {
				uiomove(drent, indrent, UIO_READ, uiop);
				if (uiop->uio_resid < reclen) {
					offset += i;
					indrent = -1;
					break;
				}
				drp = (struct dirent *) drent;
				indrent = 0;
			}
			if (uiop->uio_resid >= indrent + reclen) {
				drp->d_ino = ld->l_ino;
				drp->d_off = offset + i +
				   sizeof(struct bfs_ldirs) -
				   (dir->d_sblock * BFS_BSIZE);
				drp->d_reclen = (short)reclen;
				strncpy(drp->d_name, ld->l_name,BFS_MAXFNLEN);
				drp->d_name[namesz] = '\0';
				drp = (struct dirent *) (((char *)drp) +reclen);
				indrent += reclen;
			}
		}
		if (indrent == -1)
			break;
	}

	if (indrent > 0 && uiop->uio_resid >= indrent)
		uiomove(drent, indrent, UIO_READ, uiop);

	/*
	 * The offset must be changed manually to reflect the filesystem
	 * DEPENDENT directory size.
	 */
	if (error == 0) {
		uiop->uio_offset = offset - (dir->d_sblock * BFS_BSIZE);
		if (eofp)
			*eofp = (offset >= dir->d_eoffset);
	}

	kmem_free(dir, sizeof(struct bfs_dirent));
	kmem_free(buf, chunksize);
	kmem_free(drent, RECBUFSIZE);
	BFS_INOUNLOCK(bs, BFSROOTINO);
	BFS_IOEND(bs);
	return(error);
}

/* ARGSUSED */
STATIC int
bfs_read(vp, uiop, ioflag, cr)
	register struct vnode *vp;
	register struct uio *uiop;
	int ioflag;
	struct cred *cr;
{
	register struct bfs_dirent *dir;
	register struct bsuper *bs = (struct bsuper *)vp->v_vfsp->vfs_data;
	off_t offset;
	char *buf;
	int chunksize;
	int error, buflen;

	if (uiop->uio_resid == 0)
		return 0;

	if (uiop->uio_offset < 0)
		return EINVAL;

	dir = (struct bfs_dirent *)
	  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);

	BFS_IOBEGIN(bs);
	CHECK_LOCK(bs);

	BFS_INOLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	error = BFS_GETINODE(bs->bsup_devnode, vp->v_data, dir, cr);

	if (dir->d_sblock == 0) {		/* This file is empty */
		kmem_free(dir, sizeof(struct bfs_dirent));
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
		BFS_IOEND(bs);
		return 0;
	}

	offset = uiop->uio_offset + (dir->d_sblock * BFS_BSIZE);

	chunksize = MIN(MAXBSIZE, uiop->uio_resid);
	buf = kmem_alloc(chunksize, KM_SLEEP);
	while (uiop->uio_resid) {
		buflen =  MIN(chunksize, uiop->uio_resid);

		/*
		 * Figure out how many bytes we are reading.
		 */
		if (offset + buflen > dir->d_eoffset)
			buflen = dir->d_eoffset - offset + 1;

		if ( (offset <= dir->d_eoffset) && (buflen > 0) ) {
			error = vn_rdwr(UIO_READ, bs->bsup_devnode, buf, buflen,
			  offset, UIO_SYSSPACE, 0,0,cr, (int *)0);
			if (error) {
				BFS_IOEND(bs);
				BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
				kmem_free(buf, chunksize);
				kmem_free(dir, sizeof(struct bfs_dirent));
				return error;
			}

			/* 
			 * uiomove may take a while, so PREEMPT();
			 */
			PREEMPT();
			error = uiomove(buf, buflen, UIO_READ, uiop);
			if (error) {
				BFS_IOEND(bs);
				BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
				kmem_free(buf, chunksize);
				kmem_free(dir, sizeof(struct bfs_dirent));
				return error;
			}
			offset = offset + buflen;
		}
		else
			break;
	}

	if (!error) {
		dir->d_fattr.va_atime = hrestime.tv_sec;
		error = BFS_PUTINODE(bs->bsup_devnode, vp->v_data, dir, cr);
	}
	BFS_IOEND(bs);
	BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	kmem_free(buf, chunksize);
	kmem_free(dir, sizeof(struct bfs_dirent));
	return 0;
}

/* ARGSUSED */
STATIC int
bfs_lookup(dvp, nm, vpp, pnp, flags, rdir, cr)
	struct vnode *dvp;
	char *nm;
	struct vnode **vpp;
	struct cred *cr;
	struct pathname *pnp;
	struct vnode *rdir;
	int flags;
{
	register struct bsuper *bs = (struct bsuper *)dvp->v_vfsp->vfs_data;
	off_t offset;
	struct vnode *vp;
	int error;

	if (dvp->v_type != VDIR)	/* We can only read root */
		return ENOTDIR;

	if (error = bfs_iaccess(dvp, VEXEC, cr))
		return error;

	/*
	 * The null name means the current directory.
	 */
	if (*nm == '\0') {
		*vpp = dvp;
		VN_HOLD(dvp);
		return 0;
	}

	/*
	 * Handle  '..' even though it doesn't exist.
	 */
	if (nm[0] == '.' && nm[1] == '.' && nm[2] == '\0') {
		*vpp = bs->bsup_root;
		VN_HOLD(*vpp);
		return 0;
	}

	/*
	 * Search through the flat directory for the file.
	 */
	if ((offset = bfs_searchdir(dvp->v_vfsp, nm, cr)) <= 0)
		return ENOENT;

	/*
	 * Now create a new vnode (or get an old one from the linked list).
	 */
	bfs_fillvnode(&vp, (caddr_t)offset, dvp->v_vfsp);

	*vpp = vp;
	return 0;
}

STATIC int
bfs_remove(vp, nm, cr)
	struct vnode *vp;
	char *nm;
	struct cred *cr;
{
	struct vnode *rvp;
	register struct bsuper *bs = (struct bsuper *)vp->v_vfsp->vfs_data;
	register struct bfs_dirent *dir;
	register struct bfs_dirent *ldir;
	off_t i;
	off_t lastoff;
	int error = 0;
	daddr_t lastblock = 0;

	/*
	 * Since BFS is a flat file system, only need to check permissions
	 * of the ROOT directory.
	 */
	if (error = bfs_iaccess(vp, VEXEC|VWRITE, cr))
		return error;

	/*
	 * Get a new vnode for the file in question.
	 */
	error = bfs_lookup(vp, nm, &rvp, (struct pathname *)0, 0, NULLVP, cr);
	if (error)
		return error;

	if (rvp->v_type == VDIR) {
		VN_RELE(rvp);
		return EISDIR;
	}

	dir = (struct bfs_dirent *)
	  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);

	BFS_IOBEGIN(bs);
	CHECK_LOCK(bs);

	BFS_INOLOCK(bs, BFS_OFF2INO((off_t)rvp->v_data));
	/*
	 * If this is the file farthest into the disk, we must flag a
	 * different file as the last.
	 */
	if (error = BFS_GETINODE(bs->bsup_devnode, rvp->v_data, dir, cr)) {
		BFS_IOEND(bs);
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)rvp->v_data));
		kmem_free(dir, sizeof(struct bfs_dirent));
		VN_RELE(rvp);
		return error;
	}
	/*
	 * Zero the ino field in the root directory.
	 */
	error = bfs_rmdirent(vp->v_vfsp, nm, cr);
	if (error) { 
		BFS_IOEND(bs);
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)rvp->v_data));
		kmem_free(dir, sizeof(struct bfs_dirent));
		VN_RELE(rvp);
		return error;
	}
	/*
	 * Zero the ino entry if the link count is 1.
	 */
	if (dir->d_fattr.va_nlink == 1)
		dir->d_ino = 0;
	dir->d_fattr.va_nlink -= 1;
	dir->d_fattr.va_ctime = hrestime.tv_sec;

	error = BFS_PUTINODE(bs->bsup_devnode, rvp->v_data, dir, cr);
	if (error) { 
		BFS_IOEND(bs);
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)rvp->v_data));
		kmem_free(dir, sizeof(struct bfs_dirent));
		VN_RELE(rvp);
		return error;
	}

	/*
	 * If link count is >= 1, we are done!
	 */
	if (dir->d_fattr.va_nlink >= 1) {
		BFS_IOEND(bs);
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)rvp->v_data));
		kmem_free(dir, sizeof(struct bfs_dirent));
		VN_RELE(rvp);
		return 0;
	}
	/*
	 * If link count dropped to 0, must go on freeing up the space
	 * and the inode.
	 */
	if (dir->d_eblock != 0)
		bs->bsup_freeblocks += (dir->d_eblock +1) - dir->d_sblock;

	bs->bsup_freedrents++;

	if (bs->bsup_lastfile == (off_t)rvp->v_data) {
		lastoff = 0;
		for (i = BFS_DIRSTART; i < bs->bsup_start;
		     i += sizeof(struct bfs_dirent)) {
			BFS_GETINODE(bs->bsup_devnode, i, dir, cr);
			if (i != (off_t) rvp->v_data &&
			    dir->d_eblock > lastblock &&
			    dir->d_ino != 0) {
				lastblock = dir->d_eblock;
				lastoff = i;
			}
		}

		bs->bsup_lastfile = lastoff;
		BFS_IOEND(bs);
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)rvp->v_data));
		kmem_free(dir, sizeof(struct bfs_dirent));
		VN_RELE(rvp);
		return error;
	} else
		bs->bsup_compacted = BFS_NO;

	BFS_IOEND(bs);

	/*
	 * If the deleted file is "big" and is followed by ONLY ONE file
	 * which happens to be "small", compact the filesystem as it will
	 * cost very little and will save a huge compaction later on.
	 */
	if (dir->d_eblock - dir->d_sblock > BIGFILE) {
		ldir = (struct bfs_dirent *)
		  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);

		BFS_GETINODE(bs->bsup_devnode, bs->bsup_lastfile, ldir, cr);

		if (ldir->d_sblock == dir->d_eblock + 1 &&
		    ldir->d_eblock - ldir->d_sblock < SMALLFILE)
			bfs_compact(bs, cr);

		kmem_free(ldir, sizeof(struct bfs_dirent));
	}

	BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)rvp->v_data));
	kmem_free(dir, sizeof(struct bfs_dirent));
	VN_RELE(rvp);
	return error;
}

STATIC int
bfs_rename(sdvp, snm, tdvp, tnm, cr)
	struct vnode *sdvp;		/* old (source) parent vnode */
	char *snm;			/* old (source) entry name */
	struct vnode *tdvp;		/* new (target) parent vnode */
	char *tnm;			/* new (target) entry name */
	struct cred *cr;
{
	register int error;
	struct vnode *vp;

	/*
	 * Make sure we can delete the source entry.
	 */
	if (sdvp != tdvp)
		return EINVAL;

	if (error = bfs_iaccess(sdvp, VWRITE, cr))
		return error;

	/*
	 * Check for renaming '.' or '..'.
	 */
	if (strcmp(snm, ".") == 0 || strcmp(snm, "..") == 0)
		return EINVAL;

	/*
	 * Look up vnode of file we're supposed to rename.
	 */
	if (error = bfs_lookup(sdvp, snm, &vp, (struct pathname *)0, 0,
	    NULLVP, cr))
		return error;

	if (bfs_searchdir(sdvp->v_vfsp, tnm, cr) != 0
	  && (error = bfs_remove(sdvp, tnm, cr))) {
		VN_RELE(vp);
		return error;
	}
	error = bfs_rendirent(sdvp->v_vfsp, snm, tnm, cr);

	VN_RELE(vp);
	return error;
}

STATIC int
bfs_link(tdvp, svp, tnm, cr)
	struct vnode *tdvp;
	struct vnode *svp;
	char *tnm;
	struct cred *cr;
{
	register struct bsuper *bs = (struct bsuper *)svp->v_vfsp->vfs_data;
	register struct bfs_dirent *dir;
	ushort sino;
	int error = 0;

	if (tdvp->v_type != VDIR) 
		return ENOTDIR;

	if (svp->v_type != VREG) 
		return ENOSYS;

	/*
	 * Determine source inode
	 */
	sino = BFS_OFF2INO((off_t)svp->v_data);

	error = bfs_addirent(tdvp->v_vfsp, tnm, sino, cr);
	if (error)
		return error;

	/*
	 * Update the source inode 
	 */
	BFS_IOBEGIN(bs);
	CHECK_LOCK(bs);
	dir = (struct bfs_dirent *)
	  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);
	BFS_INOLOCK(bs, BFS_OFF2INO((off_t)svp->v_data));
	error = BFS_GETINODE(bs->bsup_devnode, svp->v_data, dir, cr);
	if (!error) {
		dir->d_fattr.va_nlink++;
		dir->d_fattr.va_ctime = hrestime.tv_sec;
		error = BFS_PUTINODE(bs->bsup_devnode, svp->v_data, dir, cr);
	}

	BFS_IOEND(bs);
	BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)svp->v_data));
	kmem_free(dir, sizeof(struct bfs_dirent));
	return error;
}

/*
 * Either creat() or open() with O_CREAT.  In BFS this simply makes an
 * empty dirent which takes up no space outside of the directory.
 */
STATIC int
bfs_create(dvp, fname, vap, excl, mode, vpp, cr)
	struct vnode *dvp;
	char *fname;
	struct vattr *vap;
	enum vcexcl excl;
	int mode;
	struct vnode **vpp;
	struct cred *cr;
{
	register struct bfs_dirent *dir;
	daddr_t block_no = 0;
	off_t dir_off = 0;
	register struct bsuper *bs = (struct bsuper *)dvp->v_vfsp->vfs_data;
	register int error;
	struct vnode *vp;
	struct vnode *avp;

	/*
	 * Can only create into root.
	 */
	if (dvp->v_type != VDIR)
		return ENOTDIR;

	if (vap->va_type != VREG)
		return ENOSYS;

	dir_off = bfs_searchdir(dvp->v_vfsp, fname, cr);

	/*
	 * It would really not be a problem to create an empty dirent while
	 * another file is being written, but if we allow it, people will
	 * end up with a lot of empty files when they try to copy files
	 * to the fs.
	 */
	if (bs->bsup_writelock != NULL &&
	    dir_off != (off_t)bs->bsup_writelock->v_data)
		return EBUSY;

	/*
	 * File already exists.  Check access and truncate if requested.
	 */
	if (dir_off) {
		if (excl == NONEXCL) {
			bfs_fillvnode(&avp, (caddr_t)dir_off, dvp->v_vfsp);

			/*
			 * If user has access, truncate if requested
			 * and return vp.
			 */
			if (mode &&
			    (error = bfs_iaccess(avp, mode, cr)) == 0) {
				if ((vap->va_mask & AT_SIZE) &&
				    vap->va_size == 0)
					bfs_truncate(bs, dir_off, 0, cr);
				*vpp = avp;
				return 0;
			} else {
				VN_RELE(avp);
				return error;
			}
		} else
			return EEXIST;
	}
			
	/*
	 * No inodes left.
	 */
	if (bs->bsup_freedrents <= 0)
		return ENFILE;

	if (error = bfs_iaccess(dvp, VWRITE, cr))
		return error;

	dir = (struct bfs_dirent *)
	  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);

	BFS_IOBEGIN(bs);
	CHECK_LOCK(bs);

	/*
	 * Get next dirent slot and the last used block number plus one.
	 */
	bfs_nextfile(bs, &dir_off, &block_no, cr);
	BFS_INOLOCK(bs, BFS_OFF2INO(dir_off));

	/*
	 * Get most attributes from argument list.
	 */
	dir->d_fattr.va_type = vap->va_type;
	dir->d_fattr.va_mode = (o_mode_t)vap->va_mode;
	dir->d_fattr.va_uid = cr->cr_uid;
	dir->d_fattr.va_gid = cr->cr_gid;
	dir->d_fattr.va_mtime = hrestime.tv_sec;
	dir->d_fattr.va_ctime = hrestime.tv_sec;
	dir->d_fattr.va_atime = hrestime.tv_sec;
	dir->d_fattr.va_nlink = 1;
	dir->d_sblock = 0;
	dir->d_eblock = 0;
	dir->d_eoffset = 0;
	dir->d_ino = BFS_OFF2INO(dir_off);

	/*
	 * Put the new dirent in the inode slot.
	 */
	error = BFS_PUTINODE(bs->bsup_devnode, dir_off, dir, cr);
	if (error) {
		BFS_INOUNLOCK(bs, BFS_OFF2INO(dir_off));
		BFS_IOEND(bs);
		kmem_free(dir, sizeof(struct bfs_dirent));
		return error;
	}

	BFS_IOEND(bs);

	/*
	 * Add the entry to the root directory
	 */
	error = bfs_addirent(dvp->v_vfsp, fname, dir->d_ino, cr);
	if (error) {
		BFS_INOUNLOCK(bs, BFS_OFF2INO(dir_off));
		kmem_free(dir, sizeof(struct bfs_dirent));
		return error;
	}

	/*
	 * Create a new vnode to return.
	 */
	bfs_fillvnode(&vp, (caddr_t)dir_off, dvp->v_vfsp);
	*vpp = vp;

	bs->bsup_freedrents--;

	BFS_INOUNLOCK(bs, BFS_OFF2INO(dir_off));
	kmem_free(dir, sizeof(struct bfs_dirent));
	return 0;
}


/* ARGSUSED */
STATIC int
bfs_freespace(vp, cmd, bfp, flag, offset, cr)
	struct vnode *vp;
	int cmd;
	struct flock *bfp;
	int flag;
	off_t offset;
	struct cred *cr;
{
	register struct bsuper *bs = (struct bsuper *)vp->v_vfsp->vfs_data;

	if (cmd != F_FREESP)
		return EINVAL;
	if (convoff(vp, bfp, 0, offset) == 0) {
		if (bfp->l_len != 0)
			return EINVAL;
		return bfs_truncate(bs, (off_t)vp->v_data, bfp->l_start, cr);
	}
	else
		return EINVAL;
}

STATIC int
bfs_write(vp, uiop, ioflag, cr)
	struct vnode *vp;
	struct uio *uiop;
	int ioflag;
	struct cred *cr;
{
	register struct bsuper *bs = (struct bsuper *)vp->v_vfsp->vfs_data;
	register struct bfs_dirent *dir;
	register int buflen;
	register int new;
	char *buf;
	daddr_t oeblock;
	int error;
	int chunksize;
	daddr_t newblock;
	off_t newdir;
	off_t offset;
	off_t bfs_lastfile;
	off_t endfile = 0;
	
	if (uiop->uio_resid == 0)
		return 0;

	BFS_IOBEGIN(bs);
	CHECK_LOCK(bs);

	if (uiop->uio_offset < 0) {
		BFS_IOEND(bs);
		return EINVAL;
	}

	dir = (struct bfs_dirent *)
	  kmem_alloc(sizeof(struct bfs_dirent), KM_SLEEP);

	BFS_INOLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	BFS_GETINODE(bs->bsup_devnode, vp->v_data, dir, cr);

	if (dir->d_ino == 0) {
		kmem_free(dir, sizeof(struct bfs_dirent));
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
		BFS_IOEND(bs);
		return EINVAL;
	}

	bfs_lastfile = bs->bsup_lastfile;

	/*
	 * If this is an empty file, we must find the last block.
	 */
	if (!dir->d_sblock) {
		bfs_nextfile(bs, &newdir, &newblock, cr);
		while (newblock*BFS_BSIZE > bs->bsup_end) {
			if (!bs->bsup_compacted) {
				bfs_compact(bs, cr);
				bfs_nextfile(bs, &newdir, &newblock, cr);
			} else {
				kmem_free(dir, sizeof(struct bfs_dirent));
				BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
				BFS_IOEND(bs);
				return ENOSPC;
			}
		}

		/*
		 * This may become the last file.
		 */
		bfs_lastfile = (off_t)vp->v_data;
		dir->d_sblock = newblock;
		dir->d_eblock = newblock+(uiop->uio_offset / BFS_BSIZE);
		dir->d_eoffset =(dir->d_sblock*BFS_BSIZE) + uiop->uio_offset -1;
		oeblock = 0;
		endfile = (dir->d_sblock*BFS_BSIZE) - 1;
	} else {
		oeblock = dir->d_eblock;
		endfile = dir->d_eoffset;
	}
	
	if (ioflag & IO_APPEND)
		offset = dir->d_eoffset + 1;
	else
		offset = (dir->d_sblock*BFS_BSIZE) + uiop->uio_offset;

	/*
	 * If this write will bring the pointer past the end of
	 * the file, we must move it to the end of all the data.
	 */
	if (offset + uiop->uio_resid > dir->d_eoffset &&
	    bfs_lastfile != (off_t)vp->v_data) {
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
		error = bfs_filetoend(bs, dir, (off_t)vp->v_data, cr);
		kmem_free(dir, sizeof(struct bfs_dirent));
		if (error) {
			BFS_IOEND(bs);
			return error;
		}

		BFS_IOEND(bs);

		/*
		 * Call recursively after moving.
		 */
		return bfs_write(vp, uiop, ioflag, cr);
	}

	/*
	 * If this write will bring the pointer past the end
	 * of the data, compact the filesystem.
	 */
	if (offset + uiop->uio_resid > bs->bsup_end) {
		kmem_free(dir, sizeof(struct bfs_dirent));
		BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
		if (!bs->bsup_compacted) {
			bfs_compact(bs, cr);
			BFS_IOEND(bs);
		} else {
			BFS_IOEND(bs);
			return ENOSPC;		/* Already compact */
		}

		/*
		 * Call recursively after compaction.
		 */
		return bfs_write(vp, uiop, ioflag, cr);
	}
	dir->d_fattr.va_mtime = hrestime.tv_sec;
	dir->d_fattr.va_ctime = hrestime.tv_sec;

	/*
	 * If we are creating a hole, zero everything between
	 * the old eof and the place we start writing.
	 */
	if (offset > endfile + 1) {
		off_t holesz = offset - endfile - 1;
		caddr_t p;
		off_t curoff = endfile + 1;

		p = (caddr_t)kmem_zalloc(BFS_BSIZE, KM_SLEEP);
		ASSERT(p);
		while (holesz > 0) {
			int count;

			count = MIN(BFS_BSIZE, holesz);
			error = vn_rdwr(UIO_WRITE, bs->bsup_devnode,
			  p, count, curoff, UIO_SYSSPACE, IO_SYNC,
			  BFS_ULT, cr, 0);
			/* 
			 * if (error)
			 *	clean up and return.
			 */
			holesz -= count;
			curoff += count;
		}
		kmem_free(p, BFS_BSIZE);
	}

	chunksize = MIN(MAXBSIZE, uiop->uio_resid);
	buf = kmem_alloc(chunksize, KM_SLEEP);
	while (uiop->uio_resid) {
		buflen = MIN(chunksize, uiop->uio_resid);

		/*
		 * Get the user buffer into our space.
		 */
		if ((error = uiomove(buf, buflen, UIO_WRITE, uiop)) != 0) {
			kmem_free(dir, sizeof(struct bfs_dirent));
			kmem_free(buf, chunksize);
			BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
			BFS_IOEND(bs);
			return error;
		}

		/*
		 * Write to the device.
		 */
		error = vn_rdwr(UIO_WRITE, bs->bsup_devnode, buf, buflen,
		 offset, UIO_SYSSPACE, IO_SYNC, BFS_ULT, cr, 0);
		if (error) {
			kmem_free(dir, sizeof(struct bfs_dirent));
			kmem_free(buf,chunksize);
			BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
			BFS_IOEND(bs);
			return error;
		}
		/*
		 * Bump the End-of-file pointer, if necessary.
		 */
		if (offset + buflen > dir->d_eoffset) {
			new = (offset + buflen) - dir->d_eoffset;
			dir->d_eoffset += new - 1;
		}

		/*
		 * Write the new dirent.
		 */
		if (dir->d_eoffset >= (dir->d_eblock * BFS_BSIZE)) {
			dir->d_eblock = dir->d_eoffset / BFS_BSIZE;

			if (!oeblock)
				bs->bsup_freeblocks -=
				  (dir->d_eblock - dir->d_sblock) + 1;
			else if (dir->d_eblock != oeblock)
				bs->bsup_freeblocks -= 
				  dir->d_eblock - oeblock;

			BFS_PUTINODE(bs->bsup_devnode, vp->v_data, dir, cr);
			oeblock = dir->d_eblock;
		}
		offset += buflen;
	}

	bs->bsup_lastfile = bfs_lastfile;
	kmem_free(dir, sizeof(struct bfs_dirent));
	kmem_free(buf, chunksize);
	BFS_INOUNLOCK(bs, BFS_OFF2INO((off_t)vp->v_data));
	BFS_IOEND(bs);
	return 0;
}
