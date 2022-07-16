/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/winchstr.c	1.3"
#include	"curses_inc.h"

winchstr(win,str)
WINDOW	*win;
chtype	*str;
{
    return (winchnstr(win,str,win->_maxx - win->_curx));
}
