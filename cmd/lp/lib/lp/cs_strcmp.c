/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/cs_strcmp.c	1.5.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "ctype.h"

/*
 * Compare strings ignoring case:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

int
#if	defined(__STDC__)
cs_strcmp(
	char *			s1,
	char *			s2
)
#else
cs_strcmp(s1, s2)
register char *s1, *s2;
#endif
{

	if(s1 == s2)
		return(0);
	while(toupper(*s1) == toupper(*s2++))
		if(*s1++ == '\0')
			return(0);
	return(toupper(*s1) - toupper(*--s2));
}
