/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_mvinsnstr.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

mvinsnstr(y, x, s, n)
int	y, x;
char	*s;
int	n;
{
    return (wmove(stdscr, y, x)==ERR?ERR:winsnstr(stdscr, s, n));
}
