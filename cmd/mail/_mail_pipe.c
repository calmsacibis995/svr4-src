/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:_mail_pipe.c	1.8.3.1"
/*
 * _mail_pipe.c 2.22 of 9/23/89
 *	"@(#)_mail_pipe.c	2.22"
 */
/*
    NAME
	mail_pipe - run mailbox piping program

    SYNOPSIS
	mail_pipe [-xdebug_level] -rrecipient -RReturn_path -ccontent_type -Ssubject

    DESCRIPTION
	mail_pipe runs the program specified in /var/mail/mailuser
	as mailuser.
*/

#include "mail.h"
#include <varargs.h>

#define MAILCNFG "/etc/mail/mailcnfg"

#define SAME	0
#define	frwrd	"Forward to"

#ifdef SVR3
extern	struct	passwd	*getpwnam();
extern		char	*strcat(), *strcpy(), *malloc(), *memcpy(), *strchr(), *memmove();
extern		void	exit();
#endif
extern		char	**environ;
extern		char	*optarg;	/* for getopt */

static		void	error_msg();

char		buf[2048];	/* work area */
char		dbgfname[20];
FILE		*dbgfp;
int		debug;
extern	void	ma_id();
char		mailbox[50] = MAILDIR;
extern	char	**setup_exec();
	/* environment stuff */
#ifdef SVR4
char		PATH[] = "PATH=/usr/bin:/usr/lbin:";
char		SHELL[] = "SHELL=/usr/bin/sh";
#else
char		PATH[] = "PATH=/bin:/usr/bin:/usr/lbin:";
char		SHELL[] = "SHELL=/bin/sh";
#endif
char		LOGNAME[50] = "LOGNAME=";
char		HOME[128] = "HOME=";
#define		HOMELOC 3
#define		TZLOC 4
char		*newenviron[] = { PATH, SHELL, LOGNAME, HOME, 0, 0 };

main(argc, argv)
int argc;
char *argv[];
{
	register	int c = 0;
	register char	ch = 0, ch1 = 0;
	register	char *p, *q;
	register struct passwd *pwentry;
	char 		*mailuser = (char *)NULL;
	char 		*Rpath = (char *)NULL;
	char 		*con_type = (char *)NULL;
	char 		*subject = (char *)NULL;
	char		**argvec;
	register unsigned int len;
	register FILE 	*mf;
	struct	stat	sbuf;
	register int	i;
	int		errflg = 0;
	int		keepdbgfile = 0;

	(void) ma_id();

	while ((c = getopt(argc, argv, "x:r:R:c:S:")) != EOF) {
		switch(c) {
		case 'x':
			/* Set debugging level */
			debug = atoi(optarg);
			if (debug < 0) {
				/* Keep trace file even if successful */
				keepdbgfile = -1;
				debug = -debug;
			}
			break;
		case 'r':
			/* Recipient name */
			mailuser = optarg;
			break;
		case 'R':
			/* Return path to originator */
			Rpath = optarg;
			break;
		case 'c':
			/* Content-Type: */
			con_type = optarg;
			break;
		case 'S':
			/* Subject: */
			subject = optarg;
			break;
		default:
			errflg++;
		}
	}

	switch (xsetenv(MAILCNFG)) {
	case -1:
		if (stat(MAILCNFG,&sbuf) == 0) {
			/* file DOES exist! */
			fprintf(stderr,
				"%s: error opening %s\n", argv[0], MAILCNFG);
			exit(15);
		}
		break;
	case 0:
		fprintf(stderr,"%s: error reading %s\n", argv[0], MAILCNFG);
		exit(16);
		break;
	case 1:
		break;
	}

	if (debug == 0) {
		char *p;
		/* If not set as an invocation option, check for system-wide */
		/* global flag */
		if ((p = xgetenv("DEBUG")) != (char *)NULL) {
			debug = atoi(p);
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
				argv[0], dbgfname);
			exit (14);
		}
		setbuf (dbgfp, NULL);
		fprintf(dbgfp, "%s: debugging level == %d\n", argv[0], debug);
		fprintf(dbgfp,
			"=====\n%s started. Invocation args are:\n", argv[0]);
		for (i=0; i < argc; i++) {
			fprintf(dbgfp,"\targ[%d] = '%s'\n", i, argv[i]);
		}
	}

	if ((mailuser == (char *)NULL) || (Rpath == (char *)NULL) ||
	    (con_type == (char *)NULL) || (subject == (char *)NULL) ||
	    errflg) {
		error_msg (1, "USAGE: %s [-xdebug_level] -rrecipient -RRpath -ccon_type -Ssubject\n", argv[0]);
	}

	if (strlen(mailuser) > 14) {
		error_msg (2, "%s: user name '%s' too long\n", argv[0], mailuser);
	}

	/*
	 * Check mailbox for piping. This will also fail if someone else
	 * is running the program who does not have permission to read
	 * the mailbox.
	 */
	(void) strcat(mailbox, mailuser);
	if ((stat(mailbox, &sbuf) != 0) || ((sbuf.st_mode & S_ISGID) == 0)) {
	     /* File doen't exist or setgid bit not set */
	     error_msg (3,"%s: mail for %s not being piped\n",argv[0],mailuser);
	}
	if ((mf = fopen(mailbox, "r")) == NULL) {
		error_msg (4,"%s: mail for %s not being piped\n",argv[0],mailuser);
	}
	fread(buf, (unsigned)(strlen(frwrd)), 1, mf);
	if (strncmp(buf, frwrd, strlen(frwrd)) != SAME) {
		error_msg (5,"%s: mail for %s not being piped\n",argv[0],mailuser);
	}
	buf[0] = '\0';
	while ((c = fgetc(mf)) != EOF) {
		ch1 = ch; /* save previous value of ch */
		ch = c;   /* kludge for byte-order problem on various hdwre */
		/*
		 * Pipe symbol ('|') must be first char of word. If not,
		 * keep looking.....
		 */
		if ((ch == '|') && (ch1 == ' ')) {
			break;
		}
	}
	if (ch != '|') {
		error_msg (6,"%s: mail for %s not being piped\n",argv[0],mailuser);
	}

	/* get/set user info */
	if (!(pwentry = getpwnam(mailuser))) {
		error_msg (7, "%s: unknown user '%s'\n", argv[0], mailuser);
	}

	if (setgid(pwentry->pw_gid) == (gid_t)-1) {
		error_msg (8, "%s: setgid failed\n", argv[0]);
	}

	if (setuid(pwentry->pw_uid) == (uid_t)-1) {
		error_msg (9, "%s: setuid failed\n", argv[0]);
	}

	if (chdir(pwentry->pw_dir) == -1) {
		error_msg (10, "%s: chdir failed\n", argv[0]);
	}

	/*
	 * Change the environment so it only has the variables
	 * PATH, SHELL, LOGNAME, HOME and TZ.
	 */
	environ = newenviron;
	(void) strcat(LOGNAME, mailuser);
	if ((len = strlen(pwentry->pw_dir) + 5) > sizeof(HOME)) {
		if (!(newenviron[HOMELOC] = malloc(len+1))) {
			error_msg (11, "%s: malloc failed\n", argv[0]);
		}
		(void) strcpy(newenviron[HOMELOC], HOME);
	}
	(void) strcat(HOME, pwentry->pw_dir);
	newenviron[TZLOC] = (p = getenv("TZ")) ? p-3 : 0;

	(void) umask(027);

	p = buf;
	ch = 0;
	while ((c = fgetc(mf)) != EOF) {
		ch = c;   /* kludge for byte-order problem on various hdwre */
		/*
		 * Scan for any %keywords and replace with appropriate
		 * values
		 */
		switch (ch) {
		case '\\':
			/* take next char regardless */
			c = fgetc(mf);
			if (c == EOF) {
				break;
			}
			ch = c;
			/* note drop-trhough */
		case '\n':
			break;
		default:
			*p++ = ch;
			continue;
		case '%':
			c = fgetc(mf);
			if (c == EOF) {
				*p++ = '%';
				break;
			}
			ch = c;
			switch (ch) {
			default:
				*p++ = '%';
				*p++ = ch;
				continue;
			case '\n':
				*p++ = '%';
				break;
			case 'R': /* Return path to originator */
				  /* Must prepend and append double-quotes */
				  /* so setup_exec() will keep as single arg. */
				*p++ = '"';
				for (q = Rpath; *q; q++) {
					if (*q == '"') {
						/* Must escape any embedded */
						/* quotes to make transparent */
						/* to setup_exec(). */
						*p++ = '\\';
					}
					*p++ = *q;
				}
				*p++ = '"';
				continue;
			case 'c': /* Content-Type */
				*p++ = '"';
				for (q = con_type; *q; q++) {
					if (*q == '"') {
						*p++ = '\\';
					}
					*p++ = *q;
				}
				*p++ = '"';
				continue;
			case 'S': /* Subject: */
				*p++ = '"';
				for (q = subject; *q; q++) {
					if (*q == '"') {
						*p++ = '\\';
					}
					*p++ = *q;
				}
				*p++ = '"';
				continue;
			}
			break;
		}
		break;
	}
	*p = '\0';
	if ((argvec = setup_exec (buf)) == (char **)NULL) {
		error_msg (12, "%s: no command specified after pipe symbol\n",
			argv[0]);
	}
	if (debug) {
		fprintf(dbgfp,"%s: arg vec to exec =\n", argv[0]);
		for (i= 0; argvec[i] != (char *)NULL; i++) {
			fprintf(dbgfp,"\targvec[%d] = '%s'\n", i, argvec[i]);
		}
	}
	fclose (dbgfp);
	if (keepdbgfile == 0) {
		unlink (dbgfname);
	}
	execvp (*argvec, argvec);
	debug = 0;
	error_msg (13, "%s: execvp failed. errno = %d\n", argv[0], errno);
	/* NOTREACHED */
}

static void
error_msg(va_alist)
va_dcl
{
	int	rc;
	char 	*fmt;
	va_list	args;

	va_start(args);
	rc = va_arg(args, int);
	fmt = va_arg(args, char *);
	vfprintf(stderr, fmt, args);

	if (debug > 0) {
		vfprintf(dbgfp, fmt, args);
	}
	va_end(args);
	exit (rc);
	/* NOTREACHED */
}
