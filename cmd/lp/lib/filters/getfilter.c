/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/filters/getfilter.c	1.6.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "string.h"
#include "stdlib.h"

#include "lp.h"
#include "filters.h"

/**
 ** getfilter() - GET FILTER FROM FILTER TABLE
 **/

FILTER *
#if	defined(__STDC__)
getfilter (
	char *			name
)
#else
getfilter (name)
	char			*name;
#endif
{
	static _FILTER		*pf	= 0;

	static FILTER		flbuf;


	if (!name || !*name) {
		errno = EINVAL;
		return (0);
	}

	/*
	 * Don't need to check for ENOENT, because if it is set,
	 * well that's what we want to return anyway!
	 */
	if (!filters && get_and_load() == -1 /* && errno != ENOENT */ )
		return (0);

	if (STREQU(NAME_ALL, name))
		if (pf) {
			if (!(++pf)->name)
				pf = 0;
		} else
			pf = filters;
	else
		pf = search_filter(name);

	if (!pf || !pf->name) {
		errno = ENOENT;
		return (0);
	}

	flbuf.name = Strdup(pf->name);
	flbuf.command = (pf->command? Strdup(pf->command) : 0);
	flbuf.type = pf->type;
	flbuf.printer_types = typel_to_sl(pf->printer_types);
	flbuf.printers = duplist(pf->printers);
	flbuf.input_types = typel_to_sl(pf->input_types);
	flbuf.output_types = typel_to_sl(pf->output_types);
	flbuf.templates = templatel_to_sl(pf->templates);

	/*
	 * Make sure a subsequent ``all'' query starts getting
	 * filters from the beginning.
	 */
	if (!STREQU(NAME_ALL, name))
		pf = 0;

	return (&flbuf);
}
