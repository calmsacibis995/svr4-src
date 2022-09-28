/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:skipspace.c	1.5.3.1"
#include <ctype.h>

/*
 * Return pointer to first non-blank character in p
 */
char *
skipspace(p)
register char	*p;
{
	while(*p && isspace(*p)) {
		p++;
	}
	return (p);
}
