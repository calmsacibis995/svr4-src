/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sti:menu/affect.c	1.4"
#include "private.h"

/* This routine checks the supplied values against the values of */
/* current and top items.  If a value has changed then one of */
/* terminate routines is called.  Following the actual change of */
/* the value is the calling of the initialize routines. */

void
_affect_change (m, newtop, newcurrent)
register MENU *m;
register int newtop;
register ITEM *newcurrent;
{
  register ITEM *oldcur;
  register int topchange=FALSE, curchange=FALSE;

  if (Posted(m)) {	/* Call term and init routines if posted */

    /* If current has changed terminate the old item. */
    if (newcurrent != Current(m)) {
      Iterm(m);
      curchange = TRUE;
    }

    /* If top has changed then call menu init function */
    if (newtop != Top(m)) {
      Mterm(m);
      topchange = TRUE;
    }

    oldcur = Current(m);
    Top(m) = newtop;
    Current(m) = newcurrent;

    if (topchange) {
      Minit(m);			/* Init the new page if top has changed */
    }

    if (curchange) {
      _movecurrent (m, oldcur);	/* Unmark the old item and mark */
				  /* the new one */
      Iinit(m);			/* Init the new item if current changed */
    }
    if (topchange || curchange) {	/* If anything changed go change user's */
      _show (m);			/* copy of menu */
    }
    else {
      _position_cursor (m);
    }
  }
  else {	/* Just change Top and Current if not posted */
    Top(m) = newtop;
    Current(m) = newcurrent;
  }
}
