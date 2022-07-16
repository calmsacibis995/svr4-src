/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/toupper.c	1.7"
/*
 * If arg is lower-case, return upper-case, otherwise return arg.
 * International version
 */
#include "synonyms.h"
#include <ctype.h>

int
toupper(c)
register int c;
{
	if (islower(c))
		c = _toupper(c); 
	return(c);
}
