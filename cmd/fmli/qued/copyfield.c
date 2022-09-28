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
#ident	"@(#)fmli:qued/copyfield.c	1.11"

#include <stdio.h>
#include <memory.h>
#include <curses.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "moremacros.h"
#include "terror.h"

extern char   *fputstring();
extern void   acsputstring();

/*
 * COPYFIELD will copy a field form one part of the screen to another
 * (including all of the field status information)
 */
copyfield(srcfld, destfld) 
ifield *srcfld, *destfld;
{
    ifield *savefield;
    long tmpoffset;
	

    if (srcfld == NULL || destfld == NULL)
	return(FAIL);
    savefield = Cfld;
    Cfld = destfld;
    if (srcfld->scrollbuf) {	/* if a scrollable field */
	register int linesize, i;

	if (destfld->scrollbuf)	/* ehr3 */
	    free(destfld->scrollbuf); /* ehr3 */

	if ((destfld->scrollbuf = (chtype *)malloc      /* added +1 abs k15 */
	     ((srcfld->buffsize + 1) * sizeof(*srcfld->scrollbuf))) == NULL)
	    fatal(NOMEM, "");
		
	destfld->buffsize = srcfld->buffsize;
	memcpy(destfld->scrollbuf, srcfld->scrollbuf,   /* added +1 abs k15 */
	       (srcfld->buffsize +1) * sizeof(*srcfld->scrollbuf));
	linesize = destfld->cols + 1;
	tmpoffset = 0L;
	for (i = 0; i < srcfld->rows; i++) {
	    /* print the copied field to the screen */
	    fgo(i, 0);
	    acsputstring(destfld->scrollbuf + tmpoffset);
	    tmpoffset += linesize;
	}
    }
    if (srcfld->value) {
	if (destfld->value)	/* ehr3 */
	    free(destfld->value); /* ehr3 */

	destfld->value = strsave(srcfld->value);

	if (!destfld->scrollbuf) /* if not a scroll field */
	    destfld->valptr = fputstring(destfld->value);
    }
    destfld->currow = srcfld->currow;
    destfld->curcol = srcfld->curcol;
    Cfld = savefield;
    return(SUCCESS);
}

/*
 * HIDEFIELD will remove the field from screen WITHOUT destroying the
 * ifield structure.
 */
hidefield(fld)
ifield *fld;
{
	ifield *savefield;
	int flags;

	savefield = Cfld;
	if (fld != NULL)
		Cfld = fld;
	flags = fld->flags;
	setfieldflags(fld, (fld->flags & ~I_FILL));
	fgo(0, 0);
	fclear();
	setfieldflags(fld, flags);
	Cfld = savefield;
}
