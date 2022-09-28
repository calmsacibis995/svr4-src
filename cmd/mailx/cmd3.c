/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:cmd3.c	1.11.7.1"

#include "rcv.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Still more user commands.
 */

static int	bangexp();
static int	diction();
static char	*getfilename();
static int	resp1();
static int	Resp1();
static char	*reedit();
static int	shell1();
static void	sort();

static char	prevfile[PATHSIZE];
static char	origprevfile[PATHSIZE];
static char	lastbang[256];

/*
 * Process a shell escape by saving signals, ignoring signals,
 * and forking a sh -c
 */

shell(str)
char	*str;
{
	shell1(str);
	printf("!\n");
	return(0);
}

static int
shell1(str)
	char *str;
{
	void (*sig[2])();
	int stat[1];
	register int t;
	register pid_t p;
	char *Shell;
	char cmd[BUFSIZ];

	strcpy(cmd, str);
	if (bangexp(cmd) < 0)
		return(-1);
	if ((Shell = value("SHELL")) == NOSTR || *Shell=='\0')
		Shell = SHELL;
	for (t = SIGINT; t <= SIGQUIT; t++)
		sig[t-SIGINT] = sigset(t, SIG_IGN);
	p = vfork();
	if (p == 0) {
		sigchild();
		for (t = SIGINT; t <= SIGQUIT; t++)
			if (sig[t-SIGINT] != SIG_IGN)
				sigset(t, SIG_DFL);
		execlp(Shell, Shell, "-c", cmd, (char *)0);
		perror(Shell);
		_exit(1);
	}
	while (wait(stat) != p)
		;
	if (p == (pid_t)-1)
		perror("fork");
	for (t = SIGINT; t <= SIGQUIT; t++)
		sigset(t, sig[t-SIGINT]);
	return(0);
}

/*
 * Fork an interactive shell.
 */

dosh(str)
	char *str;
{
	void (*sig[2])();
	int stat[1];
	register int t;
	register pid_t p;
	char *Shell;

	(void) str;
	if ((Shell = value("SHELL")) == NOSTR || *Shell=='\0')
		Shell = SHELL;
	for (t = SIGINT; t <= SIGQUIT; t++)
		sig[t-SIGINT] = sigset(t, SIG_IGN);
	p = vfork();
	if (p == 0) {
		sigchild();
		for (t = SIGINT; t <= SIGQUIT; t++)
			if (sig[t-SIGINT] != SIG_IGN)
				sigset(t, SIG_DFL);
		execlp(Shell, Shell, (char *)0);
		perror(Shell);
		_exit(1);
	}
	while (wait(stat) != p)
		;
	if (p == (pid_t)-1)
		perror("fork");
	for (t = SIGINT; t <= SIGQUIT; t++)
		sigset(t, sig[t-SIGINT]);
	putchar('\n');
	return(0);
}

/*
 * Expand the shell escape by expanding unescaped !'s into the
 * last issued command where possible.
 */
static int
bangexp(str)
	char *str;
{
	char bangbuf[BUFSIZ];
	register char *cp, *cp2;
	register int n;
	int changed = 0;
	int bangit = (value("bang")!=NOSTR);

	cp = str;
	cp2 = bangbuf;
	n = BUFSIZ;
	while (*cp) {
		if (*cp=='!' && bangit) {
			if (n < strlen(lastbang)) {
overf:
				printf("Command buffer overflow\n");
				return(-1);
			}
			changed++;
			strcpy(cp2, lastbang);
			cp2 += strlen(lastbang);
			n -= strlen(lastbang);
			cp++;
			continue;
		}
		if (*cp == '\\' && cp[1] == '!') {
			if (--n <= 1)
				goto overf;
			*cp2++ = '!';
			cp += 2;
			changed++;
		}
		if (--n <= 1)
			goto overf;
		*cp2++ = *cp++;
	}
	*cp2 = 0;
	if (changed) {
		printf("!%s\n", bangbuf);
		fflush(stdout);
	}
	strcpy(str, bangbuf);
	strncpy(lastbang, bangbuf, sizeof lastbang);
	lastbang[(sizeof lastbang)-1] = 0;
	return(0);
}

/*
 * Print out a nice help message from some file or another.
 */

help()
{
	register c;
	register FILE *f;

	if ((f = fopen(HELPFILE, "r")) == NULL) {
		printf("No help just now.\n");
		return(1);
	}
	while ((c = getc(f)) != EOF)
		putchar(c);
	fclose(f);
	return(0);
}

/*
 * Change user's working directory.
 */

schdir(str)
	char *str;
{
	register char *cp;

	for (cp = str; *cp == ' '; cp++)
		;
	if (*cp == '\0')
		cp = homedir;
	else
		if ((cp = expand(cp)) == NOSTR)
			return(1);
	if (chdir(cp) < 0) {
		perror(cp);
		return(1);
	}
	return(0);
}

/*
 * Reply to a list of messages.  Extract each name from the
 * message header and send them off to mail1()
 */

respond(msgvec)
	int *msgvec;
{
	if (value("flipr") != NOSTR) return(Resp1(msgvec, 0));
	else return(resp1(msgvec, 0));
}

followup(msgvec)
int *msgvec;
{
	if (value("flipf") != NOSTR) return(Resp1(msgvec, 1));
	else return(resp1(msgvec, 1));
}

static int
resp1(msgvec, useauthor)
	int *msgvec;
{
	char recfile[128];
	struct message *mp;
	char *cp, buf[2 * LINESIZE], *rcv, *skin_rcv, *replyto, **ap;
	struct name *np;
	struct header head;
	char mylocalname[100], mydomname[200];

	if (msgvec[1] != 0) {
		printf("Sorry, can't reply to multiple messages at once\n");
		return(1);
	}

	strcpy(mylocalname, myname);
	strcat(mylocalname, "@");
	strcpy(mydomname, mylocalname);
	strcat(mylocalname, host);
	strcat(mydomname, domain);

	mp = &message[msgvec[0] - 1];
	dot = mp;

	rcv = nameof(mp);
	strncpy(recfile, rcv, sizeof recfile);

	if ((replyto = hfield("reply-to", mp, addto)) != NOSTR)
		strcpy(buf, replyto);
	else if ((cp = hfield("to", mp, addto)) != NOSTR)
		strcpy(buf, cp);
	else
		strcpy(buf, "");
	np = elide(extract(buf, GTO));
#ifdef	OPTIM
	/* rcv = netrename(rcv); */
#endif	/* OPTIM */
	/*
	 * Delete my name from the reply list,
	 * and with it, all my alternate names.
	 */
	skin_rcv = skin(rcv);
	mapf(np, skin_rcv);
	np = delname(np, myname);
	np = delname(np, mylocalname);
	np = delname(np, mydomname);
	np = delname(np, skin_rcv);
	if (altnames != 0)
		for (ap = altnames; *ap; ap++)
			np = delname(np, *ap);
	head.h_seq = 1;
	cp = detract(np, 0);
	if (cp != NOSTR && replyto == NOSTR) {
		strcpy(buf, cp);
		strcat(buf, unuucp(rcv));
		strcat(buf, ", ");
	}
	else {
		if (cp == NOSTR && replyto != NOSTR)
			printf("Empty reply-to field -- replying to author\n");
		if (cp == NOSTR) {
			strcpy(buf, unuucp(rcv));
			strcat(buf, ", ");
		} else
			strcpy(buf, cp);
	}
	head.h_to = buf;
	head.h_subject = hfield("subject", mp, addone);
	if (head.h_subject == NOSTR)
		head.h_subject = hfield("subj", mp, addone);
	head.h_subject = reedit(head.h_subject);
	head.h_cc = NOSTR;
	cp = hfield("cc", mp, addto);
	if (cp != NOSTR) {
		np = elide(extract(cp, GCC));
		mapf(np, skin_rcv);
		np = delname(np, myname);
		np = delname(np, mylocalname);
		np = delname(np, mydomname);
		np = delname(np, skin_rcv);
		if (altnames != 0)
			for (ap = altnames; *ap; ap++)
				np = delname(np, *ap);
		head.h_cc = detract(np, 0);
	}
	head.h_bcc = NOSTR;
	head.h_defopt = NOSTR;
	head.h_others = NOSTRPTR;
	mail1(&head, useauthor ? recfile : 0);
	return(0);
}

void
getrecf(buf, recfile, useauthor)
char *buf, *recfile;
{
	register char *bp, *cp;
	register char *recf = recfile;
	register int folderize = (value("outfolder")!=NOSTR);

	if (useauthor) {
		if (folderize)
			*recf++ = '+';
		if (debug) fprintf(stderr, "buf='%s'\n", buf);
		for (bp=skin(buf), cp=recf; *bp && !any(*bp, ", "); bp++)
			if (*bp=='!')
				cp = recf;
			else
				*cp++ = *bp;
		*cp = '\0';
		if (cp==recf)
			*recfile = '\0';
	} else {
		if (cp = value("record")) {
			if (folderize && *cp!='+' && *cp!='/'
			 && *safeexpand(cp)!='/')
				*recf++ = '+';
			strcpy(recf, cp);
		} else
			*recf = '\0';
	}
	if (debug) fprintf(stderr, "recfile='%s'\n", recfile);
}

/*
 * Modify the subject we are replying to to begin with Re: if
 * it does not already.
 */

static char *
reedit(subj)
	char *subj;
{
	char sbuf[10];
	register char *newsubj;

	if (subj == NOSTR)
		return(NOSTR);
	strncpy(sbuf, subj, 3);
	sbuf[3] = 0;
	if (icequal(sbuf, "re:"))
		return(subj);
	newsubj = salloc((unsigned)(strlen(subj) + 5));
	sprintf(newsubj, "Re: %s", subj);
	return(newsubj);
}

/*
 * Preserve the named messages, so that they will be sent
 * back to the system mailbox.
 */

preserve(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register int *ip, mesg;

	if (edit) {
		printf("Cannot \"preserve\" in edit mode\n");
		return(1);
	}
	for (ip = msgvec; *ip != NULL; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		mp->m_flag |= MPRESERVE;
		mp->m_flag &= ~MBOX;
		dot = mp;
	}
	return(0);
}

/*
 * Print the size of each message.
 */

messize(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register int *ip, mesg;

	for (ip = msgvec; *ip != NULL; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		printf("%d: %ld\n", mesg, mp->m_size);
	}
	return(0);
}

/*
 * Quit quickly.  If we are sourcing, just pop the input level
 * by returning an error.
 */

int
rexit(e)
{
	if (sourcing)
		return(1);
	if (Tflag != NOSTR)
		close(creat(Tflag, TEMPPERM));
	if (!edit)
		Verhogen();
	exit(e);
	/* NOTREACHED */
}

/*
 * Set or display a variable value.  Syntax is similar to that
 * of csh.
 */

set(arglist)
	char **arglist;
{
	register struct var *vp;
	register char *cp, *cp2;
	char varbuf[BUFSIZ], **ap, **p;
	int errs, h, s;

	if (argcount(arglist) == 0) {
		for (h = 0, s = 1; h < HSHSIZE; h++)
			for (vp = variables[h]; vp != NOVAR; vp = vp->v_link)
				s++;
		ap = (char **) salloc(s * sizeof *ap);
		for (h = 0, p = ap; h < HSHSIZE; h++)
			for (vp = variables[h]; vp != NOVAR; vp = vp->v_link)
				*p++ = vp->v_name;
		*p = NOSTR;
		sort(ap);
		for (p = ap; *p != NOSTR; p++)
			if (((cp = value(*p)) != 0) && *cp)
				printf("%s=\"%s\"\n", *p, cp);
			else
				printf("%s\n", *p);
		return(0);
	}
	errs = 0;
	for (ap = arglist; *ap != NOSTR; ap++) {
		cp = *ap;
		cp2 = varbuf;
		while (*cp != '=' && *cp != '\0')
			*cp2++ = *cp++;
		*cp2 = '\0';
		if (*cp == '\0')
			cp = "";
		else
			cp++;
		if (equal(varbuf, "")) {
			printf("Non-null variable name required\n");
			errs++;
			continue;
		}
		assign(varbuf, cp);
	}
	return(errs);
}

/*
 * Unset a bunch of variable values.
 */

unset(arglist)
	char **arglist;
{
	register int errs;
	register char **ap;

	errs = 0;
	for (ap = arglist; *ap != NOSTR; ap++)
		errs += deassign(*ap);
	return(errs);
}

/*
 * Put add users to a group.
 */

group(argv)
	char **argv;
{
	register struct grouphead *gh;
	register struct mgroup *gp;
	register int h;
	int s;
	char **ap, *gname, **p;

	if (argcount(argv) == 0) {
		for (h = 0, s = 1; h < HSHSIZE; h++)
			for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
				s++;
		ap = (char **) salloc(s * sizeof *ap);
		for (h = 0, p = ap; h < HSHSIZE; h++)
			for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
				*p++ = gh->g_name;
		*p = NOSTR;
		sort(ap);
		for (p = ap; *p != NOSTR; p++)
			printgroup(*p);
		return(0);
	}
	if (argcount(argv) == 1) {
		printgroup(*argv);
		return(0);
	}
	gname = *argv;
	h = hash(gname);
	if ((gh = findgroup(gname)) == NOGRP) {
		gh = (struct grouphead *) calloc(sizeof *gh, 1);
		gh->g_name = vcopy(gname);
		gh->g_list = NOGE;
		gh->g_link = groups[h];
		groups[h] = gh;
	}

	/*
	 * Insert names from the command list into the group.
	 * Who cares if there are duplicates?  They get tossed
	 * later anyway.
	 */

	for (ap = argv+1; *ap != NOSTR; ap++) {
		gp = (struct mgroup *) calloc(sizeof *gp, 1);
		gp->ge_name = vcopy(*ap);
		gp->ge_link = gh->g_list;
		gh->g_list = gp;
	}
	return(0);
}

/*
 * Sort the passed string vecotor into ascending dictionary
 * order.
 */

static void
sort(list)
	char **list;
{
	register char **ap;

	for (ap = list; *ap != NOSTR; ap++)
		;
	if (ap-list < 2)
		return;
	qsort(list, ap-list, sizeof *list, diction);
}

/*
 * Do a dictionary order comparison of the arguments from
 * qsort.
 */
static int
diction(a, b)
	register char **a, **b;
{
	return(strcmp(*a, *b));
}

/*
 * The do nothing command for comments.
 */

null(e)
	char *e;
{
	(void) e;
	return(0);
}

/*
 * Print out the current edit file, if we are editing.
 * Otherwise, print the name of the person who's mail
 * we are reading.
 */
int
file(argv)
	char **argv;
{
	register char *cp;
	int edit;

	if (argv[0] == NOSTR) {
		newfileinfo();
		return(0);
	}

	/*
	 * Acker's!  Must switch to the new file.
	 * We use a funny interpretation --
	 *	# -- gets the previous file
	 *	% -- gets the invoker's post office box
	 *	%user -- gets someone else's post office box
	 *	& -- gets invoker's mbox file
	 *	string -- reads the given file
	 */

	cp = getfilename(argv[0], &edit);
	if (cp == NOSTR)
		return(-1);
	if (setfile(cp, edit)) {
		strcpy(origname, origprevfile);
		return(-1);
	}
	newfileinfo();
	return(0);
}

/*
 * Evaluate the string given as a new mailbox name.
 * Ultimately, we want this to support a number of meta characters.
 * Possibly:
 *	% -- for my system mail box
 *	%user -- for user's system mail box
 *	# -- for previous file
 *	& -- get's invoker's mbox file
 *	file name -- for any other file
 */

static char *
getfilename(name, aedit)
	char *name;
	int *aedit;
{
	register char *cp;
	char savename[BUFSIZ];
	char oldmailname[BUFSIZ];
	char tmp[BUFSIZ];

	/*
	 * Assume we will be in "edit file" mode, until
	 * proven wrong.
	 */
	*aedit = 1;
	switch (*name) {
	case '%':
		*aedit = 0;
		strcpy(prevfile, mailname);
		strcpy(origprevfile, origname);
		if (name[1] != 0) {
			strcpy(savename, myname);
			strcpy(oldmailname, mailname);
			strncpy(myname, name+1, PATHSIZE-1);
			myname[PATHSIZE-1] = 0;
			findmail();
			cp = savestr(mailname);
			strcpy(origname, cp);
			strcpy(myname, savename);
			strcpy(mailname, oldmailname);
			return(cp);
		}
		strcpy(oldmailname, mailname);
		findmail();
		cp = savestr(mailname);
		strcpy(mailname, oldmailname);
		strcpy(origname, cp);
		return(cp);

	case '#':
		if (name[1] != 0)
			goto regular;
		if (prevfile[0] == 0) {
			printf("No previous file\n");
			return(NOSTR);
		}
		cp = savestr(prevfile);
		strcpy(prevfile, mailname);
		strcpy(tmp, origname);
		strcpy(origname, origprevfile);
		strcpy(origprevfile, tmp);
		return(cp);

	case '&':
		strcpy(prevfile, mailname);
		strcpy(origprevfile, origname);
		if (name[1] == 0) {
			char *tmp;
			tmp=Getf("MBOX");
			strcpy(origname, tmp);
			return(tmp);
		}
		/* Fall into . . . */

	default:
regular:
		strcpy(prevfile, mailname);
		strcpy(origprevfile, origname);
		cp = safeexpand(name);
		strcpy(origname, cp);
		if (cp[0] != '/') {
			name = getcwd(NOSTR, PATHSIZE);
			strcat(name, "/");
			strcat(name, cp);
			cp = name;
		}
		return(cp);
	}
}

/*
 * Expand file names like echo
 */

echo(str)
char *str;
{
	char cmd[LINESIZE];

	sprintf(cmd, "echo %s", str);
	shell1(cmd);
	return(0);
}

/*
 * Reply to a series of messages by simply mailing to the senders
 * and not messing around with the To: and Cc: lists as in normal
 * reply.
 */

Respond(msgvec)
	int msgvec[];
{
	if (value("flipr") != NOSTR) return(resp1(msgvec, 0));
	else return(Resp1(msgvec, 0));
}

Followup(msgvec)
int *msgvec;
{
	if (value("flipf") != NOSTR) return(resp1(msgvec, 1));
	else return(Resp1(msgvec, 1));
}

static int
Resp1(msgvec, useauthor)
int *msgvec;
{
	char recfile[128];
	struct header head;
	struct message *mp;
	register int s, *ap;
	register char *cp, *subject;

	for (s = 0, ap = msgvec; *ap != 0; ap++) {
		mp = &message[*ap - 1];
		dot = mp;
		s += strlen(nameof(mp)) + 2;
	}
	if (s == 0)
		return(0);
	cp = salloc((unsigned)(++s));
	head.h_to = cp;
	for (ap = msgvec; *ap != 0; ap++) {
		mp = &message[*ap - 1];
		cp = copy(nameof(mp), cp);
		*cp++ = ',';
		*cp++ = ' ';
	}
	*cp = 0;
	strncpy(recfile, head.h_to, sizeof recfile);
	mp = &message[msgvec[0] - 1];
	subject = hfield("subject", mp, addone);
	head.h_seq = 1;
	if (subject == NOSTR)
		subject = hfield("subj", mp, addone);
	head.h_subject = reedit(subject);
	if (subject != NOSTR)
		head.h_seq++;
	head.h_cc = NOSTR;
	head.h_bcc = NOSTR;
	head.h_defopt = NOSTR;
	head.h_others = NOSTRPTR;
	mail1(&head, useauthor ? recfile : 0);
	return(0);
}

/*
 * Conditional commands.  These allow one to parameterize one's
 * .mailrc and do some things if sending, others if receiving.
 */

ifcmd(argv)
	char **argv;
{
	register char *cp;

	if (cond != CANY) {
		printf("Illegal nested \"if\"\n");
		return(1);
	}
	cond = CANY;
	cp = argv[0];
	switch (*cp) {
	case 'r': case 'R':
		cond = CRCV;
		break;

	case 's': case 'S':
		cond = CSEND;
		break;

	default:
		printf("Unrecognized if-keyword: \"%s\"\n", cp);
		return(1);
	}
	return(0);
}

/*
 * Implement 'else'.  This is pretty simple -- we just
 * flip over the conditional flag.
 */

elsecmd()
{

	switch (cond) {
	case CANY:
		printf("\"Else\" without matching \"if\"\n");
		return(1);

	case CSEND:
		cond = CRCV;
		break;

	case CRCV:
		cond = CSEND;
		break;

	default:
		printf("invalid condition encountered\n");
		cond = CANY;
		break;
	}
	return(0);
}

/*
 * End of if statement.  Just set cond back to anything.
 */

endifcmd()
{

	if (cond == CANY) {
		printf("\"Endif\" without matching \"if\"\n");
		return(1);
	}
	cond = CANY;
	return(0);
}

/*
 * Set the list of alternate names.
 */
alternates(namelist)
	char **namelist;
{
	register int c;
	register char **ap, **ap2, *cp;

	c = argcount(namelist) + 1;
	if (c == 1) {
		if (altnames == 0)
			return(0);
		for (ap = altnames; *ap; ap++)
			printf("%s ", *ap);
		printf("\n");
		return(0);
	}
	if (altnames != 0)
		free((char *) altnames);
	altnames = (char **) calloc(c, sizeof (char *));
	for (ap = namelist, ap2 = altnames; *ap; ap++, ap2++) {
		cp = (char *) calloc(strlen(*ap) + 1, sizeof (char));
		strcpy(cp, *ap);
		*ap2 = cp;
	}
	*ap2 = 0;
	return(0);
}
