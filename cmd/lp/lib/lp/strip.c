/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/strip.c	1.4.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"

/**
 ** strip() - STRIP LEADING AND TRAILING BLANKS
 **/

char *
#if	defined(__STDC__)
strip (
	char *			str
)
#else
strip (str)
	register char		*str;
#endif
{
	register char		*p;

	if (!str || !*str)
		return (0);

	str += strspn(str, " ");
	for (p = str; *p; p++)
		;
	p--;
	for (; p >= str && *p == ' '; p--)
		;
	*++p = 0;
	return (str);
}
