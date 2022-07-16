/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/ftell.c	1.13"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Return file offset.
 * Coordinates with buffering.
 */
#include "synonyms.h"
#include <stdio.h>

extern long lseek(), errno;

long
ftell(iop)
	register FILE	*iop;
{
	register int adjust;
	long	tres;

	if (iop->_cnt < 0)
		iop->_cnt = 0;
	if (iop->_flag & _IOREAD)
		adjust = -iop->_cnt;
	else if (iop->_flag & (_IOWRT | _IORW)) 
	{
		adjust = 0;
		if (((iop->_flag & (_IOWRT | _IONBF)) == _IOWRT) && (iop->_base != 0))
			adjust = iop->_ptr - iop->_base;
	}
	else {
		errno = 9;	/* EBADF -- file descriptor refers to no open file */
		return EOF;
	}

	tres = lseek(fileno(iop), 0L, 1);
	if (tres >= 0)
		tres += (long)adjust;
	return tres;
}
