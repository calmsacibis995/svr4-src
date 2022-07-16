/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/menusub.c	1.1"
#include "private.h"

int
set_menu_sub (m, sub)
MENU *m;
WINDOW *sub;
{
  if (m) {
    if (Posted(m)) {
      return E_POSTED;
    }
    UserSub(m) = sub;
    /* Since window size has changed go recalculate sizes */
    _scale (m);
  }
  else {
    UserSub(Dfl_Menu) = sub;
  }
  return E_OK;
}

WINDOW *
menu_sub (m)
register MENU *m;
{
  return UserSub((m) ? m : Dfl_Menu);
}
