/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/_border.c	1.1"

#define		NOMACROS
#include	"curses_inc.h"

border(ls, rs, ts, bs, tl, tr, bl, br)
chtype	ls, rs, ts, bs, tl, tr, bl, br;
{
    return (wborder(stdscr, ls, rs, ts, bs, tl, tr, bl, br));
}
