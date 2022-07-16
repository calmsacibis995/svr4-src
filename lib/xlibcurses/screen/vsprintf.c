/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/vsprintf.c	1.8"
/*LINTLIBRARY*/
#include "curses_inc.h"

#ifdef __STDC__
#include	<stdarg.h>
#else
#include <varargs.h>
#endif

extern	int	_doprnt();

int
vsprintf(string, format, ap)
char *string, *format;
va_list ap;
{
	register int count;
	FILE siop;

	siop._cnt = MAXINT;
#ifdef _NFILE
	siop._file = _NFILE;
#endif
	siop._flag = _IOWRT;
#ifdef SYSV
	siop._base = siop._ptr = (unsigned char *)string;
#else
	siop._flag |= _IOSTRG;
	siop._ptr = string;
#endif
	count = _doprnt(format, ap, &siop);
	*siop._ptr = '\0'; /* plant terminating null character */
	return (count);
}

int
vfprintf(fp, format, ap)
FILE *fp;
char *format;
va_list ap;
{
	int n;
	char spbuf[512];

	n = vsprintf(spbuf, format, ap);
	(void) fputs(spbuf, fp);
	return (n);
}
