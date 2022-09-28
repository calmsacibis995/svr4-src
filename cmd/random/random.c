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

#ident	"@(#)random:random.c	1.1.1.1"

/*
 *	@(#) random.c 1.1 88/03/29 random:random.c
 */
/***	random
 *
 *	MODIFICATION HISTORY
 *	M000	21 Jul 83	andyp	3.0 upgrade
 *	- Picked up v7 source (not in Bell 3.0).
 *	- Ran through cb, it was unbearable.
 *
 *	M001	13 Aug 83	ats
 *	- Made it comprehensible. Now does the following:
 *
 *		random [-s] [scale]
 *
 *	  'scale' is 1 - 255, default 1. Random prints and exits with
 *	  a number in this range. The "-s" option suppresses printing.
 *	  This is a subset of what the previous code pretended to do.
 *
 *	  Exit code is 0 on error.
 */

#include <stdio.h>

#define MAXINT (long)32768 /* rand() returns 0 - 2**15-1, this is one more */

main(argc, argv)
int argc;
char **argv;
{
	long tvec;
	register silent = 0;	/* -s flag specified */
	register exval = 2;	/* exit code - the random integer */
	long random;	/* calculated random value, not scaled */

	/*
	 * crack args, "-s" for silent, integer for range
	 */
	while(--argc) {
		++argv;
		if(**argv == '-') {
			if(strcmp(*argv,"-s")) {
				fprintf(stderr,"usage: random [-s] [scale(1 - 255)]\n");
				exit(0);
			} else
				silent++;
		} else {
			exval = atoi(*argv) + 1;
			if((exval < 2) || (exval > 256)) {
				fprintf(stderr,"usage: random [-s] [scale(1 - 255)]\n");
				exit(0);
			}
		}
	}

	/*
	 * seed random number generator with time of day.
	 */
	time(&tvec);
	srand((int)tvec);

	/*
	 * calculate random exit value, scaled appropriately.
	 * Rand() returns between 0 and 2**15-1. The calculation
	 * below scales this to 0-scale, whatever the user
	 * specified.
	 */
	random = (long)rand();
	exval = (random * exval) / MAXINT;

	if(!silent)
		printf("%d\n",exval);

	exit(exval);
}
