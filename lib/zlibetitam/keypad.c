/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:keypad.c	1.2"
#include "cvttam.h"

/*ARGSUSED*/
int
TAMkeypad (wn, flag)
int wn;
int flag;
{
  TAMWIN *tw;

  /* Set keypad to "keypadstate" for each window in used window list */

  Keypad = flag;
  for (tw=LastWin; tw; tw=Next(tw)) {
    (void)keypad (Scroll(tw), Keypad&1/*Keypad!=2?Keypad:0*/);
  }
  return (OK);
}
