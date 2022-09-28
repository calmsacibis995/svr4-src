/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:Dout.c	1.3.3.1"
/*
    NAME
	Dout - Print debug output

    SYNOPSIS
	void Dout(char *subname, int level, char *msg, ...)

    DESCRIPTION
	Dout prints debugging output if debugging is turned
	on (-x specified) and the level of this message is
	lower than the value of the global variable debug.
	The subroutine name is printed if it is not a null
	string.
*/
#include "mail.h"
#ifdef __STDC__
# include <stdarg.h>
#else
# include <varargs.h>
#endif

/* VARARGS3 PRINTFLIKE3 */
void
#ifdef __STDC__
Dout(char *subname, int level, char *fmt, ...)
#else
# ifdef lint
Dout(Xsubname, Xlevel, Xfmt, va_alist)
char *Xsubname, *Xfmt;
int Xlevel;
va_dcl
# else
Dout(va_alist)
va_dcl
# endif
#endif
{
#ifndef __STDC__
	char    *subname;
	int	level;
	char    *fmt;
#endif
	va_list args;

#ifndef __STDC__
#ifdef lint
	subname = Xsubname;
	level = Xlevel;
	fmt = Xfmt;
# endif
#endif

#ifdef __STDC__
	va_start(args, fmt);
#else
	va_start(args);
	subname = va_arg(args, char *);
	level = va_arg(args, int);
	fmt = va_arg(args, char *);
#endif

	if (debug > level) {
		if (subname && *subname) {
			fprintf(dbgfp,"%s(): ", subname);
		}
		vfprintf(dbgfp, fmt, args);
	}
	va_end(args);
}
