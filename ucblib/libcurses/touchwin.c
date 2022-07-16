/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibcurses:touchwin.c	1.1.1.1"

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
static	char sccsid[] = "@(#)touchwin.c 1.6 88/02/08 SMI"; /* from UCB 5.1 85/06/07 */
#endif not lint

# include	"curses.ext"

/*
 * make it look like the whole window has been changed.
 *
 */
touchwin(win)
register WINDOW	*win;
{
	register int	y, maxy;

# ifdef	DEBUG
	fprintf(outf, "TOUCHWIN(%0.2o)\n", win);
# endif
	maxy = win->_maxy;
	for (y = 0; y < maxy; y++)
		touchline(win, y, 0, win->_maxx - 1);
}

/*
 * touch a given line
 */
touchline(win, y, sx, ex)
register WINDOW	*win;
register int	y, sx, ex;
{
# ifdef DEBUG
	fprintf(outf, "TOUCHLINE(%0.2o, %d, %d, %d)\n", win, y, sx, ex);
	fprintf(outf, "TOUCHLINE:first = %d, last = %d\n", win->_firstch[y], win->_lastch[y]);
# endif
	sx += win->_ch_off;
	ex += win->_ch_off;
	if (win->_firstch[y] == _NOCHANGE) {
		win->_firstch[y] = sx;
		win->_lastch[y] = ex;
	}
	else {
		if (win->_firstch[y] > sx)
			win->_firstch[y] = sx;
		if (win->_lastch[y] < ex)
			win->_lastch[y] = ex;
	}
# ifdef	DEBUG
	fprintf(outf, "TOUCHLINE:first = %d, last = %d\n", win->_firstch[y], win->_lastch[y]);
# endif
}
