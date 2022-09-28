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
#ident	"@(#)fmli:qued/putfield.c	1.10"

#include <stdio.h>
#include <curses.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "moremacros.h"
#include "terror.h"

extern 	char *fputstring();

putfield(fld, str)
ifield *fld;
char *str;
{
	ifield *savefield;
	chtype *sbuf_ptr;
	char *v_ptr;

	if (str == NULL)
		return;
	savefield = Cfld;
	if (fld != NULL)
		Cfld = fld;
	else if (!Cfld)			/* no current field */
		return;
	if (Flags & I_INVISIBLE) {
		Cfld = savefield;
		return;
	}
	Flags |= I_CHANGED;
	fgo(0, 0);			/* home the cursor */

	/*
	 * Free remains of a previous field value
	 */
	if (Value)
		free(Value);
	if (Scrollbuf)
		free_scroll_buf(Cfld);	/* if used, reset scroll buffers */

	/*
	 * If Value is LESS than the visible field size
	 * then allocate at least the field size
	 * otherwise strsave the passed value.
	 */
	if (strlen(str) < FIELDBYTES) {
		if ((Value = malloc(FIELDBYTES +1)) == NULL) /* +1 abs k15 */
			fatal(NOMEM, nil);
		strcpy(Value, str);
	}
	else
		Value = strsave(str);

	Valptr = fputstring(Value);	/* update pointer into value */
	fclear();			/* clear the rest of field */
	fgo(0, 0);			/* home the cursor */
	if ((Flags & I_SCROLL) && Currtype == SINGLE) {
		/*
		 * HORIZONTAL SCROLLING
		 * initialize scroll buffer and copy string to it
		 */
		unsigned vallength, maxlength;

		vallength = strlen(Value);
		maxlength = max(vallength, FIELDBYTES);	/* removed +1 abs k15 */
		growbuf(maxlength);
/*		strcpy(Scrollbuf, Value);  abs */
		/* THE following is >>> WRONG <<< it does not
		 * process  characters like tabs. it should be
		 * handled like vertical scroll fields.
		 */
		sbuf_ptr = Scrollbuf;
		v_ptr = Value;
		while (*v_ptr!= '\0')
		    *sbuf_ptr++ = ((chtype) *v_ptr++) | Fieldattr;
		free(Value);
		Valptr = Value = NULL;
	}
	setarrows();
	Cfld = savefield;
}
