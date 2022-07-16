/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/strtol.c	2.15"
/*LINTLIBRARY*/
#include "synonyms.h"
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#define DIGIT(x)	(isdigit(x) ? (x) - '0' : \
			islower(x) ? (x) + 10 - 'a' : (x) + 10 - 'A')
#define MBASE	('z' - 'a' + 1 + 10)

/* The following macro is a local version of isalnum() which limits
 * alphabetic characters to the ranges a-z and A-Z; locale dependent
 * characters will not return 1. The members of a-z and A-Z are 
 * assumed to be in ascending order and contiguous
 */
#define lisalnum(x)	(isdigit(x) || \
			 ((x)>='a' && (x)<='z') || ((x)>='A' && (x)<='Z'))

long
strtol(str, nptr, base)
register const char *str;
char **nptr;
register int base;
{
	register long val;
	register int c;
	int xx, neg = 0;
	long	multmin;
	long	limit;
	const char **ptr = (const char **)nptr;

	if (ptr != (const char **)0)
		*ptr = str; /* in case no number is formed */
	if (base < 0 || base > MBASE || base == 1) {
		errno = EINVAL;
		return (0); /* base is invalid -- should be a fatal error */
	}
	if (!isalnum(c = *str)) {
		while (isspace(c))
			c = *++str;
		switch (c) {
		case '-':
			neg++;
			/* FALLTHROUGH */
		case '+':
			c = *++str;
		}
	}
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
	if (!lisalnum(c) || (xx = DIGIT(c)) >= base)
		return (0); /* no number formed */
	if (base == 16 && c == '0' && (str[1] == 'x' || str[1] == 'X') &&
		isxdigit(str[2]))
		c = *(str += 2); /* skip over leading "0x" or "0X" */

	/* this code assumes that abs(LONG_MIN) >= abs(LONG_MAX) */
	if (neg)
		limit = LONG_MIN;
	else
		limit = -LONG_MAX;
	multmin = limit / base;
	val = -DIGIT(c);
	for (c = *++str; lisalnum(c) && (xx = DIGIT(c)) < base; ) {
		/* accumulate neg avoids surprises near MAXLONG */
		if (val < multmin)
			goto overflow;
		val *= base;
		if (val < limit + xx)
			goto overflow;
		val -= xx;
		c = *++str;
	}
	if (ptr != (const char **)0)
		*ptr = str;
	return (neg ? val : -val);

overflow:
	for (c = *++str; lisalnum(c) && (xx = DIGIT(c)) < base; (c = *++str));
	if (ptr != (const char **)0)
		*ptr = str;
	errno = ERANGE;
	return(neg ? LONG_MIN : LONG_MAX);
}
