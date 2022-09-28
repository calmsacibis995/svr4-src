/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/setbuffer.c	1.1.2.1"
/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */
#include "shlib.h"
#include <stdio.h>
#include "stdiom.h"
#include "stdlib.h"

extern void free();
extern Uchar _smbuf[][_NFILE];

void
setbuffer(iop, abuf, asize)
	register FILE *iop;
	char	*abuf;
	int	asize;
{
	register Uchar *buf = (Uchar *)abuf;
	register int fno = iop->_file;  /* file number */
	register int size = asize - _SMBFSZ;
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
			size = BUFSIZ - _SMBFSZ;
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
	else /* regular buffered I/O, specified buffer size */
	{
		if (size <= 0)
			return;
	}
	if (buf == 0)
		return ; /* malloc() failed */
	temp = buf + PUSHBACK;
	iop->_base = temp;
	_setbufend(iop, temp + size);
	iop->_ptr = temp;
	iop->_cnt = 0;
}

/*
 * set line buffering
 */
setlinebuf(iop)
	register FILE *iop;
{
	register unsigned char *buf;

	fflush(iop);
	setbuffer(iop, (unsigned char *)NULL, 0);
	buf = (unsigned char *)malloc(128);
	if (buf != NULL) {
		setbuffer(iop, buf, 128);
		iop->_flag |= _IOLBF|_IOMYBUF;
	}
}

