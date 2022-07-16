/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wcreate.c	1.2"
#include "cvttam.h"

/* Create a window without updating the screen. */
/* Note that posting is not needed since labels and */
/* prompts could not have been created yet. */

int
wcreate (r, c, h, w, f, slk)
short r, c;		/* Row and column locations */
short h, w;		/* Height and width of window */
unsigned short f;	/* Window flags */
int slk;		/* Don't zero slks if TRUE */
{
  register int i;
  TAMWINLIST *twl;
  TAMWIN *tw;
  WINDOW *wns;
  WINDOW *wnb;

  twl = &FreeWin;
  if (Head(twl)) {

    if (!(f & NBORDER)) {
      if ((wnb = newwin (h+2, w+2, r, c)) == (WINDOW *)0) {
	return (ERR);
      }
      if ((wns = newwin (h, w, r+1, c+1)) == (WINDOW *)0) {
	(void)delwin (wnb);
	return (ERR);
      }
    }
    else {
      if ((wns = newwin (h, w, r, c)) == (WINDOW *)0) {
	return (ERR);
      }
    }

    /* Make the current window noncurrent */

    _noncurrent (CurrentWin);

    /* Get TAMWIN from free list and add it to used list */

    tw = _listdel (&FreeWin, Tail(&FreeWin));
    _listadd (&UsedWin, tw);

    /* Create needed window structures */

    State(tw) = 0;

    /* Create the scrolling portion of the screen */

    Scroll(tw) = wns;

    scrollok (Scroll(tw), TRUE);

    if (!(f & NBORDER)) {
      /* A border is indicated, so add a window with a border */
      Uflags(Wstat(tw)) &= ~NBORDER;	/* Turn off NBORDER flag */
      Border(tw) = wnb;
    }
    else {
      Border(tw) = (WINDOW *)0;	/* Assume for now no border */
      Uflags(Wstat(tw)) |= NBORDER;
    }

    /* Save the size of the window in the TAMWIN structure */

    Begy(Wstat(tw)) = r;
    Begx(Wstat(tw)) = c;
    Height(Wstat(tw)) = h;
    Width(Wstat(tw)) = w;

    if (!slk) {
      for (i=NFKEYS; i--;) {
	Slk0Char(tw, i, 0) = '\0';
	Slk1Char(tw, i, 0) = '\0';
      }
    }

    _current (tw);		/* Make this the current window */

    /* Set or reset the keypad flag for this window */

    (void)keypad (Scroll(tw), Keypad);

    return (TamWin2int(tw));
  }
  /* What do we return when no more windows are available? */
  return (ERR);
}

int
TAMwcreate (r, c, h, w, f)
short r, c;		/* Row and column locations */
short h, w;		/* Height and width of window */
unsigned short f;	/* Window flags */
{
  int i;
  TAMWIN *tw;

  if (_winsize ((int)r, (int)c, (int)h, (int)w, (int)f)) {
    i = wcreate (r, c, h, w, f, FALSE);
    if (tw = _validwindow (i)) {
      _post (tw);
    }
    (void)doupdate ();
    return (i);
  }
  return (ERR);
}
