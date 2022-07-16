/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:sys/execle.c	1.6.1.5"
/*
 *	execle(file, arg1, arg2, ..., 0, envp)
 */
#ifdef __STDC__
	#pragma weak execle = _execle
#endif
#include "synonyms.h"
#include <stdarg.h>

extern int execve();

int
#ifdef __STDC__
execle(char *file, ...)
#else
execle(file, va_alist) char *file; va_dcl
#endif
{
	register  char  *p;
	va_list args, sargs;

#ifdef __STDC__
	va_start(args,);
#else
	va_start(args);
#endif
	sargs = args;
	while ((p = va_arg(args, char *)) != 0) ;
	p = va_arg(args, char *);
	return(execve(file, sargs, p));
}
