/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvgetstr.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

mvgetstr(y, x, str)
int	y, x;
char	*str;
{
    return (wmove(stdscr, y, x)==ERR?ERR:wgetstr(stdscr, str));
}
