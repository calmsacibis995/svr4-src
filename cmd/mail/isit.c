/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:isit.c	1.3.3.1"
#include "mail.h"

/*
 * isit(lp, type) -- case independent match of "name" portion of 
 *		"name: value" pair
 *	lp	->	pointer to line to check
 *	type	->	type of header line to match
 * returns
 *	TRUE	-> 	lp matches header type (case independent)
 *	FALSE	->	no match
 */
int
isit(lp, type)
register char 	*lp;
register int	type;
{
	register char	*p;

	for (p = header[type].tag; *lp && *p; lp++, p++) {
		if (toupper(*p) != toupper(*lp))  {
			return(FALSE);
		}
	}
	if (*p == NULL) {
		return(TRUE);
	}
	return(FALSE);
}
