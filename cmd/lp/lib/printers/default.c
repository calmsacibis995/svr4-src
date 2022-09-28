/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/printers/default.c	1.6.2.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdio.h"
#include "stdlib.h"

#include "lp.h"

/**
 ** getdefault() - READ THE NAME OF THE DEFAULT DESTINATION FROM DISK
 **/

char *
#if	defined(__STDC__)
getdefault (
	void
)
#else
getdefault ()
#endif
{
	return (loadline(Lp_Default));
}

/**
 ** putdefault() - WRITE THE NAME OF THE DEFAULT DESTINATION TO DISK
 **/

int
#if	defined(__STDC__)
putdefault (
	char *			dflt
)
#else
putdefault (dflt)
	char			*dflt;
#endif
{
	register FILE		*fp;

	if (!dflt || !*dflt)
		return (deldefault());

	if (!(fp = open_lpfile(Lp_Default, "w", MODE_READ)))
		return (-1);

	fprintf (fp, "%s\n", dflt);

	close_lpfile (fp);
	return (0);
}

/**
 ** deldefault() - REMOVE THE NAME OF THE DEFAULT DESTINATION
 **/

int
#if	defined(__STDC__)
deldefault (
	void
)
#else
deldefault ()
#endif
{
	return (rmfile(Lp_Default));
}
