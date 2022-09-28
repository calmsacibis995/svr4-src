/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/strsave.c	1.3"

#include	<stdio.h>
#include	<string.h>
#include	"wish.h"

/* les: changing to MACRO
char *
strsave(s)
char	s[];
{
	char	*strnsave();

	return s ? strnsave(s, strlen(s)) : NULL;
}
*/

char	*
strnsave(s, len)
char	s[];
unsigned int	len;
{
	register char	*p;

	if ((p = malloc(len + 1)) == NULL)
		return NULL;
	strncpy(p, s, len);
	p[len] = '\0';
	return p;
}
