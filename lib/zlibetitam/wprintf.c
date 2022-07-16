/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wprintf.c	1.4"
#include "cvttam.h"
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

int
#ifdef __STDC__
TAMwprintf (short w, char *format, ...)
#else
TAMwprintf (w, format, va_alist)
short w;		/* Window to be printed */
char *format;
va_dcl
#endif
{
  va_list ap;
  int i;
  TAMWIN *tw;

  if (tw = _validwindow (w)) {
#ifdef __STDC__
    va_start (ap, format);
#else
    va_start (ap);
#endif
    i = vwprintw (Scroll(tw), format, ap);
    if (CurrentWin == tw) {
      (void)wrefresh (Scroll(tw));
    }
    va_end(ap);
    return (i);
  }
  return (-1);
}

int
#ifdef __STDC__
TAMprintw (char *format, ...)
#else
TAMprintw (format, va_alist)
char *format;
va_dcl
#endif
{
  va_list ap;
  int i;

  if (CurrentWin) {
#ifdef __STDC__
    va_start (ap, format);
#else
    va_start (ap);
#endif
    i = vwprintw (Scroll(CurrentWin), format, ap);
    (void)wrefresh (Scroll(CurrentWin));
    va_end(ap);
    return (i);
  }
  return (-1);
}
