/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/access/files.c	1.10.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "stdlib.h"

#include "lp.h"

/**
 ** getaccessfile() - BUILD NAME OF ALLOW OR DENY FILE
 **/

char *
#if	defined(__STDC__)
getaccessfile (
	char *			dir,
	char *			name,
	char *			prefix,
	char *			base
)
#else
getaccessfile (dir, name, prefix, base)
	char			*dir,
				*name,
				*prefix,
				*base;
#endif
{
	register char		*parent,
				*file,
				*f;

	/*
	 * It makes no sense talking about the access files if
	 * the directory for the form or printer doesn't exist.
	 */
	parent = makepath(dir, name, (char *)0);
	if (!parent)
		return (0);
	if (Access(parent, F_OK) == -1)
		return (0);

	if (!(f = makestr(prefix, base, (char *)0))) {
		errno = ENOMEM;
		return (0);
	}
	file = makepath(parent, f, (char *)0);
	Free (f);
	Free (parent);

	return (file);
}
