/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/menumark.c	1.3"
#include "private.h"

int
set_menu_mark (m, mark)
register MENU *m;
register char *mark;
{
  register int len;	/* Length of mark */

  if (mark && *mark) {
    len = strlen (mark);
  }
  else {
    return E_BAD_ARGUMENT;
  }
  if (m) {
    if (Posted(m) && len != Marklen(m)) {
      return E_BAD_ARGUMENT;
    }
    Mark(m) = mark;
    Marklen(m) = len;
    if (Posted(m)) {
      _draw (m);		/* Redraw menu */
      _show (m);		/* Redisplay menu */
    }
    else {
      _scale (m);		/* Redo sizing information */
    }
  }
  else {
    Mark(Dfl_Menu) = mark;
    Marklen(Dfl_Menu) = len;
  }
  return E_OK;
}

char *
menu_mark (m)
register MENU *m;
{
  return Mark(m ? m : Dfl_Menu);
}
