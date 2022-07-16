/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/putc.c	1.4"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <stdio.h>

#undef putc

int
putc(ch, iop)
	int ch;
	register FILE *iop;
{
	if (--iop->_cnt < 0)
		return _flsbuf(ch, iop);
	else
		return *iop->_ptr++ = (unsigned char)ch;
}
