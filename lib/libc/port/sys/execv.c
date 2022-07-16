/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:sys/execv.c	1.6.1.5"
/*
 *	execv(file, argv)
 *
 *	where argv is a vector argv[0] ... argv[x], NULL
 *	last vector element must be NULL
 *	environment passed automatically
 */
#ifdef __STDC__
	#pragma weak execv = _execv
#endif
#include "synonyms.h"

extern int execve();


execv(file, argv)
	char	*file;
	char	**argv;
{
	extern	char	**environ;

	return(execve(file, argv, environ));
}
