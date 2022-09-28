/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:string.c	1.8.4.1"

/*
 * UNIX shell
 */

#include	"defs.h"


/* ========	general purpose string handling ======== */


unsigned char *
movstr(a, b)
register unsigned char	*a, *b;
{
	while (*b++ = *a++);
	return(--b);
}

any(c, s)
register unsigned char	c;
unsigned char	*s;
{
	register unsigned char d;

	while (d = *s++)
	{
		if (d == c)
			return(TRUE);
	}
	return(FALSE);
}

int anys(c, s)
unsigned char *c, *s;
{
	wchar_t f, e;
	register wchar_t d;
	register int n;
	if((n = mbtowc(&f, (char *)c, MULTI_BYTE_MAX)) <= 0)
		return(FALSE);
	d = f;
	while(1) {
		if((n = mbtowc(&e, (char *)s, MULTI_BYTE_MAX)) <= 0)
			return(FALSE);
		if(d == e)
			return(TRUE);
		s += n;
	}
}

int cf(s1, s2)
register unsigned char *s1, *s2;
{
	while (*s1++ == *s2)
		if (*s2++ == 0)
			return(0);
	return(*--s1 - *s2);
}

int length(as)
unsigned char	*as;
{
	register unsigned char	*s;

	if (s = as)
		while (*s++);
	return(s - as);
}

unsigned char *
movstrn(a, b, n)
	register unsigned char *a, *b;
	register int n;
{
	while ((n-- > 0) && *a)
		*b++ = *a++;

	return(b);
}
