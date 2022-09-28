/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:main.c	1.7.3.2"
#include "mail.h"
#ifdef SVR4
#include <locale.h>
#endif
/*
	mail [ -ehpPqrtw ] [-x debuglevel] [ -f file ] [ -F user(s) ]
	mail -T file persons
	mail [ -tw ] [ -m messagetype ] persons 
	rmail [ -tw ] persons 
*/
main(argc, argv)
char	**argv;
{
	register int i;
	char *cptr, *p;
	struct stat statb;
	static char pn[] = "main";

#ifdef SVR4
	(void)setlocale(LC_ALL, "");
#endif
#ifdef SIGCONT
# ifdef SVR4
	{
	struct sigaction nsig;
	nsig.sa_handler = SIG_DFL;
	sigemptyset(&nsig.sa_mask);
	nsig.sa_flags = SA_RESTART;
	(void) sigaction(SIGCONT, &nsig, (struct sigaction*)0);
	}
# else
	sigset(SIGCONT, SIG_DFL);
# endif
#endif
	/* This is protection.  Without this, we cannot wait on children. */
	signal(SIGCLD, SIG_DFL);

	/*
		Strip off path name of this command for use in messages
	*/
	if ((program = strrchr(argv[0],'/')) != NULL) {
		program++;
	} else {
		program = argv[0];
	}

	/* Close all file descriptors except stdin, stdout & stderr */
	/* NB: _NFILE defined in stdio.h */
	for (i = 3; i < _NFILE; i++) {
		close (i);
	}

	switch (init()) {
	case -1:
		if (stat(mailcnfg,&statb) == 0) {
			/* file DOES exist! */
			fprintf(stderr,
				"%s: error opening %s\n", program, mailcnfg);
			exit(1);
			/* NOTREACHED */
		}
		break;

	case 0:
		fprintf(stderr,"%s: error reading %s\n", program, mailcnfg);
		exit(1);
		/* NOTREACHED */

	case 1:
		break;
	}

	/*
		Get group id for mail, exit if none exists
	*/
	if ((grpptr = getgrnam("mail")) == NULL) {
		errmsg(E_GROUP,"");
		exit(1);
	} else {
		mailgrp = grpptr->gr_gid;
	}

	/*
		Save the *id for later use.
	*/
	my_uid = getuid();
	my_gid = getgid();
	my_euid = geteuid();
	my_egid = getegid();

	/*
		What command (rmail or mail)?
	*/
	if (strcmp(program, "rmail") == SAME) {
		ismail = FALSE;
	}

	/*
		Parse the command line and adjust argc and argv 
		to compensate for any options
	*/
	i = parse(argc,argv);
	argv += (i - 1);
	argc -= (i - 1);

	/* block a potential security hole */
	if (flgT && (my_euid != 0)) {
		setgid(my_gid);
		Tout (pn, "Setgid unset\n");
	}

	if (debug == 0) {
		/* If not set as an invocation option, check for system-wide */
		/* global flag */
		char *xp = xgetenv("DEBUG");
		if (xp != (char *)NULL) {
			debug = atoi(xp);
			if (debug < 0) {
				/* Keep trace file even if successful */
				keepdbgfile = -1;
				debug = -debug;
			}
		}
	}
	if (debug > 0) {
		strcpy (dbgfname, "/tmp/MLDBGXXXXXX");
		mktemp (dbgfname);
		if ((dbgfp = fopen(dbgfname,"w")) == (FILE *)NULL) {
			fprintf(stderr,"%s: can't open debugging file '%s'\n",
				program, dbgfname);
			exit (13);
		}
		setbuf (dbgfp, NULL);
		fprintf(dbgfp, "main(): debugging level == %d\n", debug);
		fprintf(dbgfp, "main(): trace file ='%s': kept %s\n", dbgfname,
			((keepdbgfile < 0) ?
				"on success or failure." : "only on failure."));
	}

	if (!ismail && (goerr > 0 || !i)) {
		Dout(pn, 11, "!ismail, goerr=%d, i=%d\n", goerr, i);
		if (goerr > 0) {
			errmsg(E_SYNTAX,"Usage: rmail [-wt] person(s)");
		}
		if (!i) {
			errmsg(E_SYNTAX,"At least one user must be specified");
		}
		Dout(pn, 11, "exiting!\n");
		done(0);
	}

	umsave = umask(7);
	uname(&utsn);
	if ((p = xgetenv("CLUSTER")) != (char *)NULL) {
		/*
		 * We are not who we appear...
		 */
		thissys = p;
	} else {
		thissys = utsn.nodename;
	}
	Dout(pn, 11, "thissys = '%s', uname = '%s'\n", thissys, utsn.nodename);

	failsafe = xgetenv("FAILSAFE");
	if (failsafe)
		Dout(pn, 11, "failsafe processing enabled to %s\n", failsafe);

	/* 
		Use environment variables
	*/
	home = getenv("HOME");
	if (!home || !*home) {
		home = ".";
	}

	my_name[0] = '\0';
	pwd = getpwuid(my_uid);
	if (pwd)
		strncpy (my_name, pwd->pw_name, sizeof(my_name));

	/* If root, use LOGNAME if set */
	if (my_uid == 0) {
		/* If root, use LOGNAME if set */
		if (((cptr = getenv("LOGNAME")) != NULL) &&
		    (strlen(cptr) != 0)) {
			strncpy (my_name, cptr, sizeof(my_name));
		}
	}
	Dout(pn, 11, "my_name = '%s'\n", my_name);

	initsurrfile();
	/*
		Catch signals for cleanup
	*/
	if (setjmp(sjbuf)) {
		done(0);
	}
	for (i=SIGINT; i<SIGCLD; i++) {
		setsig(i, delete);
	}
	setsig(SIGHUP, done);

	cksaved(my_name);

	/*
		Rmail is always invoked to send mail
	*/
	Dout(pn, 11, "ismail=%d, argc=%d\n", ismail, argc);
	if (ismail && (argc==1)) {
		sending = FALSE;
		printmail();
		if (flgF) {
			doFopt();
		}
	} else {
		sending = TRUE;
		/*
		 	Guarantee some of the environment for rmail.
			In particular, this prevents problems with
			mailalias looking for ~nuucp/lib/names.
		*/
		if (!ismail)
			putenv("HOME=/");
		sendmail(argc, argv);
	}
	done(0); /*NOTREACHED*/
}
