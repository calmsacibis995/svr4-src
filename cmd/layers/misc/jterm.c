/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)misc:jterm.c	2.6.1.1"

#include <stdio.h>
#include <errno.h>
#include <sys/jioctl.h>

main(argc, argv)
int   argc;
char *argv[];
{
	int rv = 0;

	if (argc != 1) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		rv = 1;
	} else if (ioctl(2, JMPX, 0) == -1) {
		fprintf(stderr, "%s: Must be invoked with stderr attached to an xt channel.\n", argv[0]);
		rv = 1;
	} else if (ioctl(2, JTERM, 0)) {
		fprintf(stderr, "%s: xt ioctl failed with errno '%d'.\n", argv[0], errno);
		rv = 1;
	}

	exit(rv);
}
