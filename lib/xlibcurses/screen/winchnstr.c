/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/winchnstr.c	1.2"
#include	"curses_inc.h"

/*
 * Read in ncols worth of data from window win and assign the
 * chars to string. NULL terminate string upon completion.
 * Return the number of chtypes copied.
 */

winchnstr(win,string,ncols)
register	WINDOW	*win;
chtype		*string;
int		ncols;
{
    register	chtype	*ptr = &(win->_y[win->_cury][win->_curx]);
    register	int	counter;

    /*
     * Adjust ncols if it will exceed the maximum for the
     * window being referenced
     */

    if ((win->_curx + ncols) > win->_maxx)
	ncols = win->_maxx - win->_curx;

    for (counter = 0; counter < ncols; counter++)
	*string++ = *ptr++;

    *string = (chtype) 0;
    return (counter);
}
