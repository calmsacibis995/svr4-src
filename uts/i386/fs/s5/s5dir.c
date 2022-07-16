/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:s5/s5dir.c	1.3"
/*
 * Directory manipulation routines.
 * From outside this file, only dirlook(), direnter(), and dirremove()
 * should be called.
 */
#include "sys/types.h"
#include "sys/buf.h"
#include "sys/cmn_err.h"
#include "sys/cred.h"
#include "sys/debug.h"
#include "sys/errno.h"
#include "sys/fbuf.h"
#include "sys/file.h"
#include "sys/param.h"
#include "sys/pathname.h"
#include "sys/stat.h"
#include "sys/sysinfo.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/mode.h"
#include "sys/dnlc.h"

#include "sys/proc.h"	/* XXX -- needed for user-context kludge in ILOCK */
#include "sys/disp.h"	/* XXX */

#include "sys/fs/s5param.h"
#include "sys/fs/s5dir.h"
#include "sys/fs/s5inode.h"
#include "sys/fs/s5macros.h"

#include "vm/seg.h"

#define	DOT	0x01
#define	DOTDOT	0x02

/*
 * Look for a given name in a directory.  On successful return, *ipp
 * will point to the (locked) inode.
 */
int
dirlook(dp, namep, ipp, cr)
	register struct inode *dp;
	register char *namep;
	register struct inode **ipp;
	struct cred *cr;
{
	struct vnode *vp;
	int error;

	/*
	 * Check accessibility of directory.
	 */
	if ((dp->i_mode & IFMT) != IFDIR)
		return ENOTDIR;
	if (error = iaccess(dp, IEXEC, cr))
		return error;
	/*
	 * Null component name is synonym for directory being searched.
	 */
	if (*namep == '\0') {
		VN_HOLD(ITOV(dp));
		*ipp = dp;
		ILOCK(dp);
		return 0;
	}
	/*
	 * Check the directory name lookup cache.
	 */
	if (vp = dnlc_lookup(ITOV(dp), namep, NOCRED)) {
		VN_HOLD(vp);
		*ipp = VTOI(vp);
		ILOCK(*ipp);
		return 0;
	}
	ILOCK(dp);
	if (((error = dirsearch(dp, namep, ipp, (off_t *) 0)) != 0)
	  || dp != *ipp)
		IUNLOCK(dp);
	if (error == 0) {
		vp = ITOV(*ipp);
		dnlc_enter(ITOV(dp), namep, vp, NOCRED);
	}
	return error;
}

/*
 * Write a new directory entry.
 * The directory must not have been removed and must be writable.
 * We distinguish four operations which build a new entry: creating
 * a file (DE_CREATE), creating a directory (DE_MKDIR), renaming
 * (DE_RENAME) or linking (DE_LINK).  There are five possible cases 
 * to consider:
 *
 *	Name
 *	found	op			action
 *	-----	---------------------	--------------------------------------
 *	no	DE_CREATE or DE_MKDIR	create file according to vap and enter
 *	no	DE_LINK or DE_RENAME	enter the file sip
 *	yes	DE_CREATE or DE_MKDIR	error EEXIST *ipp = found file
 *	yes	DE_LINK			error EEXIST
 *	yes	DE_RENAME		remove existing file, enter new file
 *
 * Note that a directory can be created either by mknod(2) or by
 * mkdir(2); the operation (DE_CREATE or DE_MKDIR) distinguishes
 * the two cases, which differ because mkdir(2) creates the
 * appropriate "." and ".." entries while mknod(2) doesn't.
 */
int
direnter(tdp, namep, op, sdp, sip, vap, ipp, cr)
	register struct inode *tdp;	/* target directory to make entry in */
	register char *namep;		/* name of entry */
	enum de_op op;			/* entry operation */
	register struct inode *sdp;	/* source inode parent if rename */
	struct inode *sip;		/* source inode if link/rename */
	struct vattr *vap;		/* attributes if new inode needed */
	struct inode **ipp;		/* return entered inode (locked) here */
	struct cred *cr;		/* user credentials */
{
	struct inode *tip;		/* inode of (existing) target file */
	register int error;		/* error number */
	off_t offset;			/* offset of old or new dir entry */

	/*
	 * For mkdir, ensure that we won't be exceeding the maximum
	 * link count of the parent directory.
	 */
	if (op == DE_MKDIR && tdp->i_nlink >= MAXLINK)
		return EMLINK;
	/*
	 * For link and rename, ensure that the source has not been
	 * removed while it was unlocked, that the source and target
	 * are on the same file system, and that we won't be exceeding
	 * the maximum link count of the source.  If all is well,
	 * synchronously update the link count.
	 */
	if (op == DE_LINK || op == DE_RENAME) {
		ILOCK(sip);
		if (sip->i_nlink == 0) {
			IUNLOCK(sip);
			return ENOENT;
		}
		if (sip->i_dev != tdp->i_dev) {
			IUNLOCK(sip);
			return EXDEV;
		}
		if (sip->i_nlink >= MAXLINK) {
			IUNLOCK(sip);
			return EMLINK;
		}
		sip->i_nlink++;
		sip->i_flag |= ICHG|ISYN;
		iupdat(sip);
		IUNLOCK(sip);
	}
	/*
	 * Lock the directory in which we are trying to make the new entry.
	 */
	ILOCK(tdp);
	/*
	 * If target directory has not been removed, then we can consider
	 * allowing file to be created.
	 */
	if (tdp->i_nlink == 0) {
		error = ENOENT;
		goto out;
	}
	/*
	 * Check accessibility of directory.
	 */
	if ((tdp->i_mode & IFMT) != IFDIR) {
		error = ENOTDIR;
		goto out;
	}
	/*
	 * Execute access is required to search the directory.
	 */
	if (error = iaccess(tdp, IEXEC, cr))
		goto out;
	/*
	 * If this is a rename of a directory and the parent is
	 * different (".." must be changed), then the source
	 * directory must not be in the directory hierarchy
	 * above the target, as this would orphan everything
	 * below the source directory.  Also the user must have
	 * write permission in the source so as to be able to
	 * change "..".
	 */
	if (op == DE_RENAME && (sip->i_mode & IFMT) == IFDIR && sdp != tdp
	  && (((error = iaccess(sip, IWRITE, cr)) != 0)
	    || ((error = dircheckpath(sip, tdp)) != 0)))
			goto out;
	/*
	 * Search for the entry.
	 */
	if (error = dircheckforname(tdp, namep, &offset, &tip))
		goto out;

	if (tip) {
		switch (op) {

		case DE_CREATE:
		case DE_MKDIR:
			if (ipp) {
				*ipp = tip;
				error = EEXIST;
			} else
				iput(tip);
			break;

		case DE_RENAME:
			error =
			  dirrename(sdp, sip, tdp, namep, tip, offset, cr);
			iput(tip);
			break;

		case DE_LINK:
			/*
			 * Can't link to an existing file.
			 */
			iput(tip);
			error = EEXIST;
			break;
		}
	} else {
		/*
		 * The entry does not exist.  Check write permission in
		 * directory to see if entry can be created.
		 */
		if (error = iaccess(tdp, IWRITE, cr))
			goto out;
		if (op == DE_CREATE || op == DE_MKDIR) {
			/*
			 * Make new inode and directory entry as required.
			 */
			if (error = dirmakeinode(tdp, &sip, vap, op, cr))
				goto out;
		}
		if (error = diraddentry(tdp, namep, offset, sip, sdp, op)) {
			if (op == DE_CREATE || op == DE_MKDIR) {
				/*
				 * Unmake the inode we just made.
				 */
				if (op == DE_MKDIR)
					tdp->i_nlink--;
				sip->i_nlink = 0;
				sip->i_flag |= ICHG;
				ILOCK(sip);
				iput(sip);
			}
		} else if (ipp) {
			ILOCK(sip);
			*ipp = sip;
		} else if (op == DE_CREATE || op == DE_MKDIR) {
			ILOCK(sip);
			iput(sip);
		}
	}

out:
	if (error && (op == DE_LINK || op == DE_RENAME)) {
		/*
		 * Undo bumped link count.
		 */
		sip->i_nlink--;
		sip->i_flag |= ICHG;
	}
	IUNLOCK(tdp);
	return error;
}

/*
 * Check for the existence of a name in a directory, or else of an empty
 * slot in which an entry may be made.  If the requested name is found,
 * then on return *tipp points at the (locked) inode and *offp contains
 * its offset in the directory.  If the name is not found, then *tipp
 * will be NULL and *offp will contain the offset of a directory slot in
 * which an entry may be made (either an empty slot, or the first offset
 * past the end of the directory).
 * 
 * This may not be used on "." or "..", but aliases of "." are okay.
 */
int
dircheckforname(tdp, namep, offp, tipp)
	register struct inode *tdp;	/* inode of directory being checked */
	char *namep;			/* name we're checking for */
	off_t *offp;			/* return offset of old or new entry */
	struct inode **tipp;		/* return inode if we find one */
{
	int error;

	/*
	 * Search for entry.  The caller doesn't require that it exist, so
	 * don't return ENOENT.  The non-existence of the entry will be
	 * indicated by *tipp == NULL.
	 */
	if ((error = dirsearch(tdp, namep, tipp, offp)) == ENOENT)
		error = 0;
	return error;
}

int
dirsearch(dip, comp, ipp, offp)
	struct inode *dip;	/* Directory to search. */
	char *comp;		/* Component to search for. */
	struct inode **ipp;	/* Ptr-to-ptr to result inode, if found. */
	off_t *offp;		/* Offset of entry or empty entry. */
{
	register char *cp;
	register int off;
	register struct vnode *dvp = ITOV(dip);
	struct fbuf *fbp;
	struct direct dir;
	int found, error, count, n;
	off_t offset, eo;
	int bsize = VBSIZE(dvp);
	int bmask = ~(S5VFS(dvp->v_vfsp)->vfs_bmask);

	ASSERT(dip->i_flag & ILOCKED);
	*ipp = NULL;
	dir.d_ino = 0;
	strncpy(dir.d_name, comp, DIRSIZ);

	offset = 0;
	eo = -1;
	count = dip->i_size;
	fbp = NULL;
	found = 0;

	while (count) {
		/*
		 * Read the next directory block.
		 */
		sysinfo.dirblk++;
		if (error = fbread(dvp, offset & bmask, bsize, S_OTHER, &fbp))
			goto out;
		/*
		 * Search directory block.  searchdir() returns the offset
		 * of a matching entry, or the offset of an empty entry,
		 * or -1.
		 */
		n = MIN(bsize, count);
		cp = fbp->fb_addr;
		if ((off = searchdir(cp, n, dir.d_name)) != -1) {
			cp += off;
			if ((dir.d_ino = ((struct direct *)cp)->d_ino) != 0) {
				found++;
				offset += off;
				break;
			}
			/* Keep track of empty slot. */
			if (eo == -1)
				eo = offset + off;
		}
		offset += n;
		count -= n;
		fbrelse(fbp, S_OTHER);
		fbp = NULL;
	}

	if (fbp)
		fbrelse(fbp, S_OTHER);
	if (found) {
		if (strcmp(dir.d_name, "..") == 0) {
			IUNLOCK(dip);	/* remove race */
			error = iget(dvp->v_vfsp, dir.d_ino, ipp);
			ILOCK(dip);	/* caller expects to unlock this */
		} else if (dir.d_ino != dip->i_number)
			error = iget(dvp->v_vfsp, dir.d_ino, ipp);
		else {
			VN_HOLD(dvp);
			*ipp = dip;
			error = 0;
		}
	} else {
		/*
		 * If an empty slot was found, return it.  Otherwise leave the
		 * offset unchanged (pointing at the end of directory).
		 */
		if (eo != -1)
			offset = eo;
		error = ENOENT;
	}

out:
	if (offp)
		*offp = offset;
	return error;
}

/*
 * Rename the entry in the directory tdp so that it points to
 * sip instead of tip.
 */
int
dirrename(sdp, sip, tdp, namep, tip, offset, cr)
	register struct inode *sdp;	/* parent directory of source */
	register struct inode *sip;	/* source inode */
	register struct inode *tdp;	/* parent directory of target */
	char *namep;			/* entry we are trying to change */
	struct inode *tip;		/* locked target inode */
	off_t offset;			/* offset of new entry */
	struct cred *cr;		/* credentials */
{
	int error, doingdirectory, dotflag;
	struct direct dir;

	/*
	 * Check that everything is on the same filesystem.
	 */
	if (tip->i_dev != tdp->i_dev || tip->i_dev != sip->i_dev)
		return EXDEV;
	/*
	 * Short circuit rename of something to itself.
	 */
	if (sip->i_number == tip->i_number)
		return ESAME;		/* special KLUDGE error code */
	/*
	 * Must have write permission to rewrite target entry.
	 */
	if (error = iaccess(tdp, IWRITE, cr))
		return error;
	/*
	 * If the parent directory is "sticky", then the user must own
	 * either the parent directory or the destination of the rename,
	 * or else must have permission to write the destination.
	 * Otherwise the destination may not be changed (except by the
	 * super-user).  This implements append-only directories.
	 */
	if ((tdp->i_mode & ISVTX) && cr->cr_uid != 0
	  && cr->cr_uid != tdp->i_uid && cr->cr_uid != tip->i_uid
	  && (error = iaccess(tip, IWRITE, cr)))
		return error;
	/*
	 * Ensure source and target are compatible (both directories
	 * or both not directories).  If target is a directory it must
	 * be empty and have no links to it; in addition it must not
	 * be a mount point.
	 */
	doingdirectory = ((sip->i_mode & IFMT) == IFDIR);
	if ((tip->i_mode & IFMT) == IFDIR) {
		if (!doingdirectory)
			return EISDIR;
		if (ITOV(tip)->v_vfsmountedhere)
			return EBUSY;
		if (!dirempty(tip, &dotflag) || tip->i_nlink > 2)
			return EEXIST;
	} else if (doingdirectory)
		return ENOTDIR;
	/*
	 * Rewrite the inode number for the target name entry
	 * from the target inode (ip) to the source inode (sip).
	 * This prevents the target entry from disappearing
	 * during a crash.
	 */
	dir.d_ino = sip->i_number;
	(void) strncpy(dir.d_name, namep, DIRSIZ);
	if (error = rdwri(UIO_WRITE, tdp, (caddr_t) &dir, SDSIZ, offset,
	  UIO_SYSSPACE, IO_SYNC, (int *) 0))
		return error;
	dnlc_remove(ITOV(tdp), namep);
	dnlc_enter(ITOV(tdp), namep, ITOV(sip), NOCRED);
	/*
	 * Decrement the link count of the target inode.
	 * Fix the ".." entry in sip to point to dp.
	 * This is done after the new entry is on the disk.
	 */
	tip->i_nlink--;
	tip->i_flag |= ICHG;
	if (doingdirectory) {
		if (dotflag & DOT)
			tip->i_nlink--;
		/*
		 * Renaming a directory with the parent different
		 * requires that ".." be rewritten.  The window is
		 * still there for ".." to be inconsistent, but this
		 * is unavoidable, and a lot shorter than when it was
		 * done in a user process.  We decrement the link
		 * count in the new parent as appropriate to reflect
		 * the just-removed target.  If the parent is the
		 * same, this is appropriate since the original
		 * directory is going away.  If the new parent is
		 * different, dirfixdotdot() will bump the link count
		 * back.
		 */
		if (dotflag & DOTDOT) {
			tdp->i_nlink--;
			tdp->i_flag |= ICHG;
		}
		if (sdp != tdp)
			error = dirfixdotdot(sip, sdp, tdp);
	}
	return error;
}

/*
 * Fix the ".." entry of the child directory so that it points
 * to the new parent directory instead of the old one.  Routine
 * assumes that dp is a directory and that all the inodes are on
 * the same file system.
 */
int
dirfixdotdot(dp, opdp, npdp)
	register struct inode *dp;	/* child directory */
	register struct inode *opdp;	/* old parent directory */
	register struct inode *npdp;	/* new parent directory */
{
	struct direct dir;
	register int error;

	ILOCK(dp);
	/*
	 * Check whether this is an ex-directory.
	 */
	if (dp->i_nlink == 0 || dp->i_size < 2*SDSIZ) {
		IUNLOCK(dp);
		return 0;
	}
	if (error = rdwri(UIO_READ, dp, (caddr_t) &dir, SDSIZ, (off_t) SDSIZ,
	  UIO_SYSSPACE, 0, (int *) 0))
		goto out;
	if (dir.d_ino == npdp->i_number)	/* Just a no-op. */
		goto out;
	if (strcmp(dir.d_name, "..") != 0) {	/* Sanity check. */
		error = ENOTDIR;
		goto out;
	}
	/*
	 * Increment the link count in the new parent inode and force it out.
	 */
	npdp->i_nlink++;
	npdp->i_flag |= ICHG|ISYN;
	iupdat(npdp);
	/*
	 * Rewrite the child ".." entry and force it out.
	 */
	dir.d_ino = npdp->i_number;
	if (error = rdwri(UIO_WRITE, dp, (caddr_t) &dir, SDSIZ, (off_t) SDSIZ,
	  UIO_SYSSPACE, IO_SYNC, (int *) 0))
		goto out;
	dnlc_remove(ITOV(dp), "..");
	dnlc_enter(ITOV(dp), "..", ITOV(npdp), NOCRED);
	IUNLOCK(dp);
	/*
	 * Decrement the link count of the old parent inode and force
	 * it out.  If opdp is NULL, then this is a new directory link;
	 * it has no parent, so we need not do anything.
	 */
	if (opdp != NULL) {
		ILOCK(opdp);
		if (opdp->i_nlink != 0) {
			opdp->i_nlink--;
			opdp->i_flag |= ICHG|ISYN;
			iupdat(opdp);
		}
		IUNLOCK(opdp);
	}
	return 0;
out:
	IUNLOCK(dp);
	return error;
}

/*
 * Enter the file sip in the directory tdp with name namep.
 */
int
diraddentry(tdp, namep, offset, sip, sdp, op)
	struct inode *tdp;
	char *namep;
	off_t offset;
	struct inode *sip;
	struct inode *sdp;
	enum de_op op;
{
	int error;
	struct direct dir;

	if ((sip->i_mode & IFMT) == IFDIR && op == DE_RENAME
	  && (error = dirfixdotdot(sip, sdp, tdp)))
		return error;
	/*
	 * Fill in entry data.
	 */
	dir.d_ino = sip->i_number;
	(void) strncpy(dir.d_name, namep, DIRSIZ);
	/*
	 * Write out the directory entry.
	 */
	if (error = rdwri(UIO_WRITE, tdp, (caddr_t) &dir, SDSIZ, offset,
	  UIO_SYSSPACE, op == DE_MKDIR ? IO_SYNC : 0, (int *) 0))
		return error;
	dnlc_enter(ITOV(tdp), namep, ITOV(sip), NOCRED);
	return 0;
}

/*
 * Allocate and initialize a new inode that will go into directory tdp.
 */
int
dirmakeinode(tdp, ipp, vap, op, cr)
	struct inode *tdp;
	struct inode **ipp;
	register struct vattr *vap;
	enum de_op op;
	struct cred *cr;
{
	struct inode *ip;
	int imode, nlink, gid, error;
	struct vfs *vfsp;

	ASSERT(vap != NULL);
	ASSERT((vap->va_mask & (AT_TYPE|AT_MODE)) == AT_TYPE|AT_MODE);
	ASSERT(op == DE_CREATE || op == DE_MKDIR);
	/*
	 * Allocate a new inode.
	 */
	imode = MAKEIMODE(vap->va_type, vap->va_mode);
	nlink = (op == DE_MKDIR) ? 2 : 1;
	/*
	 * If ISGID is set on the containing directory, the new
	 * entry inherits the directory's gid; otherwise the gid
	 * is taken from the supplied credentials.
	 */
	if (tdp->i_mode & ISGID) {
		gid = tdp->i_gid;
		if ((imode & IFMT) == IFDIR)
			imode |= ISGID;
		else if ((imode & ISGID) && !groupmember(gid, cr)
		  && cr->cr_uid != 0)
			imode &= ~ISGID;
	} else
		gid = cr->cr_gid;

	vfsp = ITOV(tdp)->v_vfsp;
	if (error =
	  ialloc(vfsp, imode, nlink, vap->va_rdev, cr->cr_uid, gid, &ip))
		return error;
	if (op == DE_MKDIR)
		error = dirmakedirect(ip, tdp);
	if (error) {
		/*
		 * Throw away the inode we just allocated.
		 */
		ip->i_nlink = 0;
		ip->i_flag |= ICHG;
		iput(ip);
	} else {
		IUNLOCK(ip);
		*ipp = ip;
	}
	return error;
}

/*
 * Write a prototype directory into the empty inode ip, whose parent is dp.
 */
int
dirmakedirect(ip, dp)
	struct inode *ip;		/* new directory */
	register struct inode *dp;	/* parent directory */
{
	int error;
	struct direct newdir[2];

	(void) strncpy(newdir[0].d_name, ".", DIRSIZ);
	newdir[0].d_ino = ip->i_number;			/* dot */
	(void) strncpy(newdir[1].d_name, "..", DIRSIZ);	
	newdir[1].d_ino = dp->i_number;			/* dot-dot */

	if ((error = rdwri(UIO_WRITE, ip, (caddr_t) newdir, 2*SDSIZ,
	  (off_t) 0, UIO_SYSSPACE, IO_SYNC, (int *) 0)) == 0) {
		/*
		 * Synchronously update link count of parent.
		 */
		dp->i_nlink++;
		dp->i_flag |= ICHG|ISYN;
		iupdat(dp);
	}
	return error;
}

/*
 * Delete a directory entry.  If oip is nonzero the entry is checked
 * to make sure it still reflects oip.
 */
int
dirremove(dp, namep, oip, cdir, op, cr)
	register struct inode *dp;
	char *namep;
	struct inode *oip;
	struct vnode *cdir;
	enum dr_op op;
	struct cred *cr;
{
	struct inode *ip;
	int error, dotflag;
	off_t offset;
	struct direct dir;
	static struct direct emptydirect[] = {
		0, ".",
		0, "..",
	};

	ip = NULL;
	ILOCK(dp);
	/*
	 * Check accessibility of directory.
	 */
	if ((dp->i_mode & IFMT) != IFDIR) {
		error = ENOTDIR;
		goto out;
	}
	if (error = iaccess(dp, IEXEC|IWRITE, cr))
		goto out;
	if (error = dircheckforname(dp, namep, &offset, &ip))
		goto out;
	if (ip == NULL) {
		error = ENOENT;
		goto out;
	}
	if (oip && oip != ip) {
		error = ENOENT;
		goto out;
	}
	/*
	 * Don't remove a mounted-on directory (the possible result
	 * of a race between mount(2) and unlink(2) or rmdir(2)).
	 */
	if (ITOV(ip)->v_vfsmountedhere != NULL) {
		error = EBUSY;
		goto out;
	}
	/*
	 * If the parent directory is "sticky", then the user must
	 * own the parent directory or the file in it, or else must
	 * have permission to write the file.  Otherwise it may not
	 * be deleted (except by the super-user).  This implements
	 * append-only directories.
	 */
	if ((dp->i_mode & ISVTX) && cr->cr_uid != 0
	  && cr->cr_uid != dp->i_uid && cr->cr_uid != ip->i_uid
	  && (error = iaccess(ip, IWRITE, cr)))
		goto out;

	if (op == DR_RMDIR) {
		/*
		 * For rmdir(2), some special checks are required.
		 * (a) Don't remove any alias of the parent (e.g. ".").
		 * (b) Don't remove the current directory.
		 * (c) Make sure the entry is (still) a directory.
		 * (d) Make sure the directory is empty.
		 */
		if (dp == ip || ITOV(ip) == cdir)
			error = EINVAL;
		else if ((ip->i_mode & IFMT) != IFDIR)
			error = ENOTDIR;
		else if (!dirempty(ip, &dotflag))
			error = EEXIST;
		if (error)
			goto out;
	} else if (op == DR_REMOVE) {
		/*
		 * unlink(2) requires a different check: allow only
		 * the super-user to unlink a directory.
		 */
		if (ITOV(ip)->v_type == VDIR && !suser(cr)) {
			error = EPERM;
			goto out;
		}
	}
	/*
	 * Zero the i-number field of the directory entry.  Retain the
	 * file name in the empty slot, as UNIX has always done.
	 */
	dir.d_ino = 0;
	(void) strncpy(dir.d_name, namep, DIRSIZ);
	if (error = rdwri(UIO_WRITE, dp, (caddr_t) &dir, SDSIZ, offset,
	  UIO_SYSSPACE, IO_SYNC, (int *) 0))
		goto out;
	dnlc_remove(ITOV(dp), namep);
	ip->i_flag |= ICHG;
	/*
	 * Now dispose of the inode.
	 */
	if (op == DR_RMDIR) {
		if (dotflag & DOT) {
			ip->i_nlink -= 2;
			dnlc_remove(ITOV(ip), ".");
		} else
			ip->i_nlink--;
		if (dotflag & DOTDOT) {
			dp->i_nlink--;
			dnlc_remove(ITOV(ip), "..");
		}
		/*
		 * If other references exist, zero the "." and "..
		 * entries so they're inaccessible (POSIX requirement).
		 * If the directory is going away we can avoid doing
		 * this work.
		 */
		if (ITOV(ip)->v_count > 1 && ip->i_nlink <= 0)
			(void) rdwri(UIO_WRITE, ip, (caddr_t) emptydirect,
			  min(sizeof(emptydirect), ip->i_size),
			  (off_t) 0, UIO_SYSSPACE, 0, (int *) 0);
	} else
		ip->i_nlink--;

	if (ip->i_nlink < 0) {	/* Pathological */
		cmn_err(CE_WARN, "dirremove: ino %d, dev %x, nlink %d",
		  ip->i_number, ip->i_dev, ip->i_nlink);
		ip->i_nlink = 0;
	}

out:
	if (ip) {
		iput(ip);
		if (ip != dp)
			IUNLOCK(dp);
	} else
		IUNLOCK(dp);
	return error;
}

/*
 * Check whether a directory is empty (i.e. whether it contains
 * any entries apart from "." and "..").  Inode supplied must
 * be locked.  The value returned in *dotflagp encodes whether
 * "." and ".." are actually present.
 */
int
dirempty(ip, dotflagp)
	register struct inode *ip;
	int *dotflagp;
{
	register off_t off;
	struct direct dir;
	register struct direct *dp = &dir;

	*dotflagp = 0;
	for (off = 0; off < ip->i_size; off += SDSIZ) {
		if (rdwri(UIO_READ, ip, (caddr_t) dp, SDSIZ, off,
		  UIO_SYSSPACE, 0, (int *) 0))
			break;
		if (dp->d_ino != 0) {
			if (strcmp(dp->d_name, ".") == 0)
				*dotflagp |= DOT;
			else if (strcmp(dp->d_name, "..") == 0)
				*dotflagp |= DOTDOT;
			else
				return 0;
		}
	}
	return 1;
}

#define	RENAME_IN_PROGRESS	0x01
#define	RENAME_WAITING		0x02

/*
 * Check if source directory is in the path of the target directory.
 * Target is supplied locked, source is unlocked.
 * The target is always relocked before returning.
 */
int
dircheckpath(source, target)
	struct inode *source;
	struct inode *target;
{
	struct inode *ip;
	static char serialize_flag = 0;
	int error = 0;

	/*
	 * If two renames of directories were in progress at once, the partially
	 * completed work of one dircheckpath could be invalidated by the other
	 * rename. To avoid this, all directory renames in the system are
	 * serialized.
	 */
	while (serialize_flag & RENAME_IN_PROGRESS) {
		serialize_flag |= RENAME_WAITING;
		(void) sleep((caddr_t) &serialize_flag, PINOD);
	}
	serialize_flag = RENAME_IN_PROGRESS;
	ip = target;
	if (ip->i_number == source->i_number) {
		error = EINVAL;
		goto out;
	}
	if (ip->i_number == S5ROOTINO)
		goto out;
	/*
	 * Search back through the directory tree, using the ".." entries.
	 * Fail any attempt to move a directory into an ancestor directory.
	 */
	for (;;) {
		struct direct dir;

		if (((ip->i_mode & IFMT) != IFDIR) || ip->i_nlink == 0
		  || ip->i_size < 2*SDSIZ) {
			error = ENOTDIR;
			break;
		}
		if (error = rdwri(UIO_READ, ip, (caddr_t) &dir, SDSIZ,
		  (off_t) SDSIZ, UIO_SYSSPACE, 0, (int *) 0))
			break;
		if (strcmp(dir.d_name, "..") != 0) {
			error = ENOTDIR;	/* Sanity check */
			break;
		}
		if (dir.d_ino == source->i_number) {
			error = EINVAL;
			break;
		}
		if (dir.d_ino == S5ROOTINO)
			break;
		if (ip != target)
			iput(ip);
		else
			IUNLOCK(ip);
		if (error = iget(ITOV(ip)->v_vfsp, dir.d_ino, &ip))
			break;
	}
out:
	if (ip) {
		if (ip != target) {
			iput(ip);
			/*
			 * Relock target and make sure it has not gone away
			 * while it was unlocked.
			 */
			ILOCK(target);
			if (error == 0 && target->i_nlink == 0)
				error = ENOENT;
		}
	}
	/*
	 * Unserialize.
	 */
	if (serialize_flag & RENAME_WAITING)
		wakeprocs((caddr_t) &serialize_flag, PRMPT);
	serialize_flag = 0;
	return error;
}
