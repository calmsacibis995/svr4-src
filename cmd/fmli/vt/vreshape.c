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
#ident	"@(#)fmli:vt/vreshape.c	1.7"

#include	<curses.h>
#include	<term.h>
#include	"wish.h"
#include	"vt.h"
#include	"vtdefs.h"
#include	"color_pair.h"

/*
 * reshape a VT
 */
vt_reshape(vid, srow, scol, rows, cols)
vt_id	vid;
int	srow;
int	scol;
unsigned	rows;
unsigned	cols;
{
	register int	num;
	register int	wnum;
	register struct vt	*v;
	extern int	VT_firstline;

	if (off_screen(srow, scol, rows, cols)) {
#ifdef _DEBUG
		_debug(stderr, "off_screen FAILED!!!  This should never happen here!!!\n");
#endif
		return FAIL;
	}
	srow += VT_firstline;
	/* pick a window number (if appropriate) */
	v = &VT_array[vid];
	/* set up v */
	_vt_hide(vid, TRUE);
	if ((v->win = newwin(rows, cols, srow, scol)) == NULL) {
#ifdef _DEBUG
		_debug(stderr, "newwin\n");
#endif
		return FAIL;
	}
	notimeout(v->win, TRUE);
	if (v->subwin) {
		if ((v->subwin = subwin(v->win, rows-2, cols-2, srow+1, scol+1)) == NULL) {
#ifdef _DEBUG3
			_debug3(stderr, "subwin\n");
#endif
			return FAIL;
	    	}
		notimeout(v->subwin, TRUE);
	}
 	if (Color_terminal == TRUE) {
 		wbkgd(v->win, COL_ATTR(0, WINDOW_PAIR));
 		wattrset(v->win, COL_ATTR(0, WINDOW_PAIR));
 	}
	keypad(v->win, TRUE);
	if (v->flags & VT_NOBORDER)
		wmove(v->win, 0, 0);
	else
		wmove(v->win, 1, 1);
	v->flags |= VT_ANYDIRTY;
	vt_current(vid);
	return SUCCESS;
}
