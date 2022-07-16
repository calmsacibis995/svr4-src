/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/connect.c	1.8"
#include "private.h"


/* Connect and disconnect an item list from a menu */


/* Find the maximum length name and description */

static void
maxlengths (m)
MENU *m;
{
  register int maxn, maxd;
  register ITEM **ip;

  maxn = maxd = 0;
  for (ip=Items(m); *ip; ip++) {
    if (NameLen(*ip) > maxn) {
      maxn = NameLen(*ip);
    }
    if (DescriptionLen(*ip) > maxd) {
      maxd = DescriptionLen(*ip);
    }
  }
  MaxName(m) = maxn;
  MaxDesc(m) = maxd;
}

int
_connect (m, items)
register MENU *m;
register ITEM **items;
{
  register ITEM **ip;
  register int i;

  /* Is the list of items connected to any other menu? */
  for (ip=items; *ip; ip++) {
    if (Imenu(*ip)) {		/* Return Null if item points to a menu */
      return FALSE;
    }
  }
  for (i=0, ip=items; *ip; ip++) {
    if (Imenu(*ip)) {			/* Return FALSE if this item is */
      for (i=0, ip=items; *ip; ip++) {	/* a previous item. */
	Index(*ip) = 0;		/* Reset index and menu pointers */
	Imenu(*ip) = (MENU *)0;
      }
      return FALSE;		
    }
    if (OneValue(m)) {
      Value(*ip) = FALSE;	/* Set all values to FALSE if selection */
    }				/* not allowed. */
    Index(*ip) = i++;
    Imenu(*ip) = m;
  }
  Nitems(m) = i;
  Items(m) = items;
  /* Go pick up the sizes of names and descriptions */
  maxlengths (m);
  /* Set up match buffer */
  if ((Pattern(m) = (char *)malloc ((unsigned)MaxName(m)+1)) == (char *)0) {
    return FALSE;
  }
  IthPattern(m, 0) = '\0';
  Pindex(m) = 0;
  (void)set_menu_format (m, FRows(m), FCols(m));
  Current(m) = IthItem(m, 0);
  Top(m) = 0;
  return TRUE;
}

void
_disconnect (m)
register MENU *m;
{
  register ITEM **ip;

  for (ip=Items(m); *ip; ip++) {
    /* Release items for another menu */
    Imenu(*ip) = (MENU *)0;
  }
  free (Pattern(m));
  Pattern(m) = NULL;
  Items(m) = (ITEM **)0;
  Nitems(m) = 0;
}
