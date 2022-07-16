/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:demo/window/window.c	1.1"
/*
 * screen window manager.  Divides ordinary dumb crt into windows,
 * starts up a pty under each crt.  Runs entirely on one system,
 * uses Berkeley 4.2BSD features such as ptys, and select.
 *
 *		Mark Horton, 7/82.
 */

static char *sccsid = "@(#)curses:demo/window/window.c	1.1	6/1/85";
#define CCA_PAGE
#include <curses.h>
#include <unctrl.h>
#include <ctype.h>
#include <signal.h>

int defsize;
int rfds, wfds;
int rrfds, rwfds;

#define MAXWINDOW 8

struct screen {
	short ptcfd;		/* filedes for pty controller */
	short wstate;		/* state of escape sequence */
	WINDOW *wptr;		/* window on screen */
	short wcols, wrows;	/* size of window */
	short ccol, crow;	/* current cursor position */
	short toprow, botrow;	/* borders */
	char *icmd;		/* initial command to window */
} screen[MAXWINDOW];

#define S_NORM	0		/* normal state */
#define S_ESC	1		/* just got an escape */
#define S_CM1	2		/* expecting column */
#define S_CM2	3		/* expecting row */
#define S_ON	4		/* expecting standout/underline on */
#define S_OFF	5		/* expecting standout/underline off */

#define cfd	screen[curwindow].ptcfd
#define cwid	screen[curwindow].wptr
#define nrows	screen[curwindow].wrows
#define cstate	screen[curwindow].wstate

int oldmode, newmode;
int oldld, newld;
int sleepintr = 1;
int nwindow;
int curwindow = 0;
char cmdchar = '\037';	/* control underscore */
char *term;
FILE *trace;

char lineused[100];

char *ptynames[] = {
	"/dev/ptyp0",
	"/dev/ptyp1",
	"/dev/ptyp2",
	"/dev/ptyp3",
	"/dev/ptyp4",
	"/dev/ptyp5",
	"/dev/ptyp6",
	"/dev/ptyp7",
	0,
};

char *getenv();
#ifdef DEBUG
extern FILE *outf;
#endif

main(argc, argv)
char **argv;
{
	register int i, n;
	register char *p;
	int r, c;
	int seenicmd = 0;
	int ichar(), done(), checkall();
	char buf[1024];

#ifdef DEBUG
	/* Needed because we use newwin, not initscr. */
	outf = fopen("trace", "w");
#endif
	term = getenv("TERM");

	while (argc > 1 && argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'c':
			cmdchar = argv[1][2];
			break;
		case 'i':
			sleepintr = atoi(&argv[1][2]);
			break;
		case 't':
			trace = fopen("wtrace", "w");
			break;
		case 'w':
			curwindow = atoi(&argv[1][2]);
			break;
		case 'T':
			term = argv[1]+2;
			break;
		default:
			fprintf(stderr, "Usage: window [-ccmdchar] [-t] [-Tterm] [wspec ...]\n");
			fprintf(stderr, "where wspec is 'lines' or 'top.bot' or 'top.-'\n");
			fprintf(stderr, "Cmds start with cmdchar (^_), do ^_? for help.\n");
			exit(1);
		}
		argc--; argv++;
	}

	newterm(term, stdout);

	/* Default settings */
	nwindow = 2;
	defsize = (LINES-1)/nwindow;
	screen[0].toprow = 0;
	screen[0].botrow = screen[0].toprow + defsize-1;
	screen[1].toprow = screen[0].botrow + 2;
	screen[1].botrow = screen[1].toprow + defsize-1;

	/* Parse arguments of form <top>.<bottom> */
	for (i=1; i<argc; i++) {
		nwindow = i;
		for (p=argv[i]; *p && *p++ != '.'; )
			;
		if (*p == 0) {
			screen[i-1].toprow = i>1 ? screen[i-2].botrow + 2 : 0;
			screen[i-1].botrow = screen[i-1].toprow + atoi(argv[i])-1;
		} else {
			screen[i-1].toprow = atoi(argv[i]);
			if (*p == '-')
				screen[i-1].botrow = LINES-1;
			else
				screen[i-1].botrow = atoi(p);
		}
		if (i+1<argc && isalpha(argv[i+1][0])) {
			screen[i-1].icmd = argv[i+1];
			argc--; argv++; seenicmd++;
		}
	}
	for (i=0; i<nwindow; i++)
		for (r=screen[i].toprow; r<=screen[i].botrow; r++) {
			lineused[r] = 1;
		}

	rfds = wfds = 0;
	rfds |= 1<<0;	/* stdin */
	if (trace) fprintf(trace, "set rfds to %o\n",rfds);
	/* Set up windows */
	for (i=0; i<nwindow; i++) {
		screen[i].wrows = screen[i].botrow  - screen[i].toprow + 1;
		screen[i].wcols = COLS;
		if (trace) fprintf(trace,
			"window %d, %d rows, %d cols, %d-%d\n", i,
			screen[i].wrows, screen[i].wcols,
			screen[i].toprow, screen[i].botrow);
		screen[i].wptr =
			newwin(screen[i].wrows, COLS, screen[i].toprow, 0);
		screen[i].ptcfd = getpty();
		if (trace) fprintf(trace, "window %d, pty fd %d\n", i, screen[i].ptcfd);
		startshell(i);
		rfds |= 1<<screen[i].ptcfd;
		if (trace) fprintf(trace, "set rfds to %o\n",rfds);
		scrollok(screen[i].wptr, TRUE);
		idlok(screen[i].wptr, TRUE);
		r = screen[i].botrow+1;
		if (r < LINES && lineused[r] == 0) {
			wmove(stdscr, r, 0);
			for (n=0; n<COLS; n++)
				addch('-');
		}
	}
	wnoutrefresh(stdscr);
	if (seenicmd) {
		sleep(1);
		for (i=0; i<nwindow; i++) {
			if (screen[i].icmd) {
				write(screen[i].ptcfd, screen[i].icmd, strlen(screen[i].icmd));
				write(screen[i].ptcfd, "\r", 1);
			}
		}
	}

	ttinit();
	signal(SIGHUP, done);
	signal(SIGTERM, done);

	/*
	 * Main loop
	 */
	for (;;) {
		/* Is there any input waiting? */
		rrfds = rfds; rwfds = wfds;
		r = select(20, &rrfds, &rwfds, 0);
		if (trace) fprintf(trace, "checking, select(%d, %o, %o,0) sez %d\n", 20, rfds, wfds, r);
		while (r <= 0) {
			/* force cursor into current window */
			touchwin(cwid);
			wnoutrefresh(cwid);
			/* actually output since we're hanging */
			doupdate();
			/* hang, waiting for something to happen */
			if (trace) fprintf(trace, "hanging\n");
			rrfds = rfds; rwfds = wfds;
			r = select(20, &rrfds, &rwfds, 10000);
			if (trace) fprintf(trace, "done hanging, select(%d, %o, %o, 10000) sez %d, leaves rrfds %o\n", 20, rfds, wfds, r, rrfds);
		}

		if (rrfds & (1<<0)) {
			/* Character ready on keyboard */
			ichar();
			continue;
		}

		/* Find window for the fd we have */
		for (i=0; i<nwindow; i++)
			if (rrfds & (1<<screen[i].ptcfd))
				break;

		if (i >= nwindow && trace) {
			fprintf(trace, "can't find an fdes\n");
			fflush(trace);
			abort();
		}

		/* Read from that pty and process it */
		n = read(screen[i].ptcfd, buf, sizeof buf);
		if (trace) fprintf(trace, "main read returns %d\n", n);
		if (n <= 0)
			continue;
		wputs(buf, n, i);
		wnoutrefresh(screen[i].wptr);
	}
}

/*
 * String version of wputc.  p is pointer to string, n is length of string,
 * win is window to print on.
 */
wputs(p, n, win)
register char *p;
register int n;
register int win;
{
	register int i;

	if (trace) fprintf(trace, "wputs %d, %d(%x), \"", n, win, screen[win].wptr);
	for (i=0; i<n; i++) {
		if (trace) fprintf(trace, "%s", unctrl(*p&0177));
		wputc(*p++, win);
	}
	if (trace) fprintf(trace, "\"\n");
}

/*
 * Put a character onto a given window.  We behave like a terminal here,
 * interpreting escape sequences and worrying about the screen and cursor
 * position.
 */
wputc(c, win)
register char c;
register int win;
{
	int row, col;
	static int cm_row, cm_col;
	WINDOW *w = screen[win].wptr;
	int nr;

	c &= 0177;
	getyx(w, row, col);	/* Handy */
	switch(cstate) {
	case S_NORM:
		switch(c) {
		case '\n':
			/* Linefeed - special handling */
			nr = screen[win].wrows - 1;
			if (row >= nr) {
				scroll(w);
				wmove(w, nr, col);
			} else {
				wmove(w, row+1, col); /* Move down */
			}
			break;
		case '\0':
		case '\177':
			/* ignore dels and nulls as pads */
			break;
		case '\07':
			beep();
			break;
		case '\033':
			cstate = S_ESC;
			break;
		default:
			/* backspace, return, and tab are handled by waddch */
			waddch(w, c);
			break;
		}
		break;
	case S_ESC:
		cstate = S_NORM;
		if (trace) fprintf(trace, "escape '%s' ", unctrl(c));
		switch(c) {
		case 'A': wmove(w, row-1, col); break;
		case 'B': wmove(w, row+1, col); break;
		case 'C': wmove(w, row, col+1); break;
		case 'D': wmove(w, row, col-1); break;
		case 'G': cstate = S_CM1; break;
		case 'J': werase(w); break;
		case 'K': wclrtoeol(w); break;
		case 'L': wclrtobot(w); wmove(w, row, col); break;
		case 'N': wdeleteln(w); break;
		case 'O': winsch(w, ' '); break;
		case 'P': winsertln(w); break;
		case 'a': cstate = S_ON; break;
		case 'b': cstate = S_OFF; break;
		}
		break;
	case S_CM1:
		cstate = S_CM2;
		cm_row = c-' ';
		break;
	case S_CM2:
		cstate = S_NORM;
		cm_col = c-' ';
		wmove(w, cm_row, cm_col);
		if (trace) fprintf(trace, "cursor address row %d, col %d\n", cm_row, cm_col);
		break;
	case S_ON:
		if (trace) fprintf(trace, "on %d\n", c);
		if (c == '!')
			wattron(w, A_UNDERLINE);
		else if (c == '$')
			wattron(w, A_STANDOUT);
		cstate = S_NORM;
		break;
	case S_OFF:
		if (trace) fprintf(trace, "off %d\n", c);
		if (c == '!')
			wattroff(w, A_UNDERLINE);
		else if (c == '$')
			wattroff(w, A_STANDOUT);
		cstate = S_NORM;
		break;
	}
}

/* Initialize the teletype modes properly. */
ttinit()
{
	raw(); noecho(); nonl();
	ioctl(0, TIOCGETD, &oldld);
	newld = NTTYDISC;
	ioctl(0, TIOCSETD, &newld);
}

/*
 * Input one or more characters from the keyboard and process them.
 * Right now all control commands are single characters.
 */
ichar()
{
	char c;
	int n;
	static int icharstate = 0;

	for (;;) {
		/* Using interrupt tty driver, we must read until no more */
		ioctl(0, FIONREAD, &n);
		if (n <= 0)
			break;
		n = read(0, &c, 1);
		if (n <= 0)
			break;
		c &= 0177;
		if (trace) fprintf(trace, "ichar read %d: %c\n", n, c);

		/* Process special tty commands */
		switch(icharstate) {
		case 0:
			if (c == cmdchar)
				icharstate = 1;
			else {
				/*
				 * Ordinary character.
				 * Pass it through to current window
				 */
				n = write(cfd, &c, 1);
				if (n != 1)
					perror("write", n);
			}
			break;
		case 1:
			if (c == cmdchar)
				c = 037;	/* stutter shifts windows */
			switch (c) {
			/* ^q: exit */
			case 'q':
			case '\0177':
				done();

			/* ^_: jump to next window */
			case 037:
			case 'n':
				curwindow++;
				if (curwindow >= nwindow)
					curwindow = 0;
				touchwin(cwid);
				wnoutrefresh(cwid);
				break;
			
			/* ^L: clear and redraw the screen */
			case 014:
				clearok(curscr, TRUE);
				wnoutrefresh(curscr);
				break;
			
			/* ^Z: suspend */
			case '\032':
				kill(getpid(), SIGTSTP);
				break;

			/* _: pass through literal ^_ */
			case '_':
				c = cmdchar;
				write(cfd, &c, 1);
				break;
			default:
				/* usage */
				wmove(stdscr, LINES-1, 0);
				wprintw(stdscr,
				"Usage: %s then %s (next window), q (quit), ^Z (susp), ^L (redraw), _ (lit. %s)",
				unctrl(cmdchar), unctrl(cmdchar), unctrl(cmdchar));
				wrefresh(stdscr);
				wrefresh(cwid);
				break;
			}
			icharstate = 0;
		}
	}
}

/*
 * Print the error indicated in the cerror cell.
 * Internal version for debugging.
 */
extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

perror(s, i)
char *s;
int i;
{
	register char *c;
	register n;

	if (errno == 4)
		return;
	c = "Unknown error";
	if(errno < sys_nerr)
		c = sys_errlist[errno];
	if (trace) fprintf(trace, "Error: %s window %d ret %d: %s\n", s, curwindow, i, c);
	mvprintw(LINES-1, 0, "Error: %s window %d ret %d: %s", s, curwindow, i, c);
	wnoutrefresh(stdscr);
}

char lastname[100];

getpty()
{
	static int nextpty = 0;
	int fd = -1, i;

	while (fd < 0) {
		if (ptynames[nextpty] == 0)
			return -1;
		fd = open(ptynames[nextpty++], 2);
	}
	strcpy(lastname, ptynames[nextpty-1]);
	return fd;
}

/*
 * Clean up and exit to the shell.
 */
done()
{
	int i;

	if (trace) fprintf(trace, "done\n");
	move(LINES-1, 0);
	clrtoeol();
	endwin();
	ioctl(0, TIOCSETD, &oldld);
	exit();
}

/*
 * Start up a shell on window #win.
 */
startshell(win)
int win;
{
	char *getenv();
	char *shell = getenv("SHELL");
	char *term = getenv("TERM");
	char *termcap = getenv("TERMCAP");
	char tname[100];
	int pid, pg;
	int i, fd;
	/* stuff to copy tty modes */
	int ttm_ldisc;
	struct sgttyb ttm_sgttyb;
	struct tchars ttm_tchars;
	struct ltchars ttm_ltchars;
	int ttm_lflags;
	int ttm_page;
#ifdef JSWINSIZE
	struct winsize ttm_winsize;
#endif

	/* Parent returns, child left to become shell */
	if (trace) fflush(trace);
	pid = fork();
	if (pid)
		return;

	/* save tty status and flags */
	ioctl(0, TIOCGETD, &ttm_ldisc);
	ioctl(0, TIOCGETP, &ttm_sgttyb);
	ioctl(0, TIOCGETC, &ttm_tchars);
	ioctl(0, TIOCGLTC, &ttm_ltchars);
	ioctl(0, TIOCLGET, &ttm_lflags);
#ifdef TIOCGSCR
	ioctl(0, TIOCGSCR, &ttm_page);
#endif

	pg = getpid();
	/* close off file descriptors and hook us up to pty */
	for (i=0; i<20; i++)
		if (!trace || i != fileno(trace))
			close(i);
	setpgrp(pg, 0);		/* clear controlling tty */
	strcpy(tname, lastname);
	tname[5] = 't';	/* /dev/ptypx to /dev/ttypx */
	if (trace) fprintf(trace, "tty name %s\n", tname);
	fd = open(tname, 2);
	if (trace) fprintf(trace, "file %d\n", fd);
	dup(fd); dup(fd); /* setup stdin, stdout, stderr */
	setpgrp(pg, pg);
	ioctl(0, TIOCSPGRP, &pg);

	/* Force certain fields to different values */
	ttm_ldisc = 2;
	ttm_page = screen[win].wrows;
	ttm_sgttyb.sg_flags = ECHO|CRMOD|EVENP|ODDP;

	/* restore tty status and flags */
	ioctl(0, TIOCSETD, &ttm_ldisc);
	ioctl(0, TIOCSETP, &ttm_sgttyb);
	ioctl(0, TIOCSETC, &ttm_tchars);
	ioctl(0, TIOCSLTC, &ttm_ltchars);
	ioctl(0, TIOCLSET, &ttm_lflags);
#ifdef TIOCSSCR
	ioctl(0, TIOCSSCR, &ttm_page);
#endif

	/* Fix the environment */
	if (term)
		strcpy(term, "pty");	/* should consider short term */
	if (termcap)
		sprintf(termcap, "pty:co#%d:li#%d:am:cl=\\EJ:le=^H:bs:cm=\\EG%%+ %%+ :nd=\\EC:up=\\EA:ce=\\EK:cd=\\EL:al=\\EP:dl=\\EN:ic=\\EO:so=\\Ea$:se=\\Eb$:us=\\Ea!:ue=\\Eb!:", screen[win].wcols, screen[win].wrows);
#ifdef JSWINSIZE
	ttm_winsize.bitsx = ttm_winsize.bytesx = screen[win].wcols;
	ttm_winsize.bitsy = ttm_winsize.bytesy = screen[win].wrows;
	ioctl(0, JSWINSIZE, &ttm_winsize);
#endif

	/* exec the shell */
	if (trace) fflush(trace);
	execl(shell, shell, "-i", 0);
	execl("/bin/csh", "/bin/csh", "-i", 0);
	execl("/bin/sh", "/bin/sh", "-i", 0);
	if (trace) fprintf(trace, "errno %d\n", errno);
	exit(1);
}
