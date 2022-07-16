/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/puts.c	3.13"
/*LINTLIBRARY*/
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"
#include <memory.h>

int
puts(ptr)
	register const char *ptr;
{
	register char *p;
	register int ndone = 0, n;
	register unsigned char *cptr, *bufend;

	if (_WRTCHK(stdout))
		return EOF;

	bufend = _bufend(stdout);

	for ( ; ; ptr += n)
	{
		while ((n = bufend - (cptr = stdout->_ptr)) <= 0) /* full buf */
		{
			if (_xflsbuf(stdout) == EOF)
				return EOF;
		}
		if ((p = memccpy((char *) cptr, ptr, '\0', n)) != 0)
			n = p - (char *) cptr;
		stdout->_cnt -= n;
		stdout->_ptr += n;
		if (_needsync(stdout, bufend))
			_bufsync(stdout, bufend);
		ndone += n;
		if (p != 0) 
		{
			stdout->_ptr[-1] = '\n'; /* overwrite '\0' with '\n' */
			if (stdout->_flag & (_IONBF | _IOLBF)) /* flush line */
			{
				if (_xflsbuf(stdout) == EOF)
					return EOF;
			}
			return ndone;
		}
	}
}
