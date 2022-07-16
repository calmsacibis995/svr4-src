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

#ident	"@(#)xcplxcurses:addstr.c	1.1"

/*
 *	@(#) addstr.c 1.1 90/03/30 lxcurses:addstr.c
 */
# include	"ext.h"

/*
 *	This routine adds a string starting at (_cury,_curx)
 *
 * 1/26/81 (Berkeley) @(#)addstr.c	1.1
 */
waddstr(win,str)
reg WINDOW	*win; 
reg char	*str;
{
# ifdef DEBUG
	fprintf(outf, "WADDSTR(\"%s\")\n", str);
# endif
	while (*str)
		if (waddch(win, *str++) == ERR)
			return ERR;
	return OK;
}
