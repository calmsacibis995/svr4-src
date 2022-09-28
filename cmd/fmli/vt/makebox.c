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
#ident	"@(#)fmli:vt/makebox.c	1.5"

#include	<curses.h>
#include	"wish.h"
#include	"vt.h"
#include	"vtdefs.h"
#include	"attrs.h"

#define TL	0
#define BL	1
#define BR	2
#define TR	3

static vt_id	side[4] = { -1, -1, -1, -1 };

bool
make_box(flag, srow, scol, rows, cols)
bool	flag;
register int	srow;
register int	scol;
register int	rows;
register int	cols;
{
	register vt_id	vid;
	void	remove_box();
	bool	corner();

	if (srow < 0 || scol < 0 || cols < 1 || rows < 1) {
		remove_box();
		return FALSE;
	}
	if (side[TL] >= 0)
		remove_box();
	rows--;
	cols--;
	if (!corner(TL, srow, scol, ACS_ULCORNER, flag))
		return FALSE;
	if (!corner(BL, srow + rows, scol, ACS_LLCORNER, TRUE))
		return FALSE;
	if (!corner(BR, srow + rows, scol + cols, ACS_LRCORNER, !flag))
		return FALSE;
	if (!corner(TR, srow, scol + cols, ACS_URCORNER, TRUE))
		return FALSE;
	if (flag)
	    vt_current(side[BR]);
	else
	    vt_current(side[TL]);
/*	vt_current(side[flag ? BR : TL]); amdahl compatibility */
	return TRUE;
}

static bool
corner(which, row, col, ch, flag)
int	which;
int	row;
int	col;
chtype	ch;
int	flag;
{
	register vt_id	vid;
	register struct vt	*v;
	static  void remove_box();

	if ((vid = side[which] = vt_create(NULL, VT_NONUMBER | VT_NOBORDER, row, col, 1, 1)) < 0) {
		remove_box();
		return FALSE;
	}
	vt_current(vid);
	v = &VT_array[vid];
	scrollok(v->win, FALSE);
	if (flag)
		waddch(v->win, ch | Attr_visible);
	else
		waddch(v->win, ch);
	v->flags |= VT_DIRTY;
	return TRUE;
}

static void
remove_box()
{
	register int	i;

	for (i = 0; i < 4; i++) {
		if (side[i] >= 0)
			vt_close(side[i]);
		side[i] = -1;
	}
}
