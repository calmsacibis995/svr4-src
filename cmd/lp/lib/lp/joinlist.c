/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/joinlist.c	1.3.2.1"
/* LINTLIBRARY */

#include "lp.h"

/**
 ** mergelist() - ADD CONTENT OF ONE LIST TO ANOTHER
 **/

int
#if	defined(__STDC__)
joinlist (
	char ***		dstlist,
	char **			srclist
)
#else
joinlist (dstlist, srclist)
	char ***		dstlist;
	char **			srclist;
#endif
{
	if (!srclist || !*srclist)
		return (0);

	while (*srclist)
		if (appendlist(dstlist, *srclist++) == -1)
			return (-1);

	return (0);
}
