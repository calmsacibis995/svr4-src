/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wexit.c	1.2"
#include "cvttam.h"

extern int _Firsttime;

void
TAMwexit (s)
int s;
{
  TAMWIN *tw;

  /* Delete all the windows before exitting.  Note that in the following */
  /* loop that LastWin is updated each time a window is deleted and */
  /* therefore becomes the next window to be deleted. */

  if (!_Firsttime) {
    for (;tw=LastWin;) {
      (void)TAMwdelete (Id(tw));
    }
    endwin ();
  }
  exit (s);
}
