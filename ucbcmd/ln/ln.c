/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

#ident	"@(#)ucbln:ln.c	1.1.1.1"

/*
 * ln
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>

struct	stat stb;
int	fflag;		/* force flag set? */
int	sflag;
extern	int errno;

main(argc, argv)
	int argc;
	register char **argv;
{
	register int i, r;

	argc--, argv++;
again:
	if (argc && strcmp(argv[0], "-f") == 0) {
		fflag++;
		argv++;
		argc--;
	}
	if (argc && strcmp(argv[0], "-s") == 0) {
		sflag++;
		argv++;
		argc--;
	}
	if (argc == 0) 
		goto usage;
	else if (argc == 1) {
		argv[argc] = ".";
		argc++;
	}
	if (argc > 2) {
		if (stat(argv[argc-1], &stb) < 0)
			goto usage;
		if ((stb.st_mode&S_IFMT) != S_IFDIR) 
			goto usage;
	}
	r = 0;
	for(i = 0; i < argc-1; i++)
		r |= linkit(argv[i], argv[argc-1]);
	exit(r);
usage:
	(void) fprintf(stderr,
	    "Usage: ln [-s] f1 or ln [-s] f1 f2 or ln [-s] f1 ... fn d1\n");
	exit(1);
}

int	link(), symlink();

linkit(from, to)
	char *from, *to;
{
	char destname[MAXPATHLEN + 1];
	char *tail;
	int (*linkf)() = sflag ? symlink : link;
	char *strrchr();

	/* is target a directory? */
	if (sflag == 0 && fflag == 0 && stat(from, &stb) >= 0
	    && (stb.st_mode&S_IFMT) == S_IFDIR) {
		printf("%s is a directory\n", from);
		return (1);
	}
	if (stat(to, &stb) >= 0 && (stb.st_mode&S_IFMT) == S_IFDIR) {
		tail = strrchr(from, '/');
		if (tail == 0)
			tail = from;
		else
			tail++;
		if (strlen(to) + strlen(tail) >= sizeof destname - 1) {
			(void) fprintf(stderr, "ln: %s/%s: Name too long\n",
			    to, tail);
			return (1);
		}
		(void) sprintf(destname, "%s/%s", to, tail);
		to = destname;
	}
	if ((*linkf)(from, to) < 0) {
		if (errno == EEXIST || sflag)
			Perror(to);
		else if (access(from, 0) < 0)
			Perror(from);
		else
			Perror(to);
		return (1);
	}
	return (0);
}

Perror(s)
	char *s;
{

	(void) fprintf(stderr, "ln: ");
	perror(s);
}
