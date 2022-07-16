/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/valloc.c	1.2"

#ifdef __STDC__
	#pragma weak valloc = _valloc
#endif

#include "synonyms.h"
#include <stdlib.h>
#include <unistd.h>

VOID *
valloc(size)
	size_t size;
{
	static unsigned pagesize;
	if (!pagesize)
		pagesize = sysconf(_SC_PAGESIZE);
	return memalign(pagesize, size);
}
