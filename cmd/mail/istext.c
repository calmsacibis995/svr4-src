/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:istext.c	1.3.3.1"
#include "mail.h"

/*
 * istext(line, size) - check for text characters
 */
int
istext(s, size)
register unsigned char	*s;
int 	size;
{
	register unsigned char *ep;
	register c;
	
	for (ep = s+size; --ep >= s; ) {
		c = *ep;
		if ((!isprint(c)) && (!isspace(c)) &&
		    /* Since backspace is not included in either of the */
		    /* above, must do separately                        */
		    (c != 010)) {
			return(FALSE);
		}
	}
	return(TRUE);
}
