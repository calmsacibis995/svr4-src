/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/munlockall.c	1.3"
#ifdef __STDC__
	#pragma weak munlockall = _munlockall
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/mman.h>

/*
 * Function to unlock address space from memory.
 */

/*LINTLIBRARY*/
munlockall()
{

	return (memcntl(0, 0, MC_UNLOCKAS, 0, 0, 0));
}
