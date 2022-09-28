/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/slink/main.c	1.5.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

/*
 *
 *	Copyright 1987, 1988 Lachman Associates, Incorporated (LAI)
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 *
 *	System V STREAMS TCP was jointly developed by Lachman
 *	Associates and Convergent Technologies.
 */

#include <stdio.h>
#include <varargs.h>
#include <signal.h>
#include "defs.h"

char           *cmdname;
int             verbose = 0;
int		pflag = 1;	/* persistent links */
int		uflag;		/* unlink */

/*
 * error - print error message and possibly exit.
 * error(flags,fmt,arg1,arg2,...) prints error message on stderr of the form:
 * command-name: message[: system error message] If flags & E_SYS, system
 * error message is included. If flags & E_FATAL, program exits (with code
 * 1).
 */
void
error(va_alist)
va_dcl
{
	va_list         args;
	int             flags;
	char           *fmt;
	extern int      sys_nerr, errno;
	extern char    *sys_errlist[];

	va_start(args);
	flags = va_arg(args, int);
	fmt = va_arg(args, char *);
	fprintf(stderr, "%s: ", cmdname);
	vfprintf(stderr, fmt, args);
	va_end(args);
	if (flags & E_SYS) {
		if (errno > sys_nerr)
			fprintf(stderr, ": Error %d\n", errno);
		else
			fprintf(stderr, ": %s\n", sys_errlist[errno]);
	} else
		putc('\n', stderr);
	if (flags & E_FATAL)
		exit(1);
}

catch()
{
}

main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             c;
	extern char    *optarg;
	extern int      optind, opterr;
	char           *ifile = CONFIGFILE;
	char           *func;
	struct val     *arglist;
	int             i;
	struct fntab   *f;
	struct fntab   *findfunc();
	FILE           *cons;
	int             dofork = 1;
	pid_t           pid;

	cmdname = argv[0];
	opterr = 0;
	while ((c = getopt(argc, argv, "vc:fpu")) != -1) {
		switch (c) {
		case 'v':
			verbose = 1;
			break;
		case 'c':
			ifile = optarg;
			break;
		case 'f':
			/*
			 * ASSUMPTION:  "stay in foreground" ->
			 * don't use persistent links 
			 */
			dofork = pflag = 0;
			break;
		case 'p':
			if (uflag) {
				error(E_FATAL,
				"slink: -u cannot be used with -p\nusage: slink [-v] [-f] [-p] [-u] [-c file] [func arg...]");
				/* NOTREACHED */
			}
			pflag = 0;
			break;
		case 'u':
			if (!pflag) {
				error(E_FATAL,
				"slink: -u cannot be used with -p\nusage: slink [-v] [-f] [-p] [-u] [-c file] [func arg...]");
				/* NOTREACHED */
			}
			uflag = 1;
			break;
		case '?':
			error(E_FATAL,
			"usage: slink [-v] [-f] [-p] [-u] [-c file] [func arg...]");
			/* NOTREACHED */
		}
	}
	fclose(stdin);
	if (!fopen(ifile, "r"))
		error(E_FATAL, "can't open \"%s\"", ifile);
	argv += optind;
	argc -= optind;
	if (argc) {
		if (verbose)
			fprintf(stderr, "%s", *argv);
		func = *argv++;
		if (--argc) {
			arglist = (struct val *) xmalloc(argc * sizeof(struct val));
			for (i = 0; i < argc; i++) {
				if (verbose)
					fprintf(stderr, " %s", *argv);
				arglist[i].vtype = V_STR;
				arglist[i].u.sval = *argv++;
			}
			if (verbose)
				putc('\n', stderr);
		}
	} else {
		if (verbose)
			fprintf(stderr, "boot\n");
		func = "boot";
		arglist = NULL;
	}

	binit();
	parse();
	if (!(f = findfunc(func)) || f->type != F_USER)
		error(E_FATAL, "Function \"%s\" not defined", func);
	userfunc(f->u.ufunc, argc, arglist);
	if (arglist)
		free(arglist);
	if (pflag) {
		/* links are persistent, no need to hang around */
		exit(0);
	}
	if (dofork) {
		if ((pid = fork()) < 0)
			error(E_FSYS, "can't fork");
		else if (pid)
			exit(0);
		close(0);
		close(1);
		close(2);
		setpgrp();
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGTERM, catch);
		/* hang around */
		pause();
		if (cons = fopen("/dev/console", "w"))
			fprintf(cons, "%s exiting: SIGTERM\n", cmdname);
	} else
		pause();
}
