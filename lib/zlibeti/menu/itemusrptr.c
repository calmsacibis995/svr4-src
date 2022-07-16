/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/itemusrptr.c	1.1"
#include "private.h"

int
set_item_userptr (i, u)
register ITEM *i;
char *u;
{
  if (i) {
    Iuserptr(i) = u;
  }
  else {
    Iuserptr(Dfl_Item) = u;
  }
  return E_OK;
}

char *
item_userptr (i)
register ITEM *i;
{
  return Iuserptr(i ? i : Dfl_Item);
}
