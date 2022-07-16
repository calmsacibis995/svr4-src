/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibcurses:erase.c	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static	char sccsid[] = "@(#)erase.c 1.6 88/02/08 SMI"; /* from UCB 5.1 85/06/07 */
#endif not lint

# include	"curses.ext"

/*
 *	This routine erases everything on the window.
 *
 */
werase(win)
reg WINDOW	*win; {

	reg int		y;
	reg char	*sp, *end, *start, *maxx;
	reg int		minx;

# ifdef DEBUG
	fprintf(outf, "WERASE(%0.2o)\n", win);
# endif
	for (y = 0; y < win->_maxy; y++) {
		minx = _NOCHANGE;
		start = win->_y[y];
		end = &start[win->_maxx];
		for (sp = start; sp < end; sp++)
			if (*sp != ' ') {
				maxx = sp;
				if (minx == _NOCHANGE)
					minx = sp - start;
				*sp = ' ';
			}
		if (minx != _NOCHANGE)
			touchline(win, y, minx, maxx - win->_y[y]);
	}
	win->_curx = win->_cury = 0;
}
