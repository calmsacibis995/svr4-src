/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)xcplxcurses:initscr.c	1.1"

/*
 *	@(#) initscr.c 1.1 90/03/30 lxcurses:initscr.c
 */
/*
 *	MODIFICATION HISTORY
 *	M000	15 Mar 1985	ncm
 *	- added include of sys/types.h since param.h no longer does.
 */
# include	"ext.h"
# include	<signal.h>
# include	<sys/types.h>
# include	<sys/param.h>

extern char	*getenv();

/*
 *	This routine initializes the current and standard screen.
 *
 * 5/19/83 (Berkeley) @(#)initscr.c	1.3
 */
WINDOW *
initscr() {

	reg char	*sp;
	int		tstp();

# ifdef DEBUG
	fprintf(outf, "INITSCR()\n");
# endif
	save_mode();		/* save modes for SYS III/V */
	if (My_term)
		setterm(Def_term);
	else {
		if (isatty(2))
			_tty_ch = 2;
		else {
			for (_tty_ch = 0; _tty_ch < NOFILE; _tty_ch++)
				if (isatty(_tty_ch))
					break;
		}
		gettmode();
		if ((sp = getenv("TERM")) == (char *)NULL)
			sp = Def_term;
		setterm(sp);
# ifdef DEBUG
		fprintf(outf, "INITSCR: term = %s\n", sp);
# endif
	}
	_puts(TI);
	_puts(VS);
# ifdef SIGTSTP
	signal(SIGTSTP, tstp);
# endif
	if (curscr != (WINDOW *)NULL) {
# ifdef DEBUG
		fprintf(outf, "INITSCR: curscr = 0%o\n", curscr);
# endif
		delwin(curscr);
	}
# ifdef DEBUG
	fprintf(outf, "LINES = %d, COLS = %d\n", LINES, COLS);
# endif
	if ((curscr = newwin(LINES, COLS, 0, 0)) == (WINDOW *)ERR)
		return (WINDOW *)ERR;
	curscr->_clear = TRUE;
	if (stdscr != (WINDOW *)NULL) {
# ifdef DEBUG
		fprintf(outf, "INITSCR: stdscr = 0%o\n", stdscr);
# endif
		delwin(stdscr);
	}
	stdscr = newwin(LINES, COLS, 0, 0);
	return stdscr;
}
