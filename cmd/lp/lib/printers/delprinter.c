/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/printers/delprinter.c	1.10.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "printers.h"

#if	defined(__STDC__)
static int		_delprinter ( char * );
#else
static int		_delprinter();
#endif

/**
 ** delprinter()
 **/

int
#if	defined(__STDC__)
delprinter (
	char *			name
)
#else
delprinter (name)
	char			*name;
#endif
{
	long			lastdir;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (!Lp_A_Printers || !Lp_A_Interfaces) {
		getadminpaths (LPUSER);
		if (!Lp_A_Printers || !Lp_A_Interfaces)
			return (0);
	}

	if (STREQU(NAME_ALL, name)) {
		lastdir = -1;
		while ((name = next_dir(Lp_A_Printers, &lastdir)))
			if (_delprinter(name) == -1)
				return (-1);
		return (0);
	} else
		return (_delprinter(name));
}

/**
 ** _delprinter()
 **/

static int
#if	defined(__STDC__)
_delprinter (
	char *			name
)
#else
_delprinter (name)
	char			*name;
#endif
{
	register char		*path;

#define RMFILE(X)	if (!(path = getprinterfile(name, X))) \
				return (-1); \
			if (rmfile(path) == -1) { \
				Free (path); \
				return (-1); \
			} \
			Free (path)
	RMFILE (COMMENTFILE);
	RMFILE (CONFIGFILE);
	RMFILE (FALLOWFILE);
	RMFILE (FDENYFILE);
	RMFILE (UALLOWFILE);
	RMFILE (UDENYFILE);
	RMFILE (STATUSFILE);

	delalert (Lp_A_Printers, name);

	if (!(path = makepath(Lp_A_Interfaces, name, (char *)0)))
		return (-1);
	if (rmfile(path) == -1) {
		Free (path);
		return (-1);
	}
	Free (path);

	if (!(path = getprinterfile(name, (char *)0)))
		return (-1);
	if (Rmdir(path) == -1) {
		Free (path);
		return (-1);
	}
	Free (path);

	return (0);
}
