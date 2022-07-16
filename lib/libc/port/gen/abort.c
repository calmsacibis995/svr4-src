/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/abort.c	1.17"
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/
/*
 *	abort() - terminate current process with dump via SIGABRT
 */

#include "synonyms.h"
#include <signal.h>
#include <stdlib.h>

extern void _exithandle();
static pass = 0;		/* counts how many times abort has been called*/

void
abort()
{
	void (*sig)();

	if ((sig = signal(SIGABRT,SIG_DFL)) != SIG_DFL) 
		(void) signal(SIGABRT,sig); 
	else if (++pass == 1)
		_exithandle();
	kill(getpid(), SIGABRT);
	_exit(EXIT_FAILURE);
}
