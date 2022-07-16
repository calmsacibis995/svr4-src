/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wuser.c	1.1"
#include "cvttam.h"
#include <string.h>

int
TAMwuser (wn, c)
short wn;
char *c;
{
  TAMWIN *tw;

  if (tw = _validwindow (wn)) {
    if (User(tw)) {
      free (User(tw));
    }
    User(tw) = NULL;
    if (c && c[0] != '\0') {
      if ((User(tw) = malloc ((unsigned)(strlen (c)+1))) == NULL) {
	return (ERR);
      }
      (void)strcpy (User(tw), c);
      if (tw == CurrentWin) {
	/* Update the user line if this is the current window */
	_current (tw);
	(void)doupdate ();
      }
    }
    return (OK);
  }
  return (ERR);
}
