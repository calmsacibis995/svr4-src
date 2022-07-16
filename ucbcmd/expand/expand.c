/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbexpand:expand.c	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#include <stdio.h>
/*
 * expand - expand tabs to equivalent spaces
 */
char	obuf[BUFSIZ];
int	nstops;
int	tabstops[100];

main(argc, argv)
	int argc;
	char *argv[];
{
	register int c, column;
	register int n;

	setbuf(stdout, obuf);
	argc--, argv++;
	do {
		while (argc > 0 && argv[0][0] == '-') {
			getstops(argv[0]);
			argc--, argv++;
		}
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			argc--, argv++;
		}
		column = 0;
		for (;;) {
			c = getc(stdin);
			if (c == -1)
				break;
			switch (c) {

			case '\t':
				if (nstops == 0) {
					do {
						putchar(' ');
						column++;
					} while (column & 07);
					continue;
				}
				if (nstops == 1) {
					do {
						putchar(' ');
						column++;
					} while (((column - 1) % tabstops[0]) != (tabstops[0] - 1));
					continue;
				}
				for (n = 0; n < nstops; n++)
					if (tabstops[n] > column)
						break;
				if (n == nstops) {
					putchar(' ');
					column++;
					continue;
				}
				while (column < tabstops[n]) {
					putchar(' ');
					column++;
				}
				continue;

			case '\b':
				if (column)
					column--;
				putchar('\b');
				continue;

			default:
				putchar(c);
				column++;
				continue;

			case '\n':
				putchar(c);
				column = 0;
				continue;
			}
		}
	} while (argc > 0);
	exit(0);
	/* NOTREACHED */
}

getstops(cp)
	register char *cp;
{
	register int i;

	nstops = 0;
	cp++;
	for (;;) {
		i = 0;
		while (*cp >= '0' && *cp <= '9')
			i = i * 10 + *cp++ - '0';
		if (i <= 0 || i > 256) {
bad:
			fprintf(stderr, "Bad tab stop spec\n");
			exit(1);
		}
		if (nstops > 0 && i <= tabstops[nstops-1])
			goto bad;
		tabstops[nstops++] = i;
		if (*cp == 0)
			break;
		if (*cp++ != ',')
			goto bad;
	}
}
