/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/fseek.c	1.15"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Seek for standard library.  Coordinates with buffering.
 */
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>

extern long lseek();
extern int fflush();

int
fseek(iop, offset, ptrname)
register FILE *iop;
long	offset;
int	ptrname;
{
	long	p;

	if(iop->_flag & _IOREAD) {
		if(ptrname == 1 && iop->_base && !(iop->_flag&_IONBF)) {
			offset -= iop->_cnt;
		}
	} else if(iop->_flag & (_IOWRT | _IORW)) {
		if (fflush(iop) == EOF)
			return(-1);
	}
	iop->_flag &= (unsigned short)~_IOEOF;
	iop->_cnt = 0;
	iop->_ptr = iop->_base;
	if(iop->_flag & _IORW) {
		iop->_flag &= (unsigned short)~(_IOREAD | _IOWRT);
	}
	p = lseek(fileno(iop), offset, ptrname);
	return((p == -1)? -1: 0);
}
