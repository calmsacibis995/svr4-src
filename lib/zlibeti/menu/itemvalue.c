/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/itemvalue.c	1.5"
#include "private.h"

int
set_item_value (i, v)
ITEM *i;
register int v;
{
  /* Values can only be set on active values */
  if (i) {
    if (!Selectable(i) || (Imenu(i) && OneValue(Imenu(i)))) {
      return E_REQUEST_DENIED;
    }
    if (Value(i) != v) {
      Value(i) = v;
      if (Imenu(i) && Posted(Imenu(i))) {
	_move_post_item (Imenu(i), i);
	_show(Imenu(i));
      }
    }
  }
  else {
    Value(Dfl_Item) = v;
  }
  return E_OK;
}

int
item_value (i)
register ITEM *i;
{
  return Value(i ? i : Dfl_Item);
}
