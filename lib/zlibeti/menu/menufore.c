/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/menufore.c	1.3"
#include "private.h"

int
set_menu_fore(m, attr)
register MENU *m;
chtype attr;
{
  if (InvalidAttr(attr)) {
    return E_BAD_ARGUMENT;
  }
  if (m) {
    Fore(m) = attr;
    if (Posted(m)) {
      _draw (m);		/* Go redraw the menu and */
      _show (m);		/* redisplay it. */
    }
  }
  else {
    Fore(Dfl_Menu) = attr;
  }
  return E_OK;
}

chtype
menu_fore (m)
register MENU *m;
{
  return Fore(m ? m : Dfl_Menu);
}
