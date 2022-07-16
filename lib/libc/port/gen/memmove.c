/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/memmove.c	1.2"
/*LINTLIBRARY*/
/*
 * Copy s0 to s, always copy n bytes.
 * Return s
 * Copying between objects that overlap will take place correctly
 */
#include "synonyms.h"
#include <string.h>

VOID * 
memmove(s, s0, n )
VOID *s;
const VOID *s0;
register size_t n;
{
	if (n != 0) {
   		register char *s1 = s;
		register const char *s2 = s0;
		if (s1 <= s2) {
			do {
				*s1++ = *s2++;
			} while (--n != 0);
		} else {
			s2 += n;
			s1 += n;
			do {
				*--s1 = *--s2;
			} while (--n != 0);
		}
	}
	return ( s );
}
