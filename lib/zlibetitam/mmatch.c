/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:mmatch.c	1.1"
#include "tam.h"
#include "menu.h"
#include <ctype.h>

/****************************************************************************

  cnt = MMATCH(m)	- recompute typed matches

****************************************************************************/

#define UPPER(c)	((c>='a' && c<='z') ? (c-'a'+'A') : c)

int
mmatch(m)
register menu_t *m;
{
  register mitem_t *mi;
  register char *cp1, *cp2;
  register char c1,c2;
  char oldflags;
  int count = 0;
  char only = 0;
  char first_ch;
  char done = 0;
  char upperlower;

  for (mi = m->m_items ; mi->mi_name ; mi++) {
    first_ch = 1;

    oldflags = mi->mi_flags;
    cp1 = m->m_lbuf;
    if (*cp1 == '>') {		/* Means case-sensitive */
      upperlower = 1;
      cp1++;			/* skip '>' */
    }
    else {
      upperlower = 0;
    }
    if (*cp1 == '\0') {
      mi->mi_flags &= ~M_MARKED;
    }
    else {
      cp2 = mi->mi_name;

      /* check leading character */
      if (first_ch) {
        first_ch = 0;
        if (!isalnum(*cp2)) {
           cp2++;
        }
      }

      do {
        c1 = *cp1++;
        c2 = *cp2++;
      } while (c1 && (upperlower?(c1 == c2):(UPPER(c1) == UPPER(c2))));

      if (c1 == 0 && !only && !done) {
        mi->mi_flags |= M_MARKED;
        if (count++ == 0) {
           m->m_curi->mi_flags |= M_REDISP;
           m->m_curi = mi;
           mi->mi_flags |= M_REDISP;
        }				
        if (m->m_flags & M_SINGLE) {
           only = 1;
	}
	done = 1;	/* Accept 1st match only */
      }
      else {
        mi->mi_flags &= ~M_MARKED;
      }
    }

    if (mi->mi_flags != oldflags) {
      mi->mi_flags |= M_REDISP;
    }
  }

  (void)wcmd(m->m_win,m->m_lbuf);
  return(count);
}
