/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ttymon:sttyparse.c	1.14.5.1"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <termio.h>
#include <sys/stermio.h>
#include <sys/termiox.h>
#include "stty.h"

#ifdef i386
#include <sys/ioctl.h>
#include <sys/at_ansi.h>
#include <sys/kd.h>

char 	*STTY = "stty: ";
#endif

static char	*s_arg;			/* s_arg: ptr to mode to be set */
static int	match;
static int gct(), eq(), encode();

/* set terminal modes for supplied options */
char *
sttyparse(argc, argv, term, ocb, cb, termiox, winsize)
int	argc;
char	*argv[];
int	term; /* type of tty device */ 

struct	termio	*ocb;
struct	termios	*cb;
struct	termiox	*termiox;
struct	winsize	*winsize;
{
	register i;
	extern	const struct	speeds	speeds[];
	extern	const struct	mds	lmodes[];
	extern	const struct	mds	nlmodes[];
	extern	const struct	mds	cmodes[];
	extern	const struct	mds	ncmodes[];
	extern	const struct	mds	imodes[];
	extern	const struct	mds	nimodes[];
	extern	const struct	mds	omodes[];
	extern	const struct	mds	hmodes[];
	extern	const struct	mds	clkmodes[];

	while(--argc > 0) {

		s_arg = *++argv;
		match = 0;
		if (term & ASYNC) {

#ifdef i386
			if (eq("mono"))
			{
				if (ioctl(0, SWAPMONO, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			}
			else if (eq("color"))
			    {
				if (ioctl(0, SWAPCGA, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("pro"))
			    {
				if (ioctl(0, SWAPPGA, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("enhanced"))
			     {
				if (ioctl(0, SWAPEGA, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("B40x25"))
			    {
				if (ioctl(0, SW_B40x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("C40x25"))
			    {
				if (ioctl(0, SW_C40x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("B80x25"))
			    {
				if (ioctl(0, SW_B80x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("C80x25"))
			    {
				if (ioctl(0, SW_C80x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("BG320"))
			    {
				if (ioctl(0, SW_BG320, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("CG320"))
			    {
				if (ioctl(0, SW_CG320, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("BG640"))
			    {
				if (ioctl(0, SW_BG640, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("EGAMONO80x25"))
			    {
				if (ioctl(0, SW_EGAMONO80x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("CG320_D"))
			    {
				if (ioctl(0, SW_CG320_D, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("CG640_E"))
			    {
				if (ioctl(0, SW_CG640_E, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("EGAMONOAPA"))
			    {
				if (ioctl(0, SW_EGAMONOAPA, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("CG640x350"))
			    {
				if (ioctl(0, SW_CG640x350, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("ENHMONOAPA2"))
			    {
				if (ioctl(0, SW_ENH_MONOAPA2, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("ENH_CG640"))
			    {
				if (ioctl(0, SW_ENH_CG640, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("ENH_B40x25"))
			    {
				if (ioctl(0, SW_ENHB40x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("ENH_C40x25"))
			    {
				if (ioctl(0, SW_ENHC40x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("ENH_B80x25"))
			    {
				if (ioctl(0, SW_ENHB80x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("ENH_B80x43"))
			    {
				if (ioctl(0, SW_ENHB80x43, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("ENH_C80x43"))
			    {
				if (ioctl(0, SW_ENHC80x43, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("ENH_C80x25"))
			    {
				if (ioctl(0, SW_ENHC80x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("VGA_C80x25"))
			    {
				if (ioctl(0, SW_VGAC80x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("VGA_C40x25"))
			    {
				if (ioctl(0, SW_VGAC40x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("VGAMONO80x25"))
			    {
				if (ioctl(0, SW_VGAMONO80x25, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("MCAMODE"))
			    {
				if (ioctl(0, SW_MCAMODE, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("KB_ENABLE"))
			    {
				if (ioctl(0, TIOCKBON, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
			    }
			else if (eq("KB_DISABLE"))
			    {
				if (ioctl(0, TIOCKBOF, &cb) == -1)
				{
					perror(STTY);
					exit(2);
				}
		    	    }
#endif

			if (eq("erase") && --argc)
				cb->c_cc[VERASE] = gct(*++argv, term);
			else if (eq("intr") && --argc)
				cb->c_cc[VINTR] = gct(*++argv, term);
			else if (eq("quit") && --argc)
				cb->c_cc[VQUIT] = gct(*++argv, term);
			else if (eq("eof") && --argc)
				cb->c_cc[VEOF] = gct(*++argv, term);
			else if (eq("min") && --argc) {
				if(isdigit((unsigned char)argv[1][0]))
					cb->c_cc[VMIN] = atoi(*++argv);
				else
					cb->c_cc[VMIN] = gct(*++argv);
			}
			else if (eq("eol") && --argc)
				cb->c_cc[VEOL] = gct(*++argv, term);
			else if (eq("eol2") && --argc)
				cb->c_cc[VEOL2] = gct(*++argv, term);
			else if (eq("time") && --argc) {
				if(isdigit((unsigned char)argv[1][0]))
					cb->c_cc[VTIME] = atoi(*++argv);
				else
					cb->c_cc[VTIME] = gct(*++argv);
			}
			else if (eq("kill") && --argc)
				cb->c_cc[VKILL] = gct(*++argv, term);
			else if (eq("swtch") && --argc)
				cb->c_cc[VSWTCH] = gct(*++argv, term);
			if(match)
				continue;
			if (term & TERMIOS) {
				if (eq("start") && --argc)
					cb->c_cc[VSTART] = gct(*++argv, term);
				else if (eq("stop") && --argc)
					cb->c_cc[VSTOP] = gct(*++argv, term);
				else if (eq("susp") && --argc)
					cb->c_cc[VSUSP] = gct(*++argv, term);
				else if (eq("dsusp") && --argc)
					cb->c_cc[VDSUSP] = gct(*++argv, term);
				else if (eq("rprnt") && --argc)
					cb->c_cc[VREPRINT] = gct(*++argv, term);
				else if (eq("flush") && --argc)
					cb->c_cc[VDISCARD] = gct(*++argv, term);
				else if (eq("werase") && --argc)
					cb->c_cc[VWERASE] = gct(*++argv, term);
				else if (eq("lnext") && --argc)
					cb->c_cc[VLNEXT] = gct(*++argv, term);
			}
			if(match)
				continue;
			if (eq("ek")) {
				cb->c_cc[VERASE] = CERASE;
				cb->c_cc[VKILL] = CKILL;
			}
			else if (eq("line") && 
			         !(term & TERMIOS) && --argc) {
				ocb->c_line = atoi(*++argv);
				continue;
			}
			else if (eq("raw")) {
				cb->c_cc[VMIN] = 1;
				cb->c_cc[VTIME] = 1;
			}
			else if (eq("-raw") | eq("cooked")) {
				cb->c_cc[VEOF] = CEOF;
				cb->c_cc[VEOL] = CNUL;
			}
			else if(eq("sane")) {
				cb->c_cc[VERASE] = CERASE;
				cb->c_cc[VKILL] = CKILL;
				cb->c_cc[VQUIT] = CQUIT;
				cb->c_cc[VINTR] = CINTR;
				cb->c_cc[VEOF] = CEOF;
				cb->c_cc[VEOL] = CNUL;
							   /* SWTCH purposely not set */
			}
			else if((term & TERMIOS) && eq("ospeed") && --argc) { 
				s_arg = *++argv;
				match = 0;
				for(i=0; speeds[i].string; i++)
					if(eq(speeds[i].string)) {
						cb->c_cflag &= ~CBAUD;
						cb->c_cflag |= speeds[i].speed;
					}
				if(!match)
					return s_arg;
				continue;
			}
			else if((term & TERMIOS) && eq("ispeed") && --argc) { 
				s_arg = *++argv;
				match = 0;
				for(i=0; speeds[i].string; i++)
					if(eq(speeds[i].string)) {
						cb->c_cflag &= ~(CIBAUD);
						cb->c_cflag |= ((speeds[i].speed<<IBSHIFT)&CIBAUD);
					}
				if(!match)
					return s_arg;
				continue;
			}
			for(i=0; speeds[i].string; i++)
				if(eq(speeds[i].string)) {
					cb->c_cflag &= ~(CBAUD|CIBAUD);
					cb->c_cflag |= speeds[i].speed;
				}
		}
		if (!(term & ASYNC) && eq("ctab") && --argc) {
			cb->c_cc[7] = gct(*++argv, term);
			continue;
		}
			
		for(i=0; imodes[i].string; i++)
			if(eq(imodes[i].string)) {
				cb->c_iflag &= ~imodes[i].reset;
				cb->c_iflag |= imodes[i].set;
			}
		if (term & TERMIOS) {
			for(i=0; nimodes[i].string; i++)
				if(eq(nimodes[i].string)) {
					cb->c_iflag &= ~nimodes[i].reset;
					cb->c_iflag |= nimodes[i].set;
				}
		}

		for(i=0; omodes[i].string; i++)
			if(eq(omodes[i].string)) {
				cb->c_oflag &= ~omodes[i].reset;
				cb->c_oflag |= omodes[i].set;
			}
		if (!(term & ASYNC) && eq("sane")) {
			cb->c_oflag |= TAB3;
			continue;
		}
		for(i=0; cmodes[i].string; i++)
			if(eq(cmodes[i].string)) {
				cb->c_cflag &= ~cmodes[i].reset;
				cb->c_cflag |= cmodes[i].set;
			}
		if (term & TERMIOS)
			for(i=0; ncmodes[i].string; i++)
				if(eq(ncmodes[i].string)) {
					cb->c_cflag &= ~ncmodes[i].reset;
					cb->c_cflag |= ncmodes[i].set;
				}
		for(i=0; lmodes[i].string; i++)
			if(eq(lmodes[i].string)) {
				cb->c_lflag &= ~lmodes[i].reset;
				cb->c_lflag |= lmodes[i].set;
			}


		if(term & TERMIOS)
			for(i=0; nlmodes[i].string; i++)
				if(eq(nlmodes[i].string)) {
					cb->c_lflag &= ~nlmodes[i].reset;
					cb->c_lflag |= nlmodes[i].set;
				}
		if (term & FLOW) {
			for(i=0; hmodes[i].string; i++)
				if(eq(hmodes[i].string)) {
					termiox->x_hflag &= ~hmodes[i].reset;
					termiox->x_hflag |= hmodes[i].set;
				}
			for(i=0; clkmodes[i].string; i++)
				if(eq(clkmodes[i].string)) {
					termiox->x_cflag &= ~clkmodes[i].reset;
					termiox->x_cflag |= clkmodes[i].set;
				}
			
		}

		if(eq("rows") && --argc)
			winsize->ws_row = atoi(*++argv);
		else if(eq("columns") && --argc)
			winsize->ws_col = atoi(*++argv);
		else if(eq("xpixels") && --argc)
			winsize->ws_xpixel = atoi(*++argv);
		else if(eq("ypixels") && --argc)
			winsize->ws_ypixel = atoi(*++argv);

		if(!match)
			if(!encode(cb)) {
				return(s_arg); /* parsing failed */
			}
	}


	return((char *)0);
}

static int eq(string)
char *string;
{
	register i;

	if(!s_arg)
		return(0);
	i = 0;
loop:
	if(s_arg[i] != string[i])
		return(0);
	if(s_arg[i++] != '\0')
		goto loop;
	match++;
	return(1);
}

/* get pseudo control characters from terminal */
/* and convert to internal representation      */
static int gct(cp, term)
register char *cp;
int term;
{
	register c;

	c = *cp++;
	if (c == '^') {
		c = *cp;
		if (c == '?')
			c = CINTR;		/* map '^?' to DEL */
		else if (c == '-')
			c = (term & TERMIOS) ? _POSIX_VDISABLE : 0200;		/* map '^-' to undefined */
		else
			c &= 037;
	}
	return(c);
}

/* get modes of tty device and fill in applicable structures */
int 
get_ttymode(fd, termio, termios, stermio, termiox, winsize)
int fd;
struct termio *termio;
struct termios *termios;
struct stio *stermio;
struct termiox *termiox;
struct winsize *winsize;
{
	int i;
	int term = 0;
	if(ioctl(fd, STGET, stermio) == -1) {
		term |= ASYNC;
		if(ioctl(fd, TCGETS, termios) == -1) {
			if(ioctl(fd, TCGETA, termio) == -1) 
				return -1;
			termios->c_lflag = termio->c_lflag;
			termios->c_oflag = termio->c_oflag;
			termios->c_iflag = termio->c_iflag;
			termios->c_cflag = termio->c_cflag;
			for(i = 0; i < NCC; i++)
				termios->c_cc[i] = termio->c_cc[i];
		} else
			term |= TERMIOS;
	}
	else {
		termios->c_cc[7] = (unsigned)stermio->tab;
		termios->c_lflag = stermio->lmode;
		termios->c_oflag = stermio->omode;
		termios->c_iflag = stermio->imode;
	}

	
	if(ioctl(fd, TCGETX, termiox) == 0)
		term |= FLOW;

	if (ioctl(fd, TIOCGWINSZ, winsize) == 0)
		term |= WINDOW;
	return term;
}

/* set tty modes */
int 
set_ttymode(fd, term, termio, termios, stermio, termiox, winsize, owinsize)
int fd, term;
struct termio *termio;
struct termios *termios;
struct stio *stermio;
struct termiox *termiox;
struct winsize *winsize, *owinsize;
{
	int i;
	if (term & ASYNC) {
		if(term & TERMIOS) {
			if(ioctl(fd, TCSETSW, termios) == -1)
				return -1;
		} else {
			termio->c_lflag = termios->c_lflag;
			termio->c_oflag = termios->c_oflag;
			termio->c_iflag = termios->c_iflag;
			termio->c_cflag = termios->c_cflag;
			for(i = 0; i < NCC; i++)
				termio->c_cc[i] = termios->c_cc[i];
			if(ioctl(fd, TCSETAW, termio) == -1)
				return -1;
		}
			
	} else {
		stermio->imode = termios->c_iflag;
		stermio->omode = termios->c_oflag;
		stermio->lmode = termios->c_lflag;
		stermio->tab = termios->c_cc[7];
		if (ioctl(fd, STSET, stermio) == -1)
			return -1;
	}
	if(term & FLOW) {
		if(ioctl(fd, TCSETXW, termiox) == -1)
			return -1;
	}
	if ((owinsize->ws_col != winsize->ws_col ||
	    owinsize->ws_row != winsize->ws_row ||
	    owinsize->ws_xpixel != winsize->ws_xpixel ||
	    owinsize->ws_ypixel != winsize->ws_ypixel) &&
	    ioctl(0, TIOCSWINSZ, winsize) != 0)
		return -1;
	return 0;
}

static int encode(cb)
struct	termios	*cb;
{
	unsigned long grab[20];
	int last, i;

	i = sscanf(s_arg, 
	"%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx:%lx",
	&grab[0],&grab[1],&grab[2],&grab[3],&grab[4],&grab[5],&grab[6],
	&grab[7],&grab[8],&grab[9],&grab[10],&grab[11],
	&grab[12], &grab[13], &grab[14], &grab[15], 
	&grab[16], &grab[17], &grab[18], &grab[19]);

	if(i < 12) 
		return(0);
	cb->c_iflag = grab[0];
	cb->c_oflag = grab[1];
	cb->c_cflag = grab[2];
	cb->c_lflag = grab[3];

	if(i == 20)
		last = NCCS - 1;
	else
		last = NCC;
	for(i=0; i<last; i++)
		cb->c_cc[i] = (unsigned char) grab[i+4];
	return(1);
}

