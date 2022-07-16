/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:validwin.c	1.1"
#include "cvttam.h"

TAMWIN *
_validwindow (wn)
short wn;
{
  if ((wn > -1) && (wn < NWINDOW) && Scroll(int2TamWin(wn))) {
    return (int2TamWin(wn));
  }
  return (OK);
}
