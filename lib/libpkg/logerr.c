/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:logerr.c	1.6.3.1"
#include <stdio.h>
#include <string.h>
#include <varargs.h>

/*VARARGS*/
void
logerr(va_alist)
va_dcl
{
	va_list ap;
	char	*fmt;
	char	*pt, buffer[2048];
	int	flag;

	va_start(ap);
	fmt = va_arg(ap, char *);
	flag = 0;
	if(strncmp(fmt, "ERROR:", 6) && strncmp(fmt, "WARNING:", 8)) {
		flag++;
		(void) fprintf(stderr, "    ");
	}
	(void) vsprintf(buffer, fmt, ap);
	va_end(ap);

	for(pt=buffer; *pt; pt++) {
		(void) putc(*pt, stderr);
		if(flag && (*pt == '\n') && pt[1])
			(void) fprintf(stderr, "    ");
	}
	(void) putc('\n', stderr);
}
