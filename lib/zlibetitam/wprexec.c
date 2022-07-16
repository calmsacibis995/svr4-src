/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wprexec.c	1.1"
#include "cvttam.h"

int
TAMwprexec ()
{
  (void)wclear (stdscr);
  (void)wrefresh (stdscr);
  reset_shell_mode ();
  return (OK);
}
