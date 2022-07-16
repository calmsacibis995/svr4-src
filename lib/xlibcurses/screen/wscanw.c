/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/wscanw.c	1.10"
/*
 * scanw and friends
 *
 */

# include	"curses_inc.h"

#ifdef __STDC__
#include	<stdarg.h>
#else
#include <varargs.h>
#endif

/*
 *	This routine implements a scanf on the given window.
 */
/*VARARGS*/
#ifdef __STDC__
wscanw(WINDOW *win, ...)
#else
wscanw(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
	register WINDOW	*win;
#endif
	register char	*fmt;
	va_list	ap;

#ifdef __STDC__
	va_start(ap, win);
#else
	va_start(ap);
	win = va_arg(ap, WINDOW *);
#endif
	fmt = va_arg(ap, char *);
	return vwscanw(win, fmt, ap);
}
