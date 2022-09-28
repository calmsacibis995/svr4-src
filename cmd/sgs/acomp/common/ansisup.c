/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/ansisup.c	1.2"
/* ansisup.c */

/* Provide surrogate code for ANSI C library routines on
** systems with no ANSI C environment.
*/

#ifndef __STDC__

#include "p1.h"
#include <errno.h>
#include <ctype.h>

#define DIGIT(x)	(isdigit(x) ? (x) - '0' : \
			islower(x) ? (x) + 10 - 'a' : (x) + 10 - 'A')
#define MBASE	('z' - 'a' + 1 + 10)

unsigned long
strtoul(str, nptr, base)
register char *str;
char **nptr;
register int base;
/* Convert string to unsigned long.  This routine is the same as the libc
** version without the use of ANSI C functionality.  This routine is only
** included in cross-compiler versions.  Natives will use the library routine.
*/
{
	register unsigned long val;
	register int c;
	int xx;
	unsigned long	multmax;
	char 	**ptr = nptr;

	if (ptr != (char **)0)
		*ptr = str; /* in case no number is formed */
	if (base < 0 || base > MBASE)
		return (0); /* base is invalid -- should be a fatal error */
	c = *str;
	while (isspace(c))
		c = *++str;
	if (base == 0)
		if (c != '0')
			base = 10;
		else if (str[1] == 'x' || str[1] == 'X')
			base = 16;
		else
			base = 8;
	/*
	 * for any base > 10, the digits incrementally following
	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 */
	if (!isalnum(c) || (xx = DIGIT(c)) >= base)
		return (0); /* no number formed */
	if (base == 16 && c == '0' && (str[1] == 'x' || str[1] == 'X') &&
		isxdigit(str[2]))
		c = *(str += 2); /* skip over leading "0x" or "0X" */

	multmax = T_ULONG_MAX / (unsigned)base;
	for (val = DIGIT(c); isalnum(c = *++str) && (xx = DIGIT(c)) < base; ) {
		if (val > multmax)
			goto overflow;
		val *= base;
		if (T_ULONG_MAX - val < xx)
			goto overflow;
		val += xx;
	}
	if (ptr != (char **)0)
		*ptr = str;
	return (val);

overflow:
	if (ptr != (char **)0)
		*ptr = str;
	errno = ERANGE;
	return(T_ULONG_MAX);
}

/*ARGSUSED*/
char *
setlocale(cat, locale)
int cat;
const char * locale;
/* Stub for ANSI C function:  nop.  Always return a pointer
** to the null string.
*/
{
    return "";
}


int
mbtowc(pwc, s, n)
wchar_t *pwc;
char *s;
unsigned n;
/* Convert a multibyte character to a wide character.  Since
** the host machine evidently doesn't support such behavior,
** assume all multibyte characters have length 1.
*/
{
    if (n == 0)
	return( -1 );
    if (pwc)
	*pwc = (unsigned char) *s;
    return( *s != 0 );
}

#endif
