/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:minput.c	1.3"
#include "tam.h"
#include "menu.h"
#include "kcodes.h"
#include <string.h>

extern void mdisplay ();
/****************************************************************************

  ret = minput(m)		- do menu input

****************************************************************************/

int
minput(m)
register menu_t *m;
{
  int ret = 0;
  int nitems;
  int rows,cols;
  register int key;
  register mitem_t *mi;
  char flushinput = 0;
  char leaveflag;
  char self;
  int supress = 0;
  int curi, oldi, topi, oldtop;
  int c,vcols;
  int i;
  int maxwidth;
  int titheight,titwidth;
  short selcur;


  /* Call mdisplay to display the menu and return row,cols.  The supress
    flag is set when the mouse leaves the region. */

  nitems = mcitems(m, &maxwidth);
  mctitle(m,&titheight,&titwidth);
  m->m_selcnt = 0;
  cols = m->m_cols;

  while (ret == 0) {
    if (m->m_flags & M_SINGLE) {
      for (mi=m->m_items ; mi->mi_name ; mi++) {
        self = (mi==m->m_curi);
        if (self != (mi->mi_flags & M_MARKED)) {
           mi->mi_flags &= ~M_MARKED;
           mi->mi_flags |= M_REDISP;
        }
      }
    }

    mdisplay(m,supress,nitems,maxwidth,titheight,titwidth,&rows,&vcols);

    supress = 0;
  
    /* First get a key. */

    key = track(m->m_win, 0, 0, 0, 0);
    if (key == TERR_SYS) {
      return MERR_SYS;
    }

    /* Assume we're going to flush input, then dispatch on the character. */

    topi = (m->m_topi) ? m->m_topi - m->m_items : 0;
    curi = (m->m_curi) ? m->m_curi - m->m_items : 0;
    oldi = curi;
    oldtop = topi;
    c = curi / rows;

    /* If key is in the printable range, insert it into the line buffer. */

    if (key >= ' ' && key <= '~') {
      i = strlen(m->m_lbuf);
      if ((i < M_MAXLINE-1) && !(key == ' ' && i == 0)) {
        m->m_lbuf[i++] = key;
        m->m_lbuf[i] = '\0';
        (void)mmatch(m);
      }
      continue;
    }

    /* if B2 clicks on empty menu, return Cmd key	*/

    if (!m->m_items->mi_name) {
      ret = Cmd;
      break;
    }

    flushinput = 1;

    switch(key) {
      case Prev: {
	if (--curi < 0) {
	  curi = nitems-1;
	}
	break;
      }

      case Next: {
	if (++curi >= nitems) {
	  curi = 0;
	}
	break;
      }

      case Back: {
	if (c > 0) {
	  curi -= rows;
	}
	break;
      }

      case Forward: {
	if (c < cols-1 && curi < nitems-rows) {
	  curi += rows;
	}
	break;
      }

      case Up: {
	if (curi >= 1) {
	  curi--;
	}
	break;
      }

      case Down: {
	if ( curi+1 < nitems) {
	  curi++;
	}
	break;
      }

      case Home:
      case Beg: {
        curi = 0;
	break;
      }

      case End: {
	curi = nitems-1;
	break;
      }

      case Slect:
      case Mark: {
	m->m_items[curi].mi_flags ^= M_MARKED;
	break;
      }
	  
      /* Cancel first checks if anything is typed or marked.  If so, it clears
	 the selections and does not return.  If nothing is typed or marked,
	 Cancel finishes the menu. */

      case Close:
      case Cancl:
      case s_Cancl: {
	leaveflag = 1;
	for (mi = m->m_items ; mi->mi_name ; mi++) {
	  if (mi->mi_flags & M_MARKED) {
	     mi->mi_flags &= ~M_MARKED;
	     mi->mi_flags |= M_REDISP;
	     leaveflag = 0;
	  }
	}
	if (m->m_flags & M_SINGLE || leaveflag) {
	  ret = key;
	}
	break;
      }

      case s_Exit: {
	ret = Exit;
	break;
      }

      /* ClearLine clears typed input and forces a rematch. */

      case ClearLine: {
	m->m_lbuf[0] = '\0';
	(void)mmatch(m);
	break;
      }

      /* BackSpace eats away one character from the typed input and forces a
	 rematch. */

      case Backspace: {
	flushinput = 0;
	i = strlen(m->m_lbuf);
	if (i) {
	  m->m_lbuf[--i] = '\0';
	  (void)mmatch(m);
	}
	break;
      }

      /* Scrolling, paging, etc. */

      case Page: {
	flushinput = 0;
	i = (rows + rows/2 - 2) * vcols;
	if (i > 0) {
	  curi = topi + i;
	}
	if (curi >= nitems-1) {
	  curi = nitems-1;
	  ret = key;
	}
	m->m_topi = 0;
	break;
      }

      case s_Page: {
	flushinput = 0;
	i = (rows +rows/2 - 2) * vcols;
	if (i > 0) {
	  curi = topi+rows-1 - i;
	}
	if (curi <= 0) {
	  curi = 0;
	  ret = key;
	}
	m->m_topi = 0;
	break;
      }
   
      case RollUp: {
	flushinput = 0;
	if (topi > 0) {
	  m->m_curi--;
	  curi--;
	  (void)wgoto(m->m_win, titheight+M_TBMARGIN, 0);
	  mscroll(m, nitems, rows, -1);
	}
	else {
	  ret = key;
	}
	break;
      }

      case RollDn: {
	flushinput = 0;
	if (topi + (rows*cols) < nitems) {
	  m->m_curi++;
	  curi++;
	  (void)wgoto(m->m_win, titheight+M_TBMARGIN, 0);
	  mscroll(m, nitems, rows, 1);
	}
	else {
	  ret = key;
	}
	break;
      }

      case s_Back: {
	ret = key;
	break;
      }

      case s_Forward: {
	ret = key;
	break;
      }

      case Return: {
	/* If nothing is typed, handle like Next key */
	if (m->m_lbuf[0] == '\0') {
	  key = Next;
	  if (++curi >= nitems) {
	    curi = 0;
	  }
	}
	else {
	  flushinput = 0;
	  ret = Enter;		/* for existing applications */
	}
	break;
      }

      default: {
	flushinput = 0;
	ret = key;
	break;
      }
    }

    /* Check the flushinput flag and manage the redispl flags. */

    if (oldi != curi) {
      m->m_items[oldi].mi_flags |= M_REDISP;
      m->m_items[curi].mi_flags |= M_REDISP;
      m->m_curi = &m->m_items[curi];
    }

    if (oldtop != topi) {
      m->m_items[oldtop].mi_flags |= M_REDISP;
      m->m_items[topi].mi_flags |= M_REDISP;
      m->m_topi = &m->m_items[topi];
    }

    if (flushinput) {
      m->m_lbuf[0] = '\0';
      (void)wcmd(m->m_win,0);
    }
  }

  m->m_selcnt = 0;
  if (m->m_lbuf[0] != '\0') {
    if (mmatch(m) == 0) {
      return(ret);
    }
  }
  if (m->m_curi->mi_flags & M_MARKED) {
    selcur = 1;	/* remember curi selected also */
  }
  else {
    selcur = 0;
    m->m_curi->mi_flags |= M_MARKED;
  }

  for (mi=m->m_items ; mi->mi_name ; mi++) {
    if (mi->mi_flags & M_MARKED) {
      m->m_selcnt++;	
    }
  }
  if ((m->m_selcnt > 1) && (!selcur)) {	/* if any hilite other than curi */
    m->m_curi->mi_flags &= ~M_MARKED;	/* off hilite */
    m->m_selcnt--;			/* adjust cnt */
  }
  return(ret);
}
