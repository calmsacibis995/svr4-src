/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/scanf.c	1.13"
/*LINTLIBRARY*/
#include "synonyms.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

extern int _doscan();

/*VARARGS1*/
int
#ifdef __STDC__
scanf(const char *fmt, ...)
#else
scanf(fmt, va_alist) char *fmt; va_dcl
#endif
{
	va_list ap;

#ifdef __STDC__
	va_start(ap, );
#else
	va_start(ap);
#endif
	return(_doscan(stdin, fmt, ap));
}

/*VARARGS2*/
int
#ifdef __STDC__
fscanf(FILE *iop, const char *fmt, ...)
#else
fscanf(iop, fmt, va_alist) FILE *iop; char *fmt; va_dcl
#endif
{
	va_list ap;

#ifdef __STDC__
	va_start(ap, );
#else
	va_start(ap);
#endif
	return(_doscan(iop, fmt, ap));
}

/*VARARGS2*/
int
#ifdef __STDC__
sscanf(register const char *str, const char *fmt, ...)
#else
sscanf(str, fmt, va_alist) register char *str; char *fmt; va_dcl
#endif
{
	va_list ap;
	FILE strbuf;

#ifdef __STDC__
	va_start(ap, );
#else
	va_start(ap);
#endif
	/* The dummy FILE * created for sscanf has the _IOWRT
	 * flag set to distinguish it from scanf and fscanf
	 * invocations. */
	strbuf._flag = _IOREAD | _IOWRT;
	strbuf._ptr = strbuf._base = (unsigned char*)str;
	strbuf._cnt = strlen(str);
	strbuf._file = _NFILE;
	return(_doscan(&strbuf, fmt, ap));
}
