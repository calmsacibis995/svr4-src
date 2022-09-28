/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)valtools:ckkeywd.c	1.2.3.1"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "usage.h"

extern int	optind, ckquit, ckwidth;
extern char	*optarg;

extern long	atol();
extern void	exit();
extern int	getopt(), ckkeywd();
	
#define BADPID	(-2)

char	*prog,
	*deflt,
	*prompt,
	*error,
	*help;
int	kpid = BADPID;
int	signo; 

char	*keyword[128];
int	nkeyword = 0;


void
usage()
{
	(void) fprintf(stderr, "usage: %s [options] keyword [...]\n", prog);
	(void) fputs(OPTMESG, stderr);
	(void) fputs(STDOPTS, stderr);
	exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
	int	c, n;
	char	strval[256];

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c=getopt(argc, argv, "d:p:e:h:k:s:QW:?")) != EOF) {
		switch(c) {
		  case 'Q':
			ckquit = 0;
			break;

		  case 'W':
			ckwidth = atol(optarg);
			if(ckwidth < 0) {
				progerr("negative display width specified");
				exit(1);
			}
			break;

		  case 'd':
			deflt = optarg;
			break;

		  case 'p':
			prompt = optarg;
			break;

		  case 'e':
			error = optarg;
			break;

		  case 'h':
			help = optarg;
			break;

		  case 'k':
			kpid = atol(optarg);
			break;
			
		  case 's':
			signo = atol(optarg);
			break;

		  default:
			usage();
		}
	}

	if(signo) {
		if(kpid == BADPID)
			usage();
	} else
		signo = SIGTERM;

	if(optind >= argc)
		usage(); /* must be at least one keyword */

	while(optind < argc)
		keyword[nkeyword++] = argv[optind++];
	keyword[nkeyword] = NULL;

	n = ckkeywd(strval, keyword, deflt, error, help, prompt);
	if(n == 3) {
		if(kpid > -2)
			(void) kill(kpid, signo);
		(void) puts("q");
	} else if(n == 0)
		(void) fputs(strval,stdout);
	exit(n);
}
