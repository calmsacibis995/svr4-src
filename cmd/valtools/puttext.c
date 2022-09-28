/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)valtools:puttext.c	1.1.2.1"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

extern long	atol();
extern void	exit();
extern int	optind;
extern char	*optarg;

#define LSIZE	1024

char	*prog;
int	nflag;
int	lmarg, rmarg;

void
usage()
{
	fprintf(stderr, "usage: %s [-r rmarg] [-l lmarg] string\n", prog);
	exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
	int c;

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c=getopt(argc, argv, "nr:l:?")) != EOF) {
		switch(c) {
		  case 'n':
			nflag++;
			break;

		  case 'r':
			rmarg = atol(optarg);
			break;

		  case 'l':
			lmarg = atol(optarg);
			break;

		  default:
			usage();
		}
	}

	if((optind+1) != argc)
		usage();

	(void) puttext(stdout, argv[optind], lmarg, rmarg);
	if(!nflag)
		(void) fputc('\n', stdout);
	exit(0);
}
