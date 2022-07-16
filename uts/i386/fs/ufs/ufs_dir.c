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

#ident  "@(#)kern-fs:ufs/ufs_dir.c	1.3.2.4"

/*
 * Directory manipulation routines.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/cred.h>
#include <sys/proc.h>
#include <sys/disp.h>
#include <sys/user.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/stat.h>
#include <sys/mode.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/dnlc.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>
#include <sys/mount.h>
#include <sys/fs/ufs_fsdir.h>
#ifdef QUOTA
#include <sys/fs/ufs_quota.h>
#endif
#include <sys/errno.h>
#include <sys/debug.h>
#include <vm/seg.h>
#include <sys/sysmacros.h>
#include <sys/cmn_err.h>

/*
 * A virgin directory.
 */
struct dirtemplate mastertemplate = {
	0, 12, 1, ".",
	0, DIRBLKSIZ - 12, 2, ".."
};

#define DOT	0x01
#define DOTDOT	0x02

#define LDIRSIZ(len) \
    ((sizeof (struct direct) - (MAXNAMLEN + 1)) + ((len + 1 + 3) &~ 3))

int dirchk = 0;
STATIC void dirbad();
/*
 * Look for a given name in a directory.  On successful return, *ipp
 * will point to the (locked) inode.
 */
int
ufs_dirlook(dp, namep, ipp, cr)
	register struct inode *dp;
	register char *namep;
	register struct inode **ipp;
	struct cred *cr;
{
	struct fbuf *fbp = NULL;	/* a buffer of directory entries */
	register struct direct *ep;	/* the current directory entry */
	register struct inode *ip;
	struct vnode *vp, *dnlc_lookup();
	int entryoffsetinblock;		/* offset of ep in addr's buffer */
	int numdirpasses;		/* strategy for directory search */
	off_t endsearch;		/* offset to end directory search */
	int namlen = strlen(namep);	/* length of name */
	off_t offset;
	int error;
	register int i;

	/*
	 * Check accessibility of directory.
	 */
	if ((dp->i_mode & IFMT) != IFDIR)
		return (ENOTDIR);
	if (error = ufs_iaccess(dp, IEXEC, cr))
		return (error);

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
		ufs_ilock(*ipp);
		return (0);
	}

	ufs_ilock(dp);
	if (dp->i_diroff > dp->i_size) {
		dp->i_diroff = 0;
	}
	if (dp->i_diroff == 0) {
		offset = 0;
		numdirpasses = 1;
	} else {
		offset = dp->i_diroff;
		entryoffsetinblock = blkoff(dp->i_fs, offset);
		if (entryoffsetinblock != 0) {
			error = blkatoff(dp, offset, (char **)0, &fbp);
			if (error) {
				goto bad;
			}
		}
		numdirpasses = 2;
	}
	endsearch = roundup(dp->i_size, DIRBLKSIZ);

searchloop:
	while (offset < endsearch) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */
		if (blkoff(dp->i_fs, offset) == 0) {
			if (fbp != NULL)
				fbrelse(fbp, S_OTHER);
			error = blkatoff(dp, offset, (char **)0, &fbp);
			if (error) {
				goto bad;
			}
			entryoffsetinblock = 0;
		}

		/*
		 * Get pointer to next entry.
		 * Full validation checks are slow, so we only check
		 * enough to insure forward progress through the
		 * directory. Complete checks can be run by patching
		 * "dirchk" to be true.
		 */
		ep = (struct direct *)(fbp->fb_addr + entryoffsetinblock);
		if (ep->d_reclen == 0 ||
		    dirchk && dirmangled(dp, ep, entryoffsetinblock, offset) ) {
			i = DIRBLKSIZ - (entryoffsetinblock & (DIRBLKSIZ - 1));
			offset += i;
			entryoffsetinblock += i;
			continue;
		}

		/*
		 * Check for a name match.
		 * We must get the target inode before unlocking
		 * the directory to insure that the inode will not be removed
		 * before we get it.  We prevent deadlock by always fetching
		 * inodes from the root, moving down the directory tree. Thus
		 * when following backward pointers ".." we must unlock the
		 * parent directory before getting the requested directory.
		 * There is a potential race condition here if both the current
		 * and parent directories are removed before the `ufs_iget'
		 * for the inode associated with ".." returns.  We hope that
		 * this occurs infrequently since we can't avoid this race
		 * condition without implementing a sophisticated deadlock
		 * detection algorithm. Note also that this simple deadlock
		 * detection scheme will not work if the file system has any
		 * hard links other than ".." that point backwards in the
		 * directory structure.
		 * See comments at head of file about deadlocks.
		 */
		if (ep->d_ino && ep->d_namlen == namlen &&
		    *namep == *ep->d_name &&	/* fast chk 1st chr */
		    bcmp(namep, ep->d_name, (int)ep->d_namlen) == 0) {
			u_long ep_ino;

			/*
			 * We have to release the fbp early ehere to avoid
			 * a possible deadlock situation where we have the
			 * fbp and want the directory inode and someone doing
			 * a ufs_direnter has the directory inode and wants the
			 * fbp.  XXX - is this still needed?
			 */
			ep_ino = ep->d_ino;
			fbrelse(fbp, S_OTHER);
			fbp = NULL;
			dp->i_diroff = offset;
			if (namlen == 2 && namep[0] == '.' && namep[1] == '.') {
				ufs_iunlock(dp);	/* race to get inode */
				error = ufs_iget(dp->i_vnode.v_vfsp, dp->i_fs, ep_ino,
				    ipp);
				if (error) {
					goto bad2;
				}
			} else if (dp->i_number == ep_ino) {
				VN_HOLD(ITOV(dp));	/* want ourself, "." */
				*ipp = dp;
			} else {
				error = ufs_iget(dp->i_vnode.v_vfsp, dp->i_fs, ep_ino,
				    ipp);
				ufs_iunlock(dp);
				if (error) {
					goto bad2;
				}
			}
			ip = *ipp;
			dnlc_enter(ITOV(dp), namep, ITOV(ip), NOCRED);
			return (0);
		}
		offset += ep->d_reclen;
		entryoffsetinblock += ep->d_reclen;
	}
	/*
	 * If we started in the middle of the directory and failed
	 * to find our target, we must check the beginning as well.
	 */
	if (numdirpasses == 2) {
		numdirpasses--;
		offset = 0;
		endsearch = dp->i_diroff;
		goto searchloop;
	}
	error = ENOENT;
bad:
	ufs_iunlock(dp);
bad2:
	if (fbp)
		fbrelse(fbp, S_OTHER);
	return (error);
}

/*
 * If "dircheckforname" fails to find an entry with the given name, this
 * structure holds state for "ufs_direnter" as to where there is space to put
 * an entry with that name.
 * If "dircheckforname" finds an entry with the given name, this structure
 * holds state for "dirrename" and "ufs_dirremove" as to where the entry is.
 * "status" indicates what "dircheckforname" found:
 *	NONE		name not found, large enough free slot not found,
 *			can't make large enough free slot by compacting entries
 *	COMPACT		name not found, large enough free slot not found,
 *			can make large enough free slot by compacting entries
 *	FOUND		name not found, large enough free slot found
 *	EXIST		name found
 * If "dircheckforname" fails due to an error, this structure is not filled in.
 *
 * After "dircheckforname" succeeds the values are:
 *	status	offset		size		fbp, ep
 *	------	------		----		-------
 *	NONE	end of dir	needed		not valid
 *	COMPACT	start of area	of area		not valid
 *	FOUND	start of entry	of ent		not valid
 *	EXIST	start if entry	of prev ent	valid
 *
 * "endoff" is set to 0 if the an entry with the given name is found, or if no
 * free slot could be found or made; this means that the directory should not
 * be truncated.  If the entry was found, the search terminates so
 * "dircheckforname" didn't find out where the last valid entry in the
 * directory was, so it doesn't know where to cut the directory off; if no free
 * slot could be found or made, the directory has to be extended to make room
 * for the new entry, so there's nothing to cut off.
 * Otherwise, "endoff" is set to the larger of the offset of the last
 * non-empty entry in the directory, or the offset at which the new entry will
 * be placed, whichever is larger.  This is used by "diraddentry"; if a new
 * entry is to be added to the directory, any complete directory blocks at the
 * end of the directory that contain no non-empty entries are lopped off the
 * end, thus shrinking the directory dynamically.
 *
 * On success, "dirprepareentry" makes "fbp" and "ep" valid.
 */
struct slot {
	enum	{NONE, COMPACT, FOUND, EXIST} status;
	off_t	offset;		/* offset of area with free space */
	int	size;		/* size of area at slotoffset */
	struct	fbuf *fbp;	/* dir buf where slot is */
	struct direct *ep;	/* pointer to slot */
	off_t	endoff;		/* last useful location found in search */
};

/*
 * Write a new directory entry.
 * The directory must not have been removed and must be writable.
 * We distinguish three operations that build a new entry:  creating a file
 * (DE_CREATE), renaming (DE_RENAME) or linking (DE_LINK).  There are five
 * possible cases to consider:
 *
 *	Name
 *	found	op			action
 *	-----	---------------------	--------------------------------------
 *	no	DE_CREATE		create file according to vap and enter
 *	no	DE_LINK or DE_RENAME	enter the file sip
 *	yes	DE_CREATE		error EEXIST *ipp = found file
 *	yes	DE_LINK			error EEXIST
 *	yes	DE_RENAME		remove existing file, enter new file
 */
int
ufs_direnter(tdp, namep, op, sdp, sip, vap, ipp, cr)
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
	struct slot slot;		/* slot info to pass around */
	register int namlen;		/* length of name */
	register int err;		/* error number */
	register char *s;

	/* don't allow '/' characters in pathname component */
	for (s=namep,namlen=0; *s; s++,namlen++)
		if (*s == '/')
			return(EACCES);
	ASSERT(namlen != 0);
	/*
	 * If name is "." or ".." then if this is a create look it up
	 * and return EEXIST.  Rename or link TO "." or ".." is forbidden.
	 */
	if (namep[0] == '.' &&
	    (namlen == 1 || (namlen == 2 && namep[1] == '.')) ) {
		if (op == DE_RENAME) {
			return (EINVAL);	/* *SIGH* should be ENOTEMPTY */
		}
		if (ipp) {
			if (err = ufs_dirlook(tdp, namep, ipp, cr))
				return (err);
		}
		return (EEXIST);
	}
	slot.status = NONE;
	slot.fbp = NULL;
	/*
	 * For link and rename lock the source entry and check the link count
	 * to see if it has been removed while it was unlocked.  If not, we
	 * increment the link count and force the inode to disk to make sure
	 * that it is there before any directory entry that points to it.
	 */
	if (op == DE_LINK || op == DE_RENAME) {
		ufs_ilock(sip);
		if (sip->i_nlink == 0) {
			ufs_iunlock(sip);
			return (ENOENT);
		}
		if (sip->i_nlink == MAXLINK) {
			ufs_iunlock(sip);
			return (EMLINK);
		}
		sip->i_nlink++;
		sip->i_flag |= ICHG;
		ufs_iupdat(sip, 1);
		ufs_iunlock(sip);
	}
	/*
	 * Lock the directory in which we are trying to make the new entry.
	 */
	ufs_ilock(tdp);
	/*
	 * If target directory has not been removed, then we can consider
	 * allowing file to be created.
	 */
	if (tdp->i_nlink == 0) {
		err = ENOENT;
		goto out;
	}
	/*
	 * Check accessibility of directory.
	 */
	if ((tdp->i_mode & IFMT) != IFDIR) {
		err = ENOTDIR;
		goto out;
	}
	/*
	 * Execute access is required to search the directory.
	 */
	if (err = ufs_iaccess(tdp, IEXEC, cr))
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
	  && (((err = ufs_iaccess(sip, IWRITE, cr)) != 0)
	   || ((err = ufs_dircheckpath(sip, tdp)) != 0)))
			goto out;
	/*
	 * Search for the entry.
	 */
	if (err = ufs_dircheckforname(tdp, namep, namlen, &slot, &tip))
		goto out;

	if (tip) {
		switch (op) {

		case DE_CREATE:
		case DE_MKDIR:
			if (ipp) {
				*ipp = tip;
				err = EEXIST;
			} else
				ufs_iput(tip);
			break;

		case DE_RENAME:
			err = ufs_dirrename(sdp, sip, tdp, namep,
			    tip, &slot, cr);
			ufs_iput(tip);
			break;

		case DE_LINK:
			/*
			 * Can't link to an existing file.
			 */
			ufs_iput(tip);
			err = EEXIST;
			break;
		}
	} else {
		/*
		 * The entry does not exist. Check write permission in
		 * directory to see if entry can be created.
		 */
		if (err = ufs_iaccess(tdp, IWRITE, cr))
			goto out;
		if (op == DE_CREATE || op == DE_MKDIR) {
			/*
			 * Make new inode and directory entry as required.
			 */
			if (err = ufs_dirmakeinode(tdp, &sip, vap, op, cr))
				goto out;
		}
		if (err = ufs_diraddentry(tdp, namep, namlen, &slot, sip, sdp, op)) {
			if (op == DE_CREATE || op == DE_MKDIR) {
				/*
				 * Unmake the inode we just made.
				 */
				if ((sip->i_mode & IFMT) == IFDIR)
					tdp->i_nlink--;
				sip->i_nlink = 0;
				sip->i_flag |= ICHG;
				irele(sip);
				sip = NULL;
			}
		} else if (ipp) {
			ufs_ilock(sip);
			*ipp = sip;
		} else if (op == DE_CREATE || op == DE_MKDIR) {
			irele(sip);
		}
	}

out:
	if (slot.fbp)
		fbrelse(slot.fbp, S_OTHER);
	if (err && (op == DE_LINK || op == DE_RENAME)) {
		/*
		 * Undo bumped link count.
		 */
		sip->i_nlink--;
		sip->i_flag |= ICHG;
	}
	ufs_iunlock(tdp);
	return (err);
}

/*
 * Check for the existence of a name in a directory, or else of an empty
 * slot in which an entry may be made.  If the requested name is found,
 * then on return *ipp points at the (locked) inode and *offp contains
 * its offset in the directory.  If the name is not found, then *ipp
 * will be NULL and *slotp will contain information about a directory slot in
 * which an entry may be made (either an empty slot, or the first position
 * past the end of the directory).
 * The target directory inode (tdp) is supplied locked.
 *
 * This may not be used on "." or "..", but aliases of "." are ok.
 */
STATIC int
ufs_dircheckforname(tdp, namep, namlen, slotp, ipp)
	register struct inode *tdp;	/* inode of directory being checked */
	char *namep;			/* name we're checking for */
	register int namlen;		/* length of name */
	register struct slot *slotp;	/* slot structure */
	struct inode **ipp;		/* return inode if we find one */
{
	int dirsize;			/* size of the directory */
	struct fbuf *fbp;		/* pointer to directory block */
	register int entryoffsetinblk;	/* offset of ep in fbp's buffer */
	int slotfreespace;		/* free space in block */
	register struct direct *ep;	/* directory entry */
	register off_t offset;		/* offset in the directory */
	register off_t last_offset;	/* last offset */
	off_t enduseful;		/* pointer past last used dir slot */
	int i;				/* length of mangled entry */
	int needed;
	int err;

	fbp = NULL;
	entryoffsetinblk = 0;
	needed = LDIRSIZ(namlen);
	/*
	 * No point in using i_diroff since we must search whole directory
	 */
	dirsize = roundup(tdp->i_size, DIRBLKSIZ);
	enduseful = 0;
	offset = last_offset = 0;
	while (offset < dirsize) {
		/*
		 * If offset is on a block boundary,
		 * read the next directory block.
		 * Release previous if it exists.
		 */
		if (blkoff(tdp->i_fs, offset) == 0) {
			if (fbp != NULL)
				fbrelse(fbp, S_OTHER);
			err = blkatoff(tdp, offset, (char **)0, &fbp);
			if (err) {
				return (err);
			}
			entryoffsetinblk = 0;
		}
		/*
		 * If still looking for a slot, and at a DIRBLKSIZ
		 * boundary, have to start looking for free space
		 * again.
		 */
		if (slotp->status == NONE &&
		    (entryoffsetinblk&(DIRBLKSIZ-1)) == 0) {
			slotp->offset = -1;
			slotfreespace = 0;
		}
		/*
		 * Get pointer to next entry.
		 * Since we are going to do some entry manipulation
		 * we call dirmangled to do more thorough checks.
		 */
		ep = (struct direct *)(fbp->fb_addr + entryoffsetinblk);
		if (ep->d_reclen == 0 ||
		    dirmangled(tdp, ep, entryoffsetinblk, offset) ) {
			i = DIRBLKSIZ - (entryoffsetinblk & (DIRBLKSIZ - 1));
			offset += i;
			entryoffsetinblk += i;
			continue;
		}
		/*
		 * If an appropriate sized slot has not yet been found,
		 * check to see if one is available. Also accumulate space
		 * in the current block so that we can determine if
		 * compaction is viable.
		 */
		if (slotp->status != FOUND) {
			int size = ep->d_reclen;

			if (ep->d_ino != 0)
				size -= DIRSIZ(ep);
			if (size > 0) {
				if (size >= needed) {
					slotp->status = FOUND;
					slotp->offset = offset;
					slotp->size = ep->d_reclen;
				} else if (slotp->status == NONE) {
					slotfreespace += size;
					if (slotp->offset == -1)
						slotp->offset = offset;
					if (slotfreespace >= needed) {
						slotp->status = COMPACT;
						slotp->size =
						    offset + ep->d_reclen -
						    slotp->offset;
					}
				}
			}
		}
		/*
		 * Check for a name match.
		 */
		if (ep->d_ino && ep->d_namlen == namlen &&
		    *namep == *ep->d_name &&	/* fast chk 1st char */
		    bcmp(namep, ep->d_name, namlen) == 0) {
			tdp->i_diroff = offset;
			if (tdp->i_number == ep->d_ino) {
				*ipp = tdp;	/* we want ourself, ie "." */
				VN_HOLD(ITOV(tdp));
			} else {
				err = ufs_iget(tdp->i_vnode.v_vfsp, tdp->i_fs,
				    ep->d_ino, ipp);
				if (err) {
					fbrelse(fbp, S_OTHER);
					return (err);
				}
			}
			slotp->status = EXIST;
			slotp->offset = offset;
			slotp->size = offset - last_offset;
			slotp->fbp = fbp;
			slotp->ep = ep;
			slotp->endoff = 0;
			return (0);
		}
		last_offset = offset;
		offset += ep->d_reclen;
		entryoffsetinblk += ep->d_reclen;
		if (ep->d_ino)
			enduseful = offset;
	}
	if (fbp) {
		fbrelse(fbp, S_OTHER);
	}
	if (slotp->status == NONE) {
		/*
		 * We didn't find a slot; the new directory entry should be put
		 * at the end of the directory.  Return an indication of where
		 * this is, and set "endoff" to zero; since we're going to have
		 * to extend the directory, we're certainly not going to
		 * trucate it.
		 */
		slotp->offset = dirsize;
		slotp->size = DIRBLKSIZ;
		slotp->endoff = 0;
	} else {
		/*
		 * We found a slot, and will return an indication of where that
		 * slot is, as any new directory entry will be put there.
		 * Since that slot will become a useful entry, if the last
		 * useful entry we found was before this one, update the offset
		 * of the last useful entry.
		 */
		if (enduseful < slotp->offset + slotp->size)
			enduseful = slotp->offset + slotp->size;
		slotp->endoff = roundup(enduseful, DIRBLKSIZ);
	}
	*ipp = (struct inode *)NULL;
	return (0);
}

/*
 * Rename the entry in the directory tdp so that it points to
 * sip instead of tip.
 */
STATIC int
ufs_dirrename(sdp, sip, tdp, namep, tip, slotp, cr)
	register struct inode *sdp;	/* parent directory of source */
	register struct inode *sip;	/* source inode */
	register struct inode *tdp;	/* parent directory of target */
	char *namep;			/* entry we are trying to change */
	struct inode *tip;		/* locked target inode */
	struct slot *slotp;		/* slot for entry */
	struct cred *cr;		/* credentials */
{
	int error= 0;
	int doingdirectory;
	int dotflag;

	/*
	 * Check that everything is on the same filesystem.
	 */
	if ((tip->i_vnode.v_vfsp != tdp->i_vnode.v_vfsp) ||
	    (tip->i_vnode.v_vfsp != sip->i_vnode.v_vfsp))
		return (EXDEV);		/* XXX archaic */
	/*
	 * Short circuit rename of something to itself.
	 */
	if (sip->i_number == tip->i_number)
		return (ESAME);		/* special KLUDGE error code */
	/*
	 * Must have write permission to rewrite target entry.
	 */
	if (error = ufs_iaccess(tdp, IWRITE, cr))
		return (error);
	/*
	 * If the parent directory is "sticky", then the user must own
	 * either the parent directory or the destination of the rename,
	 * or else must have permission to write the destination.
	 * Otherwise the destination may not be changed (except by the
	 * super-user).  This implements append-only directories.
	 */
	if ((tdp->i_mode & ISVTX) && cr->cr_uid != 0
	    && cr->cr_uid != tdp->i_uid && cr->cr_uid != tip->i_uid
	    && (error = ufs_iaccess(tip, IWRITE, cr)))
		return (error);

	/*
	 * Ensure source and target are compatible (both directories
	 * or both not directories).  If target is a directory it must
	 * be empty and have no links to it; in addition it must not
	 * be a mount point.
	 */
	doingdirectory = ((sip->i_mode & IFMT) == IFDIR);
	if ((tip->i_mode & IFMT) == IFDIR) {
		if (!doingdirectory)
			return (EISDIR);
		if (ITOV(tip)->v_vfsmountedhere)
			return (EBUSY);
		if (!ufs_dirempty(tip, tdp->i_number, &dotflag) || tip->i_nlink > 2)
			return (EEXIST);	/* SIGH should be ENOTEMPTY */
	} else if (doingdirectory)
		return (ENOTDIR);

	/*
	 * Rewrite the inode pointer for target name entry
	 * from the target inode (ip) to the source inode (sip).
	 * This prevents the target entry from disappearing
	 * during a crash. Mark the directory inode to reflect the changes.
	 */
	dnlc_remove(ITOV(tdp), namep);
	slotp->ep->d_ino = sip->i_number;
	dnlc_enter(ITOV(tdp), namep, ITOV(sip), NOCRED);
	error = fbwrite(slotp->fbp);
	slotp->fbp = NULL;
	if (error)
		return (error);
	tdp->i_flag |= IUPD|ICHG;
	/*
	 * Decrement the link count of the target inode.
	 * Fix the ".." entry in sip to point to dp.
	 * This is done after the new entry is on the disk.
	 */
	tip->i_nlink--;
	tip->i_flag |= ICHG;
	if (doingdirectory) {
		/*
		 * Decrement target link count once more if it was a directory.
		 */
		if (--tip->i_nlink != 0)
			cmn_err(CE_PANIC, "ufs_direnter: target directory link count");
		(void) ufs_itrunc(tip, (u_long)0);
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
		tdp->i_nlink--;
		tdp->i_flag |= ICHG;
		if (sdp != tdp)
			error = ufs_dirfixdotdot(sip, sdp, tdp);
	}
	return (error);
}

/*
 * Fix the ".." entry of the child directory so that it points
 * to the new parent directory instead of the old one.  Routine
 * assumes that dp is a directory and that all the inodes are on
 * the same file system.
 */
STATIC int
ufs_dirfixdotdot(dp, opdp, npdp)
	register struct inode *dp;	/* child directory */
	register struct inode *opdp;	/* old parent directory */
	register struct inode *npdp;	/* new parent directory */
{
	struct fbuf *fbp;
	struct dirtemplate *dirp;
	int error;

	ufs_ilock(dp);
	/*
	 * Check whether this is an ex-directory.
	 */
	if (dp->i_nlink == 0 || dp->i_size < sizeof (struct dirtemplate)) {
		ufs_iunlock(dp);
		return (0);
	}
	error = blkatoff(dp, (off_t)0, (char **)&dirp, &fbp);
	if (error)
		goto bad;
	if (dirp->dotdot_ino == npdp->i_number)	/* Just a no-op. */
		goto bad;
	if (dirp->dotdot_namlen != 2 ||
	    dirp->dotdot_name[0] != '.' ||
	    dirp->dotdot_name[1] != '.') {	/* Sanity check. */
		dirbad(dp, "mangled .. entry", (off_t)0);
		error = ENOTDIR;
		goto bad;
	}

	/*
	 * Increment the link count in the new parent inode and force it out.
	 */
	npdp->i_nlink++;
	npdp->i_flag |= ICHG;
	ufs_iupdat(npdp, 1);

	/*
	 * Rewrite the child ".." entry and force it out.
	 */
	dnlc_remove(ITOV(dp), "..");
	dirp->dotdot_ino = npdp->i_number;
	dnlc_enter(ITOV(dp), "..", ITOV(npdp), NOCRED);
	error = fbwrite(fbp);
	fbp = NULL;
	if (error)
		goto bad;
	ufs_iunlock(dp);

	/*
	 * Decrement the link count of the old parent inode and force
	 * it out.  If opdp is NULL, then this is a new directory link;
	 * it has no parent, so we need not do anything.
	 */
	if (opdp != NULL) {
		ufs_ilock(opdp);
		if (opdp->i_nlink != 0) {
			opdp->i_nlink--;
			opdp->i_flag |= ICHG;
			ufs_iupdat(opdp, 1);
		}
		ufs_iunlock(opdp);
	}
	return (0);

bad:
	if (fbp)
		fbrelse(fbp, S_OTHER);
	ufs_iunlock(dp);
	return (error);
}

/*
 * Enter the file sip in the directory tdp with name namep.
 */
STATIC int
ufs_diraddentry(tdp, namep, namlen, slotp, sip, sdp, op)
	struct inode *tdp;
	char *namep;
	int namlen;
	struct slot *slotp;
	struct inode *sip;
	struct inode *sdp;
	enum de_op op;
{
	int error;

	/*
	 * Prepare a new entry.  If the caller has not supplied an
	 * existing inode, make a new one.
	 */
	error = dirprepareentry(tdp, slotp);
	if (error)
		return (error);
	/*
	 * Check inode to be linked to see if it is in the
	 * same filesystem.
	 */
	if (tdp->i_vnode.v_vfsp != sip->i_vnode.v_vfsp) {
		error = EXDEV;
		goto bad;
	}
	if ((sip->i_mode & IFMT) == IFDIR && op == DE_RENAME) {
		error = ufs_dirfixdotdot(sip, sdp, tdp);
		if (error)
			goto bad;
	}

	/*
	 * Fill in entry data.
	 */
	slotp->ep->d_namlen = namlen;
	(void) strncpy(slotp->ep->d_name, namep, (size_t)((namlen + 4) & ~3));
	slotp->ep->d_ino = sip->i_number;
	dnlc_enter(ITOV(tdp), namep, ITOV(sip), NOCRED);

	/*
	 * Write out the directory entry.
	 */

	if (op == DE_MKDIR)
		error = fbwrite(slotp->fbp);
	else
		error = fbawrite(slotp->fbp);
	slotp->fbp = NULL;
	if (error)
		return (error);		/* XXX - already fixed dotdot? */

	/*
	 * Mark the directory inode to reflect the changes.
	 * Truncate the directory to chop off blocks of empty entries.
	 */
	tdp->i_flag |= IUPD|ICHG;
	tdp->i_diroff = 0;
	if (slotp->endoff && slotp->endoff < tdp->i_size)
		(void) ufs_itrunc(tdp, (u_long)slotp->endoff);
	return (0);

bad:
	/*
	 * Clear out entry prepared by dirprepareent.
	 */
	slotp->ep->d_ino = 0;
	(void) fbwrite(slotp->fbp);	/* XXX - is this right? */
	slotp->fbp = NULL;
	return (error);
}

/*
 * Prepare a directory slot to receive an entry.
 */
STATIC
dirprepareentry(dp, slotp)
	register struct inode *dp;	/* directory we are working in */
	register struct slot *slotp;	/* available slot info */
{
	register u_short slotfreespace;
	register u_short dsize;
	register int loc;
	register struct direct *ep, *nep;
	char *dirbuf;
	off_t entryend;
	int err;

	/*
	 * If we didn't find a slot, then indicate that the
	 * new slot belongs at the end of the directory.
	 * If we found a slot, then the new entry can be
	 * put at slotp->offset.
	 */
	entryend = slotp->offset + slotp->size;
	if (slotp->status == NONE) {
		if (slotp->offset & (DIRBLKSIZ - 1))
			cmn_err(CE_PANIC, "dirprepareentry: new block");
		ASSERT(DIRBLKSIZ <= dp->i_fs->fs_fsize);
		/*
		 * Allocate the new block.
		 */
		err = BMAPALLOC(dp, (daddr_t)lblkno(dp->i_fs, slotp->offset),
		    (int)(blkoff(dp->i_fs, slotp->offset) + DIRBLKSIZ));
		if (err)
			return (err);
		dp->i_size = entryend;
		dp->i_flag |= IUPD|ICHG;
	} else if (entryend > dp->i_size) {
		/*
		 * Adjust directory size, if needed. This should never
		 * push the size past a new multiple of DIRBLKSIZ.
		 * This is an artifact of the old (4.2BSD) way of initializing
		 * directory sizes to be less than DIRBLKSIZ.
		 */
		dp->i_size = roundup(entryend, DIRBLKSIZ);
		dp->i_flag |= IUPD|ICHG;
	}

	/*
	 * Get the block containing the space for the new directory entry.
	 */
	err = blkatoff(dp, slotp->offset, (char **)&slotp->ep, &slotp->fbp);
	if (err) {
		return (err);
	}

	ep = slotp->ep;
	switch (slotp->status) {
	case NONE:
		/*
		 * No space in the directory. slotp->offset will be on a
		 * directory block boundary and we will write the new entry
		 * into a fresh block.
		 */
		ep->d_reclen = DIRBLKSIZ;
		break;

	case FOUND:
	case COMPACT:
		/*
		 * Found space for the new entry
		 * in the range slotp->offset to slotp->offset + slotp->size
		 * in the directory.  To use this space, we have to compact
		 * the entries located there, by copying them together towards
		 * the beginning of the block, leaving the free space in
		 * one usable chunk at the end.
		 */
		dirbuf = (char *)ep;
		dsize = DIRSIZ(ep);
		slotfreespace = ep->d_reclen - dsize;
		for (loc = ep->d_reclen; loc < slotp->size; ) {
			nep = (struct direct *)(dirbuf + loc);
			if (ep->d_ino) {
				/* trim the existing slot */
				ep->d_reclen = dsize;
				ep = (struct direct *)((char *)ep + dsize);
			} else {
				/* overwrite; nothing there; header is ours */
				slotfreespace += dsize;
			}
			dsize = DIRSIZ(nep);
			slotfreespace += nep->d_reclen - dsize;
			loc += nep->d_reclen;
			bcopy((caddr_t)nep, (caddr_t)ep, (unsigned)dsize);
		}
		/*
		 * Update the pointer fields in the previous entry (if any).
		 * At this point, ep is the last entry in the range
		 * slotp->offset to slotp->offset + slotp->size.
		 * Slotfreespace is the now unallocated space after the
		 * ep entry that resulted from copying entries above.
		 */
		if (ep->d_ino == 0) {
			ep->d_reclen = slotfreespace + dsize;
		} else {
			ep->d_reclen = dsize;
			ep = (struct direct *)((char *)ep + dsize);
			ep->d_reclen = slotfreespace;
		}
		break;

	default:
		cmn_err(CE_PANIC, "dirprepareentry: invalid slot status");
	}
	slotp->ep = ep;
	return (0);
}

/*
 * Allocate and initialize a new inode that will go into directory tdp.
 */
STATIC int
ufs_dirmakeinode(tdp, ipp, vap, op, cr)
	struct inode *tdp;
	struct inode **ipp;
	register struct vattr *vap;
	enum de_op op;
	struct cred *cr;
{
	struct inode *ip;
	register enum vtype type;
	int imode;			/* mode and format as in inode */
	ino_t ipref;
	int error;

	ASSERT(vap != NULL);
	ASSERT(op == DE_CREATE || op == DE_MKDIR);
	ASSERT((vap->va_mask & (AT_TYPE|AT_MODE)) == (AT_TYPE|AT_MODE));
	/*
	 * Allocate a new inode.
	 */
	type = vap->va_type;
	if (type == VDIR) {
		ipref = dirpref(tdp->i_fs);
	} else {
		ipref = tdp->i_number;
	}
	imode = MAKEIMODE(type, vap->va_mode);
	error = ufs_ialloc(tdp, ipref, imode, &ip);
	if (error)
		return (error);
#ifdef QUOTA
	if (ip->i_dquot != NULL)
		cmn_err(CE_PANIC, "ufs_direnter: dquot");
#endif
	ip->i_flag |= IACC|IUPD|ICHG;
	ip->i_mode = imode;
	ip->i_ic.ic_eftflag = (ulong)EFT_MAGIC;
	if (type == VBLK || type == VCHR || type == VXNAM) {
		ip->i_vnode.v_rdev = ip->i_rdev = vap->va_rdev;
	}
	ip->i_vnode.v_type = type;
	if (type == VDIR && op == DE_MKDIR) {
		ip->i_nlink = 2;	/* anticipating a call to dirmakedirect */
	} else {
		ip->i_nlink = 1;
	}
	ip->i_uid = cr->cr_uid;
	/*
	 * To determine the group-id of the created file:
	 *  1) If the gid is set in the attribute list (non-Sun & pre-4.0
	 *     clients are not likely to set the gid), then use it if
	 *     the process is super-user, belongs to the target group,
	 *     or the group is the same as the parent directory.
	 *  2) If the filesystem was not mounted with the Old-BSD-compatible
	 *     GRPID option, and the directory's set-gid bit is clear,
	 *     then use the process's gid.
	 *  3) Otherwise, set the group-id to the gid of the parent directory.
	 */
	if ((vap->va_mask & AT_GID) &&
	    ((cr->cr_uid == 0) || (vap->va_gid == tdp->i_gid) ||
	    groupmember(vap->va_gid, cr))) {
		/*
		 * XXX - is this only the case when a 4.0 NFS client, or a
		 * client derived from that code, makes a call over the wire?
		 */
		ip->i_gid = vap->va_gid;
	} else {
		if (tdp->i_mode & ISGID
#if 0
		    || (tdp->i_vnode.v_vfsp->vfs_flag & VFS_GRPID)
#endif
		    )
			ip->i_gid = tdp->i_gid;
		else
			ip->i_gid = cr->cr_gid;
	}

	/*
	 * If we're creating a directory, and the parent directory has the
	 * set-GID bit set, set it on the new directory.
	 * Otherwise, if the user is neither super-user nor a member of the
	 * file's new group, clear the file's set-GID bit.
	 */
	if (tdp->i_mode & ISGID && type == VDIR)
		ip->i_mode |= ISGID;
	else {
		if ((ip->i_mode & ISGID) && !groupmember((uid_t)ip->i_gid, cr)
		    && cr->cr_uid != 0)
			ip->i_mode &= ~ISGID;
	}
#ifdef QUOTA
	ip->i_dquot = getinoquota(ip);
#endif
	/*
	 * Make sure inode goes to disk before directory data and entries
	 * pointing to it.
	 * Then unlock it, since nothing points to it yet.
	 */
	ufs_iupdat(ip, 1);

	if (op == DE_MKDIR) {
		error = ufs_dirmakedirect(ip, tdp);
	}
	if (error) {
		/* Throw away inode we just allocated. */
		ip->i_nlink = 0;
		ip->i_flag |= ICHG;
		ufs_iput(ip);
	} else {
		ufs_iunlock(ip);
		*ipp = ip;
	}
	return (error);
}

/*
 * Write a prototype directory into the empty inode ip, whose parent is dp.
 */
STATIC int
ufs_dirmakedirect(ip, dp)
	register struct inode *ip;		/* new directory */
	register struct inode *dp;		/* parent directory */
{
	int error;
	register struct dirtemplate *dirp;
	struct fbuf *fbp;

	/*
	 * Allocate space for the directory we're creating.
	 */
	error = BMAPALLOC(ip, (daddr_t)0, DIRBLKSIZ);
	if (error)
		return (error);
	ASSERT(DIRBLKSIZ <= ip->i_fs->fs_fsize);
	ip->i_size = DIRBLKSIZ;
	ip->i_flag |= IUPD|ICHG;
	/*
	 * Update the tdp link count and write out the change.
	 * This reflects the ".." entry we'll soon write.
	 */
	dp->i_nlink++;
	dp->i_flag |= ICHG;
	ufs_iupdat(dp, 1);
	/*
	 * Initialize directory with "."
	 * and ".." from static template.
	 */
	error = fbread(ITOV(ip), 0, (u_int)ip->i_fs->fs_fsize, S_OTHER, &fbp);
	if (error)
		return (error);
	dirp = (struct dirtemplate *)fbp->fb_addr;
	/*
	 * Now initialize the directory we're creating
	 * with the "." and ".." entries.
	 */
	*dirp = mastertemplate;			/* structure assignment */
	dirp->dot_ino = ip->i_number;
	dirp->dotdot_ino = dp->i_number;
	error = fbwrite(fbp);
	return (error);
}

/*
 * Delete a directory entry.  If oip is nonzero the entry is checked
 * to make sure it still reflects oip.
 */
int
ufs_dirremove(dp, namep, oip, cdir, op, cr)
	register struct inode *dp;
	char *namep;
	struct inode *oip;
	struct vnode *cdir;
	enum dr_op op;
	struct cred *cr;
{
	register struct direct *ep;
	struct direct *pep;
	struct inode *ip;
	int namlen;
	struct slot slot;
	int error = 0;
	int dotflag;

	namlen = strlen(namep);
	if (namlen == 0)
		cmn_err(CE_PANIC, "ufs_dirremove");
	/*
	 * return error when removing . and ..
	 */
	if (namep[0] == '.') {
		if (namlen == 1)
			return (EINVAL);
		else if (namlen == 2 && namep[1] == '.')
			{
			return (EEXIST);	/* SIGH should be ENOTEMPTY */
			}
	}

	ip = NULL;
	slot.fbp = NULL;
	ufs_ilock(dp);
	/*
	 * Check accessibility of directory.
	 */
	if ((dp->i_mode & IFMT) != IFDIR) {
		error = ENOTDIR;
		goto out;
	}

	/*
	 * Execute access is required to search the directory.
	 * Access for write is interpreted as allowing
	 * deletion of files in the directory.
	 */
	if (error = ufs_iaccess(dp, IEXEC|IWRITE, cr))
		goto out;

	slot.status = FOUND;	/* don't need to look for empty slot */
	if (error = ufs_dircheckforname(dp, namep, namlen, &slot, &ip))
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
	 * There used to be a check here to make sure you are not removing a
	 * mounted on dir.  This was no longer correct because ufs_iget() does
	 * not cross mount points anymore so the the i_dev fields in the inodes
	 * pointed to by ip and dp will never be different.  There does need
	 * to be a check here though, to eliminate the race between mount and
	 * rmdir (It can also be a race between mount and unlink, if your
	 * kernel allows you to unlink a directory.)
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
	    && (error = ufs_iaccess(ip, IWRITE, cr)))
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
		else if (!ufs_dirempty(ip, dp->i_number, &dotflag))
		{
			error = EEXIST;	/* SIGH should be ENOTEMPTY */
		}
		if (error)
			goto out;
	} else if (op == DR_REMOVE)  {
		/*
		 * unlink(2) requires a different check: allow only
		 * the super-user to unlink a directory.
		 */
		struct vnode *vp = ITOV(ip);
		if (vp->v_type == VDIR && !suser(cr)) {
			error = EPERM;
			goto out;
		}
		ufs_dirempty(ip, dp->i_number, &dotflag);
	}
	/*
	 * Remove the cache'd entry, if any.
	 */
	dnlc_remove(ITOV(dp), namep);
	/*
	 * If the entry isn't the first in the directory, we must reclaim
	 * the space of the now empty record by adding the record size
	 * to the size of the previous entry.
	 */
	ep = slot.ep;
	if ((slot.offset & (DIRBLKSIZ - 1)) == 0) {
		/*
		 * First entry in block: set d_ino to zero.
		 */
		ep->d_ino = 0;
	} else {
		/*
		 * Collapse new free space into previous entry.
		 */
		pep = (struct direct *)((char *)ep - slot.size);
		pep->d_reclen += ep->d_reclen;
	}
	error = fbwrite(slot.fbp);
	slot.fbp = NULL;
	dp->i_flag |= IUPD|ICHG;
	ip->i_flag |= ICHG;
	if (error)
		goto out;
	/*
	 * Now dispose of the inode.
	 */
	if (ip->i_nlink > 0) {
		if (op == DR_RMDIR
		   && (ip->i_mode & IFMT) == IFDIR) {
			/*
			* If link count equals 2,
			* decrement by 2 because we're trashing the "."
			* Clear the inode, but there may be other hard
			* links so don't free the inode.
			* Decrement the dp linkcount because we're
			* trashing the ".." entry.
			*/
			if (dotflag & DOT) {
				ip->i_nlink -= 2;
				dnlc_remove(ITOV(ip), ".");
			} else
				ip->i_nlink--;
			if (dotflag & DOTDOT) {
				dp->i_nlink--;
				dnlc_remove(ITOV(ip), "..");
			}
			if (ITOV(ip)->v_count > 1 && ip->i_nlink <= 0)
				ufs_rdwri(UIO_WRITE, ip,
				    (caddr_t)&mastertemplate,
				    min(sizeof(mastertemplate), ip->i_size),
				    (off_t)0, UIO_SYSSPACE,
				    (int *)0);

		} else
			ip->i_nlink--;
	}
	if (ip->i_nlink < 0)
		ip->i_nlink = 0;

out:
	if (ip)
		ufs_iput(ip);
	if (slot.fbp)
		fbrelse(slot.fbp, S_OTHER);
	ufs_iunlock(dp);
	return (error);
}

/*
 * Return buffer with contents of block "offset"
 * from the beginning of directory "ip".  If "res"
 * is non-zero, fill it in with a pointer to the
 * remaining space in the directory.
 */
STATIC
blkatoff(ip, offset, res, fbpp)
	struct inode *ip;
	off_t offset;
	char **res;
	struct fbuf **fbpp;
{
	register struct fs *fs;
	struct fbuf *fbp;
	daddr_t lbn;
	u_int bsize;
	int err;

	fs = ip->i_fs;
	lbn = lblkno(fs, offset);
	bsize = blksize(fs, ip, lbn);
	err = fbread(ITOV(ip), (long)(offset & fs->fs_bmask), bsize, S_OTHER,
	    &fbp);
	if (err) {
		*fbpp = (struct fbuf *)NULL;
		return (err);
	}
	if (res)
		*res = fbp->fb_addr + blkoff(fs, offset);
	*fbpp = fbp;
	return (0);
}

/*
 * Do consistency checking:
 *	record length must be multiple of 4
 *	entry must fit in rest of its DIRBLKSIZ block
 *	record must be large enough to contain entry
 *	name is not longer than MAXNAMLEN
 * if dirchk is on:
 *	name must be as long as advertised, and null terminated
 * NOTE: record length must not be zero (should be checked previously).
 */
STATIC
dirmangled(dp, ep, entryoffsetinblock, offset)
	register struct inode *dp;
	register struct direct *ep;
	int entryoffsetinblock;
	off_t offset;
{
	register int i;

	i = DIRBLKSIZ - (entryoffsetinblock & (DIRBLKSIZ - 1));
	if ((ep->d_reclen & 0x3) != 0 || (int)ep->d_reclen > i ||
	    (u_int)ep->d_reclen < DIRSIZ(ep) || ep->d_namlen > MAXNAMLEN ||
	    dirchk && dirbadname(ep->d_name, (int)ep->d_namlen)) {
		dirbad(dp, "mangled entry", offset);
		return (1);
	}
	return (0);
}

STATIC void
dirbad(ip, how, offset)
	struct inode *ip;
	char *how;
	off_t offset;
{

	cmn_err(CE_NOTE, "%s: bad dir ino %d at offset %d: %s\n",
	    ip->i_fs->fs_fsmnt, ip->i_number, offset, how);
	return;
}

STATIC
dirbadname(sp, l)
	register char *sp;
	register int l;
{

	while (l--) {			/* check for nulls */
		if (*sp++ == '\0') {
			return (1);
		}
	}
	return (*sp);			/* check for terminating null */
}

/*
 * Check if a directory is empty or not.
 *
 * Using a struct dirtemplate here is not precisely
 * what we want, but better than using a struct direct.
 *
 * N.B.: does not handle corrupted directories.
 */
STATIC
ufs_dirempty(ip, parentino, dotflagp)
	register struct inode *ip;
	ino_t parentino;
	int *dotflagp;
{
	register off_t off;
	struct dirtemplate dbuf;
	register struct direct *dp = (struct direct *)&dbuf;
	int err, count;
#define	MINDIRSIZ (sizeof (struct dirtemplate) / 2)

	*dotflagp = 0;
	for (off = 0; off < ip->i_size; off += dp->d_reclen) {
		err = ufs_rdwri(UIO_READ, ip, (caddr_t)dp, (int)MINDIRSIZ,
		    off, UIO_SYSSPACE, &count);
		/*
		 * Since we read MINDIRSIZ, residual must
		 * be 0 unless we're at end of file.
		 */
		if (err || count != 0 || dp->d_reclen == 0)
			return (0);
		/* skip empty entries */
		if (dp->d_ino == 0)
			continue;
		/* accept only "." and ".." */
		if (dp->d_namlen > 2)
			return (0);
		if (dp->d_name[0] != '.')
			return (0);
		/*
		 * At this point d_namlen must be 1 or 2.
		 * 1 implies ".", 2 implies ".." if second
		 * char is also "."
		 */
		if (dp->d_namlen == 1) {
			*dotflagp |= DOT;
			continue;
		}
		/* don't check parentino. link will change it */
		if (dp->d_name[1] == '.') {
			*dotflagp |= DOTDOT;
			continue;
		}
		return (0);
	}
	return (1);
}

#define	RENAME_IN_PROGRESS	0x01
#define	RENAME_WAITING		0x02

/*
 * Check if source directory is in the path of the target directory.
 * Target is supplied locked, source is unlocked.
 * The target is always relocked before returning.
 */
STATIC int
ufs_dircheckpath(source, target)
	struct inode *source;
	struct inode *target;
{
	struct fbuf *fbp;
	struct dirtemplate *dirp;
	register struct inode *ip;
	struct inode *tip;
	static char serialize_flag = 0;
	ino_t dotdotino;
	int err = 0;

	/*
	 * If two renames of directories were in progress at once, the partially
	 * completed work of one dircheckpath could be invalidated by the other
	 * rename.  To avoid this, all directory renames in the system are
	 * serialized.
	 */
	while (serialize_flag & RENAME_IN_PROGRESS) {
		serialize_flag |= RENAME_WAITING;
		(void) sleep((caddr_t)&serialize_flag, PINOD);
	}
	serialize_flag = RENAME_IN_PROGRESS;
	ip = target;
	if (ip->i_number == source->i_number) {
		err = EINVAL;
		goto out;
	}
	if (ip->i_number == UFSROOTINO)
		goto out;
	/*
	 * Search back through the directory tree, using the ".." entries.
	 * Fail any attempt to move a directory into an ancestor directory.
	 */
	fbp = NULL;
	for (;;) {
		if (((ip->i_mode & IFMT) != IFDIR) || ip->i_nlink == 0
		    || ip->i_size < sizeof (struct dirtemplate)) {
			dirbad(ip, "bad size, unlinked or not dir", (off_t)0);
			err = ENOTDIR;
			break;
		}
		if (err = blkatoff(ip, (off_t)0, (char **)&dirp, &fbp))
			break;
		if (dirp->dotdot_namlen != 2 ||
		    dirp->dotdot_name[0] != '.' ||
		    dirp->dotdot_name[1] != '.') {
			dirbad(ip, "mangled .. entry", (off_t)0);
			err = ENOTDIR;		/* Sanity check */
			break;
		}
		dotdotino = dirp->dotdot_ino;
		if (dotdotino == source->i_number) {
			err = EINVAL;
			break;
		}
		if (dotdotino == UFSROOTINO)
			break;
		if (fbp) {
			fbrelse(fbp, S_OTHER);
			fbp = NULL;
		}
		if (ip != target)
			ufs_iput(ip);
		else
			ufs_iunlock(ip);
		/*
		 * i_dev and i_fs are still valid after ufs_iput
		 * This is a race to get ".." just like ufs_dirlook.
		 */
		if (err = ufs_iget(ip->i_vnode.v_vfsp, ip->i_fs, dotdotino, &tip)) {
			ip = NULL;
			break;
		}
		ip = tip;
	}
	if (fbp) {
		fbrelse(fbp, S_OTHER);
	}
out:
	/*
	 * Unserialize before relocking target to avoid a race.
	 */
	if (serialize_flag & RENAME_WAITING)
		wakeprocs((caddr_t)&serialize_flag, PRMPT);
	serialize_flag = 0;

	if (ip) {
		if (ip != target) {
			ufs_iput(ip);
			/*
			 * Relock target and make sure it has not gone away
			 * while it was unlocked.
			 */
			ufs_ilock(target);
			if ((err == 0) && (target->i_nlink == 0)) {
				err = ENOENT;
			}
		}
	}
	return (err);
}
