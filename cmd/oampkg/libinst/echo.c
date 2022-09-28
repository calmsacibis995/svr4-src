/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:libinst/echo.c	1.2.3.1"
#include <stdio.h>
#include <varargs.h>

extern int	nointeract;

/*VARARGS*/
void
echo(va_alist)
va_dcl
{
	va_list ap;
	char	*fmt;

	va_start(ap);
	fmt = va_arg(ap, char *);
	va_end(ap);
		
	if(nointeract)
		return;

	(void) vfprintf(stderr, fmt, ap);
	(void) putc('\n', stderr);
}
