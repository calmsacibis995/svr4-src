/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)misc:ismpx.c	2.4.1.1"

#include <stdio.h>
#include <sys/jioctl.h>

main(argc, argv)
	int   argc;
	char *argv[];
{
	if (argc != 1)
		if (!(argc == 2 && (strcmp("-s", argv[1]) == 0 ||
		                    strcmp("-",  argv[1]) == 0 ))) {
			fprintf(stderr, "usage: ismpx [-s]\n");
			exit(1);
		}
	if (ioctl(0, JMPX, 0) == -1){
		if(argc==1)
			printf("no\n");
		exit(1);
	}
	if (argc == 1)
		printf("yes\n");
	exit(0);
}
