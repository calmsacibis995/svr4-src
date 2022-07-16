/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvinchstr.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

mvinchstr(y, x, s)
int	y, x;
chtype	*s;
{
    return (wmove(stdscr, y, x)==ERR?ERR:winchstr(stdscr, s));
}
