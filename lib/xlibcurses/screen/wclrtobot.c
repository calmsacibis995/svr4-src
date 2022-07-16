/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/wclrtobot.c	1.2"
#include	"curses_inc.h"

/* This routine erases everything on the window. */
wclrtobot(win)
register	WINDOW	*win;
{
    register	int	savimmed, savsync;
    register	int	cury = win->_cury;
    register	int	curx = win->_curx;

    if (win != curscr)
    {
	savimmed = win->_immed;
	savsync = win->_sync;
	win->_immed = win->_sync = FALSE;
    }

    /* set region to be clear */
    if (cury >= win->_tmarg && cury <= win->_bmarg)
	win->_cury = win->_bmarg;
    else
	win->_cury = win->_maxy - 1;

    win->_curx = 0;
    for ( ; win->_cury > cury; win->_cury--)
	(void) wclrtoeol(win);
    win->_curx = curx;
    (void) wclrtoeol(win);

    if (win == curscr)
	return (OK);

    /* not curscr */
    win->_sync = savsync;

    if (win->_sync)
	wsyncup(win);
    
    win->_flags |= _WINCHANGED;
    return ((win->_immed = savimmed) ? wrefresh(win) : OK);
}
