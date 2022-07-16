/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/bsearch.c	1.11"
/*LINTLIBRARY*/
/*
 * Binary search algorithm, generalized from Knuth (6.2.1) Algorithm B.
 *
 */

#include "synonyms.h"
#include <stddef.h>

VOID *
bsearch(ky, bs, nel, width, compar)
const VOID *ky;		/* Key to be located */
const VOID *bs;		/* Beginning of table */
size_t nel;		/* Number of elements in the table */
size_t width;		/* Width of an element (bytes) */
int (*compar)();	/* Comparison function */
{
	typedef char *POINTER;
	POINTER key = (char *)ky;
	POINTER base = (char *)bs;
	int two_width = width + width;
	POINTER last = base + width * (nel - 1); /* Last element in table */

	while (last >= base) {

		register POINTER p = base + width * ((last - base)/two_width);
		register int res = (*compar)(key, p);

		if (res == 0)
			return (p);	/* Key found */
		if (res < 0)
			last = p - width;
		else
			base = p + width;
	}
	return ((POINTER) 0);		/* Key not found */
}
