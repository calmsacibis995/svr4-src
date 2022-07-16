/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:print/fprintf.c	1.14.4.2"
/*LINTLIBRARY*/
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

extern int _doprnt();

/*VARARGS2*/
int
#ifdef __STDC__
fprintf(FILE *iop, const char *format, ...)
#else
fprintf(iop, format, va_alist) FILE *iop; char *format; va_dcl
#endif
{
	register int count;
	int flag;
	va_list ap;

#ifdef __STDC__
	va_start(ap,);
#else
	va_start(ap);
#endif
	flag = iop->_flag;
	if (!(flag & _IOWRT)) {
		/* if no write flag */
		if ((flag & _IORW) && (flag & (_IOREAD | _IOEOF)) != _IOREAD) {
			/* if ok, cause read-write */
			iop->_flag = (flag & ~(_IOREAD | _IOEOF)) | _IOWRT;
		} else {
			/* else error */
			errno = EBADF;
			return EOF;
		}
	}
	count = _doprnt(format, ap, iop);
	va_end(ap);
	return(ferror(iop)? EOF: count);
}

