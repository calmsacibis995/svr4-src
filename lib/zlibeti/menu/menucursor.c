/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/menucursor.c	1.5"
#include "private.h"

/* Position the cursor in the user's subwindow. */

void
_position_cursor (m)
register MENU *m;
{
  register int y, x;
  register WINDOW *us, *uw;

  if (Posted(m)) {
    /* x and y represent the position in our subwindow */
    y = Y(Current(m)) - Top(m);
    x = X(Current(m))*(Itemlen(m)+1);
    if (ShowMatch(m)) {
      if (Pindex(m)) {
	x += Pindex(m) + Marklen(m) - 1;
      }
    }
    uw = UW(m);
    us = US(m);
    (void)wmove (us, y, x);
    if (us != uw) {
      wcursyncup (us);
      wsyncup (us);
      /* The next statement gets around some aberrant behavior in curses. */
      /* The subwindow is never being untouched and this results in the */
      /* parent window being touched every time a syncup is done. */
      (void)untouchwin (us);
    }
  }
}

int
pos_menu_cursor (m)
MENU *m;
{
  if (!m) {
    return E_BAD_ARGUMENT;
  }
  if (!Posted(m)) {
    return E_NOT_POSTED;
  }
  _position_cursor (m);
  return E_OK;
}
