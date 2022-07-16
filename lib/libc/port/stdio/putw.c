/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/putw.c	1.11"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * The intent here is to provide a means to make the order of
 * bytes in an io-stream correspond to the order of the bytes
 * in the memory while doing the io a `word' at a time.
 */
#ifdef __STDC__
	#pragma weak putw = _putw
#endif
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>

int
putw(w, stream)
	int w;
	register FILE *stream;
{
	register char *s = (char *)&w;
	register int i = sizeof(int);

	while (--i >= 0 && putc(*s++, stream) != EOF)
		;
	return stream->_flag & _IOERR;
}
