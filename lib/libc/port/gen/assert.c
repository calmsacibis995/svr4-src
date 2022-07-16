/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/assert.c	1.4.1.7"
/*LINTLIBRARY*/
/*
 *	called from "assert" macro; prints without printf or stdio.
 */

#ifdef __STDC__
	#pragma weak _assert = __assert
#endif
#include "synonyms.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#define WRITE(s, n)	(void) write(2, (s), (n))
#define WRITESTR(s1, n, s2)	WRITE((s1), n), \
				WRITE((s2), (unsigned) strlen(s2))

void
_assert(assertion, filename, line_num)
char *assertion;
char *filename;
int line_num;
{
	static char linestr[] = ", line NNNNN\n";
	register char *p = &linestr[7];
	register int div, digit;

	WRITESTR("Assertion failed: ", 18, assertion);
	WRITESTR(", file ", 7, filename);
	for (div = 10000; div != 0; line_num %= div, div /= 10)
		if ((digit = line_num/div) != 0 || p != &linestr[7] || div == 1)
			*p++ = digit + '0';
	*p++ = '\n';
	*p = '\0';
	WRITE(linestr, (unsigned) strlen(linestr));
	(void) abort();
}
