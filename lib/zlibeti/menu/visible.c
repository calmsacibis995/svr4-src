/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/visable.c	1.5"
#include "private.h"

/* Check to see if an item is being displayed on the current page */

int
item_visible (i)
register ITEM *i;
{
  register int bottom;
  register MENU *m;

  if (!i || !Imenu(i)) {
    return FALSE;
  }
  m = Imenu(i);
  if (Posted(m)) {
    bottom = Top(m) + Height(m) - 1;
    if (Y(i) >= Top(m) && Y(i) <= bottom) {
      return (TRUE);
    }
  }
  return (FALSE);
}
