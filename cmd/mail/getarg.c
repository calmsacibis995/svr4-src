/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:getarg.c	1.3.3.1"
#include "mail.h"
/*
	get next token
		p	-> string to be searched
		s	-> area to return token
	returns:
		p	-> updated string pointer
		s	-> token
		NULL	-> no token
*/
char *
getarg(s, p)	
register char *s, *p;
{
	while (*p == ' ' || *p == '\t') p++;
	if (*p == '\n' || *p == '\0') return(NULL);
	while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0') *s++ = *p++;
	*s = '\0';
	return(p);
}
