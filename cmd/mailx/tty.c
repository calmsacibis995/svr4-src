/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:tty.c	1.9.6.1"
/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Generally useful tty stuff.
 */

#include "rcv.h"

#ifdef	USG_TTY

static void	Echo();
static int	countcol();
static void	outstr();
static char	*readtty();
static void	resetty();
static void	rubout();
static int	savetty();
static int	setty();

static	int	c_erase;		/* Current erase char */
static	int	c_kill;			/* Current kill char */
static	int	c_intr;			/* interrupt char */
static	int	c_quit;			/* quit character */
static	int	c_word;			/* Current word erase char */
static	int	Col;			/* current output column */
static	int	Pcol;			/* end column of prompt string */
static	int	Out;			/* file descriptor of stdout */
static	struct termio savtty, ttybuf;
static	char canonb[LINESIZE];		/* canonical buffer for input */
					/* processing */
static	int	erasing;		/* we are erasing characters */

#ifdef SIGCONT
# ifdef preSVr4
typedef int	sig_atomic_t;
# endif
static	sig_atomic_t	hadcont;		/* Saw continue signal */
/*ARGSUSED*/
static void
ttycont(s)
{
	hadcont++;
}

/*ARGSUSED*/
static void
ttystop(s)
{
	resetty();
	kill(mypid, SIGSTOP);
}
#endif

/*
 * Read all relevant header fields.
 */

grabh(hp, gflags, subjtop)
register struct header *hp;
{
#ifdef SIGCONT
	void (*savecont)();
	void (*savestop)();
#endif
	if (savetty())
		return -1;
#ifdef SIGCONT
# ifdef preSVr4
	savecont = sigset(SIGCONT, ttycont);
	savestop = sigset(SIGTSTP, ttycont);
# else
	{
	struct sigaction nsig, osig;
	nsig.sa_handler = ttycont;
	sigemptyset(&nsig.sa_mask);
	nsig.sa_flags = 0;
	(void) sigaction(SIGCONT, &nsig, &osig);
	savecont = osig.sa_handler;
	nsig.sa_handler = ttystop;
	sigemptyset(&nsig.sa_mask);
	nsig.sa_flags = 0;
	(void) sigaction(SIGTSTP, &nsig, &osig);
	savestop = osig.sa_handler;
	}
# endif
#endif
	if (gflags & GSUBJECT && subjtop) {
		hp->h_subject = readtty("Subject: ", hp->h_subject);
		if (hp->h_subject != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GTO) {
		hp->h_to = addto(NOSTR, readtty("To: ", hp->h_to));
		if (hp->h_to != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GCC) {
		hp->h_cc = addto(NOSTR, readtty("Cc: ", hp->h_cc));
		if (hp->h_cc != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GBCC) {
		hp->h_bcc = addto(NOSTR, readtty("Bcc: ", hp->h_bcc));
		if (hp->h_bcc != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GSUBJECT && !subjtop) {
		hp->h_subject = readtty("Subject: ", hp->h_subject);
		if (hp->h_subject != NOSTR)
			hp->h_seq++;
	}
#ifdef SIGCONT
# ifdef preSVr4
	(void) sigset(SIGCONT, savecont);
	(void) sigset(SIGTSTP, savestop);
# else
	{
	struct sigaction nsig;
	nsig.sa_handler = savecont;
	sigemptyset(&nsig.sa_mask);
	nsig.sa_flags = SA_RESTART;
	(void) sigaction(SIGCONT, &nsig, (struct sigaction*)0);
	nsig.sa_handler = savestop;
	(void) sigaction(SIGTSTP, &nsig, (struct sigaction*)0);
	}
# endif
#endif
	return(0);
}

/*
 * Read up a header from standard input.
 * The source string has the preliminary contents to
 * be read.
 *
 */

static char *
readtty(pr, src)
	char pr[], src[];
{
	int c;
	register char *cp, *cp2;

	erasing = 0;
	fflush(stdout);
	Col = 0;
	outstr(pr);
	Pcol = Col;
	if (src != NOSTR && strlen(src) > LINESIZE - 2) {
		printf("too long to edit\n");
		return(src);
	}
	if (setty())
		return(src);
	cp2 = src==NOSTR ? "" : src;
	for (cp=canonb; *cp2; cp++, cp2++)
		*cp = *cp2;
	*cp = '\0';
	outstr(canonb);

	for (;;) {
		fflush(stdout);
#ifdef SIGCONT
		hadcont = 0;
#endif
		c = getc(stdin);

		if (c==c_erase) {
			if (cp > canonb)
				if (cp[-1]=='\\' && !erasing) {
					*cp++ = (char)c;
					Echo(c);
				} else {
					rubout(--cp);
				}
		} else if (c==c_kill) {
			if (cp > canonb && cp[-1]=='\\') {
				*cp++ = (char)c;
				Echo(c);
			} else while (cp > canonb) {
				rubout(--cp);
			}
		} else if (c==c_word) {
			if (cp > canonb)
				if (cp[-1]=='\\' && !erasing) {
					*cp++ = (char)c;
					Echo(c);
				} else {
					while (--cp >= canonb)
						if (!isspace(*cp))
							break;
						else
							rubout(cp);
					while (cp >= canonb)
						if (!isspace(*cp))
							rubout(cp--);
						else
							break;
					if (cp < canonb)
						cp = canonb;
					else if (*cp)
						cp++;
				}
		} else if (c==EOF || ferror(stdin) || c==c_intr || c==c_quit) {
#ifdef SIGCONT
			if (hadcont) {
				(void) setty();
				outstr("(continue)\n");
				Col = 0;
				outstr(pr);
				*cp = '\0';
				outstr(canonb);
				clearerr(stdin);
				continue;
			}
#endif
			resetty();
			savedead(c==c_quit? SIGQUIT: SIGINT);
		} else switch (c) {
			case '\n':
			case '\r':
				resetty();
				putchar('\n');
				fflush(stdout);
				if (canonb[0]=='\0')
					return(NOSTR);
				return(savestr(canonb));
			default:
				erasing = 0;
				*cp++ = (char)c;
				*cp = '\0';
				Echo(c);
		}
	}
}

static int
savetty()
{
	Out = fileno(stdout);
	if (ioctl(Out, TCGETA, &savtty) < 0)
	{	perror("ioctl");
		return(-1);
	}
	c_erase = savtty.c_cc[VERASE];
	c_kill = savtty.c_cc[VKILL];
	c_intr = savtty.c_cc[VINTR];
	c_quit = savtty.c_cc[VQUIT];
	c_word = 'W' & 037;	/* erase word character */
	ttybuf = savtty;
#ifdef	u370
	ttybuf.c_cflag &= ~PARENB;	/* disable parity */
	ttybuf.c_cflag |= CS8;		/* character size = 8 */
#endif	/* u370 */
	ttybuf.c_cc[VTIME] = 0;
	ttybuf.c_cc[VMIN] = 1;
	ttybuf.c_iflag &= ~(BRKINT);
	ttybuf.c_lflag &= ~(ICANON|ISIG|ECHO);
	return 0;
}

static int
setty()
{
	if (ioctl(Out, TCSETAW, &ttybuf) < 0) {
		perror("ioctl");
		return(-1);
	}
	return(0);
}

static void
resetty()
{
	if (ioctl(Out, TCSETAW, &savtty) < 0)
		perror("ioctl");
}

static void
outstr(s)
	register char *s;
{
	while (*s)
		Echo(*s++);
}

static void
rubout(cp)
	register char *cp;
{
	register int oldcol;
	register int c = *cp;

	erasing = 1;
	*cp = '\0';
	switch (c) {
	case '\t':
		oldcol = countcol();
		do
			putchar('\b');
		while (--Col > oldcol);
		break;
	case '\b':
		if (isprint(cp[-1]))
			putchar(*(cp-1));
		else
			putchar(' ');
		Col++;
		break;
	default:
		if (isprint(c)) {
			fputs("\b \b", stdout);
			Col--;
		}
	}
}

static int
countcol()
{
	register int col;
	register char *s;

	for (col=Pcol, s=canonb; *s; s++)
		switch (*s) {
		case '\t':
			while (++col % 8)
				;
			break;
		case '\b':
			col--;
			break;
		default:
			if (isprint(*s))
				col++;
		}
	return(col);
}

static void
Echo(cc)
{
	char c = (char)cc;

	switch (c) {
	case '\t':
		do
			putchar(' ');
		while (++Col % 8);
		break;
	case '\b':
		if (Col > 0) {
			putchar('\b');
			Col--;
		}
		break;
	case '\r':
	case '\n':
		Col = 0;
		fputs("\r\n", stdout);
		break;
	default:
		if (isprint(c)) {
			Col++;
			putchar(c);
		}
	}
}

#else

static	int	c_erase;		/* Current erase char */
static	int	c_kill;			/* Current kill char */
static	int	hadcont;		/* Saw continue signal */
static	jmp_buf	rewrite;		/* Place to go when continued */
#ifndef TIOCSTI
static	int	ttyset;			/* We must now do erase/kill */
#endif

/*
 * Read all relevant header fields.
 */

grabh(hp, gflags, subjtop)
	struct header *hp;
{
	struct sgttyb ttybuf;
	void ttycont(), signull();
	void (*savecont)();
	register int s;
	int errs;
#ifndef TIOCSTI
	void (*savesigs[2])();
#endif

#ifdef SIGCONT
	savecont = sigset(SIGCONT, signull);
#endif
	errs = 0;
#ifndef TIOCSTI
	ttyset = 0;
#endif
	if (gtty(fileno(stdin), &ttybuf) < 0) {
		perror("gtty");
		return(-1);
	}
	c_erase = ttybuf.sg_erase;
	c_kill = ttybuf.sg_kill;
#ifndef TIOCSTI
	ttybuf.sg_erase = 0;
	ttybuf.sg_kill = 0;
	for (s = SIGINT; s <= SIGQUIT; s++)
		if ((savesigs[s-SIGINT] = sigset(s, SIG_IGN)) == SIG_DFL)
			sigset(s, SIG_DFL);
#endif
	if (gflags & GSUBJECT && subjtop) {
#ifndef TIOCSTI
		if (!ttyset && hp->h_subject != NOSTR)
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_subject = readtty("Subject: ", hp->h_subject);
		if (hp->h_subject != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GTO) {
#ifndef TIOCSTI
		if (!ttyset && hp->h_to != NOSTR)
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_to = addto(NOSTR, readtty("To: ", hp->h_to));
		if (hp->h_to != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GCC) {
#ifndef TIOCSTI
		if (!ttyset && hp->h_cc != NOSTR)
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_cc = addto(NOSTR, readtty("Cc: ", hp->h_cc));
		if (hp->h_cc != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GBCC) {
#ifndef TIOCSTI
		if (!ttyset && hp->h_bcc != NOSTR)
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_bcc = addto(NOSTR, readtty("Bcc: ", hp->h_bcc));
		if (hp->h_bcc != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GSUBJECT && !subjtop) {
#ifndef TIOCSTI
		if (!ttyset && hp->h_subject != NOSTR)
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_subject = readtty("Subject: ", hp->h_subject);
		if (hp->h_subject != NOSTR)
			hp->h_seq++;
	}
#ifdef SIGCONT
	sigset(SIGCONT, savecont);
#endif
#ifndef TIOCSTI
	ttybuf.sg_erase = c_erase;
	ttybuf.sg_kill = c_kill;
	if (ttyset)
		stty(fileno(stdin), &ttybuf);
	for (s = SIGINT; s <= SIGQUIT; s++)
		sigset(s, savesigs[s-SIGINT]);
#endif
	return(errs);
}

/*
 * Read up a header from standard input.
 * The source string has the preliminary contents to
 * be read.
 *
 */

char *
readtty(pr, src)
	char pr[], src[];
{
	char canonb[LINESIZE];
	int c, ch, signull();
	register char *cp, *cp2;

	fputs(pr, stdout);
	fflush(stdout);
	if (src != NOSTR && strlen(src) > LINESIZE - 2) {
		printf("too long to edit\n");
		return(src);
	}
#ifndef TIOCSTI
	if (src != NOSTR)
		cp = copy(src, canonb);
	else
		cp = copy("", canonb);
	fputs(canonb, stdout);
	fflush(stdout);
#else
	cp = src == NOSTR ? "" : src;
	while (c = *cp++) {
		if (c == c_erase || c == c_kill) {
			ch = '\\';
			ioctl(0, TIOCSTI, &ch);
		}
		ioctl(0, TIOCSTI, &c);
	}
	cp = canonb;
	*cp = 0;
#endif
	cp2 = cp;
	while (cp2 < canonb + LINESIZE)
		*cp2++ = 0;
	cp2 = cp;
	if (setjmp(rewrite))
		goto redo;
#ifdef SIGCONT
	sigset(SIGCONT, ttycont);
#endif
	while (cp2 < canonb + LINESIZE) {
		c = getc(stdin);
		if (c == EOF || c == '\n')
			break;
		*cp2++ = c;
	}
	*cp2 = 0;
#ifdef SIGCONT
	sigset(SIGCONT, signull);
#endif
	if (c == EOF && ferror(stdin) && hadcont) {
redo:
		hadcont = 0;
		cp = strlen(canonb) > 0 ? canonb : NOSTR;
		clearerr(stdin);
		return(readtty(pr, cp));
	}
#ifndef TIOCSTI
	if (cp == NOSTR || *cp == '\0')
		return(src);
	cp2 = cp;
	if (!ttyset)
		return(strlen(canonb) > 0 ? savestr(canonb) : NOSTR);
	while (*cp != '\0') {
		c = *cp++;
		if (c == c_erase) {
			if (cp2 == canonb)
				continue;
			if (cp2[-1] == '\\') {
				cp2[-1] = c;
				continue;
			}
			cp2--;
			continue;
		}
		if (c == c_kill) {
			if (cp2 == canonb)
				continue;
			if (cp2[-1] == '\\') {
				cp2[-1] = c;
				continue;
			}
			cp2 = canonb;
			continue;
		}
		*cp2++ = c;
	}
	*cp2 = '\0';
#endif
	if (equal("", canonb))
		return(NOSTR);
	return(savestr(canonb));
}

#ifdef SIGCONT
/*
 * Receipt continuation.
 */
void
ttycont(s)
{

	hadcont++;
	sigrelse(SIGCONT);
	longjmp(rewrite, 1);
}
#endif

/*
 * Null routine to allow us to hold SIGCONT
 */
void
signull(s)
{}
#endif	/* USG_TTY */
