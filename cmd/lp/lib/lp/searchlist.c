/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/searchlist.c	1.5.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "string.h"

#include "lp.h"

/**
 ** searchlist() - SEARCH (char **) LIST FOR ITEM
 **/

int
#if	defined(__STDC__)
searchlist (
	char *			item,
	char **			list
)
#else
searchlist (item, list)
	register char		*item;
	register char		**list;
#endif
{
	if (!list || !*list)
		return (0);

	else if (STREQU(item, NAME_ANY) || STREQU(item, NAME_ALL))
		return (1);

	/*
	 * This is a linear search--we believe that the lists
	 * will be short.
	 */
	while (*list) {
		if (
			STREQU(*list, item)
		     || STREQU(*list, NAME_ANY)
		     || STREQU(*list, NAME_ALL)
		)
			return (1);
		list++;
	}
	return (0);
}

/**
 ** searchlist_with_terminfo() - SEARCH (char **) LIST FOR ITEM
 **/

int
#if	defined(__STDC__)
searchlist_with_terminfo (
	char *			item,
	char **			list
)
#else
searchlist_with_terminfo (item, list)
	register char		*item;
	register char		**list;
#endif
{
	if (!list || !*list)
		return (0);

	else if (STREQU(item, NAME_ANY) || STREQU(item, NAME_ALL))
		return (1);

	/*
	 * This is a linear search--we believe that the lists
	 * will be short.
	 */
	while (*list) {
		if (
			STREQU(*list, item)
		     || STREQU(*list, NAME_ANY)
		     || STREQU(*list, NAME_ALL)
		     || (
				STREQU(*list, NAME_TERMINFO)
			     && isterminfo(item)
			)
		)
			return (1);
		list++;
	}
	return (0);
}
