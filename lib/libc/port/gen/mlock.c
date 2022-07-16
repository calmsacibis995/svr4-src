/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/mlock.c	1.3"
#ifdef __STDC__
	#pragma weak mlock = _mlock
#endif
#include "synonyms.h"
#include <sys/types.h>
#include <sys/mman.h>

/*
 * Function to lock address range in memory.
 */

/*LINTLIBRARY*/
mlock(addr, len)
	caddr_t addr;
	size_t len;
{

	return (memcntl(addr, len, MC_LOCK, 0, 0, 0));
}
