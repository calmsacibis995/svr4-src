/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/calloc.c	1.14"
/*LINTLIBRARY*/
/*	calloc - allocate and clear memory block
*/
#define NULL 0
#include "synonyms.h"
#include "shlib.h"
#include <stdlib.h>
#include <string.h>

#undef calloc

VOID * 
calloc(num, size)
size_t num, size;
{
	register char *mp;
	unsigned long total;

	if (num == 0 || size == 0 ) 
		total = 0;
	else {
		total = (unsigned long) num * size;

		/* check for overflow */
		if (total / num != size)
		    return(NULL);
	}
	return((mp = malloc(total)) ? memset(mp, 0, total) : mp);
}
