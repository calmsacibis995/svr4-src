/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/fdopen.c	1.16"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Unix routine to do an "fopen" on file descriptor
 * The mode has to be repeated because you can't query its
 * status
 */
#ifdef __STDC__
	#pragma weak fdopen = _fdopen
#endif
#include "synonyms.h"
#include <stdio.h>
#include <limits.h>
#include "stdiom.h"

extern long lseek(	/* int fd, long offset, int whence */		);

FILE *
fdopen(fd, type)	/* associate file desc. with stream */
	int fd;
	const char *type;
{
	register FILE *iop;
	register int plus;
	register unsigned char flag;

	if (fd > UCHAR_MAX || (iop = _findiop()) == 0)
		return 0;
	iop->_file = (Uchar)fd;
	switch (type[0])
	{
	default:
		return 0;
	case 'r':
		flag = _IOREAD;
		break;
	case 'a':
		(void)lseek(fd, 0L, 2);
		/*FALLTHROUGH*/
	case 'w':
		flag = _IOWRT;
		break;
	}
	if ((plus = type[1]) == 'b')	/* Unix ignores 'b' ANSI std */
		plus = type[2];
	if (plus == '+')
		flag = _IORW;
	iop->_flag = flag;
	return iop;
}
