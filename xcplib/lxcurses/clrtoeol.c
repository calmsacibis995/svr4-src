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

#ident	"@(#)xcplxcurses:clrtoeol.c	1.1"

/*
 *	@(#) clrtoeol.c 1.1 90/03/30 lxcurses:clrtoeol.c
 */
# include	"ext.h"

/*
 *	This routine clears up to the end of line
 *
 * 3/5/81 (Berkeley) @(#)clrtoeol.c	1.2
 */
wclrtoeol(win)
reg WINDOW	*win; {

	reg char	*sp, *end;
	reg int		y, x;
	reg char	*maxx;
	reg int		minx;

	y = win->_cury;
	x = win->_curx;
	end = &win->_y[y][win->_maxx];
	minx = _NOCHANGE;
	maxx = &win->_y[y][x];
	for (sp = maxx; sp < end; sp++)
		if (*sp != ' ') {
			maxx = sp;
			if (minx == _NOCHANGE)
				minx = sp - win->_y[y];
			*sp = ' ';
		}
	/*
	 * update firstch and lastch for the line
	 */
# ifdef DEBUG
	fprintf(outf, "CLRTOEOL: minx = %d, maxx = %d, firstch = %d, lastch = %d\n", minx, maxx - win->_y[y], win->_firstch[y], win->_lastch[y]);
# endif
	if (minx != _NOCHANGE) {
		if (win->_firstch[y] > minx || win->_firstch[y] == _NOCHANGE)
			win->_firstch[y] = minx;
		if (win->_lastch[y] < maxx - win->_y[y])
			win->_lastch[y] = maxx - win->_y[y];
	}
}
