/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/newmenu.c	1.7"
#include "private.h"

MENU *
new_menu (items)
ITEM **items;
{
  MENU *m;

  if ((m = (MENU *)calloc (1, sizeof (MENU))) != (MENU *)0) {
    *m = *Dfl_Menu;
    Rows(m) = FRows(m);
    Cols(m) = FCols(m);
    if (items) {
      if (*items == (ITEM *)0 || !_connect (m, items)) {
	free (m);
	return (MENU *)0;
      }
    }
    return (m);
  }
  return ((MENU *)0);
}

int
free_menu(m)
register MENU *m;
{
  if (!m) {
    return E_BAD_ARGUMENT;
  }
  if (Posted(m)) {
    return E_POSTED;
  }
  if (Items(m)) {
    _disconnect (m);
  }
  free(m);
  return E_OK;
}
