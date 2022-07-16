/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:mctitle.c	1.1"
#include "tam.h"
#include "menu.h"

/****************************************************************************

  mctitle(m,&titheight,&titwidth)	- CDF: I rewrote this routine
	because the original seemed to want to count lines in the
	title, but didn't succeed very well.

****************************************************************************/

mctitle(m,pheight,pwidth)
register menu_t *m;
int *pheight,*pwidth;
{
  register char *cp,*cplast;
  register len;


  *pheight = 0;
  *pwidth = 0;

  if (!m->m_title) {
    return;
  }

  *pheight = 2;		/* for line after title, before menu */
  cplast = m->m_title;
  for (cp=cplast; *cp;) {
    *pheight += 1;
    cp = strchr (cp, '\n');
    if (cp) {
      len = cp-cplast;
      *pwidth = MAX (*pwidth, len);
      cplast = ++cp;
    }
    else {
      len = strlen (cplast);
      *pwidth = MAX (*pwidth, len);
      break;
    }
  }
}
