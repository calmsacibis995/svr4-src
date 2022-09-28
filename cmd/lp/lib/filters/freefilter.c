/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/filters/freefilter.c	1.11.3.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "stdlib.h"

#include "lp.h"
#include "filters.h"

/**
 ** freefilter() - FREE INTERNAL SPACE ALLOCATED FOR A FILTER
 ** free_filter() - FREE INTERNAL SPACE ALLOCATED FOR A _FILTER
 **/

static void
#if	defined(__STDC__)
freetypel (
	TYPE *			typel
)
#else
freetypel (typel)
	register TYPE		*typel;
#endif
{
	register TYPE		*pt;

	if (typel) {
		for (pt = typel; pt->name; pt++)
			Free (pt->name);
		Free ((char *)typel);
	}
	return;
}

void
#if	defined(__STDC__)
freetempl (
	TEMPLATE *		templ
)
#else
freetempl (templ)
	register TEMPLATE	*templ;
#endif
{
	register TEMPLATE	*pt;

	if (templ) {
		for (pt = templ; pt->keyword; pt++) {
			Free (pt->keyword);
			if (pt->pattern)
				Free (pt->pattern);
			if (pt->re)
				Free (pt->re);
			if (pt->result)
				Free (pt->result);
		}
		Free ((char *)templ);
	}
	return;
}

void
#if	defined(__STDC__)
freefilter (
	FILTER *		pf
)
#else
freefilter (pf)
	FILTER			*pf;
#endif
{
	if (!pf)
		return;
	if (pf->name)
		Free (pf->name);
	if (pf->command)
		Free (pf->command);
	freelist (pf->printers);
	freelist (pf->printer_types);
	freelist (pf->input_types);
	freelist (pf->output_types);
	freelist (pf->templates);

	return;
}

void
#if	defined(__STDC__)
free_filter (
	_FILTER *		pf
)
#else
free_filter (pf)
	_FILTER			*pf;
#endif
{
	if (!pf)
		return;
	if (pf->name)
		Free (pf->name);
	if (pf->command)
		Free (pf->command);
	freelist (pf->printers);
	freetypel (pf->printer_types);
	freetypel (pf->input_types);
	freetypel (pf->output_types);
	freetempl (pf->templates);

	return;
}
