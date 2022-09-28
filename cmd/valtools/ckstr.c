/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)valtools:ckstr.c	1.2.3.1"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "usage.h"

extern int	optind, ckquit, ckindent, ckwidth;
extern char	*optarg;

extern long	atol();
extern void	exit(), ckstr_err(), ckstr_hlp();
extern int	getopt(), ckstr(), ckstr_val();

#define	BADPID	(-2)

char	*prog,
	*deflt,
	*prompt,
	*error,
	*help;
int	kpid = BADPID;
int	signo, nregexp, length;
char	*regexp[128];

char	vusage[] = "rl";
char	husage[] = "rlWh";
char	eusage[] = "rlWe";

#define USAGE	"[-l length] [[-r regexp] [...]]"

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
		(void) fputs("\t-W width\n\t-e error\n", stderr);
		break;
	}
	exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
	int c, n;
	char strval[256];
	char itmp[256];

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c=getopt(argc, argv, "r:l:d:p:e:h:k:s:QW:?")) != EOF) {
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

		  case 'r':
			regexp[nregexp++] = optarg;
			break;

		  case 'l':
			length = atol(optarg);
			if((length <= 0) || (length > 128)) {
				progerr("length must be between 1 and 128");
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

	if(*prog == 'v') {
		if(argc != (optind+1))
			usage();
		if(ckstr_val(regexp, length, argv[optind]))
			exit(1);
		exit(0);
	}

	if(*prog == 'e') {
		if(argc > (optind+1))
			usage(); /* too many args */
		ckindent = 0;
		ckstr_err(regexp, length, error, argv[optind]);
		exit(0);
	} 

	if(optind != argc)
		usage();

	if(*prog == 'h') {
		ckindent = 0;
		ckstr_hlp(regexp, length, help, argv[optind]);
		exit(0);
	}

	regexp[nregexp] = NULL;
	n = ckstr(strval, regexp, length, deflt, error, help, prompt);
	if(n == 3) {
		if(kpid > -2)
			(void) kill(kpid, signo);
		(void) puts("q");
	} else if(n == 0)
		(void) fputs(strval, stdout);
	exit(n);
}
