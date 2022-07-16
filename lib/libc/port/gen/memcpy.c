/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/memcpy.c	1.3.1.5"
/*LINTLIBRARY*/
/*
 * Copy s0 to s, always copy n bytes.
 * Return s
 */
#include "synonyms.h"
#include <stddef.h>
#include <string.h>

VOID * 
memcpy(s, s0, n)
VOID *s;
const VOID *s0;
register size_t n;
{
	if (n != 0) {
   	    register char *s1 = s;
	    register const char *s2 = s0;
	    do {
		*s1++ = *s2++;
	    } while (--n != 0);
	}
	return ( s );
}
