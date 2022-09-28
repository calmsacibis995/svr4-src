/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:form/fcheck.c	1.5"

#include	<stdio.h>
#include        <curses.h>
#include	"wish.h"
#include	"token.h"
#include	"winp.h"
#include	"form.h"

/*
 * A field definition may contain:
 *
 * 	1) a field name only 
 * 	2) a field only 
 *	3) both a field name and a field
 *
 * The following macros are useful in determining which is the case
 */ 
#define HAS_NAME(x)	((x->nrow >= 0) && (x->ncol >= 0)) 
#define HAS_FIELD(x)	((x->cols > 0) && (x->rows > 0) && \
			 (x->frow >= 0) && (x->fcol >= 0))

/*
 * CHECKFFIELD will handle setting/resetting field values depending
 * on the current/previous state of the field value
 */ 
checkffield(fptr, pffld)
struct form *fptr;		/* pointer to the form structure */ 
register formfield *pffld;	/* how the field "should" be displayed */ 
{
	register ifield *fld;	/* how the field "is" displayed */ 
	ifield *newfield();

	if (!(*(pffld->ptr))) {
		/*
		 * this is the first time ... initialize the field 
		 */
		*(pffld->ptr) = (char *) newfield(pffld->frow, pffld->fcol,
			pffld->rows, pffld->cols, pffld->flags);
		if (!(pffld->flags & I_NOSHOW)) {
			/*
			 * if "show=true" then display the field name
			 * as well as the field itself. 
			 */
		        if (HAS_NAME(pffld)) {
				wgo(pffld->nrow, pffld->ncol);
				wputs(pffld->name, NULL);
			}
			if (HAS_FIELD(pffld))
				putfield((ifield *) *(pffld->ptr), pffld->value);
		}
		return;
	}
	else if (pffld->flags & I_NOSHOW) {
		/*
		 * field is a "show=false" field
		 */
		fld = (ifield *) *(pffld->ptr);
		if (!(fld->flags & I_NOSHOW)) {
			/*
			 * if field was recently a "show=true" field ...
			 * then remove the field name and the field value
			 */
			if (HAS_NAME(pffld)) {

				char tbuf[BUFSIZ];

				sprintf(tbuf, "%*s", strlen(pffld->name), " ");
				wgo(pffld->nrow, pffld->ncol);
				wputs(tbuf, NULL);
			}
			if (HAS_FIELD(fld))
				hidefield(fld);
		}
	}
	else {
		/*
		 * field is a "show=true" field
		 */
		fld = (ifield *) *(pffld->ptr);

		/*
		 * Only redisplay the field name if the field HAS 
		 * a name AND:
		 *
		 * 1) the form is all dirty OR
		 * 2) the field was previously "show=false"
		 */
		if (HAS_NAME(pffld) && ((fptr->flags & FORM_ALLDIRTY) ||
					(fld->flags & I_NOSHOW))) {
			wgo(pffld->nrow, pffld->ncol);
			wputs(pffld->name, NULL);
		}
		/*
		 * Only redisplay the field value if there IS a field AND:
		 *
		 * 1) the field went from active to inactive or vice versa OR
		 * 2) the form is all dirty OR
		 * 3) the new field value is different from
		 *    the old field value OR
		 * 4) the field was previously "show=false" 
		 */
		if (HAS_FIELD(pffld)) {
			if ((fld->flags & I_FILL) ^ (pffld->flags & I_FILL)) {
				setfieldflags(*(pffld->ptr), pffld->flags);
				putfield(fld, pffld->value);
			}
			else if ((fptr->flags & FORM_ALLDIRTY) ||
		 	    (strcmp(fld->value, pffld->value) != 0) ||
			    (fld->flags & I_NOSHOW)) {
				putfield(fld, pffld->value);
			}
		}
	}

	/*
	 * update field flags if necessary ....
	 */ 
	if (((ifield *) *(pffld->ptr))->flags != pffld->flags)
		setfieldflags(*(pffld->ptr), pffld->flags);
}
