/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#ident	"@(#)ucblibc:port/print/fprintf.c	1.2.3.1"

/*LINTLIBRARY*/
#include <stdio.h>
#include <stdarg.h>

extern int _doprnt();

/*VARARGS2*/
int
#ifdef	__STDC__
fprintf(FILE *iop, const char *format, ...)
#else
fprintf(iop, format, va_alist)
	FILE	*iop;
	char	*format;
	va_dcl;
#endif
{
	unsigned char localbuf[BUFSIZ];
	register int count;
	va_list ap;

#ifdef	__STDC__
	va_start(ap,);
#else
	va_start(ap);
#endif
	if (!(iop->_flag & _IOWRT)) {
		/* if no write flag */
		if (iop->_flag & _IORW) {
			/* if ok, cause read-write */
			iop->_flag |= _IOWRT;
		} else {
			/* else error */
			return EOF;
		}
	}
	count = _doprnt(format, ap, iop);
	va_end(ap);
	return(count);
}
