/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)printf:printf.c	1.2.1.1"

#include <stdio.h>

extern	char *strccpy();
extern  char *malloc();

main(argc, argv)
int	argc;
char	**argv;
{
	char	*fmt;

	if (argc == 1) {
		fprintf(stderr, "Usage: printf format [[[arg1] arg2] ... argn]\n");
		exit(1);
	}

	if ((fmt = malloc(strlen(argv[1] + 1))) == (char *)NULL) {
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}

	strccpy(fmt, argv[1]);

	printf(fmt, argv[2], argv[3], argv[4],  argv[5],
			argv[6], argv[7], argv[8], argv[9],
			argv[10], argv[11], argv[12], argv[13],
			argv[14], argv[15], argv[16], argv[17],
			argv[18], argv[19], argv[20]);
}
