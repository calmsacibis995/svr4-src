/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/setbuf.c	2.11"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"
#include "stdlib.h"


extern int isatty();
extern Uchar _smbuf[][_SMBFSZ];

void
setbuf(iop, abuf)
	register FILE *iop;
	char	*abuf;
{
	register Uchar *buf = (Uchar *)abuf;
	register int fno = iop->_file;  /* file number */
	register int size = BUFSIZ - _SMBFSZ;
	register Uchar *temp;

	if(iop->_base != 0 && iop->_flag & _IOMYBUF)
		free((char *)iop->_base - PUSHBACK);
	iop->_flag &= ~(_IOMYBUF | _IONBF | _IOLBF);
	if (buf == 0) 
	{
		iop->_flag |= _IONBF; 
#ifndef _STDIO_ALLOCATE
		if (fno < 2)
		{
			/* use special buffer for std{in,out} */
			buf = (fno == 0) ? _sibuf : _sobuf;
		}
		else /* needed for ifdef */
#endif
		if (fno < _NFILE)
		{
			buf = _smbuf[fno];
			size = _SMBFSZ - PUSHBACK;
                }
                else if ((buf = (Uchar *)malloc(_SMBFSZ * sizeof(Uchar))) != 0)
		{
                       	iop->_flag |= _IOMYBUF;
			size = _SMBFSZ - PUSHBACK;
		}
	}
	else /* regular buffered I/O, standard buffer size */
	{
		if (isatty(fno))
			iop->_flag |= _IOLBF;
	}
	if (buf == 0)
		return ; /* malloc() failed */
	temp = buf + PUSHBACK;
	iop->_base = temp;
	_setbufend(iop, temp + size);
	iop->_ptr = temp;
	iop->_cnt = 0;
}
