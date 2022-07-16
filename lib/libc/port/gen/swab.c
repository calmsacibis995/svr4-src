/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/swab.c	1.10"

/*LINTLIBRARY*/
/*
 * Swab bytes
 */

#ifdef __STDC__
	#pragma weak swab = _swab
#endif

#include "synonyms.h"
#include <stdlib.h>

#define	STEP	temp = *from++,*to++ = *from++,*to++ = temp

void
swab(from, to, n)
	register const char *from;
	register char *to;
	register int n;
{
	register char temp;
	
	if (n <= 1)
		return;
	n >>= 1; n++;
	/* round to multiple of 8 */
	while ((--n) & 07)
		STEP;
	n >>= 3;
	while ((int)--n >= 0) {
		STEP; STEP; STEP; STEP;
		STEP; STEP; STEP; STEP;
	}
}
