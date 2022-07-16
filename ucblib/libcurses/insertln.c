/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibcurses:insertln.c	1.1.1.1"

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
static	char sccsid[] = "@(#)insertln.c 1.6 88/02/08 SMI"; /* from UCB 5.1 85/06/07 */
#endif not lint

# include	"curses.ext"

/*
 *	This routine performs an insert-line on the window, leaving
 * (_cury,_curx) unchanged.
 *
 */
winsertln(win)
reg WINDOW	*win; {

	reg char	*temp;
	reg int		y;
	reg char	*end;
	reg int		x;

#ifdef	DEBUG
	fprintf(outf, "INSERTLN(%0.2o)\n", win);
#endif
	if (win->_orig == NULL)
		temp = win->_y[win->_maxy - 1];
	for (y = win->_maxy - 1; y > win->_cury; --y) {
		if (win->_orig == NULL)
			win->_y[y] = win->_y[y - 1];
		else
			bcopy(win->_y[y - 1], win->_y[y], win->_maxx);
		touchline(win, y, 0, win->_maxx - 1);
	}
	if (win->_orig == NULL)
		win->_y[y] = temp;
	else
		temp = win->_y[y];
	for (end = &temp[win->_maxx]; temp < end; )
		*temp++ = ' ';
	touchline(win, y, 0, win->_maxx - 1);
	if (win->_orig == NULL)
		_id_subwins(win);
}
