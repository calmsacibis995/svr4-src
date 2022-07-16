/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/memcmp.c	1.3.1.2"
/*LINTLIBRARY*/
/*
 * Compare n bytes:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

#include "synonyms.h"
#include <string.h>
#include <stddef.h>

int
memcmp(s1, s2, n)
const VOID *s1;
const VOID *s2;
register size_t n;
{
	if (s1 != s2 && n != 0) {
		register const char *ps1 = s1;
		register const char *ps2 = s2;
		do {
			if (*ps1++ != *ps2++)
				return(ps1[-1] - ps2[-1]);
		} while (--n != 0);
	}
	return (NULL);
}
