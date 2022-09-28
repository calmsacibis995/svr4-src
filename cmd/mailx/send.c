/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:send.c	1.11.5.1"

#include "rcv.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Mail to others.
 */

static void		fmt();
static FILE		*infix();
static void		statusput();
static int		savemail();
static int		sendmail();
static int		Sendmail();

static off_t textpos;

/*
 * Send message described by the passed pointer to the
 * passed output buffer.  Return -1 on error, but normally
 * the number of lines written.  Adjust the status: field
 * if need be.  If doign is set, suppress ignored header fields.
 */
long
send(mailp, obuf, doign)
	struct message *mailp;
	FILE *obuf;
{
	register struct message *mp;
	long clen, n, c;
	FILE *ibuf;
	char line[LINESIZE+1], field[BUFSIZ];
	int ishead, infld, fline, dostat, nread, unused;
	char *cp, *cp2;
	int oldign = 0;	/* previous line was ignored */
	long lc;

	mp = mailp;
	ibuf = setinput(mp);
	c = mp->m_size;
	ishead = 1;
	dostat = 1;
	infld = 0;
	fline = 1;
	lc = 0;
	clearerr(obuf);
	while (c > 0L) {
		nread = getline(line, LINESIZE, ibuf, &unused);
		c -= nread;
		lc++;
		if (ishead) {
			/*
			 * First line is the From line, so no headers
			 * there to worry about
			 */
			if (fline) {
				fline = 0;
				goto writeit;
			}
			/*
			 * If line is blank, we've reached end of
			 * headers, so force out status: field
			 * and note that we are no longer in header
			 * fields
			 */
			if (line[0] == '\n') {
				if (dostat) {
					statusput(mailp, obuf, doign);
					dostat = 0;
				}
				ishead = 0;
				goto writeit;
			}
			/*
			 * If this line is a continuation
			 * of a previous header field, just echo it.
			 */
			if (isspace(line[0]) && infld)
				if (oldign)
					continue;
				else
					goto writeit;
			infld = 0;
			/*
			 * If we are no longer looking at real
			 * header lines, force out status:
			 * This happens in uucp style mail where
			 * there are no headers at all.
			 */
			if (!headerp(line)) {
				if (dostat) {
					statusput(mailp, obuf, doign);
					dostat = 0;
				}
				putc('\n', obuf);
				ishead = 0;
				goto writeit;
			}
			infld++;
			/*
			 * Pick up the header field.
			 * If it is an ignored field and
			 * we care about such things, skip it.
			 */
			cp = line;
			cp2 = field;
			while (*cp && *cp != ':' && !isspace(*cp))
				*cp2++ = *cp++;
			*cp2 = 0;
			oldign = doign && isign(field);
			if (oldign)
				continue;
			/*
			 * If the field is "status," go compute and print the
			 * real Status: field
			 */
			if (icequal(field, "status")) {
				if (dostat) {
					statusput(mailp, obuf, doign);
					dostat = 0;
				}
				continue;
			}
		}
writeit:
		if ((!ishead) && (!(mp->m_text))) {
			if (line[0] == '\n')
				putc('\n', obuf);
			clen = mp->m_clen-1;
			n = clen < sizeof line ? clen : sizeof line;
			for (;;) {
				if ((n = fread(line, 1, n, ibuf)) <= 0) {
					fprintf(stderr, "\t(Unexpected end-of-file).\n");
					clen = 0;
				} else {
					if (fwrite(line, 1, n, obuf) != n) {
						fprintf(stderr, "\t Error writing to the new file.\n");
						fflush(obuf);
						if (ferror(obuf))
						return (-1);
					}
				}
				clen -= n;
				if (clen <= 0) {
					break;
				}
			}
			c = 0L;
		} else {
			fwrite(line, 1, nread, obuf);
			if (ferror(obuf))
				return(-1);
		}
	}
	fflush(obuf);
	if (ferror(obuf))
		return(-1);
	if (ishead && (mailp->m_flag & MSTATUS))
		printf("failed to fix up status field\n");
	return(lc);
}

/*
 * Test if the passed line is a header line, RFC 733 style.
 */
headerp(line)
	register char *line;
{
	register char *cp = line;

	if (*cp=='>' && strncmp(cp+1, "From", 4)==0)
		return(1);
	while (*cp && !isspace(*cp) && *cp != ':')
		cp++;
	while (*cp && isspace(*cp))
		cp++;
	return(*cp == ':');
}

/*
 * Output a reasonable looking status field.
 * But if "status" is ignored and doign, forget it.
 */
static void
statusput(mp, obuf, doign)
	register struct message *mp;
	register FILE *obuf;
{
	char statout[3];

	if (doign && isign("status"))
		return;
	if ((mp->m_flag & (MNEW|MREAD)) == MNEW)
		return;
	if (mp->m_flag & MREAD)
		strcpy(statout, "R");
	else
		strcpy(statout, "");
	if ((mp->m_flag & MNEW) == 0)
		strcat(statout, "O");
	fprintf(obuf, "Status: %s\n", statout);
}

/*
 * Interface between the argument list and the mail1 routine
 * which does all the dirty work.
 */

mail(people)
	char **people;
{
	register char *cp2, *cp3;
	register int s;
	char *buf, **ap;
	struct header head;
	char recfile[128];

	for (s = 0, ap = people; *ap; ap++)
		s += strlen(*ap) + 2;
	buf = salloc((unsigned)(s+1));
	cp2 = buf;
	for (ap = people; *ap; ap++) {
		for (cp3 = *ap; *cp3; ) {
			if (*cp3 == ' ' || *cp3 == '\t') {
				*cp3++ = ',';
				while (*cp3 == ' ' || *cp3 == '\t')
					cp3++;
			} else
				cp3++;
		}
		cp2 = copy(*ap, cp2);
		*cp2++ = ',';
		*cp2++ = ' ';
	}
	*cp2 = '\0';
	head.h_to = buf;
	strncpy(recfile, buf, sizeof recfile);
	head.h_subject = head.h_cc = head.h_bcc = head.h_defopt = NOSTR;
	head.h_others = NOSTRPTR;
	head.h_seq = 0;
	mail1(&head, Fflag ? recfile : 0);
	return(0);
}

sendm(str)
char *str;
{
	if (value("flipm") != NOSTR)
		return(Sendmail(str));
	else return(sendmail(str));
}

Sendm(str)
char *str;
{
	if (value("flipm") != NOSTR)
		return(sendmail(str));
	else return(Sendmail(str));
}

/*
 * Send mail to a bunch of user names.  The interface is through
 * the mail routine below.
 */
static int
sendmail(str)
	char *str;
{
	struct header head;

	if (blankline(str))
		head.h_to = NOSTR;
	else
		head.h_to = addto(NOSTR, str);
	head.h_subject = head.h_cc = head.h_bcc = head.h_defopt = NOSTR;
	head.h_others = NOSTRPTR;
	head.h_seq = 0;
	mail1(&head, (char *) 0);
	return(0);
}

/*
 * Send mail to a bunch of user names.  The interface is through
 * the mail routine below.
 * save a copy of the letter
 */
static int
Sendmail(str)
	char *str;
{
	char recfile[128];
	struct header head;

	if (blankline(str))
		head.h_to = NOSTR;
	else
		head.h_to = addto(NOSTR, str);
	strncpy(recfile, head.h_to, sizeof recfile);
	head.h_subject = head.h_cc = head.h_bcc = head.h_defopt = NOSTR;
	head.h_others = NOSTRPTR;
	head.h_seq = 0;
	mail1(&head, recfile);
	return(0);
}

/*
 * Mail a message on standard input to the people indicated
 * in the passed header.  (Internal interface).
 */
void
mail1(hp, rec)
	struct header *hp;
	char *rec;
{
	pid_t p, pid;
	int i, s, gotcha;
	char **namelist, *deliver;
	struct name *to, *np;
	FILE *mtf, *fp;
	int remote = rflag != NOSTR || rmail;
	char **t;
	char *deadletter;
	char recfile[128];

	/*
	 * Collect user's mail from standard input.
	 * Get the result as mtf.
	 */

	pid = (pid_t)-1;
	if ((mtf = collect(hp)) == NULL)
		return;
	hp->h_seq = 1;
	if (hp->h_subject == NOSTR)
		hp->h_subject = sflag;
	if (fsize(mtf) == 0 && hp->h_subject == NOSTR) {
		printf("No message !?!\n");
		goto out;
	}
	if (intty) {
		printf("EOT\n");
		flush();
	}

	/*
	 * Now, take the user names from the combined
	 * to and cc lists and do all the alias
	 * processing.
	 */

	senderr = 0;
	to = cat(extract(hp->h_bcc, GBCC),
	     cat(extract(hp->h_to, GTO),
	     extract(hp->h_cc, GCC)));
	to = translate(outpre(elide(usermap(to))));
	if (!senderr)
		mapf(to, myname);
	mechk(to);
	for (gotcha = 0, np = to; np != NIL; np = np->n_flink)
		if ((np->n_type & GDEL) == 0)
			gotcha++;
	hp->h_to = detract(to, GTO);
	hp->h_cc = detract(to, GCC);
	hp->h_bcc = detract(to, GBCC);
	if ((mtf = infix(hp, mtf)) == NULL) {
		fprintf(stderr, ". . . message lost, sorry.\n");
		return;
	}
	rewind(mtf);
	if (askme && isatty(0)) {
		char ans[64];
		puthead(hp, stdout, GTO|GCC|GBCC);
		printf("Send? [yes] ");
		if (fgets(ans, sizeof(ans), stdin) && ans[0] && tolower(ans[0]) != 'y')
			goto dead;
	}
	if (senderr)
		goto dead;
	/*
	 * Look through the recipient list for names with /'s
	 * in them which we write to as files directly.
	 */
	i = outof(to, mtf);
	rewind(mtf);
	if (!gotcha && !i) {
		printf("No recipients specified\n");
		goto dead;
	}
	if (senderr)
		goto dead;
	if ((gotcha - i) == 0)
		return;

	getrecf(rec, recfile, !!rec);
	if (recfile != NOSTR && *recfile)
		savemail(safeexpand(recfile), hp, mtf);
	if (!gotcha)
		goto out;
	namelist = unpack(to);
	if (debug) {
		fprintf(stderr, "Recipients of message:\n");
		for (t = namelist; *t != NOSTR; t++)
			fprintf(stderr, " \"%s\"", *t);
		fprintf(stderr, "\n");
		return;
	}

	/*
	 * Wait, to absorb a potential zombie, then
	 * fork, set up the temporary mail file as standard
	 * input for "mail" and exec with the user list we generated
	 * far above. Return the process id to caller in case he
	 * wants to await the completion of mail.
	 */

	wait(&s);
	rewind(mtf);
	pid = fork();
	if (pid == (pid_t)-1) {
		perror("fork");
dead:
		deadletter = Getf("DEAD");
		if (fp = fopen(deadletter, "a")) {
			puthead(hp, fp, GMASK);
			fseek(mtf, textpos, 0);
			lcwrite(deadletter, mtf, fp);
			fclose(fp);
			chmod(deadletter, DEADPERM);
		} else
			perror(deadletter);
		goto out;
	}
	if (pid == 0) {
		sigchild();
#ifdef SIGTSTP
		if (remote == 0) {
			sigset(SIGTSTP, SIG_IGN);
			sigset(SIGTTIN, SIG_IGN);
			sigset(SIGTTOU, SIG_IGN);
		}
#endif
		sigignore(SIGHUP);
		sigignore(SIGINT);
		sigignore(SIGQUIT);
		s = fileno(mtf);
		for (i = 3; i < 32; i++)
			if (i != s)
				close(i);
		close(0);
		dup(s);
		close(s);
#ifdef CC
		submit(getpid());
#endif /* CC */
		if ((deliver = value("sendmail")) == NOSTR)
			deliver = MAIL;
		execvp(safeexpand(deliver), namelist);
		perror(deliver);
		exit(1);
	}

	if (value("sendwait")!=NOSTR)
		remote++;
out:
	if (remote) {
		while ((p = wait(&s)) != pid && p != (pid_t)-1)
			;
		if (s != 0)
			senderr++;
		pid = 0;
	}
	fclose(mtf);
	return;
}

/*
 * Prepend a header in front of the collected stuff
 * and return the new file.
 */

static FILE *
infix(hp, fi)
	struct header *hp;
	FILE *fi;
{
	register FILE *nfo, *nfi;
	register int c;
	char *from, *postmark;

	rewind(fi);
	if ((nfo = fopen(tempMail, "w")) == NULL) {
		perror(tempMail);
		return(fi);
	}
	if ((nfi = fopen(tempMail, "r")) == NULL) {
		perror(tempMail);
		fclose(nfo);
		return(fi);
	}
	removefile(tempMail);
	postmark = value("postmark");
	from = value("from");
	if ((from != 0) || (postmark != 0)) {
		fprintf(nfo, "From: ");
		if (from)
			fprintf(nfo, "%s@%s%s", myname, host, maildomain());
		else
			fprintf(nfo, "%s!%s", host, myname);
		if (postmark && *postmark)
			fprintf(nfo, " (%s)", postmark);
		putc('\n', nfo);
	}
	puthead(hp, nfo, GMASK & ~GBCC);
	textpos = ftell(nfo);
	while ((c = getc(fi)) != EOF)
		putc(c, nfo);
	if (ferror(fi)) {
		perror("read");
		return(fi);
	}
	fflush(nfo);
	if (ferror(nfo)) {
		perror(tempMail);
		fclose(nfo);
		fclose(nfi);
		return(fi);
	}
	fclose(nfo);
	fclose(fi);
	rewind(nfi);
	return(nfi);
}

/*
 * Dump the message header on the
 * passed file buffer.
 */

puthead(hp, fo, w)
	struct header *hp;
	FILE *fo;
{
	register int gotcha;

	gotcha = 0;
	if (hp->h_to != NOSTR && (w & GTO))
		fprintf(fo, "To: "), fmt(hp->h_to, fo), gotcha++;
	if (hp->h_cc != NOSTR && (w & GCC))
		fprintf(fo, "Cc: "), fmt(hp->h_cc, fo), gotcha++;
	if (hp->h_bcc != NOSTR && (w & GBCC))
		fprintf(fo, "Bcc: "), fmt(hp->h_bcc, fo), gotcha++;
	if (hp->h_defopt != NOSTR && (w & GDEFOPT))
		fprintf(fo, "Default-Options: %s\n", hp->h_defopt), gotcha++;
	if (w & GSUBJECT)
		if (hp->h_subject != NOSTR && *hp->h_subject)
			fprintf(fo, "Subject: %s\n", hp->h_subject), gotcha++;
		else
			if (sflag && *sflag)
				fprintf(fo, "Subject: %s\n", sflag), gotcha++;
	if (hp->h_others != NOSTRPTR && (w & GOTHER)) {
		char **p;
		for (p = hp->h_others; *p; p++)
			fprintf(fo, "%s\n", *p);
		gotcha++;
	}
	if (gotcha && (w & GNL))
		putc('\n', fo);
	return(0);
}

/*
 * Format the given text to not exceed 78 characters.
 */
static void
fmt(str, fo)
	register char *str;
	register FILE *fo;
{
	register int col = 4;
	char name[256];
	int len;

	str = strcpy(salloc(strlen(str)+1), str);
	while (str = yankword(str, name, 1)) {
		len = strlen(name);
		if (col > 4) {
			if (col + len > 76) {
				fputs(",\n    ", fo);
				col = 4;
			} else {
				fputs(", ", fo);
				col += 2;
			}
		}
		fputs(name, fo);
		col += len;
	}
	putc('\n', fo);
}

/*
 * Save the outgoing mail on the passed file.
 */
static int
savemail(name, hp, fi)
	char name[];
	struct header *hp;
	FILE *fi;
{
	register FILE *fo;
	char line[BUFSIZ];
	long now;
	char *n;

	if (debug)
		fprintf(stderr, "save in '%s'\n", name);
	if ((fo = fopen(name, "a")) == NULL) {
		perror(name);
		return(-1);
	}
	time(&now);
	n = rflag;
	if (n == NOSTR)
		n = myname;
	fprintf(fo, "From %s %s", n, ctime(&now));
	puthead(hp, fo, GMASK);
	fseek(fi, textpos, 0);
	while (fgets(line, sizeof line, fi)) {
		if (!strncmp(line, "From ", 5))
			putc('>', fo);
		fputs(line, fo);
	}
	putc('\n', fo);
	fflush(fo);
	if (ferror(fo))
		perror(name);
	fclose(fo);
	return(0);
}
