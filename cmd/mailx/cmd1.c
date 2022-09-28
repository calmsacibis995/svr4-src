/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:cmd1.c	1.7.5.1"

#include "rcv.h"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * User commands.
 */

static void	brokpipe();
static char	*dispname();
static void	print();
static int	screensize();
static int	type1();

/*
 * Print the current active headings.
 * Don't change dot if invoker didn't give an argument.
 */

static int curscreen = 0, oldscreensize = 0;

headers(msgvec)
	int *msgvec;
{
	register int n, mesg, flag;
	register struct message *mp;
	int size;

	size = screensize();
	n = msgvec[0];
	if (n != 0)
		curscreen = (n-1)/size;
	if (curscreen < 0)
		curscreen = 0;
	mp = &message[curscreen * size];
	if (mp >= &message[msgCount])
		mp = &message[msgCount - size];
	if (mp < &message[0])
		mp = &message[0];
	flag = 0;
	mesg = mp - &message[0];
	if (dot != &message[n-1])
		dot = mp;
	if (Hflag)
		mp = message;
	for (; mp < &message[msgCount]; mp++) {
		mesg++;
		if (mp->m_flag & MDELETED)
			continue;
		if (flag++ >= size && !Hflag)
			break;
		printhead(mesg);
		sreset();
	}
	if (flag == 0) {
		printf("No more mail.\n");
		return(1);
	}
	return(0);
}

/*
 * Scroll to the next/previous screen
 */

scroll(arg)
	char arg[];
{
	register int s, size;
	int cur[1];

	cur[0] = 0;
	size = screensize();
	s = curscreen;
	switch (*arg) {
	case 0:
	case '+':
		s++;
		if (s * size > msgCount) {
			printf("On last screenful of messages\n");
			return(0);
		}
		curscreen = s;
		break;

	case '-':
		if (--s < 0) {
			printf("On first screenful of messages\n");
			return(0);
		}
		curscreen = s;
		break;

	default:
		printf("Unrecognized scrolling command \"%s\"\n", arg);
		return(1);
	}
	return(headers(cur));
}

/*
 * Compute what the screen size should be.
 * We use the following algorithm:
 *	If user specifies with screen option, use that.
 *	If baud rate < 1200, use  5
 *	If baud rate = 1200, use 10
 *	If baud rate > 1200, use 20
 */
static int
screensize()
{
	register char *cp;
	register int newscreensize, tmp;

	cp = value("screen");
	newscreensize = ((cp != NOSTR) && ((tmp = atoi(cp)) > 0)) ? tmp :
			(baud < B1200) ? 5 :
			(baud == B1200) ? 10 :
			20;

	/* renormalize the value of curscreen */
	if (newscreensize != oldscreensize) {
		curscreen = curscreen * oldscreensize / newscreensize;
		oldscreensize = newscreensize;
	}
	return(newscreensize);
}

/*
 * Print out the headlines for each message
 * in the passed message list.
 */

from(msgvec)
	int *msgvec;
{
	register int *ip;

	for (ip = msgvec; *ip != NULL; ip++) {
		printhead(*ip);
		sreset();
	}
	if (--ip >= msgvec)
		dot = &message[*ip - 1];
	return(0);
}

/*
 * Print out the header of a specific message.
 * This is a slight improvement to the standard one.
 */

void
printhead(mesg)
{
	struct message *mp;
	FILE *ibuf;
	char headline[LINESIZE], *subjline, dispc, curind;
	char *fromline;
	char pbuf[LINESIZE];
	char name[LINESIZE];
	struct headline hl;
	register char *cp;
	int showto;

	mp = &message[mesg-1];
	ibuf = setinput(mp);
	readline(ibuf, headline);
	if ((subjline = hfield("subject", mp, addone)) == NOSTR
	 && (subjline = hfield("subj", mp, addone)) == NOSTR
	 && (subjline = hfield("message-status", mp, addone)) == NOSTR)
		subjline = "";

	curind = (!Hflag && dot == mp) ? '>' : ' ';
	dispc = ' ';
	showto = 0;
	if ((mp->m_flag & (MREAD|MNEW)) == (MREAD|MNEW))
		dispc = 'R';
	if ((mp->m_flag & (MREAD|MNEW)) == MREAD)
		dispc = 'O';
	if ((mp->m_flag & (MREAD|MNEW)) == MNEW)
		dispc = 'N';
	if ((mp->m_flag & (MREAD|MNEW)) == 0)
		dispc = 'U';
	if (mp->m_flag & MSAVED)
		dispc = 'S';
	if (mp->m_flag & MPRESERVE)
		dispc = 'H';
	if (mp->m_flag & MBOX)
		dispc = 'M';
	parse(headline, &hl, pbuf);

	/*
	 * Netnews interface?
	 */

	if (newsflg) {
	    if ( (fromline=hfield("newsgroups",mp,addone)) == NOSTR 	/* A */
	      && (fromline=hfield("article-id",mp,addone)) == NOSTR ) 	/* B */
		  fromline = "<>";
	    else
		  for(cp=fromline; *cp; cp++) {		/* limit length */
			if( any(*cp, " ,\n")){
			      *cp = '\0';
			      break;
			}
		  }
	/*
	 * else regular.
	 */

	} else {
		fromline = nameof(mp);
		if (value("showto") && samebody(myname, skin(fromline))
		 && (cp = hfield("to", mp, addto))) {
			showto = 1;
			yankword(cp, fromline = name, docomma(cp));
		}
		fromline = dispname(fromline);
	}
	printf("%c%c%3d ", curind, dispc, mesg);
	if (showto)
		printf("To %-15.15s ", fromline);
	else
		printf("%-18.18s ", fromline);
	if (mp->m_text) {
		printf("%16.16s %4ld/%-5ld %-.25s\n",
			hl.l_date, mp->m_lines, mp->m_size, subjline);
	} else {
		printf("%16.16s binary/%-5ld %-.25s\n", hl.l_date, mp->m_size, subjline);
	}
}

/*
 * Return the full name from an RFC-822 header line
 * or the last two (or one) component of the address.
 */

static char *
dispname(hdr)
	char *hdr;
{
	char *cp, *cp2;

	if (hdr == 0)
		return 0;
	if (((cp = strchr(hdr, '<')) != 0) && (cp > hdr)) {
		*cp = 0;
		if ((*hdr == '"') && ((cp = strrchr(++hdr, '"')) != 0))
			*cp = 0;
		return hdr;
	} else if ((cp = strchr(hdr, '(')) != 0) {
		hdr = ++cp;
		if ((cp = strchr(hdr, '+')) != 0)
			*cp = 0;
		if ((cp = strrchr(hdr, ')')) != 0)
			*cp = 0;
		return hdr;
	}
	cp = skin(hdr);
	if ((cp2 = strrchr(cp, '!')) != 0) {
		while (cp2 >= cp && *--cp2 != '!');
		cp = ++cp2;
	}
	return cp;
}

/*
 * Print out the value of dot.
 */

pdot()
{
	printf("%d\n", dot - &message[0] + 1);
	return(0);
}

/*
 * Print out all the possible commands.
 */

pcmdlist()
{
	register struct cmd *cp;
	register int cc;

	printf("Commands are:\n");
	for (cc = 0, cp = cmdtab; cp->c_name != NULL; cp++) {
		cc += strlen(cp->c_name) + 2;
		if (cc > 72) {
			printf("\n");
			cc = strlen(cp->c_name) + 2;
		}
		if ((cp+1)->c_name != NOSTR)
			printf("%s, ", cp->c_name);
		else
			printf("%s\n", cp->c_name);
	}
	return(0);
}

/*
 * Type out messages, honor ignored fields.
 */
type(msgvec)
	int *msgvec;
{

	return(type1(msgvec, 1));
}

/*
 * Type out messages, even printing ignored fields.
 */
Type(msgvec)
	int *msgvec;
{

	return(type1(msgvec, 0));
}

/*
 * Type out the messages requested.
 */
static jmp_buf	pipestop;

static int
type1(msgvec, doign)
	int *msgvec;
{
	register *ip;
	register struct message *mp;
	register int mesg;
	register char *cp;
	long nlines;
	FILE *obuf;
	void (*sigint)(), (*sigpipe)();
	int setsigs = 0;

	obuf = stdout;
	if (setjmp(pipestop)) {
		if (obuf != stdout) {
			pipef = NULL;
			npclose(obuf);
		}
		goto ret0;
	}
	if (intty && outtty && (cp = value("crt")) != NOSTR) {
		for (ip = msgvec, nlines = 0; *ip && ip-msgvec < msgCount; ip++)
			nlines += message[*ip - 1].m_lines;
		if (nlines > atoi(cp)) {
			char *pg = PG;
			pg = (pg && *pg) ? pg : "cat";
			if ((obuf = npopen(pg, "w")) == NULL) {
				perror(pg);
				obuf = stdout;
			} else {
				pipef = obuf;
				sigint = sigset(SIGINT, SIG_IGN);
				sigpipe = sigset(SIGPIPE, brokpipe);
				setsigs++;
			}
		}
	}
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		dot = mp;
		print(mp, obuf, doign);
	}
	if (obuf != stdout) {
		pipef = NULL;
		npclose(obuf);
	}
ret0:
	if (setsigs) {
		sigset(SIGPIPE, sigpipe);
		sigset(SIGINT, sigint);
	}
	return(0);
}

/*
 * Respond to a broken pipe signal --
 * probably caused by user quitting pg.
 */
static void
brokpipe()
{
	sigrelse(SIGPIPE);
	longjmp(pipestop, 1);
}

/*
 * Print the indicated message on standard output.
 */

static void
print(mp, obuf, doign)
	register struct message *mp;
	FILE *obuf;
{

	if (!doign || !isign("message"))
		fprintf(obuf, "Message %2d:\n", mp - &message[0] + 1);
	touch(mp - &message[0] + 1);
	if (mp->m_text) {
		send(mp, obuf, doign);
	} else {
		fprintf(obuf, "\n%s\n", binmsg);
	}
}

/*
 * Print the top so many lines of each desired message.
 * The number of lines is taken from the variable "toplines"
 * and defaults to 5.
 */

top(msgvec)
	int *msgvec;
{
	register int *ip;
	register struct message *mp;
	register int mesg;
	int topl, lineb;
	long c, lines;
	char *valtop, linebuf[LINESIZE];
	FILE *ibuf;

	topl = 5;
	valtop = value("toplines");
	if (valtop != NOSTR) {
		topl = atoi(valtop);
		if (topl < 0 || topl > 10000)
			topl = 5;
	}
	lineb = 1;
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		dot = mp;
		if (value("quiet") == NOSTR)
			printf("Message %d:\n", mesg);
		if (mp->m_text) {
			ibuf = setinput(mp);
			c = mp->m_lines;
			if (!lineb)
				printf("\n");
			for (lines = 0; lines < c && lines < topl; lines++) {
				if (readline(ibuf, linebuf) <= 0)
					break;
				puts(linebuf);
				lineb = blankline(linebuf);
			}
		} else {
			printf("\n%s\n", binmsg);
		}
	}
	return(0);
}

/*
 * Touch all the given messages so that they will
 * get mboxed.
 */

stouch(msgvec)
	int msgvec[];
{
	register int *ip;

	for (ip = msgvec; *ip != 0; ip++) {
		dot = &message[*ip-1];
		dot->m_flag |= MTOUCH;
		dot->m_flag &= ~MPRESERVE;
	}
	return(0);
}

/*
 * Make sure all passed messages get mboxed.
 */

mboxit(msgvec)
	int msgvec[];
{
	register int *ip;

	for (ip = msgvec; *ip != 0; ip++) {
		dot = &message[*ip-1];
		dot->m_flag |= MTOUCH|MBOX;
		dot->m_flag &= ~MPRESERVE;
	}
	return(0);
}

/*
 * List the folders the user currently has.
 */
folders()
{
	char dirname[BUFSIZ], cmd[BUFSIZ];

	if (getfold(dirname) < 0) {
		printf("No value set for \"folder\"\n");
		return(-1);
	}
	sprintf(cmd, "%s %s", LS, dirname);
	return(system(cmd));
}
