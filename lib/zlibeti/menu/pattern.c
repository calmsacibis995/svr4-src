/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/pattern.c	1.13"
#include "private.h"
#include <ctype.h>

static int
substr (m, s1, s2)
register MENU *m;
register char *s1, *s2;
{
  if (IgnoreCase(m)) {
    while (*s1 && *s2) {
      if (toupper(*s1++) != toupper(*s2++)) {
	return FALSE;
      }
    }
  }
  else {
    while (*s1 && *s2) {
      if (*s1++ != *s2++) {
	return FALSE;
      }
    }
  }
  if (*s1) {
    return FALSE;
  }
  return TRUE;
}

int
_match (m, c, current)
register MENU *m;
register int c;
register ITEM **current;
{
  register int i, j;
  register int found;
  int cycled;			/* Indicates search has cycled past */
				/* the current item.  If the current */
				/* item is matched after cycled is */
				/* true then NO_MATCH results. */

  /* If a backspace is encountered then search backward from the */
  /* current item.  Otherwise, search forward from the current item. */

  i = Index(*current);
  if (c && c != '\b') {		/* c could be null */
    if (Pindex(m)+1 > MaxName(m)) {
      return E_NO_MATCH;
    }
    IthPattern(m, Pindex(m)) = c;
    IthPattern(m, ++Pindex(m)) = '\0';
    if (--i < 0) {
      i = Nitems(m)-1;
    }
  }
  j = i;
  found = FALSE;
  cycled = FALSE;
  do {
    if (c == '\b') {
      if (--i < 0) {
	i = Nitems(m)-1;
      }
    }
    else {
      if (++i >= Nitems(m)) {
	i = 0;
      }
    }
    if (substr (m, Pattern(m), Name(IthItem(m, i)))) {
      found = TRUE;
      break;
    }
    cycled = TRUE;
  } while (i != j);
  if (found) {
    if (i == Index(*current) && cycled) {
      return E_NO_MATCH;
    }
    *current = IthItem(m, i);
  }
  else {
    if (c && c != '\b') {
      Pindex(m) -= 1;
      IthPattern(m, Pindex(m)) = '\0';
    }
    return E_NO_MATCH;
  }
  return E_OK;
}

char *
menu_pattern (m)
register MENU *m;
{
  if (m) {
    if (Pattern(m)) {
      return Pattern(m);
    }
    else {
      return "";
    }
  }
  else {
    return NULL;
  }
}

int
set_menu_pattern (m, s)
register MENU *m;
register char *s;
{
  int top;
  ITEM *current;

  if (!m || !s) {
    return E_BAD_ARGUMENT;
  }
  if (!Items(m)) {
    return E_NOT_CONNECTED;
  }
  if (Indriver(m)) {
    return E_BAD_STATE;
  }
  IthPattern(m, 0) = '\0';
  Pindex(m) = 0;
  if (*s == '\0') {
    _position_cursor (m);
    return E_OK;
  }
  if (LinkNeeded(m)) {
    _link_items (m);
  }
  top = Top(m);
  current = Current(m);
  for (; *s; s++) {
    if (_match (m, *s, &current) != E_OK) {
      IthPattern(m, 0) = '\0';
      Pindex(m) = 0;
      _position_cursor (m);
      return E_NO_MATCH;
    }
  }
  _chk_current (m, &top, current);
  _affect_change (m, top, current);
  return E_OK;
}
