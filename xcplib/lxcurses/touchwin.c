/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)xcplxcurses:touchwin.c	1.1"

/*
 *	@(#) touchwin.c 1.1 90/03/30 lxcurses:touchwin.c
 */
# include	"ext.h"

static do_touch();

/*
 * make it look like the whole window has been changed.
 *
 * 5/9/83 (Berkeley) @(#)touchwin.c	1.2
 */
touchwin(win)
reg WINDOW	*win;
{
	reg WINDOW	*wp;

	do_touch(win);
	for (wp = win->_nextp; wp != win; wp = wp->_nextp)
		do_touch(wp);
}

/*
 * do_touch:
 *	Touch the window
 */
static
do_touch(win)
reg WINDOW	*win; {

	reg int		y, maxy, maxx;

	maxy = win->_maxy;
	maxx = win->_maxx - 1;
	for (y = 0; y < maxy; y++) {
		win->_firstch[y] = 0;
		win->_lastch[y] = maxx;
	}
}
