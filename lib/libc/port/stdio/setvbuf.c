/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/setvbuf.c	1.18"
/*LINTLIBRARY*/

#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"
#include "stdlib.h"

extern Uchar _smbuf[][_SMBFSZ];
extern int isatty();

int
setvbuf(iop, abuf, type, size)
	register FILE *iop;
	register int type;
	char	*abuf;
	register size_t size;
{

	register Uchar	*buf = (Uchar *)abuf;
	register Uchar *temp;
	register int	sflag = iop->_flag & _IOMYBUF;
	iop->_flag &= ~(_IOMYBUF | _IONBF | _IOLBF);
	switch (type)  
	{
	/*note that the flags are the same as the possible values for type*/
	case _IONBF:
		iop->_flag |= _IONBF;	 /* file is unbuffered */
#ifndef _STDIO_ALLOCATE
		if (iop->_file < 2)
		{
			/* use special buffer for std{in,out} */
			buf = (iop->_file == 0) ? _sibuf : _sobuf;
			size = BUFSIZ;
		}
		else /* needed for ifdef */
#endif
		if (iop->_file < _NFILE)
		{
			buf = _smbuf[iop->_file];
			size = _SMBFSZ - PUSHBACK;
		}
                else if ((buf = (Uchar *)malloc(_SMBFSZ * sizeof(Uchar))) != 0)
		{
                        iop->_flag |= _IOMYBUF;
			size = _SMBFSZ - PUSHBACK;
		}
		else 
			return EOF;
		break;
	case _IOLBF:
	case _IOFBF:
		iop->_flag |= type;	/* buffer file */
		/* need at least an 8 character buffer for out_of_sync concerns. */
		if (size <= _SMBFSZ) 
		   	buf = 0;
		if (buf == 0) 
		{
		   	size = BUFSIZ;
		   	if ((buf = (Uchar *)malloc(sizeof(Uchar) * (BUFSIZ + _SMBFSZ))) != 0)
				iop->_flag |= _IOMYBUF;
			else
                		return EOF;
		}
		else
			size -= _SMBFSZ;
		break;
	default:
		return EOF;
	}
	if(iop->_base != 0 && sflag)
		free((char *)iop->_base - PUSHBACK);
	temp = buf + PUSHBACK;
	iop->_base = temp;
        setbufend(iop, temp + size);
	iop->_ptr = temp;
	iop->_cnt = 0;
	return 0;
}
