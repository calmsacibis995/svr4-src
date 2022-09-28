/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/freelist.c	1.8.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"

/**
 ** freelist() - FREE ALL SPACE USED BY LIST
 **/

void
#if	defined(__STDC__)
freelist (
	char **			list
)
#else
freelist (list)
	char			**list;
#endif
{
	register char		**pp;

	if (list) {
		for (pp = list; *pp; pp++)
			Free (*pp);
		Free ((char *)list);
	}
	return;
}
