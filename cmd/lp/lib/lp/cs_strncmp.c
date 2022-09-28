/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/cs_strncmp.c	1.5.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "ctype.h"

/*
 * Compare strings (at most n bytes) ignoring case
 *	returns: s1>s2; >0  s1==s2; 0  s1<s2; <0
 */

int
#if	defined(__STDC__)
cs_strncmp(
	char *			s1,
	char *			s2,
	int			n
)
#else
cs_strncmp(s1, s2, n)
register char *s1, *s2;
register n;
#endif
{
	if(s1 == s2)
		return(0);
	while(--n >= 0 && toupper(*s1) == toupper(*s2++))
		if(*s1++ == '\0')
			return(0);
	return((n < 0)? 0: (toupper(*s1) - toupper(*--s2)));
}
