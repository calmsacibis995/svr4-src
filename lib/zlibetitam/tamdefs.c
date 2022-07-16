/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:tamdefs.c	1.2"
#include "cvttam.h"

int
TAMbeep ()
{
  beep ();
  return (OK);
}

int
TAMclear (wn)
short wn;
{
  TAMWIN *tw;

  if (tw = _validwindow (wn)) {
    (void)werase (Scroll(tw));
    return (OK);
  }
  return (ERR);
}

int
TAMclrtobot (wn)
short wn;
{
  TAMWIN *tw;

  if (tw = _validwindow (wn)) {
    return (wclrtobot (Scroll(tw)));
  }
  return (ERR);
}

int
TAMclrtoeol (wn)
short wn;
{
  TAMWIN *tw;

  if (tw = _validwindow (wn)) {
    (void)wclrtoeol (Scroll(tw));
    return (OK);
  }
  return (ERR);
}

int
TAMdelch (wn)
short wn;
{
  TAMWIN *tw;

  if (tw = _validwindow (wn)) {
    (void)wdelch (Scroll(tw));
    return (OK);
  }
  return (ERR);
}

int
TAMdeleteln (wn)
short wn;
{
  TAMWIN *tw;

  if (tw = _validwindow (wn)) {
    return (wdeleteln (Scroll(tw)));
  }
  return (ERR);
}

int
TAMinsch (wn, c)
short wn;
{
  TAMWIN *tw;
  int ret;

  if (tw = _validwindow (wn)) {
    ret = winsch (Scroll(tw), (chtype)c);
    return (ret);
  }
  return (ERR);
}

int
TAMinsertln (wn)
short wn;
{
  TAMWIN *tw;

  if (tw = _validwindow (wn)) {
    (void)winsertln (Scroll(tw));
    return (OK);
  }
  return (ERR);
}
