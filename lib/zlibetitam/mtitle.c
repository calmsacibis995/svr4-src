/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:mtitle.c	1.1"
#include "tam.h"
#include "menu.h"
#include <string.h>

/****************************************************************************

  mtitle(m,width)	- display a title

****************************************************************************/

mtitle(m,width)
register menu_t *m;
int width;
{
  register char *cp;
  register char *title;
  char temp[10*M_MAXLINE],temp2[M_MAXLINE];
  int len,margin;
  int i = 0;

  if (m->m_title == 0) {
    return;
  }

  (void)strcpy(temp,m->m_title);
  cp = temp;

  while (title = strtok(cp,"\n")) {
    cp = 0;
    len = strlen(title);
    mtrunc(temp2,title, width-2*M_LRMARGIN);
    margin = ((width - 2*M_LRMARGIN) - len) / 2 + M_LRMARGIN;
    if (margin < 0) {
      margin = 0;
    }

    if (m->m_flags & M_ASISTITLE) {
      (void)wgoto(m->m_win, i+1, 2+(m->m_cols==1?4:0));
    }
    else {
      (void)wgoto(m->m_win, i+1, margin);
    }

    if (i++ == 0) {
      (void)attron (A_UNDERLINE);		/* A_UNDERLINE */
      (void)wputs (m->m_win, temp2);
      (void)attroff (A_UNDERLINE);
    }
    else {
      (void)wputs(m->m_win,temp2);
    }
  }
}
