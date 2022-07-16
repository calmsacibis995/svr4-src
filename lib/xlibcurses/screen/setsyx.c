/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/setsyx.c	1.8"
/*
 * Set the current screen coordinates (y, x).
 *
 * This routine may be called before doupdate(). It tells doupdate()
 * where to leave the cursor instead of the location of (x, y) of the
 * last window that was wnoutrefreshed or pnoutrefreshed.
 * If x and y are negative, then the cursor will be left wherever
 * curses decides to leave it, as if leaveok() had been TRUE for the
 * last window refreshed.
 */
#include	"curses_inc.h"

setsyx(y, x)
int	y, x;
{
    if (y < 0 && x < 0)
    {
	SP->virt_scr->_leave = TRUE;
    }
    else
    {
	_virtscr->_cury = y + SP->Yabove;
	_virtscr->_curx = x;
	_virtscr->_leave = FALSE;
	_virtscr->_flags |= _WINMOVED;
    }
    return (OK);
}
