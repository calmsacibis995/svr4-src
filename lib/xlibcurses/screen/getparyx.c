/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/getparyx.c	1.2"
#include	"curses_inc.h"

getpary(win)
WINDOW	*win;
{
    return (win->_pary);
}

getparx(win)
WINDOW	*win;
{
    return (win->_parx);
}
