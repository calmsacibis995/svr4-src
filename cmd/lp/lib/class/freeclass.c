/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/class/freeclass.c	1.5.3.1"
/* LINTLIBRARY */

#include "lp.h"
#include "class.h"

/**
 ** freeclass() - FREE SPACE USED BY CLASS STRUCTURE
 **/

void
#if	defined(__STDC__)
freeclass (
	CLASS *			clsbufp
)
#else
freeclass (clsbufp)
	CLASS			*clsbufp;
#endif
{
	if (!clsbufp)
		return;
	if (clsbufp->name)
		Free (clsbufp->name);
	freelist (clsbufp->members);
	return;
}
