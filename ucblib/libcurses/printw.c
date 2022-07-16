/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibcurses:printw.c	1.1.1.1"

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
static	char sccsid[] = "@(#)printw.c 1.8 88/02/08 SMI"; /* from UCB 5.1 85/06/07 */
#endif not lint

# include	<varargs.h>

/*
 * printw and friends
 *
 */

# include	"curses.ext"

/*
 *	This routine implements a printf on the standard screen.
 */
/* VARARGS1 */
printw(fmt, va_alist)
char	*fmt;
va_dcl	{
	va_list ap;

	va_start(ap);
	return _sprintw(stdscr, fmt, ap);
}

/*
 *	This routine implements a printf on the given window.
 */
/* VARARGS2 */
wprintw(win, fmt, va_alist)
WINDOW	*win;
char	*fmt;
va_dcl	{
	va_list ap;

	va_start(ap);
	return _sprintw(win, fmt, ap);
}
/*
 *	This routine actually executes the printf and adds it to the window
 *
 *	This code now uses the vsprintf routine, which portably digs
 *	into stdio.  We provide a vsprintf for older systems that don't
 *	have one.
 */
_sprintw(win, fmt, ap)
WINDOW	*win;
char	*fmt;
va_list	ap; {

	char	buf[512];

	vsprintf(buf, fmt, ap);
	return waddstr(win, buf);
}
