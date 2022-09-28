/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:vt/vinit.c	1.24"

#include	<curses.h>
#include	<signal.h>
#include	<term.h>
#include	"wish.h"
#include	"vt.h"
#include	"vtdefs.h"
#include	"ctl.h"
#include	"attrs.h"
#include	"var_arrays.h"
#include	"token.h"

int Color_terminal = FALSE;

void
vt_init(labfmt)
int labfmt;		/* format for the SLKS (3-2-3 or 4-4) */
{
	register int	numcols;
	register struct vt	*v;
	static char	virt[] = "abdehijklnoprtuvwxyz";
	static char	virt_f[] = "12345678cdbemruy";
	static token	tok_virt[] = {
		/* single key virtualizations */
		TOK_IC,		TOK_BEG,	TOK_DOWN,	TOK_END,
		TOK_BACKSPACE,	TOK_TAB,	TOK_COMMAND,	TOK_DL,
		TOK_LEFT,	TOK_NEXT,	TOK_IL,		TOK_PREVIOUS,
		TOK_RIGHT,	TOK_BTAB,	TOK_UP,		TOK_PPAGE,
		TOK_NPAGE,	TOK_DC,		TOK_EOL,	TOK_COMMAND,	
		/* virtualizations starting with control-F */
		TOK_SLK1,	TOK_SLK2,	TOK_SLK3,	TOK_SLK4,
		TOK_SLK5,	TOK_SLK6,	TOK_SLK7,	TOK_SLK8,
		TOK_COMMAND,	TOK_SF,		TOK_HOME,	TOK_SHOME,
		TOK_MARK, 	TOK_RESET,	TOK_SR,		TOK_SEOL,
	};
	token	*setvirt();

	slk_init(labfmt);
	initscr();
	if (start_color() == OK)
		Color_terminal = TRUE;	
	set_mouse_info();
	nonl();
	noecho();
	set_term_ioctl();
	/* set up key virtualizations */
	setvt_attrs();		/* set up video attribute array */
	(void) setvirt('f', virt_f, setvirt('\0', virt, tok_virt));
	VT_array = NULL;
	VT_front = VT_UNDEFINED;
	VT_curid = VT_UNDEFINED;
	vt_ctl(VT_UNDEFINED, CTSETLIM, 0, LINES);

	/*
	 * banner line
	 */
	vt_current(vt_create(NULL, VT_NOBORDER | VT_NONUMBER, 0, 0, 1, columns));
	VT_back = VT_curid;

	/*
	 * command line 
	 */
	vt_current(vt_create(NULL, VT_NOBORDER | VT_NONUMBER, LINES - 1, 0, 1, columns));

	/*
	 * message line
	 */
	vt_current(vt_create(NULL, VT_NOBORDER | VT_NONUMBER, LINES - 2, 0, 1, columns));
	vt_ctl(VT_UNDEFINED, CTSETLIM, 1, LINES - 2);
}

#define control(X)	((X) & ' ' - 1)

set_mouse_info()
{
/* #ifdef i386 abs k18 */
	mouse_set(BUTTON1_PRESSED | BUTTON1_RELEASED |
		  BUTTON2_PRESSED | BUTTON2_RELEASED |
		  BUTTON3_PRESSED | BUTTON3_RELEASED);
	map_button(BUTTON1_RELEASED);
/*
 *
#else
	return;
#endif
 * abs k18
 */
}

token *
setvirt(first, string, toks)
char	first;
char	*string;
token	*toks;
{
	char	keybuf[4];

	keybuf[1] = keybuf[2] = '\0';
	keybuf[0] = control(first);
	while (*string) {
		if (first)
			keybuf[1] = *string++;
		else
			keybuf[0] = control(*string++);
		newkey(keybuf, (int) (*toks++), TRUE);
	}
	return toks;
}

set_term_ioctl()
{
	register int	fd;
	struct termio	tbuf;

	fd = -1;
	if (ioctl(0, TCGETA, &tbuf) == 0)
		fd = 0;
	else if (ioctl(1, TCGETA, &tbuf) == 0)
		fd = 1;
	else if (ioctl(2, TCGETA, &tbuf) == 0)
		fd = 2;
	if (fd >= 0) {
/*		tbuf.c_cc[VINTR] = 0xff;  ignore instead...      abs */
		sigignore(SIGINT);        /* ... abs */
		tbuf.c_cc[VQUIT] = 0xff;
		tbuf.c_cc[VMIN] = 1;
		tbuf.c_cc[VTIME] = 1;
		tbuf.c_iflag &= ~(ICRNL | INLCR);
		tbuf.c_iflag |= IGNBRK;
		tbuf.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL);
#ifdef TOSTOP   /* for job control - to prevent running in background.. */
		/* ..we want to suspend on pending output */
		tbuf.c_lflag |= TOSTOP;
#endif
		tbuf.c_oflag &= ~(OPOST);
		ioctl(fd, TCSETAW, &tbuf);
		def_prog_mode();
	}
}
