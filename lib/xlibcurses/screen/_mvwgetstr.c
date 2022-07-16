/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvwgetstr.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

mvwgetstr(win, y, x, str)
WINDOW	*win;
int	y, x;
char	*str;
{
    return (wmove(win, y, x)==ERR?ERR:wgetstr(win, str));
}
