/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:mtrunc.c	1.1"
#include "tam.h"
#include "track.h"
#include <string.h>

/****************************************************************************

  mtrunc(s1,s2,n)	- truncate s2 and put it in s1.

  If the string doesn't fit, put a ">" in the last position.  If
  N is zero, no truncation occurs.

****************************************************************************/

mtrunc(s1,s2,n)
char *s1, *s2;
int n;
{
  int i;

  if (n == 0) {
    (void)strcpy(s1,s2);
  }
  else {
    i = strlen(s2);
    (void)strncpy(s1,s2,n+1);
    if (i>n) {
      s1[n-1] = '>';
      s1[n] = '\0';
    }
  }
}
