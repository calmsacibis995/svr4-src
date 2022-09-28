/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/filters/putfilter.c	1.9.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "errno.h"
#include "string.h"
#include "stdlib.h"

#include "lp.h"
#include "filters.h"

/**
 ** putfilter() - PUT FILTER INTO FILTER TABLE
 **/

int
#if	defined(__STDC__)
putfilter (
	char *			name,
	FILTER *		flbufp
)
#else
putfilter (name, flbufp)
	char			*name;
	FILTER			*flbufp;
#endif
{
	_FILTER			_flbuf;

	register _FILTER	*pf;


	if (!name || !*name) {
		errno = EINVAL;
		return (-1);
	}

	if (STREQU(NAME_ALL, name)) {
		errno = EINVAL;
		return (-1);
	}

	_flbuf.name = Strdup(name);
	_flbuf.command = (flbufp->command? Strdup(flbufp->command) : 0);
	_flbuf.type = flbufp->type;
	_flbuf.printer_types = sl_to_typel(flbufp->printer_types);
	_flbuf.printers = duplist(flbufp->printers);
	_flbuf.input_types = sl_to_typel(flbufp->input_types);
	_flbuf.output_types = sl_to_typel(flbufp->output_types);
	if (!flbufp->templates)
		_flbuf.templates = 0;
	else if (!(_flbuf.templates = sl_to_templatel(flbufp->templates))) {
		free_filter (&_flbuf);
		errno = EBADF;
		return (-1);
	}

	if (!filters && get_and_load() == -1 && errno != ENOENT) {
		free_filter (&_flbuf);
		return (-1);
	}

	if (filters) {

		if ((pf = search_filter(name)))
			free_filter (pf);
		else {
			nfilters++;
			filters = (_FILTER *)Realloc(
				(char *)filters,
				(nfilters + 1) * sizeof(_FILTER)
			);
			if (!filters) {
				free_filter (&_flbuf);
				errno = ENOMEM;
				return (-1);
			}
			filters[nfilters].name = 0;
			pf = filters + nfilters - 1;
		}

	} else {

		nfilters = 1;
		pf = filters = (_FILTER *)Malloc(
			(nfilters + 1) * sizeof(_FILTER)
		);
		if (!filters) {
			free_filter (&_flbuf);
			errno = ENOMEM;
			return (-1);
		}
		filters[nfilters].name = 0;

	}

	*pf = _flbuf;

	return (dumpfilters((char *)0));
}
