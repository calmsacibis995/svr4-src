/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:parse.c	1.6.3.1"
#include "mail.h"
/*
	Parse the command line.
	Return index of first non-option field (i.e. user)
*/
parse(argc, argv)
int	argc;
char	**argv;
{
	register int	 	c;
	register char		*tmailsurr;
	static char		pn[] = "parse";

	/*
		"mail +" means to print in reverse order and is
		equivalent to "mail -r"
	*/

#ifdef i386
	if ((argc > 1) && (argv[1] != NULL) && (argv[1][0] == '+')) {
#else
	if ((argc > 1) && (argv[1][0] == '+')) {
#endif

		if (ismail) {
			argv[1] = "-r";
		} else {
			goerr++;
		}
	}

	while ((c = getopt(argc, argv, "m:f:x:shrpPqetT:wF:")) != EOF) {
		switch(c) {
		/*
			Set debugging level...
		*/
		case 'x':
			debug = atoi(optarg);
			orig_dbglvl = debug;
			if (debug < 0) {
				/* Keep trace file even if successful */
				keepdbgfile = -1;
				debug = -debug;
			}
			break;

		/*
			for backwards compatability with mailx...
		*/
		case 's':
			/* ignore this option */
			break;

		/*
			do not print mail
 		*/
		case 'e':
			if (ismail) {
				flge = 1;
			} else {
				goerr++;
			}
			optcnt++;
			break;

		/*
			use alternate file as mailfile
		*/
		case 'f':
			if (ismail) {
				flgf = 1;
				mailfile = optarg;
				if (optarg[0] == '-') {
					errmsg(E_SYNTAX,"Files names must not begin with '-'");
			   		done(0);
				}
			} else {
				goerr++;
			}
			optcnt++;
			break;

		/*
			Print headers first
		*/
		case 'h':
			if (ismail) {
				flgh = 1;
			} else {
				goerr++;
			}
			optcnt++;
			break;

		/*
			Install/Remove Forwarding
 		*/
		case 'F':
			if (ismail) {
				flge = 1;	/* set -e option */
                        	flgF = 1;	/* Indicate Forwarding */
				strcpy(uval, optarg);
			} else {
				goerr++;
			}
			optcnt++;
			break;

		/* 
			print without prompting
		*/
		case 'p':
			if (ismail) {
				flgp++;
			} else {
				goerr++;
			}
			optcnt++;
			break;

		/*
			override selective display default setting
			when reading mail...
		*/
		case 'P':
			if (ismail) {
				flgP++;
			}
			optcnt++;
			break;

		/* 
			terminate on deletes
		*/
		case 'q':
			if (ismail) {
				delflg = 0;
			} else {
				goerr++;
			}
			optcnt++;
			break;

		/* 
			print by first in, first out order
		*/
		case 'r':
			if (ismail) {
				flgr = 1;
			} else {
				goerr++;
			}
			optcnt++;
			break;

		/*
			add To: line to letters
		*/
		case 't':
			flgt = 1;
			optcnt++;
			break;

		/*
			test mode; display mailsurr transformations but
			don't actually send any messages. Allows specification
			of dummy mailsurr file for testing.
		*/
		case 'T':
			flgT++;
			tmailsurr = optarg;
			if ((tmailsurr != (char *)NULL) &&
			    (*tmailsurr != '\0')) {
				mailsurr = tmailsurr;
			}
			break;

		/*
			don't wait on sends
		*/
		case 'w':
			flgw = 1;
			break;

		/*
			set message-type:
		*/
		case 'm':
			msgtype = optarg;
			if (msgtype[0] == '\0' || msgtype[0] == '-') {
				goerr++;
			} else {
				flgm = 1;
			}
			break;

		/*
			bad option
		*/
		case '?':
			goerr++;
			break;
		}
	}

	if (flgF) {
		if (optcnt != 1) {
			errmsg(E_SYNTAX,
				"Forwarding is mutually exclusive of other options");
			goerr++;
		}

		if (argc != optind) {
			fprintf(stderr,
				"%s: Too many arguments for forwarding\n",
				program);
			fprintf(stderr,
				"%s: To forward to multiple users say '%s -F ",
				program, program);
			fprintf(stderr, "\"user1 user2 ...\"'\n");
			error = E_SYNTAX;
			goerr++;
		}
	}

	if (argc == optind) {
	    if (flgT) {
		errmsg(E_SYNTAX,
			"-T option used but no recipient(s) specified.");
		goerr++;
	    }
	    if (flgm) {
		errmsg(E_SYNTAX,
			"-m option used but no recipient(s) specified.");
		goerr++;
	    }
	    if (flgt) {
		errmsg(E_SYNTAX,
			"-t option used but no recipient(s) specified.");
		goerr++;
	    }
	    if (flgw) {
		errmsg(E_SYNTAX,
			"-w option used but no recipient(s) specified.");
		goerr++;
	    }
	}

	if (ismail && (goerr > 0)) {
		errmsg(E_SYNTAX,"Usage: [-ehpPqr] [-f file] [-x debuglevel]");
		(void) fprintf (stderr, "or\t[-tw] [-m message_type] [-T file] [-x debuglevel] persons\n");
		(void) fprintf (stderr, "or\t[-x debuglevel] -F user(s)\n");
		done(0);
	}

	return (optind);
}
