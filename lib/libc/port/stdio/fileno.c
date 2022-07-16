/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/fileno.c	1.6"
/*LINTLIBRARY*/

#ifdef __STDC__
	#pragma weak fileno = _fileno
#endif
#include "synonyms.h"
#include <stdio.h>

#undef fileno

int
#ifdef __STDC__
_fileno(iop)
#else
fileno(iop)
#endif
	FILE *iop;
{
	return iop->_file;
}
