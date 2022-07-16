/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wsetstat.c	1.3"
#include "cvttam.h"

#define MIN(a,b) ((a<b)?a:b)

int
TAMwsetstat (wn, wsp)
short wn;
WSTAT *wsp;
{
  WINDOW *ws;	/* New window being passed to wcreate */
  TAMWIN *tw;
  int h, w;
  int y, x;

  /* Make sure this is a valid window */

  if ((tw = _validwindow (wn)) &&
      _winsize ((int)(Begy(wsp)), (int)(Begx(wsp)), (int)(Height(wsp)), (int)(Width(wsp)), (int)(Uflags(wsp)))) {

    if ((ws = newwin (Height(wsp), Width(wsp), Begy(wsp), Begx(wsp)))
	 == (WINDOW *)0) {
      return (ERR);
    }
    getmaxyx (Scroll(tw), h, w);
    getyx (Scroll(tw), y, x);
    h = MIN (Height(wsp), h)-1;
    w = MIN (Width(wsp), w)-1;
    (void)copywin (Scroll(tw), ws, 0, 0, 0, 0, h, w, FALSE);
    (void)wdelete (tw);
    /* Update other windows without refresh */
    /* NOTE: First thing wcreate does is make CurrentWin noncurrent, so */
    /* don't do it here. */
    for (tw=LastWin; tw!=CurrentWin; tw=Next(tw)) {
      _noncurrent (tw);
    }
    wn = wcreate (Begy(wsp), Begx(wsp), Height(wsp), Width(wsp), Uflags(wsp), TRUE);
    /* wn become CurrentWin */
    (void)copywin (ws, Scroll(CurrentWin), 0, 0, 0, 0,
		 Height(wsp)-1, Width(wsp)-1, FALSE);
    (void)wmove (Scroll(CurrentWin),MIN(y,Height(wsp)-1),MIN(x,Width(wsp)-1));
    (void)wnoutrefresh (Scroll(CurrentWin));
    (void)delwin (ws);

    /* Display this windows associated windows (cmd, prompt, slk) */

    _post (CurrentWin);
    (void)doupdate ();

    return (OK);
  }
  return (ERR);
}
