/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/post.c	1.12"
#include "private.h"

void
_post_item (m, k)
MENU *m;
register ITEM *k;
{
  int foreon = FALSE;
  int backon = FALSE;
  int greyon = FALSE;
  chtype c;
  register int i;

  /* Display the mark region of the item */

  if (!Selectable(k)) {
    (void)wattron (Win(m), Grey(m));
    greyon = TRUE;
    for (i=Marklen(m); i>0; i--) {
      (void)waddch (Win(m), ' ');
    }
  }
  else {
    if (Value(k) || k==Current(m)) {
      (void)wattron (Win(m), Fore(m)); 
      foreon = TRUE;
    }
    else {
      (void)wattron (Win(m), Back(m));
      backon = TRUE;
    }

    /* Display the mark */
    if (Value(k) || (OneValue(m) && k==Current(m))) {
      if (Marklen(m)) {
	(void)waddstr (Win(m), Mark(m));
      }
    }
    else {
      for (i=Marklen(m); i>0; i--) {
	(void)waddch (Win(m), ' ');
      }
    }
  }

  /* Display the name */

  (void)waddnstr (Win(m), Name(k), MaxName(m));
  if (ShowDesc(m) && MaxDesc(m) != 0) {
    c = Pad(m);
  }
  else {
    c = ' ';
  }
  for (i=MaxName(m)-NameLen(k); i>0; i--) {
    (void)waddch (Win(m), c);
  }

  /* Display the description */

  if (ShowDesc(m) && MaxDesc(m) != 0) {
    (void)waddch (Win(m), Pad(m));
    if (DescriptionLen(k) != 0) {
      (void)waddstr (Win(m), Description(k));
    }
    for (i=MaxDesc(m)-DescriptionLen(k); i>0; i--) {
      (void)waddch (Win(m), ' ');
    }
  }
  if (foreon) {
    (void)wattroff (Win(m), Fore(m)); 
  }
  if (backon) {
    (void)wattroff (Win(m), Back(m)); 
  }
  if (greyon) {
    (void)wattroff (Win(m), Grey(m)); 
  }
}

void
_move_post_item (m, k)
register MENU *m;
register ITEM *k;
{
  (void)wmove (Win(m), Y(k), X(k)*(Itemlen(m)+1));
  _post_item (m, k);
}

int
unpost_menu (m)
register MENU *m;
{
  if (!m) {
    return E_BAD_ARGUMENT;
  }
  if (Indriver(m)) {
    return E_BAD_STATE;
  }
  if (!Posted(m)) {
    return E_NOT_POSTED;
  }
  Iterm(m);
  Mterm(m);
  (void)werase (US(m));
  wsyncup (US(m));
  (void)delwin (Sub(m));
  Sub(m) = (WINDOW *)0;
  (void)delwin (Win(m));
  Win(m) = (WINDOW *)0;
  ResetPost(m);
  return E_OK;
}

/* This routine draws the item indicated by oldcur first and then */
/* draws the item indicated by Current.  This will have the affect */
/* of unselecting the first item and selecting the next. */
void
_movecurrent (m, oldcur)
register MENU *m;
register ITEM *oldcur;
{
  if (oldcur != Current(m)) {
    _move_post_item (m, oldcur);
    _move_post_item (m, Current(m));
  }
}

/* Draw the entire menu into the super window */
/* This routine assumes all items have been linked and */
/* that the menu is in at least a pre-post state. */

void
_draw (m)
register MENU *m;
{
  register int k;
  register ITEM *i, *j;
  register ITEM *si, *sj;

  k = 0;		/* Line number */
  i = IthItem(m, 0);
  si = Cyclic(m) ? i : (ITEM *)0;
  do {
    (void)wmove (Win(m), k++, 0);
    j = i;
    sj = Cyclic(m) ? j : (ITEM *)0;
    do {
      _post_item (m, j);
      if ((j = Right(j)) != sj) {
	(void)waddch (Win(m), ' ');
      }
    } while (j != sj);
  } while ((i = Down(i)) != si);
}

int 
post_menu (m)
register MENU *m;
{
  register ITEM **ip;
  int r, c;		/* visible # of rows and cols */

  if (!m) {
    return E_BAD_ARGUMENT;
  }
  if (Indriver(m)) {
    return E_BAD_STATE;
  }
  if (Posted(m)) {
    return E_POSTED;
  }
  /* Make sure there is at least one item present */
  if (Items(m) && IthItem(m, 0)) {
    getmaxyx (US(m), r, c);
    /* Make sure the menu fits into the window horizontally */
    if (c < Width(m) || r < Height(m)) {
      return E_NO_ROOM;
    }

    /* Create the menu window and derived windows */

    if ((Win(m) = newwin (Rows(m), Width(m), 0, 0)) == (WINDOW *)0) {
      return E_SYSTEM_ERROR;
    }
    /* Take the minimum of the height of the menu (Height), the physical
       height of the window (r), and the number of rows in the menu (Rows). */
    r = min (min (r, Rows(m)), Height(m));
    if ((Sub(m) = derwin (Win(m), r, Width(m), 0, 0)) == (WINDOW *)0) {
      return E_SYSTEM_ERROR;
    }

    /* If needed, link all items in the menu */

    if (LinkNeeded(m)) {
      _link_items (m);
    }

    SetPost(m);

    /* If only one value can be set then unset all values to start. */
    if (OneValue(m)) {
      for (ip=Items(m); *ip; ip++) {
	Value(*ip) = FALSE;
      }
    }

    /* Go do the drawing of the menu */
    _draw (m);

    Minit(m);
    Iinit(m);
    _show (m);		/* Display the menu */
    return E_OK;
  }
  return E_NOT_CONNECTED;
}
