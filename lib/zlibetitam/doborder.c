/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:doborder.c	1.3"
#include "cvttam.h"

void
_noncurrent (tw)
TAMWIN *tw;
{
  int width, height;

  if (tw) {
    if (Border(tw)) {
      (void)wborder (Border(tw),'.','.','.','.','.','.','.','.');
      if (Label(tw)) {
	getmaxyx (Border(tw), height, width);
	(void)mvwaddnstr (Border(tw), 0, 1, Label(tw), width-1);
      }
      (void)wnoutrefresh (Border(tw));
    }
    (void)touchwin (Scroll(tw));
    (void)wnoutrefresh (Scroll(tw));
  }
  wncur = -1;
}

void
_current (tw)
TAMWIN *tw;
{
  chtype hc, vc, cc;
  int width, height;

  if (tw) {
    if (Border(tw)) {
      cc = (Reverse) ? ' '|A_REVERSE : '+';
      hc = (Reverse) ? ' '|A_REVERSE : '-';
      vc = (Reverse) ? ' '|A_REVERSE : '|';
      (void)wborder (Border(tw), vc, vc, hc, hc, cc, cc, cc, cc);
      /* Put in top line of border */
      if (Reverse) {
	wattron (Border(tw), A_REVERSE);
      }
      getmaxyx (Border(tw), height, width);
      if (Label(tw)) {
	(void)mvwaddnstr (Border(tw), 0, 1, Label(tw), width-1);
      }
      if (Reverse) {
	wattroff (Border(tw), A_REVERSE);
      }
      (void)wnoutrefresh (Border(tw));
    }

    /* Redraw scrolling portion of screen */

    (void)touchwin (Scroll(tw));
    (void)wnoutrefresh (Scroll(tw));

    /* Update current window  */

    wncur = Id(tw);
  }
}

/* This routine draws the border around the window tw and puts periods */
/* around the window CurrentWin.  It also displays the label and user */
/* portions of the border associated with the window. */

void
_doborder (tw)
TAMWIN *tw;
{
  /* Redraw the border of the current window and the border of tw */

  _noncurrent (CurrentWin);		/* Undo current window's border */
  _current (tw);			/* Make tw's border current */
}
