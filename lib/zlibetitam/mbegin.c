/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:mbegin.c	1.3"
#include "tam.h"
#include "menu.h"
#include "wind.h"

/****************************************************************************

  ret = mbegin(m)	- begin a new menu structure.

  This routine computes the "best" possible shape for a menu.  It
  is called only once, when the menu is initialized.

****************************************************************************/

int
mbegin(m)
register menu_t *m;
{
  int nitems;
  int maxwidth;
  int rows, cols;
  int titheight, titwidth;
  int height,width;
  int windop;
  int w;
  int maxlines = LINES-2;    /* LINES and COLS are */
  int maxcols  = COLS-3;     /* extern in tam.h    */

  nitems = mcitems(m,&maxwidth);

  /* Copy the number of rows and cols of items from the caller's argument. */

  rows = m->m_rows;
  cols = m->m_cols;



  /* If rows and cols are both zero, we pick them.  If the menu is small
    enough, make it just one column.  Otherwise, call mshape to shape
    the window. */

  if (rows == 0 && cols == 0) {
    if (nitems < M_SHORT) {
      rows = nitems;
      cols = 1;
    }
    else {
      (void)mshape(nitems,maxwidth,&rows,&cols);
    }
  }

  /* If rows is non-zero and cols is zero, compute cols.  Note that the code
    way below will fix an overly-large cols. */

  else if (rows != 0 && cols == 0) {
    cols = ROUND(nitems,rows);
  }

  /* Same is true if cols is non-zero and rows is zero. */

  else if (cols != 0 && rows == 0) {
    rows = ROUND(nitems,cols);
  }

  m->m_rows = rows;
  m->m_cols = cols;
  mctitle(m,&titheight,&titwidth);

  if (titwidth > cols*(maxwidth+M_CSPACE)) {
    maxwidth = titwidth;
  }

  /* Compute the width and height of the window from rows and cols. */

  height = MIN(rows + titheight + 2*M_TBMARGIN, maxlines);
  width = MAX(cols * (maxwidth+M_CSPACE), titwidth);
  if (width < strlen (m->m_label)) {
    width = strlen (m->m_label);
  }
  width = MIN(width+2*M_LRMARGIN, maxcols);

  /* Now we have height and width, make a window that size.  If USEWIN is
    set, just alter it. */

  if (m->m_flags & M_USEWIN) {
    WSTAT ws;
    w = m->m_win;
    if (wgetstat((short)w,&ws) < 0) {
      return(MERR_GETSTAT);
    }
    if (ws.width != width || ws.height != height) {

      /* Now that we have a guess at ws.begx and ws.begy, try to fit 
	 it. If we cannot, slide towards the upper left until we hit 1,0 
	 in case of bit-map and 0,0 in case of remote terminal.
	 The '1' is to keep off the status line. */

      while (ws.begy + height > maxlines) {
        ws.begy--;
        if (ws.begy < 0) {
           return(MERR_BIG);
        }
      }

      while (ws.begx + width > maxcols) {
        ws.begx--;
        if (ws.begx < 0) {
           return(MERR_BIG);
        }
      }
      ws.width = width;
      ws.height = height;

      if (wsetstat((short)w,&ws) < 0) {
        return(MERR_SETSTAT);
      }
    }
  }
  else {
    if (m->m_flags & M_WINSON) {
      windop = W_SON;
    }
    else if (m->m_flags & M_WINNEW) {
      windop = W_NEW;
    }
    else {
      windop = W_POPUP;
    }

    w = wind(windop, height, width, 0, 0);

    if (w < 0) {
      return(MERR_NOWIN);
    }
    m->m_win = w;
  }

  /* We have created the window. Clean out other fields, as appropriate. */

  m->m_topi = 0;
  m->m_oldwidth = 0;
  m->m_oldheight = 0;

  (void)wprompt(w,m->m_prompt);
  (void)wlabel(w,m->m_label);

  /* Return okay. */

  return(MERR_OK);
}
