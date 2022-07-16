/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/fgets.c	3.15"
/*LINTLIBRARY*/

#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"
#include <memory.h>
#include <errno.h>

char *
fgets(buf, size, iop)	/* read size-max line from stream, including '\n' */
	char *buf;
	register int size;
	register FILE *iop;
{
	register char *ptr = buf;
	register int n;
	register Uchar *bufend;
	register char *p;

	if (!(iop->_flag & (_IOREAD | _IORW))) {
		errno = EBADF;
		return 0;
	}

	if (iop->_base == 0)
	{
		if ((bufend = _findbuf(iop)) == 0)
			return 0;
	}
	else
		bufend = _bufend(iop);

	size--;		/* room for '\0' */
	while (size > 0)
	{
		if (iop->_cnt <= 0)	/* empty buffer */
		{
			if (_filbuf(iop) != EOF)
			{
				iop->_ptr--;	/* put back the character */
				iop->_cnt++;
			}
			else if (ptr == buf)	/* never read anything */
				return 0;
			else
				break;		/* nothing left to read */
		}
		n = size < iop->_cnt ? size : iop->_cnt;
		if ((p = (char *)memccpy(ptr, (char *)iop->_ptr, '\n', n)) != 0)
			n = p - ptr;
		ptr += n;
		iop->_cnt -= n;
		iop->_ptr += n;
		if (_needsync(iop, bufend))
			_bufsync(iop, bufend);
		if (p != 0)
			break; /* newline found */
		size -= n;
	}
	*ptr = '\0';
	return buf;
}
