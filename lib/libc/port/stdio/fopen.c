/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/fopen.c	1.20"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <stdio.h>
#include "stdiom.h"
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

static FILE *
_endopen(name, type, iop)	/* open UNIX file name, associate with iop */
	const char *name;
	const char *type;
	register FILE *iop;
{
	register int oflag, plus, fd;

	if (iop == 0)
		return 0;
	switch (type[0])
	{
	default:
		return 0;
	case 'r':
		oflag = O_RDONLY;
		break;
	case 'w':
		oflag = O_WRONLY | O_TRUNC | O_CREAT;
		break;
	case 'a':
		oflag = O_WRONLY | O_APPEND | O_CREAT;
		break;
	}
	/* UNIX ignores 'b' and treats text and binary the same */
	if ((plus = type[1]) == 'b')
		plus = type[2];
	if (plus == '+')
		oflag = (oflag & ~(O_RDONLY | O_WRONLY)) | O_RDWR;
	if ((fd = open(name, oflag, 0666)) < 0)
		return 0;
	if (fd > UCHAR_MAX) {
		(void)close(fd);
		return 0;
	}
	iop->_file = (char)fd;	/* assume that fd fits in unsigned char */
	if (plus == '+')
		iop->_flag = _IORW;
	else if (type[0] == 'r')
		iop->_flag = _IOREAD;
	else
		iop->_flag = _IOWRT;
	if (oflag == (O_WRONLY | O_APPEND | O_CREAT))	/* type == "a" */
		if (lseek(fd, 0L, 2) < 0L)
			return NULL;
	return iop;	
}

FILE *
fopen(name, type)		/* open name, return new stream */
	const char *name;
	const char *type;
{
	return _endopen(name, type, _findiop());
}

FILE *
freopen(name, type, iop)	/* open name, associate with existing stream */
	const char *name;
	const char *type;
	FILE *iop;
{
	(void)fclose(iop);
	return _endopen(name, type, iop);
}
