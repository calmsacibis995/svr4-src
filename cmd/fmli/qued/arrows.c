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

#ident	"@(#)fmli:qued/arrows.c	1.9"


#include <stdio.h>
#include <curses.h>
#include "winp.h"
#include "wish.h"
#include "ctl.h"
#include "fmacs.h"
#include "vtdefs.h"

/*
 * SETARROWS is used to set/clear scroll indicators for both
 * single-line and multi-line scrollable fields
 */
setarrows()
{
    register unsigned line;
    register int ch, savecol;

    line = 0;
    if (!(Flags & I_SCROLL))
	vt_ctl(VT_UNDEFINED, CTSETSARROWS, 0);
    else if (Cfld->rows == 1) {
	vt_ctl(VT_UNDEFINED, CTSETSARROWS, 0);
	savecol = Cfld->curcol;
	if (Buffoffset > 0)
	    line |= VT_UPSARROW;
	if ((Buffoffset + Cfld->cols + 1) < Bufflast) 
	    line |= VT_DNSARROW;
	if (line & VT_UPSARROW) {
	    if (line & VT_DNSARROW)
		ch = '=';
	    else
		ch = '<';
	}
	else if (line & VT_DNSARROW) {
	    if (line & VT_UPSARROW)
		ch = '=';
	    else
		ch = '>';
	}
	else
	    ch = ' ';
	fgo(0, LASTCOL + 1);
	fputchar(ch);
	fgo(0, savecol);
    }
    else {
	/*
	 * If the field takes up the entire frame
	 * or is a text object, then use the
	 * scroll box rather than the scroll indicators
	 */
	if (Buffoffset > 0)
	    line |= (Flags & (I_FULLWIN | I_TEXT)) ? VT_UPPARROW : VT_UPSARROW;
	if ((Valptr != NULL) || ((Buffoffset + FIELDBYTES) < Bufflast)) 
	    line |= (Flags & (I_FULLWIN | I_TEXT)) ? VT_DNPARROW : VT_DNSARROW;
	vt_ctl(VT_UNDEFINED, (Flags & (I_FULLWIN | I_TEXT)) ?
	       CTSETPARROWS : CTSETSARROWS, line);
    }
}	
