/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/fs/next.c	1.10.3.1"
/* LINTLIBRARY */

#include "string.h"
#include "errno.h"

#include "lp.h"

#if	defined(__STDC__)
static int		is ( char *, char *, unsigned int );
#else
static int		is();
#endif

/**
 ** next_x() - GO TO NEXT ENTRY UNDER PARENT DIRECTORY
 **/

char *
#if	defined(__STDC__)
next_x (
	char *			parent,
	long *			lastdirp,
	unsigned int		what
)
#else
next_x (parent, lastdirp, what)
	char			*parent;
	long			*lastdirp;
	unsigned int		what;
#endif
{
	DIR			*dirp;

	register char		*ret = 0;

	struct dirent		*direntp;


	if (!(dirp = Opendir(parent)))
		return (0);

	if (*lastdirp != -1)
		Seekdir (dirp, *lastdirp);

	do
		direntp = Readdir(dirp);
	while (
		direntp
	     && (
			STREQU(direntp->d_name, ".")
		     || STREQU(direntp->d_name, "..")
		     || !is(parent, direntp->d_name, what)
		)
	);

	if (direntp) {
		if (!(ret = Strdup(direntp->d_name)))
			errno = ENOMEM;
		*lastdirp = Telldir(dirp);
	} else {
		errno = ENOENT;
		*lastdirp = -1;
	}

	Closedir (dirp);

	return (ret);
}

static int
#if	defined(__STDC__)
is (
	char *			parent,
	char *			name,
	unsigned int		what
)
#else
is (parent, name, what)
	char			*parent;
	char			*name;
	unsigned int		what;
#endif
{
	char			*path;

	struct stat		statbuf;

	if (!(path = makepath(parent, name, (char *)0)))
		return (0);
	if (Stat(path, &statbuf) == -1) {
		Free (path);
		return (0);
	}
	Free (path);
	return ((statbuf.st_mode & S_IFMT) == what);
}
