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

#ident	"@(#)kern-fs:pathname.c	1.3"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/debug.h"
#include "sys/systm.h"
#include "sys/uio.h"
#include "sys/errno.h"
#include "sys/pathname.h"
#include "sys/kmem.h"
#include "sys/cred.h"
#include "sys/vnode.h"

/*
 * Pathname utilities.
 *
 * In translating file names we copy each argument file
 * name into a pathname structure where we operate on it.
 * Each pathname structure can hold MAXPATHLEN characters
 * including a terminating null, and operations here support
 * allocating and freeing pathname structures, fetching
 * strings from user space, getting the next character from
 * a pathname, combining two pathnames (used in symbolic
 * link processing), and peeling off the first component
 * of a pathname.
 */

STATIC char *pn_freelist;
STATIC int pnbufs = 0;

/*
 * Allocate contents of pathname structure.  Structure is typically
 * an automatic variable in calling routine for convenience.
 *
 * May sleep in the call to kmem_alloc() and so must not be called
 * from interrupt level.
 */
void
pn_alloc(pnp)
	register struct pathname *pnp;
{

	if (pn_freelist) {
		pnp->pn_buf = pn_freelist;
		pn_freelist = *(char **) pnp->pn_buf;
	} else {
		pnp->pn_buf = (char *)kmem_alloc((u_int)MAXPATHLEN, KM_SLEEP);
		pnbufs++;
	}
	pnp->pn_path = (char *)pnp->pn_buf;
	pnp->pn_pathlen = 0;
}

/*
 * Free pathname resources.
 */
void
pn_free(pnp)
	register struct pathname *pnp;
{

	/* kmem_free((caddr_t)pnp->pn_buf, (u_int)MAXPATHLEN); */
	*(char **) pnp->pn_buf = pn_freelist;
	pn_freelist = pnp->pn_buf;
	pnp->pn_buf = 0;
}

/*
 * Pull a path name from user or kernel space.  Allocates storage (via
 * pn_alloc()) to hold it.
 */
int
pn_get(str, seg, pnp)
	register char *str;
	enum uio_seg seg;
	register struct pathname *pnp;
{
	register int error;

	pn_alloc(pnp);
	if (seg == UIO_USERSPACE)
		error =
		    copyinstr(str, pnp->pn_path, MAXPATHLEN, &pnp->pn_pathlen);
	else
		error =
		    copystr(str, pnp->pn_path, MAXPATHLEN, &pnp->pn_pathlen);
	pnp->pn_pathlen--;		/* don't count null byte */
	if (error)
		pn_free(pnp);
	return error;
}

/*
 * Set path name to argument string.  Storage has already been allocated
 * and pn_buf points to it.
 *
 * On error, all fields except pn_buf will be undefined.
 */
int
pn_set(pnp, path)
	register struct pathname *pnp;
	register char *path;
{
	register int error;

	pnp->pn_path = pnp->pn_buf;
	error = copystr(path, pnp->pn_path, MAXPATHLEN, &pnp->pn_pathlen);
	pnp->pn_pathlen--;		/* don't count null byte */
	return error;
}

/*
 * Combine two argument path names by putting the second argument before
 * the first in the first's buffer, and freeing the second argument.
 * This isn't very general: it is designed specifically for symbolic
 * link processing.
 */
int
pn_insert(pnp, sympnp)
	register struct pathname *pnp;
	register struct pathname *sympnp;
{

	if (pnp->pn_pathlen + sympnp->pn_pathlen >= MAXPATHLEN)
		return ENAMETOOLONG;
	ovbcopy(pnp->pn_path, pnp->pn_buf + sympnp->pn_pathlen,
	  (u_int)pnp->pn_pathlen);
	bcopy(sympnp->pn_path, pnp->pn_buf, (u_int)sympnp->pn_pathlen);
	pnp->pn_pathlen += sympnp->pn_pathlen;
	pnp->pn_buf[pnp->pn_pathlen] = '\0';
	pnp->pn_path = pnp->pn_buf;
	return 0;
}

int
pn_getsymlink(vp, pnp, crp)
	vnode_t *vp;
	struct pathname *pnp;
	cred_t *crp;
{
	struct iovec aiov;
	struct uio auio;
	register int error;

	aiov.iov_base = pnp->pn_buf;
	aiov.iov_len = MAXPATHLEN;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = 0;
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_resid = MAXPATHLEN;
	if ((error = VOP_READLINK(vp, &auio, crp)) == 0)
		pnp->pn_pathlen = MAXPATHLEN - auio.uio_resid;
	return error;
}

/*
 * Get next component from a path name and leave in
 * buffer "component" which should have room for
 * MAXNAMELEN bytes (including a null terminator character).
 * If PN_PEEK is set in flags, just peek at the component,
 * i.e., don't strip it out of pnp.
 */
int
pn_getcomponent(pnp, component, flags)
	register struct pathname *pnp;
	register char *component;
	int flags;
{
	register char *cp;
	register int l, n;

	cp = pnp->pn_path;
	l = pnp->pn_pathlen;
	n = MAXNAMELEN - 1;
	while (l > 0 && *cp != '/') {
		if (--n < 0)
			return ENAMETOOLONG;
		*component++ = *cp++;
		--l;
	}
	if ((flags & PN_PEEK) == 0) {
		pnp->pn_path = cp;
		pnp->pn_pathlen = l;
	}
	*component = 0;
	return 0;
}

/*
 * Skip over consecutive slashes in the path name.
 */
void
pn_skipslash(pnp)
	register struct pathname *pnp;
{
	while (pnp->pn_pathlen > 0 && *pnp->pn_path == '/') {
		pnp->pn_path++;
		pnp->pn_pathlen--;
	}
}

/*
 * Sets pn_path to the last component in the pathname, updating
 * pn_pathlen.  If pathname is empty, or degenerate, leaves pn_path
 * pointing at NULL char.  The pathname is explicitly null-terminated
 * so that any trailing slashes are effectively removed.
 */
void
pn_setlast(pnp)
	register struct pathname *pnp;
{
	register char *buf = pnp->pn_buf;
	register char *path = pnp->pn_path + pnp->pn_pathlen - 1;
	register char *endpath;

	while (path > buf && *path == '/')
		--path;
	endpath = path;
	while (path > buf && *path != '/')
		--path;
	if (*path == '/')
		path++;
	*(endpath + 1) = '\0';
	pnp->pn_path = path;
	pnp->pn_pathlen = endpath - path + 1;
}

/*
 * Eliminate any trailing slashes in the pathname.
 */
void
pn_fixslash(pnp)
	register struct pathname *pnp;
{
	register char *buf = pnp->pn_buf;
	register char *path = pnp->pn_path + pnp->pn_pathlen - 1;

	while (path > buf && *path == '/')
		--path;
	*(path + 1) = '\0';
	pnp->pn_pathlen = path - pnp->pn_path + 1;
}
