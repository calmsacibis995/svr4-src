/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/fwrite.c	3.24"
/*LINTLIBRARY*/
 
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"
#include <values.h>
#include <memory.h>

extern int write();

#ifdef __STDC__
size_t
#else
int
#endif
fwrite(ptr1, size, count, iop)
	const VOID *ptr1;
	size_t size, count;
	register FILE *iop;
{
	register unsigned long nleft;	/* total number of bytes to write */
	register unsigned long n;	/* number of bytes transfered this time */
	register Uchar *cptr;
	register size_t ndone = 0;	/* number of size bytes done */
	register Uchar *bufend;		/* saved end-of-buffer location */
	unsigned long leftover = 0;	/* bytes left after last whole size */
	size_t orig_count = count;
	const char *ptr = ptr1;

	/* check if size or count is zero;
	 * check if iop is writeable & call _findbuf if needed */
	if (size == 0 || count == 0 || _WRTCHK(iop)) 
		return (0);
	bufend = _bufend(iop);
	/* Only loops if (count * size) overflows.  */
	for (;;) 
	{
		nleft = (unsigned long)count * size;	/* may overflow */
		if (nleft < count || nleft < size)
		{
			if ((nleft = size) == 0)  /* overflow: use a safe amount */
				return 0;
			count--;		/* one-at-a-time */
		}
		else
			count = 0;		/* going for the whole thing */
		do
		{
			cptr = iop->_ptr; /* for memcpy() call */
			/* Empty a full buffer.  */
			if (bufend <= cptr && _xflsbuf(iop) == EOF)
				return ndone;  /* ignore any partial part of a size */
			/*
			* Choose whether to use a direct write(2) call
			* or to just copy into the associated buffer.
			* If there is nothing in the buffer and we are
			* either unbuffered or we are writing "enough"
			* bytes.
			*/
			if (iop->_base >= cptr
				&& ((iop->_flag & _IONBF) || (nleft >= BUFSIZ)))
			{
				register int res;
				int tmp;
				/* only want to write in BUFSIZ chunks - performance*/
				if (nleft > MAXVAL )
					n = MAXVAL;
				else if (iop->_flag & _IONBF)
					n = nleft;
				else
					n = MULTIBFSZ(nleft);

				tmp = n;
				while ((res = write(fileno(iop), ptr, tmp)) != tmp)
				{
					if (res <= 0)
					{
						iop->_flag |= _IOERR;
						ndone += ((n - tmp) + leftover) / size;
						/* ignore any partial part of a size left */ 
						return ndone;
					}
					tmp -= res;
					ptr += res;
				}
			}
			else	/* copy into local buffer */
			{
				if ((n = bufend - cptr) > nleft)
					n = nleft;
				(void)memcpy((char *)cptr, ptr, (unsigned) n);
				iop->_cnt -= n;
				iop->_ptr += n;
				if (_needsync(iop, bufend))	
					_bufsync(iop, bufend);
			}
			if (((nleft -= n) <= 0) && count == 0)
			{
				/*
				 * If we are not buffered or line  buffered with 
				 *  a '\n', flush the rest of the buffer. 
				*/
				if ((iop->_flag & _IONBF) || ((iop->_flag & _IOLBF)
				 && memchr (cptr, '\n', (iop->_ptr - cptr)) != 0))
					(void)_xflsbuf(iop);
				return orig_count;
			}
			ptr += n;
			n += leftover;
			ndone += n / size;
			leftover = n % size;
		} while (nleft != 0);
	}
}
