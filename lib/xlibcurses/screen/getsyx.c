/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/getsyx.c	1.7"
#include	"curses_inc.h"

/*
 * Get the current screen coordinates (y, x).
 *
 * The current screen coordinates are defined as the last place that
 * the cursor was placed by a wnoutrefresh(), pnoutrefresh() or setsyx()
 * call. If leaveok() was true for the last window refreshed, then
 * return (-1, -1) so that setsyx() can reset the leaveok flag.
 *
 * This function is actually called by the macro getsyx(y, x), which is
 * defined in curses.h as:
 *
 * #define getsyx(y, x)	_getsyx(&y, &x)
 *
 * Note that this macro just adds in the '&'. In this way, getsyx()
 * is parallel with the other getyx() routines which don't require
 * ampersands. The reason that this can't all be a macro is that
 * that we need to access SP, which is normally not available in
 * user-level routines.
 */

_getsyx(yp, xp)
int	*yp, *xp;
{
    if (SP->virt_scr->_leave)
	*yp = *xp = -1;
    else
    {
	*yp = _virtscr->_cury - SP->Yabove;
	*xp = _virtscr->_curx;
    }
    return (OK);
}
