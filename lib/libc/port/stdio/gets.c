/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/gets.c	3.13"
/*LINTLIBRARY*/
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"
#include <memory.h>
#include <errno.h>

char *
gets(buf)	/* read a single line from stdin, replace the '\n' with '\0' */
	char *buf;
{
	register char *ptr = buf;
	register int n;
	register char *p;
	register Uchar *bufend;

	if (!(stdin->_flag & (_IOREAD | _IORW))) {
		errno = EBADF;
		return 0;
	}
	if (stdin->_base == 0)
	{
		if ((bufend = _findbuf(stdin)) == 0)
			return 0;
	}
	else
		bufend = _bufend(stdin);

	for (;;)	/* until get a '\n' */
	{
		if (stdin->_cnt <= 0)	/* empty buffer */
		{
			if (_filbuf(stdin) != EOF)
			{
				stdin->_ptr--;	/* put back the character */
				stdin->_cnt++;
			}
			else if (ptr == buf)	/* never read anything */
				return 0;
			else
				break;		/* nothing left to read */
		}
		n = stdin->_cnt;
		if ((p = (char *)memccpy(ptr, (char *)stdin->_ptr, '\n', n)) != 0)
			n = p - ptr;
		ptr += n;
		stdin->_cnt -= n;
		stdin->_ptr += n;
		if (_needsync(stdin, bufend))
			_bufsync(stdin, bufend);
		if (p != 0) /* found a '\n' */
		{
			ptr--;	/* step back over the '\n' */
			break;
		}
	}
	*ptr = '\0';
	return buf;
}
