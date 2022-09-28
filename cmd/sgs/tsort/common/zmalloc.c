/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tsort:zmalloc.c	1.1"

/*	malloc(3C) with error checking
*/

#include <stdio.h>
#include "errmsg.h"


char *
zmalloc( severity, n)
int		severity;
unsigned	n;
{
	char	*p;

	if( (p = (char *)malloc(n)) == NULL )
		_errmsg("UXzmalloc1", severity,
			"Cannot allocate a block of %d bytes.",
			n);

	return  p;
}
