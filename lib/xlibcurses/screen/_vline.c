/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_vline.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

vline(vertch, num_chars)
chtype	vertch;
int	num_chars;
{
    return (wvline(stdscr, vertch, num_chars));
}
