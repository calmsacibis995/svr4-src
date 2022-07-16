/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/mlockall.c	1.3"
#ifdef __STDC__
	#pragma weak mlockall = _mlockall
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/mman.h>

/*
 * Function to lock address space in memory.
 */

/*LINTLIBRARY*/
mlockall(flags)
	int flags;
{

	return (memcntl(0, 0, MC_LOCKAS, flags, 0, 0));
}
