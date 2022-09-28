/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/class/getclass.c	1.9.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "string.h"
#include "errno.h"
#include "sys/types.h"

#include "lp.h"
#include "class.h"

/**
 ** getclass() - READ CLASS FROM TO DISK
 **/

CLASS *
#if	defined(__STDC__)
getclass (
	char *			name
)
#else
getclass (name)
	char			*name;
#endif
{
	static long		lastdir		= -1;

	static CLASS		clsbuf;

	char			*file,
				buf[BUFSIZ];

	FILE			*fp;


	if (!name || !*name) {
		errno = EINVAL;
		return (0);
	}

	/*
	 * Getting ``all''? If so, jump into the directory
	 * wherever we left off.
	 */
	if (STREQU(NAME_ALL, name)) {
		if (!Lp_A_Classes) {
			getadminpaths (LPUSER);
			if (!Lp_A_Classes)
				return (0);
		}
		if (!(name = next_file(Lp_A_Classes, &lastdir)))
			return (0);
	} else
		lastdir = -1;

	/*
	 * Get the class list.
	 */

	if (!(file = getclassfile(name)))
		return (0);

	if (!(fp = open_lpfile(file, "r", 0))) {
		Free (file);
		return (0);
	}
	Free (file);

	if (!(clsbuf.name = Strdup(name))) {
		close_lpfile (fp);
		errno = ENOMEM;
		return (0);
	}

	clsbuf.members = 0;
	while (fgets(buf, BUFSIZ, fp)) {
		buf[strlen(buf) - 1] = 0;
		addlist (&clsbuf.members, buf);
	}
	if (ferror(fp)) {
		int			save_errno = errno;

		freelist (clsbuf.members);
		Free (clsbuf.name);
		close_lpfile (fp);
		errno = save_errno;
		return (0);
	}
	close_lpfile (fp);

	if (!clsbuf.members) {
		Free (clsbuf.name);
		errno = EBADF;
		return (0);
	}

	return (&clsbuf);
}
