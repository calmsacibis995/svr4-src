/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)valtools:ckrange.c	1.2.3.1"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <values.h>
#include "usage.h"

extern int	optind, ckquit, ckindent, ckwidth;
extern char	*optarg;

extern long	atol(), strtol();
extern void	exit(), ckrange_err(), ckrange_hlp();
extern int	getopt(), ckrange(), ckrange_val();

#define BADPID	(-2)

char	*prog,
	*deflt,
	*prompt,
	*error,
	*help;
int	kpid = BADPID;
int	signo; 
short	base = 10;
char	*upper;
char	*lower;

char	vusage[] = "bul";
char	husage[] = "bulWh";
char	eusage[] = "bulWe";

#define USAGE	"[-l lower] [-u upper] [-b base]"

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
		(void) fprintf(stderr, "usage: %s %s input\n", USAGE, prog);
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
	long	lvalue, uvalue, intval;
	char	*ptr = 0;

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c=getopt(argc, argv, "l:u:b:d:p:e:h:k:s:QW:?")) != EOF) {
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

		  case 'b':
			base = atol(optarg);
			if((base < 2) || (base > 36)) {
				progerr("base must be between 2 and 36");
				exit(1);
			}
			break;

		  case 'u':
			upper = optarg;
			break;

		  case 'l':
			lower = optarg;
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

	if (upper) {
		uvalue = strtol(upper, &ptr, base);
		if (ptr == upper) {
			progerr("invalid upper value specification");
			exit(1);
		}
	} else
		uvalue = LONG_MAX;
	if (lower) {
		lvalue =  strtol(lower, &ptr, base);
		if(ptr == lower) {
			progerr("invalid lower value specification");
			exit(1);
		}
	} else
		lvalue = LONG_MIN;

	if (uvalue < lvalue) {
		progerr("upper value is less than lower value");
		exit(1);
	}

	if(*prog == 'v') {
		if(argc != (optind+1))
			usage();
		exit(ckrange_val(lvalue, uvalue, base, argv[optind]));
	}

	if(optind != argc)
		usage();

	if(*prog == 'e') {
		ckindent = 0;
		ckrange_err(lvalue, uvalue, base, error);
		exit(0);
	} else if(*prog == 'h') {
		ckindent = 0;
		ckrange_hlp(lvalue, uvalue, base, help);
		exit(0);
	}
	
	n = ckrange(&intval, lvalue, uvalue, base, deflt, error, help, prompt);
	if(n == 3) {
		if(kpid > -2)
			(void) kill(kpid, signo);
		(void) puts("q");
	} else if(n == 0)
		(void) printf("%d", intval);
	exit(n);
}
