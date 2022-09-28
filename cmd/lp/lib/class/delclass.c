/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/class/delclass.c	1.9.3.1"
/* LINTLIBRARY */

#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "sys/types.h"
#include "string.h"

#include "lp.h"
#include "class.h"

#if	defined(__STDC__)
static int		_delclass ( char * );
#else
static int		_delclass();
#endif

/**
 ** delclass() - WRITE CLASS OUT TO DISK
 **/

int
#if	defined(__STDC__)
delclass (
	char *			name
)
#else
delclass (name)
	char			*name;
#endif
{
	long			lastdir;
	static int		_delclass();


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(NAME_ALL, name)) {
		lastdir = -1;
		if (!Lp_A_Classes) {
			getadminpaths (LPUSER);
			if (!Lp_A_Classes)
				return (0);
		}
		while ((name = next_file(Lp_A_Classes, &lastdir)))
			if (_delclass(name) == -1)
				return (-1);
		return (0);
	} else
		return (_delclass(name));
}

/**
 ** _delclass()
 **/

static int
#if	defined(__STDC__)
_delclass (
	char *			name
)
#else
_delclass (name)
	char			*name;
#endif
{
	char			*path;

	if (!(path = getclassfile(name)))
		return (-1);
	if (rmfile(path) == -1) {
		Free (path);
		return (-1);
	}
	Free (path);
	return (0);
}

