/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)valtools:ckuid.c	1.2.3.1"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "usage.h"

extern long	atol();
extern void	exit();
extern int	optind, ckquit, ckindent, ckwidth;
extern char	*optarg;

#define BADPID	(-2)

char	*prog,
	*deflt,
	*prompt,
	*error,
	*help;
int	kpid = BADPID;
int	signo;

char	husage[] = "mWh";
char	eusage[] = "mWe";

#define USAGE	"[-m]"

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

	  case 'd':
		(void) fprintf(stderr, "usage: %s\n", prog);
		break;

	  case 'v':
		(void) fprintf(stderr, "usage: %s input\n", prog);
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
		(void) fputs("\t-W width\n\t-e error\n", stderr);
		break;
	}
	exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
	int	c, n;
	char	strval[64];
	int	disp = 0;	/* display set to no-display */

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c=getopt(argc, argv, "md:p:e:h:k:s:QW:?")) != EOF) {
		/* check for invalid option */
		if((*prog == 'v') || (*prog == 'd'))
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

		  case 'm':
			disp = 1;
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
		exit(ckuid_val(argv[optind]));
	}

	if(optind != argc)
		usage();

	if(*prog == 'e') {
		ckindent = 0;
		ckuid_err(disp, error);
		exit(0);
	} else if(*prog == 'h') {
		ckindent = 0;
		ckuid_hlp(disp, help);
		exit(0);
	} else if(*prog == 'd') {
		exit(ckuid_dsp());
	}

	n = ckuid(strval, disp, deflt, error, help, prompt);
	if(n == 3) {
		if(kpid > -2)
			(void) kill(kpid, signo);
		(void) puts("q");
	} else if(n == 0)
		(void) fputs(strval, stdout);

	exit(n);
}
