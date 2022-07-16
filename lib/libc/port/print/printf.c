/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:print/printf.c	1.14"
/*LINTLIBRARY*/
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

extern int _doprnt();

/*VARARGS1*/
int
#ifdef __STDC__
printf(const char *format, ...)
#else
printf(format, va_alist) char *format; va_dcl
#endif
{
	register int count;
	va_list ap;

#ifdef __STDC__
	va_start(ap,);
#else
	va_start(ap);
#endif
	if (!(stdout->_flag & _IOWRT)) {
		/* if no write flag */
		if (stdout->_flag & _IORW) {
			/* if ok, cause read-write */
			stdout->_flag |= _IOWRT;
		} else {
			/* else error */
			errno = EBADF;
			return EOF;
		}
	}
	count = _doprnt(format, ap, stdout);
	va_end(ap);
	return(ferror(stdout)? EOF: count);
}
