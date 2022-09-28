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
#ident	"@(#)fmli:vt/wscrollwin.c	1.1"

#include	<curses.h>
#include	"wish.h"
#include	"vtdefs.h"
#include	"vt.h"
#include	"attrs.h"

void
wscrollwin(n)
int	n;
{
	register struct vt	*v;
	int	r;
	int	c;
	register int	top;

	v = &VT_array[VT_curid];
	getmaxyx(v->win, r, c);
	r--;
	top = 0;
	if (!(v->flags & VT_NOBORDER)) {
		top++;
		r--;
	}
	while (n < 0) {
		wmove(v->win, r, 0);
		wdeleteln(v->win);
		wmove(v->win, top, 0);
		winsertln(v->win);
		n++;
	}
	while (n > 0) {
		wmove(v->win, top, 0);
		wdeleteln(v->win);
		wmove(v->win, r, 0);
		winsertln(v->win);
		n--;
	}
	v->flags |= VT_BDIRTY;
	wmove(v->win, r, c);
}
