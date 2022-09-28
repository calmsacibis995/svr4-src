#ident	"@(#)syntax.c	1.2	91/09/21	JPB"
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/syntax.c	1.5.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "ctype.h"
#include "string.h"

#include "lp.h"

int
#if	defined(__STDC__)
syn_name (
	char *			str
)
#else
syn_name (str)
	char			*str;
#endif
{
	register char		*p;

	if (!*str)
	  	return(0);

	if (strlen(str) > (size_t)14)
		return (0);

	for (p = str; *p; p++)
		if (!isalnum(*p) && *p != '_')
			return (0);

	return (1);
}

int
#if	defined(__STDC__)
syn_type (
	char *			str
)
#else
syn_type (str)
	char			*str;
#endif
{
	register char		*p;

	if (strlen(str) > (size_t)14)
		return (0);

	for (p = str; *p; p++)
		if (!isalnum(*p) && *p != '-')
			return (0);

	return (1);
}

int
#if	defined(__STDC__)
syn_text (
	char *			str
)
#else
syn_text (str)
	char			*str;
#endif
{
	register char		*p;

	for (p = str; *p; p++)
		if (!isgraph(*p) && *p != '\t' && *p != ' ')
			return (0);

	return (1);
}

int
#if	defined(__STDC__)
syn_comment (
	char *			str
)
#else
syn_comment (str)
	char			*str;
#endif
{
	register char		*p;

	for (p = str; *p; p++)
		if (!isgraph(*p) && *p != '\t' && *p != ' ' && *p != '\n')
			return (0);

	return (1);
}

int
#if	defined(__STDC__)
syn_machine_name (
	char *			str
)
#else
syn_machine_name (str)
	char			*str;
#endif
{
	if (strlen(str) > (size_t)8)
		return (0);

	return (1);
}

int
#if	defined(__STDC__)
syn_option (
	char *			str
)
#else
syn_option (str)
	char			*str;
#endif
{
	register char		*p;

	for (p = str; *p; p++)
		if (!isprint(*p))
			return (0);

	return (1);
}
