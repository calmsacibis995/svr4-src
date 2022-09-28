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
#ident	"@(#)fmli:vt/highlight.c	1.5"

#include	<curses.h>
#include	<term.h>
#include	"color_pair.h"

/*
 * the UNDERLINE flag is the second bit of the "ncv" (no_color_video) 
 * terminfo variable (see set_underline_attrs() below)
 */ 
#define UNDERLINE	(0x02)

chtype Attr_normal; 	/* normal video */
chtype Attr_hide;	/* border of non-current window */
chtype Attr_highlight;	/* border of current window */
chtype Attr_select;	/* attribute of "selector bar" */
chtype Attr_show;	/* something visible (errors, etc) */
chtype Attr_visible;	/* the most annoying thing terminal can do */
chtype Attr_underline;	/* attribute of underline */
chtype Attr_mark;	/* attribute of "marked" items */



setvt_attrs()
{
	static chtype	modes;

	/*
	 * Determine modes
	 */
	if (enter_blink_mode)
		modes |= A_BLINK;
	if (enter_bold_mode)
		modes |= A_BOLD;
	if (enter_dim_mode)
		modes |= A_DIM;
	if (enter_reverse_mode)
		modes |= A_REVERSE;
	if (enter_standout_mode)
		modes |= A_STANDOUT;
	if (enter_underline_mode)
		modes |= A_UNDERLINE;

	/*
	 * Set up Attribute array
	 */
	Attr_normal = A_NORMAL;
	Attr_underline = A_UNDERLINE;	/* let curses decide */
	Attr_highlight = modes & A_STANDOUT;
	if (modes & A_REVERSE)
		Attr_highlight = A_REVERSE;
	Attr_visible = Attr_show = Attr_select = Attr_hide = Attr_highlight;
	if (modes & A_DIM)
		Attr_select = Attr_hide = modes & (A_REVERSE | A_DIM);
	else if (modes & A_BOLD) {
		Attr_highlight |= A_BOLD;
		Attr_select = A_BOLD;
	}
	if (modes & A_BLINK)
		Attr_visible |= A_BLINK;
	Attr_mark = Attr_select;
	if (modes & A_UNDERLINE)
		Attr_mark = A_UNDERLINE;
}

/*
 * SET_UNDERLINE_COLOR will change the underline attribute to be
 * "colpair" IF the terminal supports color BUT the terminal CAN NOT
 * support color attributes with underlining. 
 */
set_underline_attr(colpair)
int colpair;
{
	if (Color_terminal == TRUE && no_color_video >= 0 &&
	   (no_color_video & UNDERLINE))
		Attr_underline = COL_ATTR(A_REVERSE, colpair);
}
