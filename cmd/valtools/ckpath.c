/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)valtools:ckpath.c	1.3.3.1"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <valtools.h>
#include "usage.h"

extern int	optind, ckquit, ckindent, ckwidth;
extern char	*optarg;

extern long	atol();
extern void	exit(), ckpath_err(), ckpath_help();
extern int	getopt(), ckpath(), ckpath_val();

#define BADPID	(-2)

char	*prog,
	*deflt,
	*prompt,
	*error,
	*help;
int	kpid = BADPID;
int	signo, pflags; 

char	vusage[] = "abcfglrtwxyzno";
char	eusage[] = "abcfglrtwxyznoWe";
char	husage[] = "abcfglrtwxyznoWh";

#define USAGE "[-[a|l][b|c|f|y][n|[o|z]]rtwx]"
#define MYOPTS "\
\t-a  #absolute path\n\
\t-b  #block special device\n\
\t-c  #character special device\n\
\t-f  #ordinary file\n\
\t-l  #relative path\n\
\t-n  #must not exist (new)\n\
\t-o  #must exist (old)\n\
\t-r  #read permission\n\
\t-t  #permission to create (touch)\n\
\t-w  #write permission\n\
\t-x  #execute permisiion\n\
\t-y  #directory\n\
\t-z  #non-zero length\n\
"
void
usage()
{
	switch(*prog) {
	  default:
		(void) fprintf(stderr, "usage: %s [options] %s\n", 
			prog, USAGE);
		(void) fputs(MYOPTS, stderr);
		(void) fputs(OPTMESG, stderr);
		(void) fputs(STDOPTS, stderr);
		break;

	  case 'v':
		(void) fprintf(stderr, "usage: %s %s input\n", 
			prog, USAGE);
		(void) fputs(OPTMESG, stderr);
		(void) fputs(MYOPTS, stderr);
		break;

	  case 'h':
		(void) fprintf(stderr, "usage: %s [options] %s\n", 
			prog, USAGE);
		(void) fputs(MYOPTS, stderr);
		(void) fputs(OPTMESG, stderr);
		(void) fputs("\t-W width\n\t-h help\n", stderr);
		break;

	  case 'e':
		(void) fprintf(stderr, "usage: %s [options] %s [input]\n", 
			prog, USAGE);
		(void) fputs(MYOPTS, stderr);
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
	char pathval[256];
	char itmp[256];

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c=getopt(argc, argv, "abcfglrtwxyznod:p:e:h:k:s:QW:?")) != EOF) {
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

		  case 'a':
			pflags |= P_ABSOLUTE;
			break;

		  case 'b':
			pflags |= P_BLK;
			break;

		  case 'c':
			pflags |= P_CHR;
			break;

		  case 'f':
		  case 'g': /* outdated */
			pflags |= P_REG;
			break;

		  case 'l':
			pflags |= P_RELATIVE;
			break;

		  case 'n':
			pflags |= P_NEXIST;
			break;

		  case 'o':
			pflags |= P_EXIST;
			break;

		  case 't':
			pflags |= P_CREAT;
			break;

		  case 'r':
			pflags |= P_READ;
			break;

		  case 'w':
			pflags |= P_WRITE;
			break;

		  case 'x':
			pflags |= P_EXEC;
			break;

		  case 'y':
			pflags |= P_DIR;
			break;

		  case 'z':
			pflags |= P_NONZERO;
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

	if(ckpath_stx(pflags)) {
		fprintf(stderr, "ERROR: mutually exclusive options used\n");
		exit(4);
	}

	if(*prog == 'v') {
		if(argc != (optind+1))
			usage(); /* too many paths listed */
		exit(ckpath_val(argv[optind], pflags));
	} else if(*prog == 'e') {
		if(argc > (optind+1))
			usage();
		ckindent = 0;
		ckpath_err(pflags, error, argv[optind]);
		exit(0);
	} 

	if(optind != argc)
		usage();

	if(*prog == 'h') {
		ckindent = 0;
		ckpath_hlp(pflags, help);
		exit(0);
	}

	n = ckpath(pathval, pflags, deflt, error, help, prompt);
	if(n == 3) {
		if(kpid > -2)
			(void) kill(kpid, signo);
		(void) puts("q");
	} else if(n == 0)
		(void) fputs(pathval, stdout);
	exit(n);
}
