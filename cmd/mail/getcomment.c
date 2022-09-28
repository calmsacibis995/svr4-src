/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:getcomment.c	1.4.3.1"
#include "mail.h"
/*
 * Get comment field, if any, from line.
 *	1 ==> found comment.
 *	0 ==> no comment found.
 *     -1 ==> no closing (terminating) paren found for comment.
 */

getcomment(s, q)
register char	*s;
register char	*q;	/* Copy comment, if found, to here */
{
	register char	*p, *sav_q;
	register int	depth = 0;
	
	if ((p = strchr(s, '(')) == (char *)NULL) {
		/* no comment found */
		return (0);
	}
	sav_q = q;
	while (*p) {
		*q++ = *p;
		if (*p == ')') {
			/* account for nested parens within comment */
			depth--;
			if (depth == 0) {
				break;
			}
		} else if (*p == '(') {
			depth++;
		}
		p++;
	}
	*q = '\0';
	if (*p != ')') {
		/* closing paren not found */
		*sav_q = '\0';
		return (-1);
	}
	/* found comment */
	return (1);
}
