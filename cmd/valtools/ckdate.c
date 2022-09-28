/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)valtools:ckdate.c	1.3.3.1"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "usage.h"

extern long	atol();
extern void	exit();
extern int	getopt(), ckdate();
extern int	ckdate_val(), ckdate_err(), ckdate_hlp();

extern int	optind, ckquit, ckindent, ckwidth;
extern char	*optarg;

#define BADPID	(-2)
char	*prog,
	*deflt,
	*prompt,
	*error,
	*help;
int	signo;
int	kpid = BADPID;
char	*fmt = NULL;

char	vusage[] = "f";
char	husage[] = "fWh";
char	eusage[] = "fWe";

#define USAGE	"[-f format]"
#define MYFMT "%s:ERROR:invalid format\n\
valid format descriptors are:\n\
\t%%b  #abbreviated month name\n\
\t%%B  #full month name\n\
\t%%d  #day of month (01-31)\n\
\t%%D  #date as %%m/%%d/%%y or %%m-%%d-%%y (default)\n\
\t%%e  #day of month ( 1-31)\n\
\t%%m  #month of year (01-12)\n\
\t%%y  #year within century (YY)\n\
\t%%Y  #year as CCYY\n\
"

void
usage()
{
	switch(*prog) {
	  default:
		(void) fprintf(stderr, "usage: %s [options] %s\n", 
			prog, USAGE);
		(void) fputs(OPTMESG, stderr);
		(void) fputs(STDOPTS, stderr);
		break;

	  case 'v':
		(void) fprintf(stderr, "usage: %s %s input\n", prog, USAGE);
		break;

	  case 'h':
		(void) fprintf(stderr, "usage: %s [options] %s\n", 
			prog, USAGE);
		(void) fputs(OPTMESG, stderr);
		(void) fputs("\t-W width\n\t-h help\n", stderr);
		break;

	  case 'e':
		(void) fprintf(stderr, "usage: %s [options] %s\n", 
			prog, USAGE);
		(void) fputs(OPTMESG, stderr);
		(void) fputs("\t-W width\n\t-h error\n", stderr);
		break;
	}
	exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
	int c, n;
	char date[64];

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c=getopt(argc, argv, "f:d:p:e:h:k:s:QW:?")) != EOF) {
		/* check for invalid option */
		if((*prog == 'v') && !strchr(vusage, c))
			usage();
		if((*prog == 'e') && !strchr(eusage, c))
			usage();
		if((*prog == 'h') && !strchr(husage, c))
			usage();

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

		  case 'f':
			fmt = optarg;
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

	if(*prog == 'v') {
		if(argc != (optind+1))
			usage();
		n = (ckdate_val(fmt, argv[optind]));
		if(n == 4)
			(void) fprintf(stderr, MYFMT, prog);
		exit(n);
	}

	if(optind != argc)
		usage();

	if(*prog == 'e') {
		ckindent = 0;
		if(ckdate_err(fmt, error)) {
			(void) fprintf(stderr, MYFMT, prog);
			exit(4);
		} else
			exit(0);
	} else if(*prog == 'h') {
		ckindent = 0;
		if(ckdate_hlp(fmt, help)) {
			(void) fprintf(stderr, MYFMT, prog);
			exit(4);
		} else
			exit(0);
	}

	n = ckdate(date, fmt, deflt, error, help, prompt);
	if(n == 3) {
		if(kpid > -2)
			(void) kill(kpid, signo);
		(void) puts("q");
	} else if(n == 0)
		(void) printf("%s", date);
	if(n == 4)
		(void) fprintf(stderr, MYFMT, prog);
	exit(n);
}
