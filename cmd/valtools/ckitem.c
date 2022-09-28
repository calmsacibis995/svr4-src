/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)valtools:ckitem.c	1.2.3.1"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <valtools.h>
#include "usage.h"

extern int	optind, ckquit, ckindent, ckwidth;
extern char	*optarg;

extern long	atol();
extern int	getopt(), ckitem(), setitem(), setinvis();
extern void	exit();
extern CKMENU	*allocmenu();

#define BADPID	(-2)
char	*prog,
	*deflt,
	*prompt,
	*error,
	*help;
int	kpid = BADPID;
int	signo;

char	*label,
	*invis[36];
int	ninvis = 0;
int	max = 1;
int	attr = CKALPHA;

#define MAXSIZE	128
#define LSIZE	1024
#define INTERR	"internal error occurred while attempting menu setup"
#define MYOPTS "\
\t-f file     #file containing choices\n\
\t-l label    #menu label\n\
\t-i invis    #invisible menu choices\n\
\t-m max      #maximum choices user may select\n\
\t-n          #do not sort choices alphabetically\n\
\t-o          #don't prompt if only one choice\n\
\t-u          #unnumbered choices\n\
"
char	husage[] = "Wh";
char	eusage[] = "We";

void
usage()
{
	switch(*prog) {
	  default:
		(void) fprintf(stderr, "usage: %s [options] [choice [...]]\n", prog);
		(void) fputs(OPTMESG, stderr);
		(void) fputs(MYOPTS, stderr);
		(void) fputs(STDOPTS, stderr);
		break;

	  case 'h':
		(void) fprintf(stderr, "usage: %s [options] [choice [...]]\n", prog);
		(void) fputs(OPTMESG, stderr);
		(void) fputs("\t-W width\n\t-h help\n", stderr);
		break;

	  case 'e':
		(void) fprintf(stderr, "usage: %s [options] [choice [...]]\n", prog);
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
	CKMENU *mp;
	FILE *fp;
	int c, n, i;
	char *item[MAXSIZE];
	char line[LSIZE];
	char temp[LSIZE];

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	fp = (FILE *)0;
	while((c=getopt(argc, argv, "m:oni:l:f:ud:p:e:h:k:s:QW:?")) != EOF) {
		/* check for invalid option */
		if((*prog == 'e') && !strchr(eusage, c))
			usage(); /* no valid options */
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
			max = atol(optarg);
			break;

		  case 'o':
			attr |= CKONEFLAG;
			break;

		  case 'n':
			attr &= ~CKALPHA;
			break;

		  case 'i':
			invis[ninvis++] = optarg;
			break;

		  case 'l':
			label = optarg;
			break;

		  case 'f':
			if((fp = fopen(optarg, "r")) == NULL) {
				fprintf(stderr, "%s: ERROR: can't open %s\n",
					prog, optarg);
				exit(1);
			}
			break;

		  case 'u':
			attr |= CKUNNUM;
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

	mp = allocmenu(label, attr);
	if(fp) {
		*temp = '\0';
		while(fgets(line, LSIZE, fp)) {
			n = strlen(line);
			if(n && (line[--n] == '\n'))
				line[n] = '\0';
			if(isspace(*line)) {
				strcat(temp, "\n");
				strcat(temp, line);
			} else {
				if(*temp) {
					if(setitem(mp, temp)) {
						fprintf(stderr, INTERR);
						exit(1);
					}
				}
				strcpy(temp, line);
			}
		}
		if(*temp) {
			if(setitem(mp, temp)) {
				progerr(INTERR);
				exit(1);
			}
		}
	}

	while(optind < argc) {
		if(setitem(mp, argv[optind++])) {
			progerr(INTERR);
			exit(1);
		}
	}

	for(n=0; n < ninvis; ) {
		if(setinvis(mp, invis[n++])) {
			progerr(INTERR);
			exit(1);
		}
	}

	if(*prog == 'e') {
		ckindent = 0;
		ckitem_err(mp, error);
		exit(0);
	} else if(*prog == 'h') {
		ckindent = 0;
		ckitem_hlp(mp, help);
		exit(0);
	}

	n = ckitem(mp, item, max, deflt, error, help, prompt);
	if(n == 3) {
		if(kpid > -2)
			(void) kill(kpid, signo);
		(void) puts("q");
	} else if(n == 0) {
		i = 0;
		while(item[i])
			(void) puts(item[i++]);
	}
	exit(n);
}
