/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:compat.c	1.1"
#include "cvttam.h"

static int Echo = TRUE;

int
TAMinitscr ()
{
  TAMwinit ();
  /* Create a full size window which becomes the current window */
  /* and stdscr. */
  if (TAMwcreate (0, 0, LINES, COLS, NBORDER) != -1) {
    return (OK);
  }
  return (ERR);
}

int
TAMcbreak ()
{
  cbreak ();
  return (OK);
}

int
TAMnocbreak ()
{
  nocbreak ();
  return (OK);
}

int
TAMecho ()
{
  Echo = TRUE;
  return (OK);
}

int
TAMnoecho ()
{
  Echo = FALSE;
  return (OK);
}

int
TAMinch ()
{
  TAMWIN *tw;

  if (tw = _validwindow (wncur)) {
    return (winch (Scroll(tw)));
  }
  return (ERR);
}

int
TAMgetch ()
{
  TAMWIN *tw;
  int i;

  if (tw = _validwindow (wncur)) {
    if (Echo) {
      echo ();
      i = wgetch (Scroll(tw));
      noecho ();
    }
    else {
      i = wgetch (Scroll(tw));
    }
    return (i);
  }
  return (ERR);
}

int
TAMflushinp ()
{
  flushinp ();
  return (OK);
}

int
TAMattron (m)
long m;
{
  TAMWIN *tw;

  if (tw = _validwindow (wncur)) {
    wattron (Scroll(tw), (chtype)m);
    return (OK);
  }
  return (ERR);
}

int
TAMattroff (m)
long m;
{
  TAMWIN *tw;

  if (tw = _validwindow (wncur)) {
    wattroff (Scroll(tw), (chtype)m);
    return (OK);
  }
  return (ERR);
}

int
TAMsavetty ()
{
  savetty ();
  return (OK);
}

int
TAMresetty ()
{
  resetty ();
  return (OK);
}
