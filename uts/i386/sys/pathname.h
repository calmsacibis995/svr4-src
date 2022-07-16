/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PATHNAME_H
#define _SYS_PATHNAME_H

#ident	"@(#)head.sys:sys/pathname.h	1.7.3.1"
/*
 * Pathname structure.
 * System calls that operate on path names gather the path name
 * from the system call into this structure and reduce it by
 * peeling off translated components.  If a symbolic link is
 * encountered the new path name to be translated is also
 * assembled in this structure.
 *
 * By convention pn_buf is not changed once it's been set to point
 * to the underlying storage; routines which manipulate the pathname
 * do so by changing pn_path and pn_pathlen.  pn_pathlen is redundant
 * since the path name is null-terminated, but is provided to make
 * some computations faster.
 */
typedef struct pathname {
	char	*pn_buf;		/* underlying storage */
	char	*pn_path;		/* remaining pathname */
	u_int	pn_pathlen;		/* remaining length */
} pathname_t;

#define PN_STRIP	0	/* Strip next component from pn */
#define PN_PEEK		1	/* Only peek at next component of pn */
#define pn_peekcomponent(pnp, comp) pn_getcomponent(pnp, comp, PN_PEEK)
#define pn_stripcomponent(pnp, comp) pn_getcomponent(pnp, comp, PN_STRIP)

#define	pn_peekchar(pnp)	((pnp)->pn_pathlen > 0 ? *((pnp)->pn_path) : 0)
#define pn_pathleft(pnp)	((pnp)->pn_pathlen)

extern void	pn_alloc();		/* allocate buffer for pathname */
extern int	pn_get();		/* allocate buffer, copy path into it */
extern int	pn_set();		/* set pathname to string */
extern int	pn_insert();		/* combine two pathnames (symlink) */
extern int	pn_getsymlink();	/* get symlink into pathname */
extern int	pn_getcomponent();	/* get next component of pathname */
extern void	pn_setlast();		/* set pathname to last component */
extern void	pn_skipslash();		/* skip over slashes */
extern void	pn_fixslash();		/* eliminate trailing slashes */
extern void	pn_free();		/* free pathname buffer */

extern int	lookupname();		/* convert name to vnode */
extern int	lookuppn();		/* convert pathname buffer to vnode */
extern int	traverse();		/* traverse a mount point */

#endif	/* _SYS_PATHNAME_H */
