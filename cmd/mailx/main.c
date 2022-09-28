/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:main.c	1.13.7.1"

#include "rcv.h"
#ifndef preSVr4
#include <locale.h>
#endif

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Startup -- interface with user.
 */

static void		hdrstop();

static jmp_buf	hdrjmp;

/*
 * Find out who the user is, copy his mail file (if exists) into
 * /tmp/Rxxxxx and set up the message pointers.  Then, print out the
 * message headers and read user commands.
 *
 * Command line syntax:
 *	mailx [ -i ] [ -r address ] [ -h number ] [ -f [ name ] ]
 * or:
 *	mailx [ -i ] [ -r address ] [ -h number ] people ...
 *
 * and a bunch of other options.
 */

main(argc, argv)
	char **argv;
{
	register char *ef;
	register int argp;
	int mustsend, f, goerr = 0;
	void (*prevint)();
	struct termio tbuf;
	int c;
	char *cwd, *mf;

	/*
	 * Set up a reasonable environment.
	 * Figure out whether we are being run interactively, set up
	 * all the temporary files, buffer standard output, and so forth.
	 */

#ifndef preSVr4
	(void)setlocale(LC_ALL, "");
#endif
#ifdef SIGCONT
# ifdef preSVr4
	sigset(SIGCONT, SIG_DFL);
# else
	{
	struct sigaction nsig;
	nsig.sa_handler = SIG_DFL;
	sigemptyset(&nsig.sa_mask);
	nsig.sa_flags = SA_RESTART;
	(void) sigaction(SIGCONT, &nsig, (struct sigaction*)0);
	}
# endif
#endif
	progname = argv[0];
	myegid = getegid();
	myrgid = getgid();
	myeuid = geteuid();
	myruid = getuid();
	mypid = getpid();
	setgid(myrgid);
	setuid(myruid);
	inithost();
	intty = isatty(0);
	if (ioctl(1, TCGETA, &tbuf)==0) {
		outtty = 1;
		baud = tbuf.c_cflag & CBAUD;
	}
	else
		baud = B9600;
	image = -1;

	/*
	 * Now, determine how we are being used.
	 * We successively pick off instances of -r, -h, -f, and -i.
	 * If called as "rmail" we note this fact for letter sending.
	 * If there is anything left, it is the base of the list
	 * of users to mail to.  Argp will be set to point to the
	 * first of these users.
	 */

	ef = NOSTR;
	argp = -1;
	mustsend = 0;
	if (argc > 0 && **argv == 'r')
		rmail++;
	while ((c = getopt(argc, argv, "efFh:HinNr:s:u:UdIT:V~")) != EOF)
		switch (c) {
		case 'e':
			/*
			 * exit status only
			 */
			exitflg++;
			break;

		case 'r':
			/*
			 * Next argument is address to be sent along
			 * to the mailer.
			 */
			mustsend++;
			rflag = optarg;
			break;

		case 'T':
			/*
			 * Next argument is temp file to write which
			 * articles have been read/deleted for netnews.
			 */
			Tflag = optarg;
			if ((f = creat(Tflag, TEMPPERM)) < 0) {
				perror(Tflag);
				exit(1);
			}
			close(f);
			/* fall through for -I too */
			/* FALLTHROUGH */

		case 'I':
			/*
			 * print newsgroup in header summary
			 */
			newsflg++;
			break;

		case 'u':
			/*
			 * Next argument is person's mailbox to use.
			 * Treated the same as "-f /var/mail/user".
			 */
			{
			static char u[PATHSIZE];
			strcpy(u, maildir);
			strncat(u, optarg, PATHSIZE);
			ef = u;
			break;
			}

		case 'i':
			/*
			 * User wants to ignore interrupts.
			 * Set the variable "ignore"
			 */
			assign("ignore", "");
			break;

		case 'U':
			UnUUCP++;
			break;

		case 'd':
			assign("debug", "");
			break;

		case 'h':
			/*
			 * Specified sequence number for network.
			 * This is the number of "hops" made so
			 * far (count of times message has been
			 * forwarded) to help avoid infinite mail loops.
			 */
			mustsend++;
			hflag = atoi(optarg);
			if (hflag == 0) {
				fprintf(stderr, "-h needs non-zero number\n");
				goerr++;
			}
			break;

		case 's':
			/*
			 * Give a subject field for sending from
			 * non terminal
			 */
			mustsend++;
			sflag = optarg;
			break;

		case 'f':
			/*
			 * User is specifying file to "edit" with mailx,
			 * as opposed to reading system mailbox.
			 * If no argument is given after -f, we read his/her
			 * $MBOX file or mbox in his/her home directory.
			 */
			ef = (argc == optind || *argv[optind] == '-')
				? "" : argv[optind++];
			if (*ef && *ef != '/' && *ef != '+')
				cwd = getcwd(NOSTR, PATHSIZE);
			break;

		case 'F':
			Fflag++;
			mustsend++;
			break;

		case 'n':
			/*
			 * User doesn't want to source
			 *	/etc/mail/mailx.rc
			 */
			nosrc++;
			break;

		case 'N':
			/*
			 * Avoid initial header printing.
			 */
			noheader++;
			break;

		case 'H':
			/*
			 * Print headers and exit
			 */
			Hflag++;
			break;

		case 'V':
			puts(version);
			return 0;

		case '~':
			/*
			 * Permit tildas no matter where
			 * the input is coming from.
			 */
			escapeokay++;
			break;

		case '?':
		default:
			goerr++;
			break;
		}

	if ( optind != argc )
		argp = optind;

	/*
	 * Check for inconsistent arguments.
	 */

	if (newsflg && ef==NOSTR) {
		fprintf(stderr, "Need -f with -I flag\n");
		goerr++;
	}
	if (ef != NOSTR && argp != -1) {
		fprintf(stderr, "Cannot give -f and people to send to.\n");
		goerr++;
	}
	if (exitflg && (mustsend || argp != -1))
		exit(1);	/* nonsense flags involving -e simply exit */
	if (mustsend && argp == -1) {
		fprintf(stderr, "The flags you gave are used only when sending mail.\n");
		goerr++;
	}
	if (goerr) {
		fprintf(stderr,"Usage: %s -eiIUdFnNHV~ -T FILE -u USER -h hops -r address -s SUBJECT -f FILE users\n", progname);
		exit(1);
	}
	tinit();
	input = stdin;
	rcvmode = argp == -1;
	if (!nosrc)
		load(MASTER);
	load(Getf("MAILRC"));

	if (argp != -1) {
		mail(&argv[argp]);
		exit(senderr);
	}

	/*
		If $MAIL is set, but is NOT the 'standard' place, then
		use it but pretend it was specified with -f.
	*/
	mf = getenv("MAIL");
	if ((mf != NOSTR) && (*mf != 0) &&
	    (ef == NOSTR) && (strcmp(mailname, mf) != 0)
#ifndef preSVr4
	    && (strcmp(altmailname, mf) != 0)
#endif
	    )
		ef = mf;

	/*
	 * Ok, we are reading mail.
	 * Decide whether we are editing a mailbox or reading
	 * the system mailbox, and open up the right stuff.
	 */

	strcpy(origname, mailname);

	if (ef != NOSTR) {
		ef = *ef ? safeexpand(ef) : Getf("MBOX");
		strcpy(origname, ef);
		if (ef[0] != '/') {
			if (cwd == NOSTR)
				cwd = getcwd(NOSTR, PATHSIZE);
			strcat(cwd, "/");
			strcat(cwd, ef);
			ef = cwd;
		}
		strcpy(mailname, ef);
		/* If the file is not under /var/mail, we're editing it. */
		if ((strncmp(mailname, maildir, strlen(maildir)) == 0)
#ifndef preSVr4
		   /* or under /usr/mail */
		   || (strncmp(mailname, altmaildir, strlen(altmaildir)) == 0)
#endif
		    ) {
			lockname = strrchr(ef, '/') + 1;
		} else {
			editfile = ef;
			edit++;
		}
	}

	if (setfile(mailname, edit) < 0)
		exit(1);

	if (msgCount > 0 && !noheader && value("header") != NOSTR) {
		if (setjmp(hdrjmp) == 0) {
			if ((prevint = sigset(SIGINT, SIG_IGN)) != SIG_IGN)
				sigset(SIGINT, hdrstop);
			announce();
			fflush(stdout);
			sigset(SIGINT, prevint);
		}
	}
	if (Hflag || (!edit && msgCount == 0)) {
		if (!Hflag) {
			fprintf(stderr, "No mail for %s\n", myname);
			Verhogen();
		}
		fflush(stdout);
		exit(0);
	}
	commands();
	if (!edit) {
		sigset(SIGHUP, SIG_IGN);
		sigset(SIGINT, SIG_IGN);
		sigset(SIGQUIT, SIG_IGN);
		quit();
		Verhogen();
	}
	exit(0);
	/* NOTREACHED */
}

/*
 * Interrupt printing of the headers.
 */
static void
hdrstop()
{

	clrbuf(stdout);
	printf("\nInterrupt\n");
	fflush(stdout);
	sigrelse(SIGINT);
	longjmp(hdrjmp, 1);
}
