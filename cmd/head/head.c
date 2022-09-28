/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)head:head.c	1.1.1.1"

/*	Copyright (c) 1988, Sun Microsystems, Inc.		*/
/*	All Rights Reserved.					*/

#include <stdio.h>
/*
 * head - give the first few lines of a stream or of each of a set of files
 *
 */

int	linecnt	= 10;

main(Argc, argv)
	int Argc;
	char *argv[];
{
	static int around;
	register int argc;
	char *name;

	Argc--, argv++;
	argc = Argc;
	do {
		while (argc > 0 && argv[0][0] == '-') {
			linecnt = getnum(argv[0] + 1);
			argc--, argv++, Argc--;
		}
		if (argc == 0 && around)
			break;
		if (argc > 0) {
			(void)close(0);
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			name = argv[0];
			argc--, argv++;
		} else
			name = 0;
		if (around)
			(void)putchar('\n');
		around++;
		if (Argc > 1 && name)
			(void)printf("==> %s <==\n", name);
		copyout(linecnt);
		(void)fflush(stdout);
	} while (argc > 0);
	exit(0);
}

copyout(cnt)
	register int cnt;
{
	char lbuf[BUFSIZ];

	while (cnt > 0 && fgets(lbuf, sizeof lbuf, stdin) != 0) {
		(void)printf("%s", lbuf);
		(void)fflush(stdout);
		cnt--;
	}
}

getnum(cp)
	register char *cp;
{
	register int i;

	for (i = 0; *cp >= '0' && *cp <= '9'; cp++)
		i *= 10, i += *cp - '0';
	if (*cp) {
		(void)fprintf(stderr, "usage: head [-n] [filename...]\n");
		exit(1);
	}
	return (i);
}
