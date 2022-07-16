/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)xcplxterm:tc2.c	1.1"

/*
 *	@(#) tc2.c 1.1 90/04/06 lxtermlib:tc2.c
 */
/*
 * tc2 [term]
 * Dummy program to test out termlib.
 * Commands are "tcc\n" where t is type (s for string, f for flag,
 * or n for number) and cc is the name of the capability.
 */
#include <stdio.h>
char buf[1024];
char *getenv(), *tgetstr();

main(argc, argv) char **argv; {
	char *p, *q;
	int rc;
	char b[3], c;
	char area[200];

	if (argc < 2)
		p = getenv("TERM");
	else
		p = argv[1];
	rc = tgetent(buf,p);
	for (;;) {
		c = getchar();
		if (c < 0)
			exit(0);
		b[0] = getchar();
		if (b[0] < ' ')
			exit(0);
		b[1] = getchar();
		b[2] = 0;
		getchar();
		switch(c) {
			case 'f':
				printf("%s: %d\n",b,tgetflag(b));
				break;
			case 'n':
				printf("%s: %d\n",b,tgetnum(b));
				break;
			case 's':
				q = area;
				printf("%s: %s\n",b,tgetstr(b,&q));
				break;
			default:
				exit(0);
		}
	}
}
