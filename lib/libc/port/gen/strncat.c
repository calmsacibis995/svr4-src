/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/strncat.c	1.9"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Concatenate s2 on the end of s1.  S1's space must be large enough.
 * At most n characters are moved.
 * Return s1.
 */

#include "synonyms.h"
#include <string.h>

char *
strncat(s1, s2, n)
register char *s1;
register const char *s2;
register size_t n;
{
	char *os1 = s1;

	n++;
	while(*s1++)
		;
	--s1;
	while(*s1++ = *s2++)
		if(--n == 0) {
			s1[-1] = '\0';
			break;
		}
	return(os1);
}
