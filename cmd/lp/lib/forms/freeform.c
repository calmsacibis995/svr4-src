/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:lib/forms/freeform.c	1.1.1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "sys/types.h"
#include "stdlib.h"

#include "lp.h"
#include "form.h"

/**
 **  freeform() - FREE MEMORY ALLOCATED FOR FORM STRUCTURE
 **/

void
#if	defined(__STDC__)
freeform (
	FORM *			pf
)
#else
freeform (pf)
	FORM *			pf;
#endif
{
	if (!pf)
		return;
	if (pf->chset)
		Free (pf->chset);
	if (pf->rcolor)
		Free (pf->rcolor);
	if (pf->comment)
		Free (pf->comment);
	if (pf->conttype)
		Free (pf->conttype);
	if (pf->name)
		Free (pf->name);
	pf->name = 0;

	return;
}
