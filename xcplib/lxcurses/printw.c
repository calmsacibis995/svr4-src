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

#ident	"@(#)xcplxcurses:printw.c	1.1"

/*
 *	$Header: RCS/printw.c,v 1.2 88/04/26 12:21:10 root Exp $
 */
/*
 *	MODIFICATION HISTORY
 *	M000	15 Mar 1985	ncm
 *	- Removed references to _IOSTRG, it is no longer used.
 *	M001	19 Mar 1985	ncm
 *	- Changed include file from curses.ext to ext.h
 */
/*
 * printw and friends
 *
 * 1/26/81 (Berkeley) @(#)printw.c	1.1
 */

# include	"ext.h"		/* M001 */
#define MAXINT 32767

/*
 *	This routine implements a printf on the standard screen.
 */
printw(fmt, args)
char	*fmt;
int	args; {

	return _sprintw(stdscr, fmt, &args);
}

/*
 *	This routine implements a printf on the given window.
 */
wprintw(win, fmt, args)
WINDOW	*win;
char	*fmt;
int	args; {

	return _sprintw(win, fmt, &args);
}
/*
 *	This routine actually executes the printf and adds it to the window
 *
 *	This is really a modified version of "sprintf".  As such,
 * it assumes that sprintf interfaces with the other printf functions
 * in a certain way.  If this is not how your system works, you
 * will have to modify this routine to use the interface that your
 * "sprintf" uses.
 */
_sprintw(win, fmt, args)
WINDOW	*win;
char	*fmt;
int	*args; {

	FILE	junk;
	char	buf[512];

	/* junk._flag = _IOWRT + _IOSTRG;	M000 */
	junk._file = _NFILE;
	junk._flag = _IOWRT;
	junk._base = junk._ptr = (unsigned char *)buf;
	junk._cnt = MAXINT;
	_doprnt(fmt, args, &junk);
	*junk._ptr = '\0'; /* plant terminating null character */
	return waddstr(win, buf);
}
