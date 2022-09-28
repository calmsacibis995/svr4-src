/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)login:license.c	1.1.2.1"

#include <values.h>
#include <sys/sysi86.h>

main()
{
	int i;

	if ((i = sysi86(SI86LIMUSER, 0)) < 0) {
		printf ("This sysytem does not implement per user licensing.\n");
		exit (100);
	}
	if (i == MAXINT) {
		printf ("This system has unlimited per user licensing.\n");
		exit (100);
	}
	printf ("This system has per user licensing limited to %d concurrent users.\n", i);
	exit (i);
}
