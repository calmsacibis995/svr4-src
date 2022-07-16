/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:listop.c	1.1"
#include "cvttam.h"

TAMWIN *
_listdel (list, lp)
TAMWINLIST *list;
TAMWIN *lp;		/* List item to be expurgated from list */
{
  if (Next(lp)) {
    Last(Next(lp)) = Last(lp);
  }
  if (Last(lp)) {
    Next(Last(lp)) = Next(lp);
  }
  if (Head(list) == lp) {
    Head(list) = Next(lp);
  }
  if (Tail(list) == lp) {
    Tail(list) = Last(lp);
  }
  Next(lp) = (TAMWIN *)0;
  Last(lp) = (TAMWIN *)0;
  return (lp);
}

void
_listadd (list, lp)
TAMWINLIST *list;
TAMWIN *lp;		/* List item to be add to list */
{
  lp->last = Tail(list);
  if (Tail(list)) {
    Next(Tail(list)) = lp;
  }
  if (!Head(list)) {
    Head(list) = lp;
  }
  Next(lp) = (TAMWIN *)0;
  Tail(list) = lp;
}
