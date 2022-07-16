/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/strncpy.c	1.11"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */

#include "synonyms.h"
#include <string.h>

char *
strncpy(s1, s2, n)
register char *s1;
register const char *s2;
register size_t n;
{
	register char *os1 = s1;

	n++;				
	while ((--n != 0) &&  ((*s1++ = *s2++) != '\0'))
		;
	if (n != 0)
		while (--n != 0)
			*s1++ = '\0';
	return (os1);
}
