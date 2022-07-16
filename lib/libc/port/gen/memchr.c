/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/memchr.c	1.3.1.7"
/*LINTLIBRARY*/
/*
 * Return the ptr in sptr at which the character c1 appears;
 *   NULL if not found in n chars; don't stop at \0.
 */
#include "synonyms.h"
#include <stddef.h>
#include <string.h>

VOID * 
memchr(sptr, c1, n)
const VOID *sptr;
int c1;
register size_t n;
{
	if (n != 0) {
	    register unsigned char c = (unsigned char)c1;
	    register const char *sp = sptr;
	    do {
		if (*sp++ == c)
			return ((VOID *)--sp);
	    } while (--n != 0);
	}
	return (NULL);
}
