/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/talk/display.c	1.3.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


/* The window 'manager', initializes curses and handles the actual
 * displaying of text
 */

#include "talk.h"
#include <ctype.h>

xwin_t my_win;
xwin_t rem_win;
WINDOW *line_win;

int curses_initialized = 0;

    /* max HAS to be a function, it is called with
     * a argument of the form --foo at least once.
     */

max(a,b)
int a, b;
{
    if (a > b) {
	return(a);
    } else {
	return(b);
    }
}

/*
 * Display some text on somebody's window, processing some control
 * characters while we are at it.
 */

display(win, text, size)
register xwin_t *win;
register char *text;
int size;
{
    register int i;
    char cch;

    for (i = 0; i < size; i++) {
	int itext;

	if (*text == '\n'|| *text == '\r') {
	    xscroll(win, 0);
	    text++;
	    continue;
	}

	    /* erase character */

	if (*text == win->cerase) {
	    wmove(win->x_win, win->x_line, max(--win->x_col, 0));
	    getyx(win->x_win, win->x_line, win->x_col);
	    waddch(win->x_win, ' ');
	    wmove(win->x_win, win->x_line, win->x_col);
	    getyx(win->x_win, win->x_line, win->x_col);
	    text++;
	    continue;
	}
	/*
	 * On word erase search backwards until we find
	 * the beginning of a word or the beginning of
	 * the line.
	 */
	if (*text == win->werase) {
	    int endcol, xcol, i, c;

	    endcol = win->x_col;
	    xcol = endcol - 1;
	    while (xcol >= 0) {
		c = readwin(win->x_win, win->x_line, xcol);
		if (c != ' ')
			break;
		xcol--;
	    }
	    while (xcol >= 0) {
		c = readwin(win->x_win, win->x_line, xcol);
		if (c == ' ')
			break;
		xcol--;
	    }
	    wmove(win->x_win, win->x_line, xcol + 1);
	    for (i = xcol + 1; i < endcol; i++)
		waddch(win->x_win, ' ');
	    wmove(win->x_win, win->x_line, xcol + 1);
	    getyx(win->x_win, win->x_line, win->x_col);
	    continue;
	}
	    /* line kill */
	if (*text == win->kill) {
	    wmove(win->x_win, win->x_line, 0);
	    wclrtoeol(win->x_win);
	    getyx(win->x_win, win->x_line, win->x_col);
	    text++;
	    continue;
	}
	if (*text == '\f') {
	    if (win == &my_win)
		wrefresh(curscr);
	    text++;
	    continue;
	}

	/* check for wrap around */
	getyx(win->x_win, win->x_line, win->x_col);
	if (win->x_col == COLS-1) {
		xscroll(win, 0);
	}

	itext = (unsigned int) *text;
	if (isprint(itext) || *text == ' '    || *text == '\t' || 
	                      *text == '\013' || *text == '\007' /* bell */ ) {
	    	waddch(win->x_win, *text);
	} else {

        	if (!isascii(*text)) {
	    	    /* check for wrap around */
	    	    if (win->x_col == COLS-3) {
			xscroll(win, 0);
	    	    }
	    	    waddch(win->x_win, 'M');
	    	    waddch(win->x_win, '-');
                    *text = toascii(*text);
               	}
               	if (iscntrl(*text)) {

	    	    /* check for wrap around */
	    	    getyx(win->x_win, win->x_line, win->x_col);
		    if (win->x_col == COLS-2) {
			xscroll(win, 0);
	    	    }

	    	    waddch(win->x_win, '^');
	    	    waddch(win->x_win, *text + 0100);
		}
                else
	    	    waddch(win->x_win, *text);
	}                                         

	getyx(win->x_win, win->x_line, win->x_col);
	text++;

    }  /* for loop */
wrefresh(win->x_win);
}

/*
* Read the character at the indicated position in win
*/
readwin(win, line, col)
WINDOW *win;
{
int oldline, oldcol;
register int c;

getyx(win, oldline, oldcol);
wmove(win, line, col);
c = winch(win);
wmove(win, oldline, oldcol);
return(c);
}

/*
* Scroll a window, blanking out the line following the current line
* so that the current position is obvious
*/

xscroll(win, flag)
register xwin_t *win;
int flag;
{
    if (flag == -1) {
	wmove(win->x_win, 0, 0);
	win->x_line = 0;
	win->x_col = 0;
	return;
    }
    win->x_line = (win->x_line + 1) % win->x_nlines;
    win->x_col = 0;
    wmove(win->x_win, win->x_line, win->x_col);
    wclrtoeol(win->x_win);
    wmove(win->x_win, (win->x_line + 1) % win->x_nlines, win->x_col);
    wclrtoeol(win->x_win);
    wmove(win->x_win, win->x_line, win->x_col);
}
