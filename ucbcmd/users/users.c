/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984, 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All rights reserved.
 */

#ident	"@(#)ucbusers:users.c	1.1.1.1"

/*
 * users
 */
char	*malloc();

#include <stdio.h>
#include <sys/types.h>
#include <utmp.h>


#define NMAX sizeof(utmp.ut_name)

struct utmp utmp;

main(argc, argv)
char **argv;
{
	register char *tp, *s;
	register FILE *fi;

	s = UTMP_FILE;
	if(argc == 2)
		s = argv[1];
	if ((fi = fopen(s, "r")) == NULL) {
		perror(s);
		exit(1);
	}
	while (fread((char *)&utmp, sizeof(utmp), 1, fi) == 1) {
		if(utmp.ut_name[0] == '\0')
			continue;
		if (utmp.ut_type != USER_PROCESS)
			continue;
		putline();
	}
	summary();
	exit(0);
}

char	*names[128];
char	**namp = names;
putline()
{
	char temp[NMAX+1];
	strncpy(temp, utmp.ut_name, NMAX);
	temp[NMAX] = 0;
	*namp = malloc(strlen(temp) + 1);
	strcpy(*namp++, temp);
}

scmp(p, q)
char **p, **q;
{
	return(strcmp(*p, *q));
}
summary()
{
	register char **p;

	qsort(names, namp - names, sizeof names[0], scmp);
	for (p=names; p < namp; p++) {
		if (p != names)
			putchar(' ');
		fputs(*p, stdout);
	}
	if (namp != names)		/* at least one user */
		putchar('\n');
}
