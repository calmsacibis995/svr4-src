/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ogetopt:ogetopt.c	1.7"

#include	<stdio.h>

extern void exit();
extern char *strcat();
extern char *strchr();

main(argc, argv)
int argc;
char **argv;
{
	extern	int optind;
	extern	char *optarg;
	register int	c;
	int	errflg = 0;
	char	tmpstr[4];
	char	outstr[5120];
	char	*goarg;

	if(argc < 2) {
		fputs("usage: getopt legal-args $*\n", stderr);
		exit(2);
	}

	goarg = argv[1];
	argv[1] = argv[0];
	argv++;
	argc--;
	outstr[0] = '\0';

	while((c=getopt(argc, argv, goarg)) != EOF) {
		if(c=='?') {
			errflg++;
			continue;
		}

		tmpstr[0] = '-';
		tmpstr[1] = c;
		tmpstr[2] = ' ';
		tmpstr[3] = '\0';

		strcat(outstr, tmpstr);

		if(*(strchr(goarg, c)+1) == ':') {
			strcat(outstr, optarg);
			strcat(outstr, " ");
		}
	}

	if(errflg) {
		exit(2);
	}

	strcat(outstr, "-- ");
	while(optind < argc) {
		strcat(outstr, argv[optind++]);
		strcat(outstr, " ");
	}

	(void) printf("%s\n", outstr);
	exit(0);	/*NOTREACHED*/
}
