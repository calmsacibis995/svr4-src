/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:sys/execl.c	1.5.1.6"
/*
 *	execl(name, arg0, arg1, ..., argn, 0)
 *	environment automatically passed.
 */

#ifdef __STDC__
	#pragma weak execl = _execl
#endif
#include "synonyms.h"
#include <stdarg.h>

extern int execve();

int
#ifdef __STDC__
execl(char *name, ...)
#else
execl(name, va_alist) char *name; va_dcl
#endif
{
	va_list args;
	extern char **environ;

#ifdef __STDC__
	va_start(args,);
#else
	va_start(args);
#endif
	return (execve(name, args, environ));
}
