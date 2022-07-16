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

#ident	"@(#)xcplxcurses:clrtobot.c	1.1"

/*
 *	@(#) clrtobot.c 1.1 90/03/30 lxcurses:clrtobot.c
 */
# include	"ext.h"

/*
 *	This routine erases everything on the window.
 *
 * 1/26/81 (Berkeley) @(#)clrtobot.c	1.1
 */
wclrtobot(win)
reg WINDOW	*win; {

	reg int		y;
	reg char	*sp, *end, *maxx;
	reg int		startx, minx;

	startx = win->_curx;
	for (y = win->_cury; y < win->_maxy; y++) {
		minx = _NOCHANGE;
		end = &win->_y[y][win->_maxx];
		for (sp = &win->_y[y][startx]; sp < end; sp++)
			if (*sp != ' ') {
				maxx = sp;
				if (minx == _NOCHANGE)
					minx = sp - win->_y[y];
				*sp = ' ';
			}
		if (minx != _NOCHANGE) {
			if (win->_firstch[y] > minx
			     || win->_firstch[y] == _NOCHANGE)
				win->_firstch[y] = minx;
			if (win->_lastch[y] < maxx - win->_y[y])
				win->_lastch[y] = maxx - win->_y[y];
		}
		startx = 0;
	}
	win->_curx = win->_cury = 0;
}
