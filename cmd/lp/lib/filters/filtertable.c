/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/filters/filtertable.c	1.6.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "lp.h"
#include "filters.h"

/**
 ** get_and_load() - LOAD REGULAR FILTER TABLE
 **/

int
#if	defined(__STDC__)
get_and_load (
	void
)
#else
get_and_load ()
#endif
{
	register char		*file;

	if (!(file = getfilterfile(FILTERTABLE)))
		return (-1);
	if (loadfilters(file) == -1) {
		Free (file);
		return (-1);
	}
	Free (file);
	return (0);
}

/**
 ** open_filtertable()
 **/

FILE *
#if	defined(__STDC__)
open_filtertable (
	char *			file,
	char *			mode
)
#else
open_filtertable (file, mode)
	char			*file,
				*mode;
#endif
{
	int			freeit;

	FILE			*fp;

	if (!file) {
		if (!(file = getfilterfile(FILTERTABLE)))
			return (0);
		freeit = 1;
	} else
		freeit = 0;
	
	fp = open_lpfile(file, mode, MODE_READ);

	if (freeit)
		Free (file);

	return (fp);
}

/**
 ** close_filtertable()
 **/

void
#if	defined(__STDC__)
close_filtertable (
	FILE *			fp
)
#else
close_filtertable (fp)
	FILE			*fp;
#endif
{
	(void)close_lpfile (fp);
	return;
}
