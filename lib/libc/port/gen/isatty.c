/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/isatty.c	1.11"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Returns 1 iff file is a tty
 */
#ifndef ABI
#ifdef __STDC__
	#pragma weak isatty = _isatty
#endif
#endif
#include "synonyms.h"
#include "shlib.h"
#include <sys/termio.h>

extern int ioctl();
extern int errno;

int
isatty(f)
int	f;
{
	struct termio tty;
	int err ;

	err = errno;
	if(ioctl(f, TCGETA, &tty) < 0)
	{
		errno = err; 
		return(0);
	}
	return(1);
}
