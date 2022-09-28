/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/wherelist.c	1.4.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"

#include "lp.h"

/**
 ** wherelist() - RETURN POINTER TO ITEM IN LIST
 **/

char **
#if	defined(__STDC__)
wherelist (
	char *			item,
	char **			list
)
#else
wherelist (item, list)
	register char		*item;
	register char		**list;
#endif
{
	if (!list || !*list)
		return (0);

	while (*list) {
		if (STREQU(*list, item))
			return (list);
		list++;
	}
	return (0);
}
