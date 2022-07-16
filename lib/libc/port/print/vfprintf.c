/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:print/vfprintf.c	1.7.1.4"
/*LINTLIBRARY*/
#include "synonyms.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

extern int _doprnt();

/*VARARGS2*/
int
vfprintf(iop, format, ap)
FILE *iop;
const char *format;
va_list ap;
{
	register int count;

	if (!(iop->_flag & _IOWRT)) {
		/* if no write flag */
		if (iop->_flag & _IORW) {
			/* if ok, cause read-write */
			iop->_flag |= _IOWRT;
		} else {
			/* else error */
			errno = EBADF;
			return EOF;
		}
	}
	count = _doprnt(format, ap, iop);
	return(ferror(iop)? EOF: count);
}
