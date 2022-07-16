/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibcurses:scanw.c	1.1.1.1"

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
static	char sccsid[] = "@(#)scanw.c 1.8 88/02/08 SMI"; /* from UCB 5.1 85/06/07 */
#endif not lint

# include	<varargs.h>

/*
 * scanw and friends
 *
 */

# include	"curses.ext"

/*
 *	This routine implements a scanf on the standard screen.
 */
/*VARARGS1*/
scanw(fmt, va_alist)
char	*fmt;
va_dcl	{
	va_list ap;

	va_start(ap);
	return _sscans(stdscr, fmt, ap);
}
/*
 *	This routine implements a scanf on the given window.
 */
/*VARARGS2*/
wscanw(win, fmt, va_alist)
WINDOW	*win;
char	*fmt;
va_dcl	{
	va_list ap;

	va_start(ap);
	return _sscans(win, fmt, ap);
}
/*
 *	This routine actually executes the scanf from the window.
 *
 *	This is really a modified version of "sscanf".  As such,
 * it assumes that sscanf interfaces with the other scanf functions
 * in a certain way.  If this is not how your system works, you
 * will have to modify this routine to use the interface that your
 * "sscanf" uses.
 */
_sscans(win, fmt, ap)
WINDOW	*win;
char	*fmt;
va_list	ap; {

	char	buf[100];
	FILE	junk;

	junk._flag = _IOREAD|_IOWRT;
	junk._base = junk._ptr = (unsigned char *)buf;
	if (wgetstr(win, buf) == ERR)
		return ERR;
	junk._cnt = strlen(buf);
	return _doscan(&junk, fmt, ap);
}
