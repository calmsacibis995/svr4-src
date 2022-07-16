/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wdelete.c	1.1"
#include "cvttam.h"

int
wdelete (tw)
TAMWIN *tw;
{
  if (tw) {

    /* First free up curses windows */

    _undowindow (Scroll(tw));
    if (Border(tw)) {
      _undowindow (Border(tw));
    }
    Scroll(tw) = (WINDOW *)0;
    Border(tw) = (WINDOW *)0;

    /* Delete the TAMWIN from the used list and add it to the free list */

    tw = _listdel (&UsedWin, tw);
    _listadd (&FreeWin, tw);
    return (OK);
  }
  return (ERR);
}

int
TAMwdelete (w)
short w;		/* Index of window to be deleted */
{
  int i;
  TAMWIN *tw;

  if (tw = _validwindow (w)) {
    if (Cmd(tw)) {
      free (Cmd(tw));
      Cmd(tw) = NULL;
    }
    if (Prompt(tw)) {
      free (Prompt(tw));
      Prompt(tw) = NULL;
    }
    if (Label(tw)) {
      free (Label(tw));
      Label(tw) = NULL;
    }
    if (User(tw)) {
      free (User(tw));
      User(tw) = NULL;
    }
    for (i=NFKEYS; i--;) {
      Slk0Char(tw, i, 0) = '\0';
      Slk1Char(tw, i, 0) = '\0';
    }
    i = wdelete (tw);

    /* Remove the window from the screen by refreshing all other windows */

    (void)TAMwrefresh (-1);
    return (i);
  }
  return (ERR);
}
