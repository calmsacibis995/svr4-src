/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)profil-3b15:prfstat.c	1.6.1.1"

/*
 *	prfstat - change and/or report profiler status
 */

#include <fcntl.h>

#define	PRF_ON	 1
#define	PRF_VAL	 2

main(argc, argv)
	char	**argv;
{
	register  int  prf;

	if((prf = open("/dev/prf", O_RDWR)) < 0) {
		printf("cannot open /dev/prf\n");
		exit(1);
	}
	if(argc > 2) {
		printf("usage: prfstat  [ on ]  [ off ]\n");
		exit(1);
	}

	if(argc == 2) {
		if(strcmp("off", argv[1]) == 0)
			ioctl(prf, 3, 0);
		else if(strcmp("on", argv[1]) == 0)
			ioctl(prf, 3, PRF_ON);
		else {
			printf("eh?\n");
			exit(1);
		}
	}
	printf("profiling %s\n", ioctl(prf, 1, 0) & PRF_ON ?
	    "enabled" : "disabled");
	if(ioctl(prf, 1, 0) & PRF_VAL)
		printf("%d kernel text addresses\n", ioctl(prf, 2, 0));
	exit(0);
}
