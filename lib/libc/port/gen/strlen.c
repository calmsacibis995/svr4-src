/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/strlen.c	1.7"
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * Returns the number of
 * non-NULL bytes in string argument.
 */

#include "synonyms.h"
#include <string.h>

size_t
strlen(s)
register const char *s;
{
	register const char *s0 = s + 1;

	while (*s++ != '\0')
		;
	return (s - s0);
}
