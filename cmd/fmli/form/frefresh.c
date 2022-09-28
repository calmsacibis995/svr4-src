/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:form/frefresh.c	1.12"

#include <stdio.h>
#include <curses.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "form.h"
#include "attrs.h"

form_refresh(fid)
form_id fid;
{
	register int i, maxrows, maxcols;
	register char *argptr;
	struct ifield *curfld = NULL;
	int curmaxrows, curmaxcols;
	formfield ff, (*disp)();
	struct form *fptr;
	vt_id oldvid;
	int   retval;		/* abs */

	fptr = &FORM_array[fid];
	oldvid = vt_current(fptr->vid);
	disp = fptr->display;
	argptr = fptr->argptr;
	curmaxrows = fptr->rows;
	curmaxcols = fptr->cols;
	maxrows = maxcols = 0;

	ff = (*disp)(0, argptr);
	for (i = 0; ff.name != NULL; ff = (*disp)(++i, argptr)) {
		/*
		 * For all fields that are visible on the current page ...
		 * display/hide/update the field as appropriate
		 * (see fcheck.c) 
		 *
		 * ... also, determine the size of the entire form.
		 */
		checkffield(fptr, &ff);
		maxrows = max(maxrows, max(ff.frow + ff.rows, ff.nrow + 1));
		maxcols = max(maxcols, max(ff.fcol + ff.cols, ff.ncol + strlen(ff.name)));
		if (i == (fptr->curfldnum))
			curfld = (struct ifield *) *(ff.ptr);
	}
	if (maxrows > curmaxrows || maxcols > curmaxcols) {
		/*
		 * If the form should grow in size then reinitialize
		 * the form altogether.
		 */
	        retval = form_reinit(fid, fptr->flags, disp, argptr);
		fptr->flags &= ~(FORM_DIRTY | FORM_ALLDIRTY);
		return(retval);	/* abs */
	}
	else {
		/*
		 * clear dirty bits ... set/reset the form to the
		 * previously current field ... make the "oldvid"
		 * current again.
		 */
		fptr->flags &= ~(FORM_DIRTY | FORM_ALLDIRTY);
		gotofield(curfld, 0, 0);
		(void) vt_current(oldvid);
		return(SUCCESS);		/* abs */
	}
}
