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
#ident	"@(#)fmli:vt/hide.c	1.9"

#include	<curses.h>
#include	"wish.h"
#include	"vt.h"
#include	"vtdefs.h"
#include	"color_pair.h"

/*
 * hide a window until it is made current again
 */
void
_vt_hide(vid, destructive)
vt_id	vid;
bool	destructive;
{
	register vt_id	n;
	register struct vt	*v;
	register struct vt	*vp;
	register WINDOW	*w;

/* debug stuff

	fprintf( stderr, "\t\t\t\t\tInto _vt_hide( %d )\n", vid );

	fprintf( stderr, "VT_front = %d\n", VT_front );
	fprintf( stderr, "VT_back =  %d\n", VT_back );
	fprintf( stderr, "VT_curid = %d\n\n", VT_curid );

        for ( n = VT_front; n != VT_UNDEFINED; n = v->next )
	{
		v = &VT_array[ n ];

		fprintf( stderr, "prev = %d\n", v->prev );
		fprintf( stderr, "VT_array index = %d\n", n );
		fprintf( stderr, "next = %d\n\n", v->next );
	}
/**/

	v = &VT_array[vid];
	if (vid < 0 || !(v->flags & VT_USED))
		return;
	/* mark windows we are covering */
	_vt_mark_overlap(v);
	/* physically remove from screen */
	if (destructive)
		w = v->win;
	else {
		int	row, col, rows, cols;

		getbegyx(v->win, row, col);
		getmaxyx(v->win, rows, cols);
		w = newwin(rows, cols, row, col);
	}
	wbkgd(w, COL_ATTR(A_NORMAL, WINDOW_PAIR));
	werase(w);
	wnoutrefresh(w);
	if (destructive && v->subwin)
		delwin(v->subwin);
	delwin(w);
	/* remove from window list */
	if (VT_front == vid)
	{
		VT_front = v->next;

		if ( VT_front != VT_UNDEFINED )
			VT_array[ VT_front ].prev = VT_UNDEFINED;
	}
	if (VT_curid == vid)
	{
		VT_curid = VT_front;
		vp = &VT_array[ VT_curid ];
		vp->flags |= VT_TDIRTY;
		/*
		 * Since active/inactive border colors can be specified
		 * for color terminals, border should also be marked dirty.
		 */
		if ((!(vp->flags & VT_NOBORDER)) &&
		    Color_terminal == TRUE && Border_colors_differ)
			vp->flags |= VT_BDIRTY;	
	}

	for (n = VT_front; n != VT_UNDEFINED; n = vp->next) {
		vp = &VT_array[n];
		if (vp->next == vid)
		{
			vp->next = VT_array[vid].next;

			if ( VT_back == vid )
				VT_back = VT_array[ VT_back ].prev;
			else
			{
				vp = &VT_array[ vid ];
				VT_array[ v->next ].prev = n;
			}
	
			break;
		}
	}
/* NEEDED ??? */
	v->prev = v->next = VT_UNDEFINED;

/* debug stuff

	fprintf( stderr, "\t\t\tAfter change\n" );

	fprintf( stderr, "VT_front = %d\n", VT_front );
	fprintf( stderr, "VT_back =  %d\n", VT_back );
	fprintf( stderr, "VT_curid = %d\n\n", VT_curid );

        for ( n = VT_front; n != VT_UNDEFINED; n = v->next )
	{
		v = &VT_array[ n ];

		fprintf( stderr, "prev = %d\n", v->prev );
		fprintf( stderr, "VT_array index = %d\n", n );
		fprintf( stderr, "next = %d\n\n", v->next );
	}
/**/
}
