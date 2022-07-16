/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/isastream.c	1.2"
/*
 * Check to see if a file descriptor is that of a stream.
 * Return 1 with errno set to 0 if it is. Otherwise,
 * return 0 with errno set to 0. 
 * The only error returned is that case of a bad file desc.
 * 
 */
#ifdef __STDC__
	#pragma weak isastream = _isastream
#endif
#include "synonyms.h"
#include <stdio.h>
#include <errno.h>
#include <stropts.h>

int
isastream(fd)
	int fd;
{
	extern int errno;
	int rval;

	rval = ioctl(fd, I_CANPUT, 0);
	if (rval == -1 && errno == EBADF)
		return (-1);

	errno = 0;
	if (rval == 0 || rval == 1)
		return (1);

	return (0);
}
