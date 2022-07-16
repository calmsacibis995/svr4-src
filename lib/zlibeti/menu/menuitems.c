/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/menuitems.c	1.6"
#include "private.h"

int
set_menu_items (m, i)
register MENU *m;
register ITEM **i;
{
  if (!m) {
    return E_BAD_ARGUMENT;
  }
  if (i && *i == (ITEM *)0) {
    return E_BAD_ARGUMENT;
  }
  if (Posted(m)) {
    return E_POSTED;
  }

  if (Items(m)) {
    _disconnect (m);
  }
  if (i) {
    /* Go test the item and make sure its not already connected to */
    /* another menu and then connect it to this one. */
    if (!_connect (m, i)) {
      return E_CONNECTED;
    }
  }
  else {
    Items(m) = i;
  }
  return E_OK;
}

ITEM **
menu_items (m)
register MENU *m;
{
  if (!m) {
    return (ITEM **)0;
  }
  return (Items(m));
}
