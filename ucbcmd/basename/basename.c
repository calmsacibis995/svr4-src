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

#ident	"@(#)ucbbasename:basename.c	1.1.1.1"

#include	<stdio.h>

main(argc, argv)
char **argv;
{
	register char *p1, *p2, *p3;

	if (argc < 2) {
		(void)putchar('\n');
		exit(1);
	}
	p1 = argv[1];
	p2 = p1;
	while (*p1) {
		if (*p1++ == '/')
			p2 = p1;
	}
	if (argc>2) {
		for(p3=argv[2]; *p3; p3++) 
			;
		while(p3>argv[2])
			if(p1 <= p2 || *--p3 != *--p1)
				goto output;
		*p1 = '\0';
	}
output:
	fputs(p2, stdout);
	putc('\n', stdout);
	exit(0);
	/* NOTREACHED */
}
