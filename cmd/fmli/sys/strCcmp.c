/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/strCcmp.c	1.1"

#include	<stdio.h>
#include	<ctype.h>

int
strCcmp(s1, s2)
char	*s1;
char	*s2;
{
	register int	c1;
	register int	c2;

	while ((c1 = *s1++) != 0 & (c2 = *s2++) != 0) {
		if (isupper(c1))
			c1 = tolower(c1);
		if (isupper(c2))
			c2 = tolower(c2);
		if (c1 != c2)
			break;
	}
	return c1 - c2;
}

int
strnCcmp(s1, s2, n)
char	*s1;
char	*s2;
int	n;
{
	register int	c1 = '\0';
	register int	c2 = '\0';

	while (n-- > 0 && (c1 = *s1++) != 0 & (c2 = *s2++) != 0) {
		if (isupper(c1))
			c1 = tolower(c1);
		if (isupper(c2))
			c2 = tolower(c2);
		if (c1 != c2)
			break;
	}
	return c1 - c2;
}
