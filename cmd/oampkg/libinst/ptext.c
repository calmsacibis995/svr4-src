/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:libinst/ptext.c	1.2.3.1"
#include <stdio.h>
#include <varargs.h>

extern int	puttext();

/*VARARGS*/
void
ptext(va_alist)
va_dcl
{
	va_list ap;
	FILE	*fp;
	char	*fmt;
	char	buffer[2048];

	va_start(ap);
	fp = va_arg(ap, FILE *);
	fmt = va_arg(ap, char *);
	va_end(ap);
		
	(void) vsprintf(buffer, fmt, ap);

	(void) puttext(fp, buffer, 0, 70);
	(void) putc('\n', fp);
}
