/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)devmgmt:getvol/getvol.c	1.5.5.1"


/*LINTLIBRARY*/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <devmgmt.h>

extern char	*optarg;
extern int	optind,
		ckquit,
		ckwidth;

char	*prog;
char	*label, *fsname;
char	*prompt;
int	options = 0;
int	kpid = (-2);
int	signo = SIGKILL;

main(argc, argv)
int argc;
char *argv[];
{
	char newlabel[128];
	int c, n;

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while((c=getopt(argc, argv, "fFownx:l:p:k:s:?QW:")) != EOF) {
		switch(c) {
		  case 'Q':
			ckquit = 0;
			break;

		  case 'W':
			ckwidth = atol(optarg);
			break;

		  case 'f':
			options |= DM_FORMAT;
			break;

		  case 'F':
			options |= DM_FORMFS;
			break;

		  case 'o':
			options |= DM_OLABEL;
			break;

		  case 'n':
			options |= DM_BATCH;
			break;

		  case 'w':
			options |= DM_WLABEL;
			break;

		  case 'l':
			if(label)
				usage();
			label = strcpy(newlabel, optarg);
			break;

		  case 'p':
			prompt = optarg;
			break;

		  case 'x':
			if(label)
				usage();
			label = optarg;
			options |= DM_ELABEL;
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

	if((optind+1) != argc)
		usage();

	switch(n = getvol(argv[optind], label, options, prompt)) {
	  case 0:
		break;

	  case 1:
		progerr("unable to access device <%s>\n", argv[optind]);
		exit(1);

	  case 2:
		progerr("unknown device <%s>\n", argv[optind]);
		exit(2);

	  case 3:
		if(kpid > -2)
			(void) kill(kpid, signo);
		exit(3);

	  case 4:
		progerr("bad label on <%s>", argv[optind]);
		break;

	  default:
		progerr("unknown device error");
	}

	exit(n);
}

usage()
{
	fprintf(stderr, "usage: %s [-owfF] [-x extlabel] [-l [fsname],volname] device\n", prog);
	fprintf(stderr, "usage: %s [-n] [-x extlabel] [-l [fsname],volname] device\n", prog);
	exit(1);
}
