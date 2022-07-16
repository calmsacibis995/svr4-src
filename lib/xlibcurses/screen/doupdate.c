/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/doupdate.c	1.7"
#include	"curses_inc.h"

/*
 * Doupdate is a real function because _virtscr
 * is not accessible to application programs.
 */

doupdate()
{
    return (wrefresh(_virtscr));
}
