/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/forms/delform.c	1.11.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "form.h"

#if	defined(__STDC__)
static int		_delform ( char * );
#else
static int		_delform();
#endif

/**
 ** delform()
 **/

int
#if	defined(__STDC__)
delform (
	char *			name
)
#else
delform (name)
	char			*name;
#endif
{
	long			lastdir;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(NAME_ALL, name)) {
		lastdir = -1;
		while ((name = next_dir(Lp_A_Forms, &lastdir)))
			if (_delform(name) == -1)
				return (-1);
		return (0);
	} else
		return (_delform(name));
}

/**
 ** _delform()
 **/

static int
#if	defined(__STDC__)
_delform (
	char *			name
)
#else
_delform (name)
	char			*name;
#endif
{
	register char		*path;

#define RMFILE(X)	if (!(path = getformfile(name, X))) \
				return (-1); \
			if (rmfile(path) == -1) { \
				Free (path); \
				return (-1); \
			} \
			Free (path)
	RMFILE (DESCRIBE);
	RMFILE (COMMENT);
	RMFILE (ALIGN_PTRN);
	RMFILE (ALLOWFILE);
	RMFILE (DENYFILE);

	delalert (Lp_A_Forms, name);

	if (!(path = getformfile(name, (char *)0)))
		return (-1);
	if (Rmdir(path) == -1) {
		Free (path);
		return (-1);
	}
	Free (path);

	return (0);
}
