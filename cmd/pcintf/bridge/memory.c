/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/memory.c	1.1"
#ifndef	AIX_RT
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)memory.c	3.6	LCC);	/* Modified: 15:02:12 5/24/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/*	@(#)memccpy.c	1.1	*/
/*LINTLIBRARY*/
/*
 * Copy s2 to s1, stopping if character c is copied. Copy no more than n bytes.
 * Return a pointer to the byte after character c in the copy,
 * or NULL if c is not found in the first n bytes.
 */
char *
Memccpy(s1, s2, c, n)
register char *s1, *s2;
register int c, n;
{
	while (--n >= 0)
		if ((*s1++ = *s2++) == c)
			return (s1);
	return (0);
}
/*	@(#)memchr.c	1.1	*/
/*LINTLIBRARY*/
/*
 * Return the ptr in sp at which the character c appears;
 *   NULL if not found in n chars; don't stop at \0.
 */
char *
Memchr(sp, c, n)
register char *sp, c;
register int n;
{
	while (--n >= 0)
		if (*sp++ == c)
			return (--sp);
	return (0);
}
/*	@(#)memcmp.c	1.1	*/
/*LINTLIBRARY*/
/*
 * Compare n bytes:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */
int
Memcmp(s1, s2, n)
register char *s1, *s2;
register int n;
{
	int diff;

	if (s1 != s2)
		while (--n >= 0)
			if (diff = *s1++ - *s2++)
				return (diff);
	return (0);
}
/*	@(#)memcpy.c	1.1	*/
/*LINTLIBRARY*/
/*
 * Copy s2 to s1, always copy n bytes.
 * Return s1
 */
char *
Memcpy(s1, s2, n)
register char *s1, *s2;
register int n;
{
	register char *os1 = s1;

	while (--n >= 0)
		*s1++ = *s2++;
	return (os1);
}
/*	@(#)memset.c	1.1	*/
/*LINTLIBRARY*/
/*
 * Set an array of n chars starting at sp to the character c.
 * Return sp.
 */
char *
Memset(sp, c, n)
register char *sp, c;
register int n;
{
	register char *sp0 = sp;

	while (--n >= 0)
		*sp++ = c;
	return (sp0);
}
#endif	/* AIX_RT */
