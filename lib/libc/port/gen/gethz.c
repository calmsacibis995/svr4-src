/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/gethz.c	1.4"
/*LINTLIBRARY*/

#ifndef DSHLIB
#ifdef __STDC__
	#pragma weak gethz = _gethz
#endif
#endif
#include "synonyms.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

/***	gethz -- get the value of the clock hertz from the environment.
 *
 *	return the clock hertz value if the string "HZ" in the environment is:
 *		1) Composed entirely of numbers.
 *		2) Not equal to zero.
 *	Otherwise 0 is returned.
 */


gethz()
{
	register char *sptr, *cptr;

	if ((sptr = getenv("HZ")) == 0)
		return(0);
	else {
		cptr = sptr;

		/* Check that all characters are numeric */
		while (*cptr) {
			if (!isdigit(*cptr))
				return(0);
			cptr++;
		}
		return(atoi(sptr));

	}
}
