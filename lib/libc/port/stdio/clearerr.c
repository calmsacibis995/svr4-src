/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/clearerr.c	1.3"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <stdio.h>

#undef clearerr

void
clearerr(iop)
	FILE *iop;
{
	iop->_flag &= (unsigned char)~(_IOERR | _IOEOF);
}
