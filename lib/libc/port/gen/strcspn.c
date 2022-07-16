/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/strcspn.c	1.6"
/*LINTLIBRARY*/
#include "synonyms.h"
#include <string.h>
/*
 * Return the number of characters in the maximum leading segment
 * of string which consists solely of characters NOT from charset.
 */
size_t
strcspn(string, charset)
const char *string;
const char *charset;
{
	register const char *p, *q;

	for(q=string; *q != '\0'; ++q) {
		for(p=charset; *p != '\0' && *p != *q; ++p)
			;
		if(*p != '\0')
			break;
	}
	return(q-string);
}
