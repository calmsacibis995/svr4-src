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

#ident	"@(#)fmli:sys/backslash.c	1.1"

#include	<stdio.h>
#include	<ctype.h>
#include	"wish.h"

char	*strchr();

static char	withbs[] = "\b\f\n\r\t\\\33";
static char	woutbs[] = "bfnrt\\E";

char *
backslash(s, n)
char	*s;
int	n;
{
	char	*_backslash();

	return _backslash(s, n, withbs, woutbs);
}

char *
_backslash(s, n, in, out)
char	*s;
int	n;
char	*in;
char	*out;
{
	register char	*dst;
	register char	*p;

	n -= strlen(s);
	for (dst = s; *dst; dst++) {
		if (!isprint(*dst)) {
			if ((p = strchr(in, *dst)) && n > 0) {
				*dst++ = '\\';
				memshift(dst + 1, dst, strlen(dst) + 1);
				*dst = out[p - in];
				n--;
			}
			else {
				register int	c;

				memshift(dst + 3, dst, strlen(dst) + 1);
				c = *dst;
				*dst++ = '\\';
				*dst++ = ((c >> 6) & 3) + '0';
				*dst++ = ((c >> 3) & 7) + '0';
				*dst = (c & 7) + '0';
			}
		}
	}
	return s;
}

char *
unbackslash(s)
char	*s;
{
	register char	*src;
	register char	*dst;
	register char	*p;

	for (dst = src = s; *src; src++) {
		if (*src == '\\') {
			if (p = strchr(woutbs, src[1])) {
				*dst++ = withbs[p - woutbs];
				src++;
			}
			else if (isdigit(src[1])) {
				register int	c;

				c = *++src - '0';
				if (isdigit(src[1])) {
					c = (c << 3) + *++src - '0';
					if (isdigit(src[1]))
						c = (c << 3) + *++src - '0';
				}
				*dst++ = c;
			}
		}
		else
			*dst++ = *src;
	}
	*dst = '\0';
	return s;
}
