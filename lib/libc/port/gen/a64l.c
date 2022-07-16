/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/a64l.c	1.13"
/*	3.0 SID #	1.3	*/
/*LINTLIBRARY*/
/*
 * convert base 64 ascii to long int
 * char set is [./0-9A-Za-z]
 *
 */
#ifdef __STDC__
	#pragma weak a64l = _a64l
#endif
#include "synonyms.h"

#define BITSPERCHAR	6 /* to hold entire character set */

long
a64l(s)
register char *s;
{
	register int i, c;
	long lg = 0;

	for (i = 0; (c = *s++) != '\0'; i += BITSPERCHAR) {
		if (c > 'Z')
			c -= 'a' - 'Z' - 1;
		if (c > '9')
			c -= 'A' - '9' - 1;
		lg |= (long)(c - ('0' - 2)) << i;
	}
	return (lg);
}
