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

#ident	"@(#)xcplxcurses:deleteln.c	1.1"

/*
 *	@(#) deleteln.c 1.1 90/03/30 lxcurses:deleteln.c
 */
# include	"ext.h"

/*
 *	This routine deletes a line from the screen.  It leaves
 * (_cury,_curx) unchanged.
 *
 * 5/11/81 (Berkeley) @(#)deleteln.c	1.4
 */
wdeleteln(win)
reg WINDOW	*win; {

	reg char	*temp;
	reg int		y;
	reg char	*end;

	temp = win->_y[win->_cury];
	for (y = win->_cury; y < win->_maxy - 1; y++) {
		win->_y[y] = win->_y[y+1];
		win->_firstch[y] = 0;
		win->_lastch[y] = win->_maxx - 1;
	}
	for (end = &temp[win->_maxx]; temp < end; )
		*temp++ = ' ';
	win->_y[win->_maxy-1] = temp - win->_maxx;
}
