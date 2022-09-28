/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)windowing:cmd/layers/deodx.c	1.1"

#include <stdio.h>
#include <ctype.h>

/*
 * For sites without 3b2 cross-compilation ability, this program
 * translates a hex dump of the 5620 executables back into binary.
 */

/* libc */
extern void perror();
extern void exit();

main(argc,argv)
int argc;
char **argv;
{
	register int c;
	register int d;

	if (argc != 1) {
		(void) fprintf(stderr,"Usage: %s\n",argv[0]);
		exit(1);
	}

	c = ' ';
	while(c != EOF) {
		for(;;) {
			if ((c = getchar()) == EOF) break;
			if (c >='0' && c<='9') c=c-'0';
			else if (c >='a' && c<='f') c=c-'a'+10;
			else if (c >='A' && c<='F') c=c-'A'+10;
			else break;
			if ((d = getchar()) == EOF) {
				(void) fprintf(stderr,"Odd-nybble boundary contains EOF\n");
				exit(3);
			}
			if (d >='0' && d<='9') d=d-'0';
			else if (d >='a' && d<='f') d=d-'a'+10;
			else if (d >='A' && d<='F') d=d-'A'+10;
			else break;
			putchar(c<<4 | d);
		}
		while(c != EOF && c !='\n') c=getchar();
	}
	return 0;
}
