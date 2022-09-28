/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)face:src/filecab/fileb/termtest.c	1.6"

#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#include	<ctype.h>
#include	<termio.h>
#include	<sys/types.h>
#include	<sys/times.h>
#include	"wish.h"

#define BREAK		0
#define CASE		1
#define END		2
#define EXIT		3
#define PRINT		4
#define QUERY		5
#define UNKNOWN		6
#define NUMTOKENS	6

#define ARGSIZ	128
#define MAXSEQ	256

char    *strnsave();

#define strsave(s)      ((s) ? strnsave(s, strlen(s)) : NULL )

char	Withbs[] = "\b\f\n\r \t\\\33";
char	Woutbs[] = "bfnrst\\E";
char	*Token[] = { "break", "case", "end", "exit", "print", "query", };
int	Retval;
int	Debug;
int	Level;
int	Tok;
FILE	*Fp;
struct termio	Restore;

struct SR {
	char	*send;
	char	*recv;
} SendRecv[MAXSEQ];
int	Num_used;

/* general purpose interrupt catcher */
void
intr(n)
int	n;
{
	(void)signal(n, intr);
}

void
cleanup(sig)
int	sig;
{
	if (sig)
		(void)signal(sig, SIG_IGN);
	ioctl(0, TCSETAF, &Restore);
	exit(sig);
}

main(argc, argv)
int	argc;
char	*argv[];
{
	int	i;
	struct termio	tty;
	char	*getenv();
	char	*get_response();

	if (argc > 1 && strcmp(argv[1], "-d") == 0)
		Debug = TRUE;
	ioctl(0, TCGETA, &tty);
	Restore = tty;
	tty.c_iflag &= ~(INLCR | ICRNL | IUCLC);
	tty.c_oflag &= ~OPOST;
	tty.c_lflag &= ~(ICANON | ECHO | ECHONL);
	tty.c_cc[VINTR] = 255;
	tty.c_cc[VQUIT] = 255;
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 10;
	(void)signal(SIGINT, cleanup);
	(void)signal(SIGALRM, intr);
	ioctl(0, TCSETAF, &tty);
	Retval = 1;
	for (i = 0; i < 2; i++) {
		if (i && Debug)
			puts("Trying again...");
		(void) get_response(NULL);
		proc_file(getenv("IDENTIFY"));
		proc_file("/usr/lib/terminfo");
	}
	ioctl(0, TCSETAF, &Restore);
	if (Retval)
		fputs("Can't find identify file\n", stderr);
	exit(Retval);
}

/*
 * interprets an identify file
 */
proc_file(dirname)
char	*dirname;
{
	char	path[512];

	if (dirname == NULL)
		return;
	Level = 0;
	strcat(strcpy(path, dirname), "/identify");
	debug_out(FALSE, "Processing file:", path);
	if ((Fp = fopen(path, "r")) == NULL)
		return;
	Retval = 0;
	for (get_token(); Tok != EOF && Tok != END; )
		interpret(NULL, FALSE, NULL);
	if (Tok == END)
		write(2, "Extra 'end' ignored\n", 20);
	fclose(Fp);
}

/*
 * gets a string and returns static buffer
 * expands "$0" to expand string
 */
char *
get_string(expand)
char	expand[10][ARGSIZ];
{
	register char	*p;
	register char	*s;
	register  int	c;
	static char	buf[BUFSIZ];

	/* skip white space, comments */
	for(c = getc(Fp); ; ) {
		while (isspace(c))
			c = getc(Fp);
		if (c == '#') {
			while (c != EOF && c != '\n')
				c = getc(Fp);
		}
		else
			break;
	}
	/* collect string up to white space */
	s = buf;
	while (c != EOF && !isspace(c)) {
		/* remove backslashes */
		if (c == '\\') {
			c = getc(Fp);
			if (p = strchr(Woutbs, c))
				c = Withbs[p - Woutbs];
			else if (isdigit(c)) {
				register int	ac;

				c -= '0';
				if (isdigit(ac = getc(Fp))) {
					c = 10 * c + ac - '0';
					if (isdigit(ac = getc(Fp)))
						c = 10 * c + ac - '0';
					else
						ungetc(ac, Fp);
				}
				else
					ungetc(ac, Fp);
			}
		}
		else if (c == '$') {
			/* expand $0 */
			if (isdigit(c = getc(Fp)) && expand) {
				strncpy(s, expand[c - '0'], BUFSIZ - 2 - (s - buf));
				buf[BUFSIZ - 2] = '\0';
				s += strlen(s);
				c = getc(Fp);
				continue;
			}
			*s++ = '$';
		}
		*s++ = c;
		c = getc(Fp);
		if (s >= buf + sizeof(buf) - 1)
			break;
	}
	*s = '\0';
	if (c != EOF)
		ungetc(c, Fp);
	return buf;
}

/*
 * get string and tokenize it
 */
get_token()
{
	register char	*s;
	register int	i;

	s = get_string(NULL);
	if (s[0] == '\0')
		return Tok = EOF;
	for (i = 0; i < NUMTOKENS; i++)
		if (strcmp(s, Token[i]) == 0)
			return Tok = i;
	return Tok = UNKNOWN;
}

/*
 * interpreter - executes 1 statement and returns
 * expects a token to be preread into Tok
 */
interpret(recv, skip, arg)
char	*recv;
bool	skip;
char	arg[10][ARGSIZ];
{
	register char	*s;
	char	*get_response();

	switch (Tok) {
	case QUERY:
		s = get_string(NULL);
		debug_out(skip, "query", s);
		if (!skip)
			s = get_response(s);
		Level++;
		for (get_token(); Tok != END && Tok != EOF; )
			interpret(s, skip, arg);
		Level--;
		if (Tok == EOF) {
			write(2, "Missing 'end'\n", 14);
			return;
		}
		debug_out(skip, "end", NULL);
		break;
	case CASE:
		do_case(recv, skip, arg);
		return;
	case EOF:
		return;
	case BREAK:
		debug_out(skip, "break", NULL);
		if (!skip) {
			if (s) {
				Level--;
				for (get_token(); Tok != END && Tok != EOF; )
					interpret(s, TRUE, arg);
				Level++;
				return;
			}
			else
				fputs("Syntax error: 'break' outside of 'query'\n", stderr);
		}
		break;
	case PRINT:
		s = get_string(arg);
		debug_out(skip, "print", s);
		if (!skip)
			puts(s);
		break;
	case EXIT:
		debug_out(skip, "exit", NULL);
		if (!skip)
			cleanup(0);
		break;
	default:
		write(2, "Syntax error\n", 13);
		break;
	}
	get_token();
}

/* case statement */
/*ARGSUSED*/
do_case(recv, skip, arg)
char	*recv;
bool	skip;
char	arg[10][ARGSIZ];
{
	register char	*s;
	register char	*ex;
	int	i;
	register bool	fail;
	char	newarg[10][ARGSIZ];

	fail = TRUE;
	for (i = 0; i < 10; i++)
		newarg[i][0] = '\0';
	s = get_string(NULL);
	debug_out(skip, "case", s);
	Level++;
	if (!skip) {
		if (recv) {
			char	*regex();
			char	*regcmp();

			if ((ex = regcmp(s, NULL)) == NULL) {
				fputs("Bad regular expression\n", stderr);
				cleanup(2);
			}
			s = regex(ex, recv, newarg[0], newarg[1], newarg[2],
				newarg[3], newarg[4], newarg[5], newarg[6],
				newarg[7], newarg[8], newarg[9], NULL);
			fail = (s == NULL || *s);
			free(ex);
		}
		else
			fputs("Syntax error: 'case' outside of 'query'\n", stderr);
	}
	for (get_token(); Tok != CASE && Tok != END && Tok != EOF; )
		interpret(recv, fail, newarg);
	Level--;
}

/* print debugging info */
debug_out(skip, s1, s2)
bool	skip;
char	*s1;
char	*s2;
{
	register int	i;

	if (!Debug)
		return;
	for (i = 0; i < Level; i++)
		putchar('\t');
	if (skip)
		fputs("# ", stdout);
	fputs(s1, stdout);
	if (s2) {
		putchar(' ');
		putchar('\'');
		output(s2, strlen(s2));
		putchar('\'');
	}
	putchar('\n');
}

/* print a string "nicely" */
output(s)
register char	*s;
{
	while (*s) {
		if (isprint(*s))
			putchar(*s);
		else if (*s < ' ') {
			putchar('^');
			putchar(*s + '@');
		}
		else {
			putchar('\\');
			if (*s & 0300)
				putchar('0' + ((*s >> 6) & 3));
			if (*s & 0370)
				putchar('0' + ((*s >> 3) & 7));
			putchar('0' + (*s & 7));
		}
		s++;
	}
}

/* query the terminal and return response */
char *
get_response(s)
char	*s;
{
	char	*recv;
	char	recvbuf[512];
	char	*rcvptr;
	int	numread;
	long	times();
	char	*lookup();
	char	*enter();
	extern int errno;

	if (s == NULL || *s == '\0') {
		initialize();
		return s;
	}
	if (recv = lookup(s))
		debug_out(FALSE, "found previous response of", recv);
	else {
		recv = recvbuf;
		if (Debug)
			fputs("sending... ", stdout);
		ioctl(0, TCFLSH, 2);
		ioctl(2, TCXONC, 1);
		write(2, s, strlen(s));
		ioctl(2, TCSBRK, 1);
		rcvptr = recvbuf;
		while ( read(0, rcvptr, 1) > 0 )
			rcvptr++;
		*rcvptr = '\0';
		numread = Level;
		Level = 0;
		debug_out(FALSE, "received", recvbuf);
		Level = numread;
		recv = enter(s, recv);
	}
	return recv;
}

/*
 * if we've already used this query, return what the response was
 * to save sending/reading it again
 */
char *
lookup(s)
char	*s;
{
	register int	i;

	for (i = 0; i < Num_used; i++)
		if (!strcmp(s, SendRecv[i].send))
			return SendRecv[i].recv;
	return NULL;
}

/*
 * enter a send/response pair into the array of used sequences
 */
char *
enter(s, r)
char	*s;
char	*r;
{
	struct SR	dummy;

	if (Num_used >= MAXSEQ)
		return strsave(r);
	SendRecv[Num_used].send = strsave(s);
	SendRecv[Num_used].recv = strsave(r);
	return SendRecv[Num_used++].recv;
}

/*
 * clear out the used sequences array, so we can try a second time
 * to query the terminal
 */
initialize()
{
	Num_used = 0;
}

char	*
strnsave(s, len)
char	s[];
unsigned int	len;
{
	register char	*p;

	if ((p = malloc(len + 1)) == NULL)
		return NULL;
	strncpy(p, s, len);
	p[len] = '\0';
	return p;
}
