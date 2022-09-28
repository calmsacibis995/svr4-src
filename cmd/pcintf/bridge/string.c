/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/string.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)string.c	3.4	LCC);	/* Modified: 10:09:29 7/14/87 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#if	!(defined(SYS5)) || defined(XENIX)
#include <ctype.h>
#endif	/* !(defined(SYS5)) || defined(XENIX) */

#ifndef	SYS5
#define NULL 0


/*
 * Concatenate s2 on the end of s1.  S1's space must be large enough.
 * Return s1.
 */

char *
strcat(s1, s2)
register char *s1, *s2;
{
	register char *os1;

	os1 = s1;
	while(*s1++)
		;
	--s1;
	while(*s1++ = *s2++)
		;
	return(os1);
}

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */

char *
strchr(sp, c)
register char *sp, c;
{
	do {
		if(*sp == c)
			return(sp);
	} while(*sp++);
	return(NULL);
}



/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

int
strcmp(s1, s2)
register char *s1, *s2;
{

	if (!s1 || !s2)			/* check for NULL ptrs. */
		return(1);
	if(s1 == s2)
		return(0);
	while(*s1 == *s2++)
		if(*s1++ == '\0')
			return(0);
	return(*s1 - *--s2);
}



/*
 * Copy string s2 to s1.  s1 must be large enough.
 * return s1
 */

char *
strcpy(s1, s2)
register char *s1, *s2;
{
	register char *os1;

	os1 = s1;
	while(*s1++ = *s2++)
		;
	return(os1);
}



/*
 * Return the number of characters in the maximum leading segment
 * of string which consists solely of characters NOT from charset.
 */

int
strcspn(string, charset)
char	*string;
register char	*charset;
{
	register char *p, *q;

	for(q=string; *q != '\0'; ++q) {
		for(p=charset; *p != '\0' && *p != *q; ++p)
			;
		if(*p != '\0')
			break;
	}
	return(q-string);
}



/*
 * Returns the number of
 * non-NULL bytes in string argument.
 */

int
strlen(s)
register char *s;
{
	register char *s0 = s + 1;

	while (*s++ != '\0')
		;
	return (s - s0);
}



/*
 * Concatenate s2 on the end of s1.  S1's space must be large enough.
 * At most n characters are moved.
 * Return s1.
 */

char *
strncat(s1, s2, n)
register char *s1, *s2;
register n;
{
	register char *os1;

	os1 = s1;
	while(*s1++)
		;
	--s1;
	while(*s1++ = *s2++)
		if(--n < 0) {
			*--s1 = '\0';
			break;
		}
	return(os1);
}



/*
 * Compare strings (at most n bytes)
 *	returns: s1>s2; >0  s1==s2; 0  s1<s2; <0
 */

int
strncmp(s1, s2, n)
register char *s1, *s2;
register n;
{
	if(s1 == s2)
		return(0);
	while(--n >= 0 && *s1 == *s2++)
		if(*s1++ == '\0')
			return(0);
	return((n < 0)? 0: (*s1 - *--s2));
}



/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */

char *
strncpy(s1, s2, n)
register char *s1, *s2;
register int n;
{
	register char *os1 = s1;

	while (--n >= 0)
		if ((*s1++ = *s2++) == '\0')
			while (--n >= 0)
				*s1++ = '\0';
	return (os1);
}



/*
 * Return ptr to first occurance of any character from `brkset'
 * in the character string `string'; NULL if none exists.
 */

char *
strpbrk(string, brkset)
register char *string, *brkset;
{
	register char *p;

	do {
		for(p=brkset; *p != '\0' && *p != *string; ++p)
			;
		if(*p != '\0')
			return(string);
	}
	while(*string++);
	return(NULL);
}




/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
*/

char *
strrchr(sp, c)
register char *sp, c;
{
	register char *r;

	r = NULL;
	do {
		if(*sp == c)
			r = sp;
	} while(*sp++);
	return(r);
}



/*
 * Return the number of characters in the maximum leading segment
 * of string which consists solely of characters from charset.
 */

int
strspn(string, charset)
char	*string;
register char	*charset;
{
	register char *p, *q;

	for(q=string; *q != '\0'; ++q) {
		for(p=charset; *p != '\0' && *p != *q; ++p)
			;
		if(*p == '\0')
			break;
	}
	return(q-string);
}



/*
 * uses strpbrk and strspn to break string into tokens on
 * sequentially subsequent calls.  returns NULL when no
 * non-separator characters remain.
 * `subsequent' calls are calls with first argument NULL.
 */

extern int strspn();
extern char *strpbrk();

char *
strtok(string, sepset)
char	*string, *sepset;
{
	register char	*p, *q, *r;
	static char	*savept;

	/*first or subsequent call*/
	p = (string == NULL)? savept: string;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + strspn(p, sepset);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = strpbrk(q, sepset)) == NULL)	/* move past token */
		savept = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		savept = ++r;
	}
	return(q);
}

#endif	/* !SYS5 */

#if	!(defined(SYS5)) || (defined(XENIX))
#define TOLOWER(c)	(isupper(c)? tolower(c): (c))

#define DIGIT(x) (isdigit(x)? ((x)-'0'): (10+TOLOWER(x)-'a'))
#define MBASE 36

long
strtol(str, ptr, base)
char *str, **ptr;
int base;
{
	long val;
	int xx, sign;

	val = 0L;
	sign = 1;
	if(base < 0 || base > MBASE)
		goto OUT;
	while(isspace(*str))
		++str;
	if(*str == '-') {
		++str;
		sign = -1;
	} else if(*str == '+')
		++str;
	if(base == 0) {
		if(*str == '0') {
			++str;
			if(*str == 'x' || *str == 'X') {
				++str;
				base = 16;
			} else
				base = 8;
		} else
			base = 10;
	} else if(base == 16)
		if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
			str += 2;
	/*
	 * for any base > 10, the digits incrementally following
	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 */
	while(isalnum(*str) && (xx=DIGIT(*str)) < base) {
		/* accumulate neg avoids surprises near maxint */
		val = base*val - xx;
		++str;
	}
OUT:
	if(ptr != (char**)0)
		*ptr = str;
	return(sign*(-val));
}

#endif	/* !(defined(SYS5)) || (defined(XENIX)) */
