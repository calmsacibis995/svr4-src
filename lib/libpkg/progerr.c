/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*LINTLIBRARY*/
#ident	"@(#)libpkg:progerr.c	1.5.3.1"

#include <stdio.h>
#include <varargs.h>

extern char	*prog;

/*VARARGS*/
void
progerr(va_alist)
va_dcl
{
	va_list ap;
	char	*fmt;

	va_start(ap);

	(void) fprintf(stderr, "%s: ERROR: ", prog);

	fmt = va_arg(ap, char *);
	(void) vfprintf(stderr, fmt, ap);
	(void) fprintf(stderr, "\n");

	va_end(ap);
}
