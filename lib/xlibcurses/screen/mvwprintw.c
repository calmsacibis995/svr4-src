/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/mvwprintw.c	1.8"

# include	"curses_inc.h"

#ifdef __STDC__
#include	<stdarg.h>
#else
#include <varargs.h>
#endif

/*
 * implement the mvprintw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Sigh....
 *
 */

/*VARARGS*/
#ifdef __STDC__
mvwprintw(WINDOW *win, int y, int x, ...)
#else
mvwprintw(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
	register WINDOW	*win;
	register int	y, x;
#endif
	register char	*fmt;
	va_list ap;

#ifdef __STDC__
	va_start(ap, x);
#else
	va_start(ap);
	win = va_arg(ap, WINDOW *);
	y = va_arg(ap, int);
	x = va_arg(ap, int);
#endif
	fmt = va_arg(ap, char *);
	return wmove(win, y, x) == OK ? vwprintw(win, fmt, ap) : ERR;
}
