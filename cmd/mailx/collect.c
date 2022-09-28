/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:collect.c	1.18.5.1"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Collect input from standard input, handling
 * ~ escapes.
 */

#include "rcv.h"

#ifdef SIGCONT
static void	collcont();
#endif
static void	collrub();
static void	cpout();
static int	exwrite();
static int	forward();
static void	intack();
static FILE	*mesedit();
static FILE	*mespipe();
static void	resetsigs();
static int	stripnulls();
static long	transmit();
static void	xhalt();
static char	**Xaddone();

/*
 * Read a message from standard output and return a read file to it
 * or NULL on error.
 */

/*
 * The following hokiness with global variables is so that on
 * receipt of an interrupt signal, the partial message can be salted
 * away on dead.letter.  The output file must be available to flush,
 * and the input to read.  Several open files could be saved all through
 * mailx if stdio allowed simultaneous read/write access.
 */

static void		(*savesig)();	/* Previous SIGINT value */
static void		(*savehup)();	/* Previous SIGHUP value */
#ifdef SIGCONT
static void		(*savecont)();	/* Previous SIGCONT value */
#endif
static FILE		*newi;		/* File for saving away */
static FILE		*newo;		/* Output side of same */
static int		hf;		/* Ignore interrups */
static int		hadintr;	/* Have seen one SIGINT so far */
static struct header	*savehp;
static jmp_buf		coljmp;		/* To get back to work */

FILE *
collect(hp)
	struct header *hp;
{
	FILE *ibuf, *fbuf, *obuf;
	int escape, eof;
	long lc, cc;
	register int c, t;
	char linebuf[LINESIZE+1], *cp;
	char *iprompt;
	void (*sigpipe)(), (*sigint)();

	noreset++;
	ibuf = obuf = NULL;
	newi = newo = NULL;
	if ((obuf = fopen(tempMail, "w")) == NULL) {
		perror(tempMail);
		goto err;
	}
	newo = obuf;
	if ((ibuf = fopen(tempMail, "r")) == NULL) {
		perror(tempMail);
		newo = NULL;
		fclose(obuf);
		goto err;
	}
	newi = ibuf;
	removefile(tempMail);

	hf = !!value("ignore");
	hadintr = 1;
	savehp = hp;
	if ((savesig = sigset(SIGINT, SIG_IGN)) != SIG_IGN)
		sigset(SIGINT, hf ? intack : collrub);
	if ((savehup = sigset(SIGHUP, SIG_IGN)) != SIG_IGN)
		sigset(SIGHUP, collrub);
#ifdef SIGCONT
# ifdef preSVr4
	savecont = sigset(SIGCONT, collcont);
# else
	{
	struct sigaction nsig, osig;
	nsig.sa_handler = collcont;
	sigemptyset(&nsig.sa_mask);
	nsig.sa_flags = SA_RESTART;
	(void) sigaction(SIGCONT, &nsig, &osig);
	savecont = osig.sa_handler;
	}
# endif
#endif
	/*
	 * If we are going to prompt for subject/cc/bcc,
	 * refrain from printing a newline after
	 * the headers (since some people mind).
	 */

	if (hp->h_subject == NOSTR) {
		hp->h_subject = sflag;
		sflag = NOSTR;
	}
	t = GMASK;
	c = 0;
	if (intty) {
		if (hp->h_subject == NOSTR && value("asksub"))
			c |= GSUBJECT;
		if (hp->h_cc == NOSTR && value("askcc"))
			c |= GCC;
		if (hp->h_bcc == NOSTR && value("askbcc"))
			c |= GBCC;
		if (c)
			t &= ~GNL;
	}
	if (hp->h_seq != 0) {
		puthead(hp, stdout, t);
		fflush(stdout);
	}
	if (setjmp(coljmp))
		goto err;
	if (c)
		grabh(hp, c, 1);
	escape = SENDESC;
	if ((cp = value("escape")) != NOSTR)
		escape = *cp;
	eof = 0;
	if ((cp = value("MAILX_HEAD")) != NOSTR) {
	      cpout( cp, obuf);
	      if (isatty(fileno(stdin)))
		    cpout( cp, stdout);
	}
	iprompt = value("iprompt");
	fflush(obuf);
	hadintr = 0;
	for (;;) {
		int nread, hasnulls;
		setjmp(coljmp);
		sigrelse(SIGINT);
		sigrelse(SIGHUP);
		if (intty && outtty && iprompt)
			fputs(iprompt, stdout);
		flush();
		if ((nread = getline(linebuf,LINESIZE,stdin,&hasnulls)) == NULL) {
			if (intty && value("ignoreeof") != NOSTR) {
				if (++eof > 35)
					break;
				printf("Use \".\" to terminate letter\n");
				continue;
			}
			break;
		}
		eof = 0;
		hadintr = 0;
		if (intty && equal(".\n", linebuf) &&
		    (value("dot") != NOSTR || value("ignoreeof") != NOSTR))
			break;
		if ((linebuf[0] != escape) || (rflag != NOSTR) || (!intty && !escapeokay)) {
			if (write(fileno(obuf),linebuf,nread) != nread)
				goto err;
			continue;
		}
		/*
		 * On double escape, just send the single one.
		 */
		if ((nread > 1) && (linebuf[1] == escape)) {
			if (write(fileno(obuf),linebuf+1,nread-1) != (nread-1))
				goto err;
			continue;
		}
		if (hasnulls)
			nread = stripnulls(linebuf, nread);
		c = linebuf[1];
		linebuf[nread - 1] = '\0';
		switch (c) {
		default:
			/*
			 * Otherwise, it's an error.
			 */
			printf("Unknown tilde escape.\n");
			break;

		case 'a':
		case 'A':
			/*
			 * autograph; sign the letter.
			 */

			if (cp = value(c=='a' ? "sign":"Sign")) {
			      cpout( cp, obuf);
			      if (isatty(fileno(stdin)))
				    cpout( cp, stdout);
			}

			break;

		case 'i':
			/*
			 * insert string
			 */
			for (cp = &linebuf[2]; any(*cp, " \t"); cp++)
				;
			if (*cp)
				cp = value(cp);
			if (cp != NOSTR) {
				cpout(cp, obuf);
				if (isatty(fileno(stdout)))
					cpout(cp, stdout);
			}
			break;

		case '!':
			/*
			 * Shell escape, send the balance of the
			 * line to sh -c.
			 */

			shell(&linebuf[2]);
			break;

		case ':':
		case '_':
			/*
			 * Escape to command mode, but be nice!
			 */

			execute(&linebuf[2], 1);
			iprompt = value("iprompt");
			if (cp = value("escape"))
				escape = *cp;
			printf("(continue)\n");
			break;

		case '.':
			/*
			 * Simulate end of file on input.
			 */
			goto eofl;

		case 'q':
		case 'Q':
			/*
			 * Force a quit of sending mail.
			 * Act like an interrupt happened.
			 */

			hadintr++;
			collrub(SIGINT);
			exit(1);
			/* NOTREACHED */

		case 'x':
			xhalt();
			break; 	/* not reached */

		case 'h':
			/*
			 * Grab a bunch of headers.
			 */
			if (!intty || !outtty) {
				printf("~h: no can do!?\n");
				break;
			}
			grabh(hp, GMASK, 0);
			printf("(continue)\n");
			break;

		case 't':
			/*
			 * Add to the To list.
			 */

			hp->h_to = addto(hp->h_to, &linebuf[2]);
			hp->h_seq++;
			break;

		case 's':
			/*
			 * Set the Subject list.
			 */

			cp = &linebuf[2];
			while (any(*cp, " \t"))
				cp++;
			hp->h_subject = savestr(cp);
			hp->h_seq++;
			break;

		case 'c':
			/*
			 * Add to the CC list.
			 */

			hp->h_cc = addto(hp->h_cc, &linebuf[2]);
			hp->h_seq++;
			break;

		case 'b':
			/*
			 * Add stuff to blind carbon copies list.
			 */
			hp->h_bcc = addto(hp->h_bcc, &linebuf[2]);
			hp->h_seq++;
			break;

		case 'R':
			hp->h_defopt = addone(hp->h_defopt, "/receipt");
			hp->h_seq++;
			fprintf(stderr, "Return receipt marked.\n");
			break;

		case 'd':
			copy(Getf("DEAD"), &linebuf[2]);
			/* FALLTHROUGH */

		case '<':
		case 'r': {
			int	ispip;
			/*
			 * Invoke a file:
			 * Search for the file name,
			 * then open it and copy the contents to obuf.
			 *
			 * if name begins with '!', read from a command
			 */

			cp = &linebuf[2];
			while (any(*cp, " \t"))
				cp++;
			if (*cp == '\0') {
				printf("Interpolate what file?\n");
				break;
			}
			if (*cp=='!') {
				/* take input from a command */
				ispip = 1;
				if ((fbuf = npopen(++cp, "r"))==NULL) {
					perror("");
					break;
				}
				sigint = sigset(SIGINT, SIG_IGN);
			} else {
				ispip = 0;
				cp = expand(cp);
				if (cp == NOSTR)
					break;
				if (isdir(cp)) {
					printf("%s: directory\n",cp);
					break;
				}
				if ((fbuf = fopen(cp, "r")) == NULL) {
					perror(cp);
					break;
				}
			}
			printf("\"%s\" ", cp);
			flush();
			lc = cc = 0;
			while ((t = getc(fbuf)) != EOF) {
				if (t == '\n')
					lc++;
				if (putc(t, obuf) == EOF) {
					if (ispip) {
						npclose(fbuf);
						sigset(SIGINT, sigint);
					} else
						fclose(fbuf);
					goto err;
				}
				cc++;
			}
			if (ispip) {
				npclose(fbuf);
				sigset(SIGINT, sigint);
			} else
				fclose(fbuf);
			printf("%ld/%ld\n", lc, cc);
			fflush(obuf);
			break;
			}

		case 'w':
			/*
			 * Write the message on a file.
			 */

			cp = &linebuf[2];
			while (any(*cp, " \t"))
				cp++;
			if (*cp == '\0') {
				fprintf(stderr, "Write what file!?\n");
				break;
			}
			if ((cp = expand(cp)) == NOSTR)
				break;
			fflush(obuf);
			rewind(ibuf);
			exwrite(cp, ibuf);
			break;

		case 'm':
		case 'f':
			/*
			 * Interpolate the named messages, if we
			 * are in receiving mail mode.  Does the
			 * standard list processing garbage.
			 * If ~f is given, we don't shift over.
			 */

			if (!rcvmode) {
				printf("No messages to send from!?!\n");
				break;
			}
			cp = &linebuf[2];
			while (any(*cp, " \t"))
				cp++;
			if (forward(cp, obuf, c) < 0)
				goto err;
			fflush(obuf);
			printf("(continue)\n");
			break;

		case '?':
			if ((fbuf = fopen(THELPFILE, "r")) == NULL) {
				printf("No help just now.\n");
				break;
			}
			t = getc(fbuf);
			while (t != -1) {
				putchar(t);
				t = getc(fbuf);
			}
			fclose(fbuf);
			break;

		case 'p':
			/*
			 * Print out the current state of the
			 * message without altering anything.
			 */

			fflush(obuf);
			rewind(ibuf);
			if (value("crt")) {
				char *pg = PG;
				if (!*pg)
					pg = "cat";
				if (fbuf = npopen(PG, "w")) {
					sigint = sigset(SIGINT, SIG_IGN);
					sigpipe = sigset(SIGPIPE, SIG_IGN);
				} else
					fbuf = stdout;
			} else
				fbuf = stdout;
			fprintf(fbuf, "-------\nMessage contains:\n");
			puthead(hp, fbuf, GMASK);
			while ((t = getc(ibuf))!=EOF)
				putc(t, fbuf);
			if (fbuf != stdout) {
				npclose(fbuf);
				sigset(SIGPIPE, sigpipe);
				sigset(SIGINT, sigint);
			}
			printf("(continue)\n");
			break;

		case '^':
		case '|':
			/*
			 * Pipe message through command.
			 * Collect output as new message.
			 */

			obuf = mespipe(ibuf, obuf, &linebuf[2]);
			newo = obuf;
			ibuf = newi;
			newi = ibuf;
			printf("(continue)\n");
			break;

		case 'v':
		case 'e':
			/*
			 * Edit the current message.
			 * 'e' means to use EDITOR
			 * 'v' means to use VISUAL
			 */

			if ((obuf = mesedit(ibuf, obuf, c, hp)) == NULL)
				goto err;
			newo = obuf;
			ibuf = newi;
			printf("(continue)\n");
			break;
		}
		fflush(obuf);
	}
eofl:
	fflush(obuf);
	if ((cp = value("MAILX_TAIL")) != NOSTR) {
	      cpout( cp, obuf);
	      if (isatty(fileno(stdin)))
		    cpout( cp, stdout);
	}
	fclose(obuf);
	rewind(ibuf);
	resetsigs(0);
	noreset = 0;
	return(ibuf);

err:
	if (ibuf != NULL)
		fclose(ibuf);
	if (obuf != NULL)
		fclose(obuf);
	resetsigs(0);
	noreset = 0;
	return(NULL);
}

static void resetsigs(resethup)
int resethup;
{
	(void) sigset(SIGINT, savesig);
	if (resethup)
		(void) sigset(SIGHUP, savehup);
#ifdef SIGCONT
# ifdef preSVr4
	(void) sigset(SIGCONT, savecont);
# else
	{
	struct sigaction nsig;
	nsig.sa_handler = savecont;
	sigemptyset(&nsig.sa_mask);
	nsig.sa_flags = SA_RESTART;
	(void) sigaction(SIGCONT, &nsig, (struct sigaction*)0);
	}
# endif
#endif
}

/*
 * Write a file ex-like.
 */

static int
exwrite(name, ibuf)
	char name[];
	FILE *ibuf;
{
	register FILE *of;
	struct stat junk;
	void (*sigint)(), (*sigpipe)();
	int pi = (*name == '!');

	if (!pi && stat(name, &junk) >= 0
	 && (junk.st_mode & S_IFMT) == S_IFREG) {
		fprintf(stderr, "%s: File exists\n", name);
		return(-1);
	}
	if ((of = pi ? npopen(++name, "w") : fopen(name, "w")) == NULL) {
		perror(name);
		return(-1);
	}
	if (pi) {
		sigint = sigset(SIGINT, SIG_IGN);
		sigpipe = sigset(SIGPIPE, SIG_IGN);
	}
	lcwrite(name, ibuf, of);
	pi ? npclose(of) : fclose(of);
	if (pi) {
		sigset(SIGPIPE, sigpipe);
		sigset(SIGINT, sigint);
	}
	return(0);
}

void
lcwrite(fn, fi, fo)
char *fn;
FILE *fi, *fo;
{
	register int c;
	long lc, cc;

	printf("\"%s\" ", fn);
	fflush(stdout);
	lc = cc = 0;
	while ((c = getc(fi)) != EOF) {
		cc++;
		if (putc(c, fo) == '\n')
			lc++;
		if (ferror(fo)) {
			perror("");
			return;
		}
	}
	printf("%ld/%ld\n", lc, cc);
	fflush(stdout);
}

/*
 * Edit the message being collected on ibuf and obuf.
 * Write the message out onto some poorly-named temp file
 * and point an editor at it.
 *
 * On return, make the edit file the new temp file.
 */

static FILE *
mesedit(ibuf, obuf, c, hp)
	FILE *ibuf, *obuf;
	struct header *hp;
{
	pid_t pid;
	FILE *fbuf;
	register int t;
	void (*sigint)();
	struct stat sbuf;
	register char *edit;
	char hdr[LINESIZE];

	if (stat(tempEdit, &sbuf) >= 0) {
		printf("%s: file exists\n", tempEdit);
		goto out;
	}
	close(creat(tempEdit, TEMPPERM));
	if ((fbuf = fopen(tempEdit, "w")) == NULL) {
		perror(tempEdit);
		goto out;
	}
	fflush(obuf);
	rewind(ibuf);
	puthead(hp, fbuf, GMASK);
	while ((t = getc(ibuf)) != EOF)
		putc(t, fbuf);
	fflush(fbuf);
	if (ferror(fbuf)) {
		perror(tempEdit);
		removefile(tempEdit);
		goto out;
	}
	fclose(fbuf);
	if ((edit = value(c == 'e' ? "EDITOR" : "VISUAL")) == NOSTR)
		edit = c == 'e' ? EDITOR : VISUAL;
	edit = safeexpand(edit);

	/*
	 * Fork/execlp the editor on the edit file
	*/

	pid = vfork();
	if ( pid == (pid_t)-1 ) {
		perror("fork");
		removefile(tempEdit);
		goto out;
	}
	if ( pid == 0 ) {
		sigchild();
		execlp(edit, edit, tempEdit, (char *)0);
		perror(edit);
		_exit(1);
	}
	sigint = sigset(SIGINT, SIG_IGN);
	while (wait((int *)0) != pid)
		;
	sigset(SIGINT, sigint);
	/*
	 * Now switch to new file.
	 */

	if ((fbuf = fopen(tempEdit, "r")) == NULL) {
		perror(tempEdit);
		removefile(tempEdit);
		goto out;
	}
	removefile(tempEdit);
	hp->h_to = hp->h_subject = hp->h_cc = hp->h_bcc = hp->h_defopt = NOSTR;
	hp->h_others = NOSTRPTR;
	hp->h_seq = 0;
	while (gethfield(fbuf, hdr, 9999L) > 0) {
		if (ishfield(hdr, "to"))
			hp->h_to = addto(hp->h_to, hcontents(hdr));
		else if (ishfield(hdr, "subject"))
			hp->h_subject = addone(hp->h_subject, hcontents(hdr));
		else if (ishfield(hdr, "cc"))
			hp->h_cc = addto(hp->h_cc, hcontents(hdr));
		else if (ishfield(hdr, "bcc"))
			hp->h_bcc = addto(hp->h_bcc, hcontents(hdr));
		else if (ishfield(hdr, "default-options"))
			hp->h_defopt = addone(hp->h_defopt, hcontents(hdr));
		else
			hp->h_others = Xaddone(hp->h_others, hdr);
		hp->h_seq++;
	}
	if ((obuf = fopen(tempMail, "w")) == NULL) {
		perror(tempMail);
		fclose(fbuf);
		goto out;
	}
	if ((ibuf = fopen(tempMail, "r")) == NULL) {
		perror(tempMail);
		removefile(tempMail);
		fclose(fbuf);
		fclose(obuf);
		goto out;
	}
	removefile(tempMail);
	if (strlen(hdr) > 0) {
		fputs(hdr, obuf);
		putc('\n', obuf);
	}
	while ((t = getc(fbuf)) != EOF)
		putc(t, obuf);
	fclose(fbuf);
	fclose(newo);
	fclose(newi);
	newo = obuf;
	newi = ibuf;
out:
	return(newo);
}

/*
 * Pipe the message through the command.
 * Old message is on stdin of command;
 * New message collected from stdout.
 * Sh -c must return 0 to accept the new message.
 */

static FILE *
mespipe(ibuf, obuf, cmd)
	FILE *ibuf, *obuf;
	char cmd[];
{
	register FILE *ni, *no;
	pid_t pid;
	int s;
	void (*sigint)();
	char *Shell;

	newi = ibuf;
	if ((no = fopen(tempEdit, "w")) == NULL) {
		perror(tempEdit);
		return(obuf);
	}
	if ((ni = fopen(tempEdit, "r")) == NULL) {
		perror(tempEdit);
		fclose(no);
		removefile(tempEdit);
		return(obuf);
	}
	removefile(tempEdit);
	fflush(obuf);
	rewind(ibuf);
	if ((Shell = value("SHELL")) == NULL || *Shell=='\0')
		Shell = SHELL;
	if ((pid = vfork()) == (pid_t)-1) {
		perror("fork");
		goto err;
	}
	if (pid == 0) {
		/*
		 * stdin = current message.
		 * stdout = new message.
		 */

		sigchild();
		close(0);
		dup(fileno(ibuf));
		close(1);
		dup(fileno(no));
		for (s = 4; s < 15; s++)
			close(s);
		execlp(Shell, Shell, "-c", cmd, (char *)0);
		perror(Shell);
		_exit(1);
	}
	sigint = sigset(SIGINT, SIG_IGN);
	while (wait(&s) != pid)
		;
	sigset(SIGINT, sigint);
	if (s != 0 || pid == (pid_t)-1) {
		fprintf(stderr, "\"%s\" failed!?\n", cmd);
		goto err;
	}
	if (fsize(ni) == 0) {
		fprintf(stderr, "No bytes from \"%s\" !?\n", cmd);
		goto err;
	}

	/*
	 * Take new files.
	 */

	newi = ni;
	fclose(ibuf);
	fclose(obuf);
	return(no);

err:
	fclose(no);
	fclose(ni);
	return(obuf);
}

/*
 * Interpolate the named messages into the current
 * message, preceding each line with a tab.
 * Return a count of the number of characters now in
 * the message, or -1 if an error is encountered writing
 * the message temporary.  The flag argument is 'm' if we
 * should shift over and 'f' if not.
 */
static int
forward(ms, obuf, f)
	char ms[];
	FILE *obuf;
{
	register int *msgvec, *ip;

	msgvec = (int *) salloc((msgCount+1) * sizeof *msgvec);
	if (msgvec == NOINTPTR)
		return(0);
	if (getmsglist(ms, msgvec, 0) < 0)
		return(0);
	if (*msgvec == NULL) {
		*msgvec = first(0, MMNORM);
		if (*msgvec == NULL) {
			printf("No appropriate messages\n");
			return(0);
		}
		msgvec[1] = NULL;
	}
	printf("Interpolating:");
	for (ip = msgvec; *ip != NULL; ip++) {
		touch(*ip);
		printf(" %d", *ip);
		if (f == 'm') {
			if (transmit(&message[*ip-1], obuf) < 0L) {
				perror(tempMail);
				return(-1);
			}
		} else
			if (send(&message[*ip-1], obuf, 0) < 0) {
				perror(tempMail);
				return(-1);
			}
	}
	printf("\n");
	return(0);
}

/*
 * Send message described by the passed pointer to the
 * passed output buffer.  Insert a tab in front of each
 * line.  Return a count of the characters sent, or -1
 * on error.
 */

static long
transmit(mailp, obuf)
	struct message *mailp;
	FILE *obuf;
{
	register struct message *mp;
	register int ch;
	register long c, n;
	int bol;
	FILE *ibuf;
	char *mprefix = value("mprefix");

	if (!mprefix)
		mprefix = "\t";
	mp = mailp;
	ibuf = setinput(mp);
	c = mp->m_size;
	n = c;
	bol = 1;
	while (c-- > 0L) {
		if (bol) {
			bol = 0;
			fputs(mprefix, obuf);
			n++;
			if (ferror(obuf)) {
				perror("/tmp");
				return(-1L);
			}
		}
		ch = getc(ibuf);
		if (ch == '\n')
			bol++;
		putc(ch, obuf);
		if (ferror(obuf)) {
			perror("/tmp");
			return(-1L);
		}
	}
	return(n);
}

/*
 * Print (continue) when continued after ^Z.
 */
#ifdef SIGCONT
/*ARGSUSED*/
static void
collcont(s)
{
	printf("(continue)\n");
	fflush(stdout);
}
#endif /* SIGCONT */

/*
 * On interrupt, go here to save the partial
 * message on ~/dead.letter.
 * Then restore signals and execute the normal
 * signal routine.  We only come here if signals
 * were previously set anyway.
 */
static void
collrub(s)
{
	register FILE *dbuf;
	register int c;
	register char *deadletter = Getf("DEAD");
	long cc, lc;

	if (s == SIGHUP)
		sigignore(SIGHUP);
	if (s == SIGINT && hadintr == 0) {
		hadintr++;
		clrbuf(stdout);
		printf("\n(Interrupt -- one more to kill letter)\n");
		sigrelse(s);
		longjmp(coljmp, 1);
	}
	fclose(newo);
	rewind(newi);
	if (s == SIGINT && value("save")==NOSTR || fsize(newi) == 0)
		goto done;
	if ((dbuf = fopen(deadletter, "a")) == NULL) {
		perror(deadletter);
		goto done;
	}
	chmod(deadletter, DEADPERM);
	printf("\"%s\" ", deadletter);
	fflush(stdout);
	puthead(savehp, dbuf, GMASK);
	for (cc = lc = 0; (c = getc(newi)) != EOF; cc++)
		if (putc(c, dbuf) == '\n')
			lc++;
	fclose(dbuf);
	printf("%ld/%ld\n", lc, cc);
done:
	fclose(newi);
	resetsigs(1);
	if (rcvmode) {
		if (s == SIGHUP)
			hangup();
		else
			stop(s);
	}
	else
		exit(1);
}

/*
 * Acknowledge an interrupt signal from the tty by typing an @
 */
static void
intack(s)
{
	(void) s;
	puts("@");
	fflush(stdout);
	clearerr(stdin);
	longjmp(coljmp,1);
}

/* Read line from stdin, noting any NULL characters.
   Return the number of characters read. Note that the buffer
   passed must be 1 larger than "size" for the trailing NUL byte.
 */
int
getline(line,size,f,hasnulls)
char *line;
int size;
FILE *f;
int *hasnulls;
{
	register int i, ch;
	for (i = 0; (i < size) && ((ch=getc(f)) != EOF); ) {
		if ( ch == '\0' )
			*hasnulls = 1;
		if ((line[i++] = ch) == '\n') break;
	}
	line[i] = '\0';
	return(i);
}

void
savedead(s)
{
	(void) s;
	collrub(SIGINT);
	exit(1);
	/* NOTREACHED */
}

/*
 * Add a list of addresses to the end of a header entry field.
 */
char *
addto(hf, news)
	char hf[], news[];
{
	char name[LINESIZE];
	int comma = docomma(news);

	while (news = yankword(news, name, comma)) {
		strcat(name, ", ");
		hf = addone(hf, name);
	}
	return hf;
}

/*
 * Add a string to the end of a header entry field.
 */
char *
addone(hf, news)
	char hf[], news[];
{
	register char *cp, *cp2, *linebuf;

	if (hf == NOSTR)
		hf = "";
	if (*news == '\0')
		return(hf);
	linebuf = salloc((unsigned)(strlen(hf) + strlen(news) + 2));
	for (cp = hf; any(*cp, " \t"); cp++)
		;
	for (cp2 = linebuf; *cp;)
		*cp2++ = *cp++;
	if (cp2 > linebuf && cp2[-1] != ' ')
		*cp2++ = ' ';
	for (cp = news; any(*cp, " \t"); cp++)
		;
	while (*cp != '\0')
		*cp2++ = *cp++;
	*cp2 = '\0';
	return(linebuf);
}

static int nptrs(hf)
char **hf;
{
	register int i;
	if (!hf)
		return 0;
	for (i = 0; *hf; hf++)
		i++;
	return i;
}

/*
 * Add a non-standard header to the end of the non-standard headers.
 */
static char **
Xaddone(hf, news)
	char **hf, news[];
{
	register char *linebuf;
	char **ohf = hf;
	int nhf = nptrs(hf);

	if (hf == NOSTRPTR)
		hf = (char**)salloc(sizeof(char*) * 2);
	else
		hf = (char**)srealloc(hf, sizeof(char*) * (nhf + 2));
	if (hf == NOSTRPTR) {
		fprintf(stderr, "No room, header lost: %s\n", news);
		return(ohf);
	}
	linebuf = salloc((unsigned)(strlen(news) + 1));
	strcpy(linebuf, news);
	hf[nhf++] = linebuf;
	hf[nhf] = NOSTR;
	return(hf);
}

static void
cpout( str, ofd )
char *str;
FILE *ofd;
{
      register char *cp = str;

      while ( *cp ) {
	    if ( *cp == '\\' ) {
		  switch ( *(cp+1) ) {
			case 'n':
			      putc('\n',ofd);
			      cp++;
			      break;
			case 't':
			      putc('\t',ofd);
			      cp++;
			      break;
			default:
			      putc('\\',ofd);
		  }
	    }
	    else {
		  putc(*cp,ofd);
	    }
	    cp++;
      }
      putc('\n',ofd);
      fflush(ofd);
}

static void
xhalt()
{
	fclose(newo);
	fclose(newi);
	sigset(SIGINT, savesig);
	sigset(SIGHUP, savehup);
	if (rcvmode)
		stop(0);
	exit(1);
	/* NOTREACHED */
}

/*
	Strip the nulls from a buffer of length n
*/
static int
stripnulls(linebuf, nread)
register char *linebuf;
register int nread;
{
	register int i, j;
	for (i = 0; i < nread; i++)
		if (linebuf[i] == '\0')
			break;
	for (j = i; j < nread; j++)
		if (linebuf[j] != '\0')
			linebuf[i++] = linebuf[j];
	linebuf[i] = '\0';
	return i;
}
