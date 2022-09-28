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
#ident	"@(#)fmli:vt/vmark.c	1.2"

#include	<curses.h>
#include	"wish.h"
#include	"vt.h"
#include	"vtdefs.h"

void
_vt_mark_overlap(v)
register struct vt	*v;
{
	register int	n;
	int	sr1, r1, sc1, c1;
	int	sr2, r2, sc2, c2;
	register struct vt	*vp;

	getbegyx(v->win, sr1, sc1);
	getmaxyx(v->win, r1, c1);
#ifdef _DEBUG
	_debug3(stderr, "vmark: window %d(#%d) - %d,%d %d,%d\n", v - VT_array, v->number, sr1, sc1, r1, c1);
#endif
	for (n = VT_front; n != VT_UNDEFINED; n = vp->next) {
		vp = &VT_array[n];
		getbegyx(vp->win, sr2, sc2);
		getmaxyx(vp->win, r2, c2);
		if (_vt_overlap(sr1, r1, sr2, r2) && _vt_overlap(sc1, c1, sc2, c2)) {
#ifdef _DEBUG
			_debug3(stderr, "\t\tmarking %d(#%d) dirty\n", n, vp->number);
#endif
			vp->flags |= VT_BDIRTY;
		}
	}
}
