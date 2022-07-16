/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/ungetc.c	2.11"
/*	3.0 SID #	1.3	*/
/*LINTLIBRARY*/
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"

int
ungetc(c, iop)
	int c;
	register FILE *iop;
{
	if (c == EOF)   
		return EOF;
	if (iop->_ptr <= iop->_base)
	{
		if (iop->_base == 0)
		{
			if (_findbuf(iop) == 0)
				return EOF;
		}
		else if (iop->_ptr <= iop->_base - PUSHBACK)
			return EOF;
	}
	if ((iop->_flag & _IOREAD) == 0) /* basically a no-op on write stream */
		++iop->_ptr;
	*--iop->_ptr = c;
	++iop->_cnt;
	iop->_flag &= (unsigned short)~_IOEOF;
	return c;
}
