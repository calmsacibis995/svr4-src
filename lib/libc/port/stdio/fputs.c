/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/fputs.c	3.19"
/*LINTLIBRARY*/
/*
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"
#include <string.h>

int
fputs(ptr, iop)
const char *ptr;
register FILE *iop;
{
	register int ndone = 0, n;
	register unsigned char *cptr, *bufend;
	char *p;

	if (_WRTCHK(iop))
		return EOF;
	bufend = _bufend(iop);

	if ((iop->_flag & _IONBF) == 0)  
	{
		for ( ; ; ptr += n) 
		{
			while ((n = bufend - (cptr = iop->_ptr)) <= 0)  
			{
				/* full buf */
				if (_xflsbuf(iop) == EOF)
					return(EOF);
			}
			if ((p = memccpy((char *) cptr, ptr, '\0', n)) != 0)
				n = (p - (char *) cptr) - 1;
			iop->_cnt -= n;
			iop->_ptr += n;
			if (_needsync(iop, bufend))
				_bufsync(iop, bufend);
			ndone += n;
			if (p != 0)  
			{ 
				/* done; flush buffer if line-buffered */
	       			if (iop->_flag & _IOLBF)
	       				if (_xflsbuf(iop) == EOF)
	       					return EOF;
	       			return ndone;
	       		}
		}
	}
	else  
	{
		/* write out to an unbuffered file */
                unsigned int cnt = strlen(ptr);
                register int num_wrote;
                int count = (int)cnt;

                while((num_wrote = write(iop->_file, ptr,
                        (unsigned)count)) != count) {
                                if(num_wrote <= 0) {
                                        iop->_flag |= _IOERR;
                                        return EOF;
                                }
                                count -= num_wrote;
                                ptr += num_wrote;
                }
                return cnt;
	}
}
