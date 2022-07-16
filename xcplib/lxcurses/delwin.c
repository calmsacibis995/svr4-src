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

#ident	"@(#)xcplxcurses:delwin.c	1.1"

/*
 *	@(#) delwin.c 1.1 90/03/30 lxcurses:delwin.c
 */
# include	"ext.h"

/*
 *	This routine deletes a window and releases it back to the system.
 *
 * 4/6/83 (Berkeley) @(#)delwin.c	1.5
 */
delwin(win)
reg WINDOW	*win; {

	reg int		i;
	reg WINDOW	*wp, *np;

	if (win->_orig == (WINDOW *)NULL) {
		/*
		 * If we are the original window, delete the space for
		 * all the subwindows, and the array of space as well.
		 */
		for (i = 0; i < win->_maxy && win->_y[i]; i++)
			cfree(win->_y[i]);
		wp = win->_nextp;
		while (wp != win) {
			np = wp->_nextp;
			delwin(wp);
			wp = np;
		}
	}
	else {
		/*
		 * If we are a subwindow, take ourself out of the
		 * list.  NOTE: if we are a subwindow, the minimum list
		 * is orig followed by this subwindow, so there are
		 * always at least two windows in the list.
		 */
		for (wp = win->_nextp; wp->_nextp != win; wp = wp->_nextp)
			continue;
		wp->_nextp = win->_nextp;
	}
	cfree(win->_y);
	cfree(win->_firstch);
	cfree(win->_lastch);
	cfree(win);
}
