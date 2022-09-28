/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)gettxt:gettxt.c	1.4.4.1"

#include <stdio.h>
#include <locale.h>

extern	char	*gettxt();
extern char	*strccpy();
extern	char	*malloc();

main(argc, argv)
int	argc;
char	*argv[];
{
	char	*dfltp;
	char	*locp;

	if (argc != 2 && argc != 3) {
		fprintf(stderr, "Incorrect usage.\n");
		fprintf(stderr, "usage: gettxt msgid [ dflt_msg ] \n");
		exit(1);
	}

	locp = setlocale(LC_ALL, "");
	if (locp == (char *)NULL) {
		(void)setlocale(LC_CTYPE, "");
		(void)setlocale(LC_MESSAGES, "");
	}

	if (argc == 2) {
		fputs(gettxt(argv[1], ""), stdout);
		exit(0);
	}

	if ((dfltp = malloc(strlen(argv[2]) + 1)) == (char *)NULL) {
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}

	strccpy(dfltp, argv[2]);

	fputs(gettxt(argv[1], dfltp), stdout);

	(void)free(dfltp);

	exit(0);
}
