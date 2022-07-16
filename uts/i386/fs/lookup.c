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

#ident	"@(#)kern-fs:lookup.c	1.3"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/uio.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/pathname.h"
#include "sys/conf.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/disp.h"

/*
 * Lookup the user file name,
 * Handle allocation and freeing of pathname buffer, return error.
 */
int
lookupname(fnamep, seg, followlink, dirvpp, compvpp)
	char *fnamep;			/* user pathname */
	enum uio_seg seg;		/* addr space that name is in */
	enum symfollow followlink;	/* follow sym links */
	vnode_t **dirvpp;		/* ret for ptr to parent dir vnode */
	vnode_t **compvpp;		/* ret for ptr to component vnode */
{
	struct pathname lookpn;
	register int error;

	if (error = pn_get(fnamep, seg, &lookpn))
		return error;
	error = lookuppn(&lookpn, followlink, dirvpp, compvpp);
	pn_free(&lookpn);
	return error;
}

/*
 * Starting at current directory, translate pathname pnp to end.
 * Leave pathname of final component in pnp, return the vnode
 * for the final component in *compvpp, and return the vnode
 * for the parent of the final component in dirvpp.
 *
 * This is the central routine in pathname translation and handles
 * multiple components in pathnames, separating them at /'s.  It also
 * implements mounted file systems and processes symbolic links.
 */
int
lookuppn(pnp, followlink, dirvpp, compvpp)
	register struct pathname *pnp;	/* pathname to lookup */
	enum symfollow followlink;	/* (don't) follow sym links */
	vnode_t **dirvpp;		/* ptr for parent vnode */
	vnode_t **compvpp;		/* ptr for entry vnode */
{
	register vnode_t *vp;	/* current directory vp */
	register vnode_t *cvp;	/* current component vp */
	vnode_t *tvp;		/* addressable temp ptr */
	char component[MAXNAMELEN];	/* buffer for component (incl null) */
	register int error;
	register int nlink;
	int lookup_flags;

	sysinfo.namei++;
	nlink = 0;
	cvp = NULL;
	lookup_flags = dirvpp ? LOOKUP_DIR : 0;

	/*
	 * Start at current directory.
	 */
	vp = u.u_cdir;
	VN_HOLD(vp);

begin:
	/*
	 * Disallow the empty path name.
	 */
	if (pnp->pn_pathlen == 0) {
		error = ENOENT;
		goto bad;
	}

	/*
	 * Each time we begin a new name interpretation (e.g.
	 * when first called and after each symbolic link is
	 * substituted), we allow the search to start at the
	 * root directory if the name starts with a '/', otherwise
	 * continuing from the current directory.
	 */
	if (pn_peekchar(pnp) == '/') {
		VN_RELE(vp);
		pn_skipslash(pnp);
		vp = u.u_rdir ? u.u_rdir : rootdir;
		VN_HOLD(vp);
	}

	/*
	 * Eliminate any trailing slashes in the pathname.
	 */
	pn_fixslash(pnp);

next:
	PREEMPT();

	/*
	 * Make sure we have a directory.
	 */
	if (vp->v_type != VDIR) {
		error = ENOTDIR;
		goto bad;
	}

	/*
	 * Process the next component of the pathname.
	 */
	if (error = pn_stripcomponent(pnp, component))
		goto bad;

	/*
	 * Check for degenerate name (e.g. / or "")
	 * which is a way of talking about a directory,
	 * e.g. "/." or ".".
	 */
	if (component[0] == 0) {
		/*
		 * If the caller was interested in the parent then
		 * return an error since we don't have the real parent.
		 */
		if (dirvpp != NULL) {
			VN_RELE(vp);
			return EINVAL;
		}
		(void) pn_set(pnp, ".");
		if (compvpp != NULL)
			*compvpp = vp;
		else
			VN_RELE(vp);
		return 0;
	}

	/*
	 * Handle "..": two special cases.
	 * 1. If we're at the root directory (e.g. after chroot)
	 *    then ignore ".." so we can't get out of this subtree.
	 * 2. If this vnode is the root of a mounted file system,
	 *    then replace it with the vnode that was mounted on
	      so that we take the ".." in the other file system.
	 */
	if (strcmp(component, "..") == 0) {
checkforroot:
		if (VN_CMP(vp, u.u_rdir) || VN_CMP(vp, rootdir)) {
			cvp = vp;
			VN_HOLD(cvp);
			goto skip;
		}
		if (vp->v_flag & VROOT) {
			cvp = vp;
			vp = vp->v_vfsp->vfs_vnodecovered;
			VN_HOLD(vp);
			VN_RELE(cvp);
			goto checkforroot;
		}
	}

	/*
	 * Perform a lookup in the current directory.
	 */
	error = VOP_LOOKUP(vp, component, &tvp, pnp, lookup_flags,
	  u.u_rdir ? u.u_rdir : rootdir, u.u_cred);
	cvp = tvp;
	if (error) {
		cvp = NULL;
		/*
		 * On error, return hard error if
		 * (a) we're not at the end of the pathname yet, or
		 * (b) the caller didn't want the parent directory, or
		 * (c) we failed for some reason other than a missing entry.
		 */
		if (pn_pathleft(pnp) || dirvpp == NULL || error != ENOENT)
			goto bad;
		pn_setlast(pnp);
		*dirvpp = vp;
		if (compvpp != NULL)
			*compvpp = NULL;
		return 0;
	}

	/*
	 * Traverse mount points.
	 */
	if (cvp->v_vfsmountedhere != NULL) {
		tvp = cvp;
		if ((error = traverse(&tvp)) != 0)
			goto bad;
		cvp = tvp;
	}

	/*
	 * If we hit a symbolic link and there is more path to be
	 * translated or this operation does not wish to apply
	 * to a link, then place the contents of the link at the
	 * front of the remaining pathname.
	 */
	if (cvp->v_type == VLNK && (followlink == FOLLOW || pn_pathleft(pnp))) {
		struct pathname linkpath;

		if (++nlink > MAXSYMLINKS) {
			error = ELOOP;
			goto bad;
		}
		pn_alloc(&linkpath);
		if (error = pn_getsymlink(cvp, &linkpath, u.u_cred)) {
			pn_free(&linkpath);
			goto bad;
		}
		if (pn_pathleft(&linkpath) == 0)
			(void) pn_set(&linkpath, ".");
		error = pn_insert(pnp, &linkpath);	/* linkpath before pn */
		pn_free(&linkpath);
		if (error)
			goto bad;
		VN_RELE(cvp);
		cvp = NULL;
		goto begin;
	}

skip:
	/*
	 * If no more components, return last directory (if wanted) and
	 * last component (if wanted).
	 */
	if (pn_pathleft(pnp) == 0) {
		pn_setlast(pnp);
		if (dirvpp != NULL) {
			/*
			 * Check that we have the real parent and not
			 * an alias of the last component.
			 */
			if (VN_CMP(vp, cvp)) {
				VN_RELE(vp);
				VN_RELE(cvp);
				return EINVAL;
			}
			*dirvpp = vp;
		} else
			VN_RELE(vp);
		if (compvpp != NULL)
			*compvpp = cvp;
		else
			VN_RELE(cvp);
		return 0;
	}

	/*
	 * Skip over slashes from end of last component.
	 */
	pn_skipslash(pnp);

	/*
	 * Searched through another level of directory:
	 * release previous directory handle and save new (result
	 * of lookup) as current directory.
	 */
	VN_RELE(vp);
	vp = cvp;
	cvp = NULL;
	goto next;

bad:
	/*
	 * Error.  Release vnodes and return.
	 */
	if (cvp)
		VN_RELE(cvp);
	VN_RELE(vp);
	return error;
}

/*
 * Traverse a mount point.  Routine accepts a vnode pointer as a reference
 * parameter and performs the indirection, releasing the original vnode.
 */
int
traverse(cvpp)
	vnode_t **cvpp;
{
	register struct vfs *vfsp;
	register int error = 0;
	register vnode_t *cvp;
	vnode_t *tvp;

	cvp = *cvpp;

	/*
	 * If this vnode is mounted on, then we transparently indirect
	 * to the vnode which is the root of the mounted file system.
	 * Before we do this we must check that an unmount is not in
	 * progress on this vnode.
	 */
mloop:
	while ((vfsp = cvp->v_vfsmountedhere) != NULL) {
		if (vfsp->vfs_flag & VFS_MLOCK) {
			vfsp->vfs_flag |= VFS_MWAIT;
			if (sleep((caddr_t)vfsp, PVFS|PCATCH))
				return EINTR;
			goto mloop;
		}
		if (error = VFS_ROOT(vfsp, &tvp))
			break;
		VN_RELE(cvp);
		cvp = tvp;
	}

	*cvpp = cvp;
	return error;
}
