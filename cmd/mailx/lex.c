/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:lex.c	1.21.7.1"

#include "rcv.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Lexical processing of commands.
 */

#ifdef SIGCONT
static void		contin();
#endif
static int		isprefix();
static struct cmd	*lex();
static int		Passeren();
static void		setmsize();

/*
 * Set up editing on the given file name.
 * If isedit is true, we are considered to be editing the file,
 * otherwise we are reading our mail which has signficance for
 * mbox and so forth.
 */

setfile(name, isedit)
	char *name;
	int isedit;
{
	FILE *ibuf;
	int i;
	static int shudclob;
	static char efile[PATHSIZE];
	char fortest[128];
	struct stat stbuf;
	int exrc = 1;
	int rc = -1;

	if (!isedit)
		lockmail();
	if ((ibuf = fopen(name, "r")) == NULL) {
		extern int errno; 
		int sverrno = errno;
		int filethere = (access(name,0) == 0);
		errno = sverrno;
		if (exitflg)
			goto doexit;	/* no mail, return error */
		if (isedit || filethere)
			perror(name);
		else if (!Hflag) {
			char *f = strrchr(name,'/');
			if (f == NOSTR)
				fprintf(stderr, "No mail.\n");
			else
				fprintf(stderr, "No mail for %s\n", f+1);
		}
		goto doret;
	}
	fstat(fileno(ibuf), &stbuf);
	if (stbuf.st_size == 0L) {
		if (exitflg)
			goto doexit;	/* no mail, return error */
		if (isedit)
			fprintf(stderr, "%s: empty file\n", name);
		else if (!Hflag) {
			if (strrchr(name,'/') == NOSTR)
				fprintf(stderr, "No mail.\n");
			else
				fprintf(stderr, "No mail for %s\n", strrchr(name,'/')+1);
		}
		fclose(ibuf);
		goto doret;
	}

	fgets(fortest, sizeof fortest, ibuf);
	fseek(ibuf, 0L, 0);
	if (strncmp(fortest, "Forward to ", 11) == 0) {
		if (exitflg)
			goto doexit;	/* no mail, return error */
		fprintf(stderr, "Your mail is being forwarded to %s", fortest+11);
		fclose(ibuf);
		goto doret;
	}
	if (exitflg) {
		exrc = 0;
		goto doexit;	/* there is mail, return success */
	}

	/*
	 * Looks like all will be well. Must hold signals
	 * while we are reading the new file, else we will ruin
	 * the message[] data structure.
	 * Copy the messages into /tmp and set pointers.
	 */

	if (shudclob) {
		holdsigs();
		if (edit)
			edstop();
		else {
			quit();
			Verhogen();
		}
		relsesigs();
	}

	readonly = 0;
	if (!isedit && !Hflag)
		readonly = Passeren()==-1;
	lock(ibuf, "r", 1);
	fstat(fileno(ibuf), &stbuf);
	utimep->actime = stbuf.st_atime;
	utimep->modtime = stbuf.st_mtime;

	holdsigs();
	if (!readonly)
		if ((i = open(name, O_WRONLY)) < 0)
			readonly++;
		else
			close(i);
	if (shudclob) {
		fclose(itf);
		fclose(otf);
		free(message);
		space=0;
	}
	shudclob = 1;
	edit = isedit;
	strncpy(efile, name, PATHSIZE);
	editfile = efile;
	if (name != mailname)
		strcpy(mailname, name);
	mailsize = fsize(ibuf);
	if ((otf = fopen(tempMesg, "w")) == NULL) {
		perror(tempMesg);
		if (!edit)
			Verhogen();
		goto doexit;
	}
	if ((itf = fopen(tempMesg, "r")) == NULL) {
		perror(tempMesg);
		if (!edit)
			Verhogen();
		goto doexit;
	}
	removefile(tempMesg);
	setptr(ibuf);
	setmsize(msgCount);
	fclose(ibuf);
	relsesigs();
	sawcom = 0;
	rc = 0;

    doret:
	if (!isedit)
		unlockmail();
	return(rc);

    doexit:
	if (!isedit)
		unlockmail();
	exit(exrc);
	/* NOTREACHED */
}

/* global to semaphores */
static char semfn[128];
static FILE *semfp = NULL;

/*
 *  return -1 if file is already being read, 0 otherwise
 */
static
Passeren()
{
	char *home;

	if ((home = getenv("HOME")) == NULL)
		return 0;
	strcpy(semfn, home);
	strcat(semfn, "/.Maillock");
	if ((semfp = fopen(semfn, "w")) != NULL && lock(semfp, "w", 0) < 0) {
		fprintf(stderr, "WARNING: You are already reading mail.\n");
		fprintf(stderr, "\t This instance of mail is read only.\n");
		fclose(semfp);
		semfp = NULL;
		return -1;
	}
	return 0;
}

void
Verhogen()
{
	if (semfp != NULL) {
		unlink(semfn);
		fclose(semfp);
	}
}

/*
 * Interpret user commands one by one.  If standard input is not a tty,
 * print no prompt.
 */

static int	*msgvec;
static int	shudprompt;

void
commands()
{
	int eofloop;
	register int n;
	char linebuf[LINESIZE];
	struct stat minfo;
	FILE *ibuf;

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
	if (rcvmode && !sourcing) {
		if (sigset(SIGINT, SIG_IGN) != SIG_IGN)
			sigset(SIGINT, stop);
		if (sigset(SIGHUP, SIG_IGN) != SIG_IGN)
			sigset(SIGHUP, hangup);
	}
	for (;;) {
		setjmp(srbuf);

		/*
		 * Print the prompt, if needed.  Clear out
		 * string space, and flush the output.
		 */

		if (!rcvmode && !sourcing)
			return;
		eofloop = 0;
top:
		if ((shudprompt = (intty && !sourcing)) != 0) {
			if (prompt==NOSTR)
				prompt = "? ";
#ifdef SIGCONT
# ifdef preSVr4
			sigset(SIGCONT, contin);
# else
			{
			struct sigaction nsig;
			nsig.sa_handler = contin;
			sigemptyset(&nsig.sa_mask);
			nsig.sa_flags = SA_RESTART;
			(void) sigaction(SIGCONT, &nsig, (struct sigaction*)0);
			}
# endif
#endif
			if (stat(mailname, &minfo) >=0 && minfo.st_size > mailsize) {
				int OmsgCount, i;
				OmsgCount = msgCount;
				fseek(otf, 0L, 2);
				holdsigs();
				lockmail();
				if ( (ibuf = fopen(mailname, "r")) == NULL ) {
					fprintf(stderr,"Can't reopen %s\n",mailname);
					unlockmail();
					exit(1);
					/* NOTREACHED */
				}
				/* lock(ibuf, "r", 1); */
				fseek(ibuf, mailsize, 0);
				setptr(ibuf);
				setmsize(msgCount);
				stat(mailname, &minfo);
				mailsize = minfo.st_size;
				fclose(ibuf);
				unlockmail();
				if (msgCount-OmsgCount > 0) {
					printf("New mail has arrived.\n");
					printf("Loaded %d new message%s\n",
						msgCount-OmsgCount,
						plural(msgCount-OmsgCount));
					if (value("header") != NOSTR)
						for (i = OmsgCount+1;
						     i <= msgCount; i++) {
							printhead(i);
							sreset();
						}
				}
				relsesigs();
			}
			printf("%s", prompt);
		}
		flush();
		sreset();

		/*
		 * Read a line of commands from the current input
		 * and handle end of file specially.
		 */

		n = 0;
		for (;;) {
			if (readline(input, &linebuf[n]) <= 0) {
				if (n != 0)
					break;
				if (loading)
					return;
				if (sourcing) {
					unstack();
					goto more;
				}
				if (value("ignoreeof") != NOSTR && shudprompt) {
					if (++eofloop < 25) {
						printf("Use \"quit\" to quit.\n");
						goto top;
					}
				}
				if (edit)
					edstop();
				return;
			}
			if ((n = strlen(linebuf)) == 0)
				break;
			n--;
			if (linebuf[n] != '\\')
				break;
			linebuf[n++] = ' ';
		}
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
		if (execute(linebuf, 0))
			return;
more:		;
	}
}

/*
 * Execute a single command.  If the command executed
 * is "quit," then return non-zero so that the caller
 * will know to return back to main, if he cares.
 * Contxt is non-zero if called while composing mail.
 */

execute(linebuf, contxt)
	char linebuf[];
{
	char word[LINESIZE];
	char *arglist[MAXARGC];
	struct cmd *com;
	register char *cp, *cp2;
	register int c, e;
	int muvec[2];

	/*
	 * Strip the white space away from the beginning
	 * of the command, then scan out a word, which
	 * consists of anything except digits and white space.
	 *
	 * Handle |, ! and # differently to get the correct
	 * lexical conventions.
	 */

	cp = linebuf;
	while (any(*cp, " \t"))
		cp++;
	cp2 = word;
	if (any(*cp, "!|#"))
		*cp2++ = *cp++;
	else
		while (*cp && !any(*cp, " \t0123456789$^.:/-+*'\""))
			*cp2++ = *cp++;
	*cp2 = '\0';

	/*
	 * Look up the command; if not found, complain.
	 * Normally, a blank command would map to the
	 * first command in the table; while sourcing,
	 * however, we ignore blank lines to eliminate
	 * confusion.
	 */

	if (sourcing && equal(word, ""))
		return(0);
	com = lex(word);
	if (com == NONE) {
		printf("Unknown command: \"%s\"\n", word);
		if (loading) {
			cond = CANY;
			return(1);
		}
		if (sourcing) {
			cond = CANY;
			unstack();
		}
		return(0);
	}

	/*
	 * See if we should execute the command -- if a conditional
	 * we always execute it, otherwise, check the state of cond.
	 */

	if ((com->c_argtype & F) == 0)
		if (cond == CRCV && !rcvmode || cond == CSEND && rcvmode)
			return(0);

	/*
	 * Special case so that quit causes a return to
	 * main, who will call the quit code directly.
	 * If we are in a source file, just unstack.
	 */

	if (com->c_func == edstop && sourcing) {
		if (loading)
			return(1);
		unstack();
		return(0);
	}
	if (!edit && com->c_func == edstop) {
		sigset(SIGINT, SIG_IGN);
		return(1);
	}

	/*
	 * Process the arguments to the command, depending
	 * on the type he expects.  Default to an error.
	 * If we are sourcing an interactive command, it's
	 * an error.
	 */

	if (!rcvmode && (com->c_argtype & M) == 0) {
		printf("May not execute \"%s\" while sending\n",
		    com->c_name);
		if (loading)
			return(1);
		if (sourcing)
			unstack();
		return(0);
	}
	if (sourcing && com->c_argtype & I) {
		printf("May not execute \"%s\" while sourcing\n",
		    com->c_name);
		if (loading)
			return(1);
		unstack();
		return(0);
	}
	if (readonly && com->c_argtype & W) {
		printf("May not execute \"%s\" -- message file is read only\n",
		   com->c_name);
		if (loading)
			return(1);
		if (sourcing)
			unstack();
		return(0);
	}
	if (contxt && com->c_argtype & R) {
		printf("Cannot recursively invoke \"%s\"\n", com->c_name);
		return(0);
	}
	e = 1;
	switch (com->c_argtype & ~(F|P|I|M|T|W|R)) {
	case MSGLIST:
		/*
		 * A message list defaulting to nearest forward
		 * legal message.
		 */
		if (msgvec == 0) {
			printf("Illegal use of \"message list\"\n");
			return(-1);
		}
		if ((c = getmsglist(cp, msgvec, com->c_msgflag)) < 0)
			break;
		if (c  == 0)
			if (msgCount == 0)
				*msgvec = NULL;
			else {
				*msgvec = first(com->c_msgflag,
					com->c_msgmask);
				msgvec[1] = NULL;
			}
		if (*msgvec == NULL) {
			printf("No applicable messages\n");
			break;
		}
		e = (*com->c_func)(msgvec);
		break;

	case NDMLIST:
		/*
		 * A message list with no defaults, but no error
		 * if none exist.
		 */
		if (msgvec == 0) {
			printf("Illegal use of \"message list\"\n");
			return(-1);
		}
		if (getmsglist(cp, msgvec, com->c_msgflag) < 0)
			break;
		e = (*com->c_func)(msgvec);
		break;

	case STRLIST:
		/*
		 * Just the straight string, with
		 * leading blanks removed.
		 */
		while (any(*cp, " \t"))
			cp++;
		e = (*com->c_func)(cp);
		break;

	case RAWLIST:
		/*
		 * A vector of strings, in shell style.
		 */
		if ((c = getrawlist(cp, arglist)) < 0)
			break;
		if (c < com->c_minargs) {
			printf("%s requires at least %d arg(s)\n",
				com->c_name, com->c_minargs);
			break;
		}
		if (c > com->c_maxargs) {
			printf("%s takes no more than %d arg(s)\n",
				com->c_name, com->c_maxargs);
			break;
		}
		e = (*com->c_func)(arglist);
		break;

	case NOLIST:
		/*
		 * Just the constant zero, for exiting,
		 * eg.
		 */
		e = (*com->c_func)(0);
		break;

	default:
		panic("Unknown argtype");
	}

	/*
	 * Exit the current source file on
	 * error.
	 */

	if (e && loading)
		return(1);
	if (e && sourcing)
		unstack();
	if (com->c_func == edstop)
		return(1);
	if (value("autoprint") != NOSTR && com->c_argtype & P)
		if ((dot->m_flag & MDELETED) == 0) {
			muvec[0] = dot - &message[0] + 1;
			muvec[1] = 0;
			type(muvec);
		}
	if (!sourcing && (com->c_argtype & T) == 0)
		sawcom = 1;
	return(0);
}

#ifdef SIGCONT
/*
 * When we wake up after ^Z, reprint the prompt.
 */
static void
contin()
{
	if (shudprompt)
		printf("%s", prompt);
	fflush(stdout);
}
#endif

/*
 * Branch here on hangup signal and simulate quit.
 */
void
hangup()
{

	holdsigs();
	sigignore(SIGHUP);
	if (edit) {
		if (setjmp(srbuf))
			exit(0);
		edstop();
	} else {
		Verhogen();
		if (value("exit") != NOSTR)
			exit(1);
		else
			quit();
	}
	exit(0);
}

/*
 * Set the size of the message vector used to construct argument
 * lists to message list functions.
 */

static void
setmsize(sz)
{

	if (msgvec != (int *) 0)
		free(msgvec);
	if (sz < 1)
		sz = 1; /* need at least one cell for terminating 0 */
	msgvec = (int *) calloc((unsigned) (sz + 1), sizeof *msgvec);
}

/*
 * Find the correct command in the command table corresponding
 * to the passed command "word"
 */

static struct cmd *
lex(word)
	char word[];
{
	register struct cmd *cp;

	for (cp = &cmdtab[0]; cp->c_name != NOSTR; cp++)
		if (isprefix(word, cp->c_name))
			return(cp);
	return(NONE);
}

/*
 * Determine if as1 is a valid prefix of as2.
 */
static int
isprefix(as1, as2)
	char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ == *s2)
		if (*s2++ == '\0')
			return(1);
	return(*--s1 == '\0');
}

/*
 * The following gets called on receipt of a rubout.  This is
 * to abort printout of a command, mainly.
 * Dispatching here when command() is inactive crashes rcv.
 * Close all open files except 0, 1, 2, and the temporary.
 * The special call to getuserid() is needed so it won't get
 * annoyed about losing its open file.
 * Also, unstack all source files.
 */

static int	inithdr;		/* am printing startup headers */

void
stop(s)
{
	register NODE *head;

	noreset = 0;
	if (!inithdr)
		sawcom++;
	inithdr = 0;
	while (sourcing)
		unstack();
	getuserid((char *) 0);
	for (head = fplist; head != (NODE *)NULL; head = head->next) {
		if (head->fp == stdin || head->fp == stdout)
			continue;
		if (head->fp == itf || head->fp == otf)
			continue;
		if (head->fp == stderr)
			continue;
		if (head->fp == pipef) {
			npclose(pipef);
			pipef = NULL;
			continue;
		}
		fclose(head->fp);
	}
	if (image >= 0) {
		close(image);
		image = -1;
	}
	clrbuf(stdout);
	if (s) {
		printf("Interrupt\n");
		sigrelse(s);
	}
	longjmp(srbuf, 1);
}

/*
 * Announce the presence of the current mailx version,
 * give the message count, and print a header listing.
 */

static char	*greeting	= "%s  Type ? for help.\n";

void
announce()
{
	int vec[2], mdot;

	if (!Hflag && value("quiet")==NOSTR)
		printf(greeting, version);
	mdot = newfileinfo();
	vec[0] = mdot;
	vec[1] = 0;
	dot = &message[mdot - 1];
	if (msgCount > 0 && !noheader) {
		inithdr++;
		headers(vec);
		inithdr = 0;
	}
}

/*
 * Announce information about the file we are editing.
 * Return a likely place to set dot.
 */
newfileinfo()
{
	register struct message *mp;
	register int u, n, mdot, d, s;
	char fname[BUFSIZ], zname[BUFSIZ], *ename;

	if (Hflag)
		return(1);
	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if (mp->m_flag & MNEW)
			break;
	if (mp >= &message[msgCount])
		for (mp = &message[0]; mp < &message[msgCount]; mp++)
			if ((mp->m_flag & MREAD) == 0)
				break;
	if (mp < &message[msgCount])
		mdot = mp - &message[0] + 1;
	else
		mdot = 1;
	s = d = 0;
	for (mp = &message[0], n = 0, u = 0; mp < &message[msgCount]; mp++) {
		if (mp->m_flag & MNEW)
			n++;
		if ((mp->m_flag & MREAD) == 0)
			u++;
		if (mp->m_flag & MDELETED)
			d++;
		if (mp->m_flag & MSAVED)
			s++;
	}
	ename=origname;
	if (getfold(fname) >= 0) {
		strcat(fname, "/");
		if (strncmp(fname, mailname, strlen(fname)) == 0) {
			sprintf(zname, "+%s", mailname + strlen(fname));
			ename = zname;
		}
	}
	printf("\"%s\": ", ename);
	if (msgCount == 1)
		printf("1 message");
	else
		printf("%d messages", msgCount);
	if (n > 0)
		printf(" %d new", n);
	if (u-n > 0)
		printf(" %d unread", u);
	if (d > 0)
		printf(" %d deleted", d);
	if (s > 0)
		printf(" %d saved", s);
	if (readonly)
		printf(" [Read only]");
	printf("\n");
	return(mdot);
}

/*
 * Print the current version number.
 */

pversion(e)
	char *e;
{
	(void) e;
	printf("%s\n", version);
	return(0);
}

/*
 * Load a file of user definitions.
 */
void
load(name)
	char *name;
{
	register FILE *in, *oldin;

	if ((in = fopen(name, "r")) == NULL)
		return;
	oldin = input;
	input = in;
	loading = 1;
	sourcing = 1;
	commands();
	loading = 0;
	sourcing = 0;
	input = oldin;
	fclose(in);
}
