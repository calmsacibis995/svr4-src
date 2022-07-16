/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/menuwin.c	1.2"
#include "private.h"

int
set_menu_win (m, win)
MENU *m;
WINDOW *win;
{
  if (m) {
    if (Posted(m)) {
      return E_POSTED;
    }
    UserWin(m) = win;
    /* Call scale because the menu subwindow may not be defined */
    _scale (m);
  }
  else {
    UserWin(Dfl_Menu) = win;
  }
  return E_OK;
}

WINDOW *
menu_win (m)
register MENU *m;
{
  return UserWin((m) ? m : Dfl_Menu);
}
