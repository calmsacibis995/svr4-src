/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/V2._sprintw.c	1.7"

# include	"curses_inc.h"
#ifdef __STDC__
#include	<stdarg.h>
#else
# include	<varargs.h>
#endif

#ifdef _VR2_COMPAT_CODE
/*
	This is only here for compatibility with SVR2 curses.
	It will go away someday. Programs should reference
	vwprintw() instead.
 */

_sprintw(win, fmt, ap)
register WINDOW	*win;
register char * fmt;
va_list ap;
{
	return vwprintw(win, fmt, ap);
}
#endif
