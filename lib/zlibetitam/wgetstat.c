/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wgetstat.c	1.1"
#include "cvttam.h"

int
TAMwgetstat (w, s)
short w;
WSTAT *s;
{
  TAMWIN *tw;

  if (tw = _validwindow(w)) {
    Begy(s) = Begy(Wstat(tw));
    Begx(s) = Begx(Wstat(tw));
    Height(s) = Height(Wstat(tw));
    Width(s) = Width(Wstat(tw));
    Uflags(s) = Uflags(Wstat(tw));
    return (OK);
  }
  return (ERR);
}
