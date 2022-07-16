/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/whline.c	1.5"
#include	"curses_inc.h"

whline(win, horch, num_chars)
register	WINDOW	*win;
chtype		horch;
int		num_chars;
{
    int     cury = win->_cury, curx = win->_curx;
    chtype  a, *fp = &(win->_y[cury][curx]);

    if (num_chars <= 0)
	return (ERR);
	
    if (num_chars > win->_maxx - curx + 1)
	num_chars = win->_maxx - curx + 1;
    if (horch == 0)
	horch = ACS_HLINE;
    a = _ATTR(horch);
    horch = _WCHAR(win, horch) | a;
    memSset (fp, horch | win->_attrs, num_chars);
    if (curx < win->_firstch[cury])
	win->_firstch[cury] = curx;
    if ((curx += (num_chars - 1)) > win->_lastch[cury])
	win->_lastch[cury] = curx;
    win->_flags |= _WINCHANGED;

    if (win->_sync)
	wsyncup(win);

    return (win->_immed ? wrefresh(win) : OK);
}
