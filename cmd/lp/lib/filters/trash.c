/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/filters/trash.c	1.6.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdlib.h"

#include "lp.h"
#include "filters.h"

/**
 ** trash_filters() - FREE ALL SPACE ALLOCATED FOR FILTER TABLE
 **/

void			trash_filters ()
{
	register _FILTER	*pf;

	if (filters) {
		for (pf = filters; pf->name; pf++)
			free_filter (pf);
		Free ((char *)filters);
		nfilters = 0;
		filters = 0;
	}
	return;
}
