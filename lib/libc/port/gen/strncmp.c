/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/strncmp.c	1.11"
/*LINTLIBRARY*/
/*
 * Compare strings (at most n bytes)
 *	returns: s1>s2; >0  s1==s2; 0  s1<s2; <0
 */

#include "synonyms.h"
#include <string.h>

int
strncmp(s1, s2, n)
register const char *s1, *s2;
register size_t n;
{
	n++;
	if(s1 == s2)
		return(0);
	while(--n != 0 && *s1 == *s2++)
		if(*s1++ == '\0')
			return(0);
	return((n == 0)? 0: (*s1 - s2[-1]));
}
