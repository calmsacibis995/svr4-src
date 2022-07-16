/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/div.c	1.2"
/*LINTLIBRARY*/

#include "synonyms.h"
#include <stdlib.h>

div_t
div(numer, denom)
register int numer;
register int denom;
{
	div_t	sd;

	if (numer >= 0 && denom < 0) {
		numer = -numer;
		sd.quot = -(numer / denom);
		sd.rem  = -(numer % denom);
	} else if (numer < 0 && denom > 0) {
		denom = -denom;
		sd.quot = -(numer / denom);
		sd.rem  = numer % denom;
	} else {
		sd.quot = numer / denom;
		sd.rem  = numer % denom;
	}
	return(sd);
}

ldiv_t
ldiv(numer, denom)
register long int numer;
register long int denom;
{
	ldiv_t	sd;

	if (numer >= 0 && denom < 0) {
		numer = -numer;
		sd.quot = -(numer / denom);
		sd.rem  = -(numer % denom);
	} else if (numer < 0 && denom > 0) {
		denom = -denom;
		sd.quot = -(numer / denom);
		sd.rem  = numer % denom;
	} else {
		sd.quot = numer / denom;
		sd.rem  = numer % denom;
	}
	return(sd);
}
