/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibcurses:mvscanw.c	1.1.1.1"

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
static	char sccsid[] = "@(#)mvscanw.c 1.8 88/02/08 SMI"; /* from UCB 5.1 85/06/07 */
#endif not lint

# include	<varargs.h>

# include	"curses.ext"

/*
 * implement the mvscanw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Another sigh....
 *
 */

/* VARARGS3 */
mvscanw(y, x, fmt, va_alist)
reg int		y, x;
char		*fmt;
va_dcl	{
	va_list ap;

	va_start(ap);
	return move(y, x) == OK ? _sscans(stdscr, fmt, ap) : ERR;
}

/*VARARGS4*/
mvwscanw(win, y, x, fmt, va_alist)
reg WINDOW	*win;
reg int		y, x;
char		*fmt;
va_dcl	{
	va_list ap;

	va_start(ap);
	return wmove(win, y, x) == OK ? _sscans(win, fmt, ap) : ERR;
}
