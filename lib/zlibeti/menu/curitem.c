/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/curitem.c	1.6"
#include "private.h"

int
set_current_item (m, current)
register MENU *m;
ITEM *current;
{
  int top;

  if (m && current && Imenu(current) == m) {
    if (Indriver(m)) {
      return E_BAD_STATE;
    }
    if (current != Current(m)) {
      if (LinkNeeded(m)) {
	_link_items (m);
      }

      top = Top(m);
      _chk_current (m, &top, current);

      /* Next clear the pattern buffer. */
      Pindex(m) = 0;
      IthPattern(m, Pindex(m)) = '\0';
      _affect_change (m, top, current);
    }
  }
  else {
    return (E_BAD_ARGUMENT);
  }
  return E_OK;
}

ITEM *
current_item (m)
register MENU *m;
{
  if (m && Items(m)) {
    return Current(m);
  }
  else {
    return (ITEM *)0;
  }
}

int
item_index (i)
register ITEM *i;
{
  if (i && Imenu(i)) {
    return Index(i);
  }
  else {
    return -1;
  }
}
