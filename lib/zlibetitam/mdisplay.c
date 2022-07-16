/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:mdisplay.c	1.5"
#include "tam.h"
#include "menu.h"

/****************************************************************************

  mdisplay(m,supress,nitems,maxwidth,titheight,titwidth,prows)
    - display a menu

  m->m_topi is the upper left item, m->m_curi is the current one.

****************************************************************************/

/*ARGSUSED*/
void
mdisplay(m,supress,nitems,maxwidth,titheight,titwidth,prows,pvcols)
register menu_t *m;
int supress,nitems,maxwidth,titheight,titwidth;
int *prows, *pvcols;
{
  int w;
  WSTAT ws;
  int rows, vcols, dispcols;
  int toprow, topcol, currow, curcol, botrow;
  register int i;
  register mitem_t *mi;
  char redisp;
  char flag;
  char self;
  char dim;
  int r,c,x,y;
  int cursorx,cursory;
  int maxlines = LINES-2;
  int maxcols  = COLS-3;

  /* First see whether there are new dimensions to the window. */

  flag = 0;
  w = m->m_win;
  (void)wgetstat((short)w, &ws);
  redisp = (ws.width != m->m_oldwidth) || (ws.height != m->m_oldheight);

  /* Make sure window height is big enough to show vscroll arrows if there are
    enough rows to justify vertical scrolling.	*/

  if ((ws.height < m->m_oldheight) &&	/* after resize */
     (m->m_rows >= MINROW)) {		/* vscroll arrows w/ MINROW or more */
    if (ws.height < MINROW + titheight + 2*M_TBMARGIN) {
      ws.height = MINROW + titheight + 2*M_TBMARGIN;
      flag = 1;	/* do wsetstat and redisplay */
    }
  }

  /* Compute the title sizes, etc. and the number of rows.  The number of
    real cols is constant (m->m_cols).  The number of visible cols is 
    computed from the width. */

  rows = ws.height - titheight - 2*M_TBMARGIN;
  vcols = (ws.width - 2*M_LRMARGIN) / (maxwidth+M_CSPACE);

  /* If the number of rows is less than 1, or the number of visible cols
    if less than 1, increase the window size. */

  if (rows < 1) {
    rows = 1;
    ws.height = titheight + 2*M_TBMARGIN + 1;
    flag = 1;
  }
  if (vcols < 1) {
    vcols = 1;
    ws.width = maxwidth + 2*M_LRMARGIN + M_CSPACE;
    flag = 1;
  }
  if ((rows*vcols < nitems) && (m->m_cols > 1)) {
    vcols = m->m_cols = 1;
    rows =  M_SHORT;
    m->m_rows = nitems;
    ws.height = titheight + 2*M_TBMARGIN + M_SHORT;
    ws.width = maxwidth + 2*M_LRMARGIN + M_CSPACE;
    flag = 1;
  }

/*  Check if resized smaller than original than turn on scroll arrows	*/

  /* Check if too low or to right then slide window to upper left */
  while (ws.begy + ws.height > maxlines+1) {
    ws.begy--;
    flag = 1;
    if (ws.begy < 0) {
      ws.begy = 0;
      ws.height = maxlines;
    }
  }

  while (ws.begx + ws.width > maxcols) {
    ws.begx--;
    flag = 1;
    if (ws.begx < 0) {
      ws.begx = 0;
      ws.width = maxcols;
    }
  }

  if (flag) {
    (void)wsetstat((short)w,&ws);
    redisp = 1;
  }

  /* Compute row and column number of the top item and the current item. */

  if (m->m_cols == 1) {
    toprow = m->m_topi - m->m_items;
    /*
    toprow = (m->m_topi) ? m->m_topi - m->m_items : 0;
    */
    topcol = 0;
    currow = m->m_curi - m->m_items;
    curcol = 0;
  }
  else {
    toprow = (m->m_topi - m->m_items) % m->m_rows;
    topcol = (m->m_topi - m->m_items) / m->m_rows;
    currow = (m->m_curi - m->m_items) % m->m_rows;
    curcol = (m->m_curi - m->m_items) / m->m_rows;
  }

  /* If we're invisible due to being in a different column, force total
    redisp.  If invisible due to row, maybe we can scroll to get there. */

  i = curcol - topcol;
  if (i < 0 || i > vcols) {
    redisp = 1;
  }
  if (!redisp) {
    i = currow - toprow;
    if (i < -1 || i > rows) {
      redisp = 1;
    }
    else if (i == -1) {
      (void)wgoto(w,M_TBMARGIN+titheight,0);
      mscroll(m,nitems,rows,-1);
      toprow--;
    }
    else if (i == rows) {
      (void)wgoto(w,M_TBMARGIN+titheight,0);
      mscroll(m,nitems,rows,1);
      toprow++;
    }
  }

  /* If we're re-displaying for any reason, clear the display, show the
    title and re-compute a good new top row/col based on the cur row/col. */

  if (redisp) {
    (void)TAMwerase (w);

    mtitle(m,ws.width);
    botrow = (nitems-1) / m->m_cols;
    toprow = currow - rows/2;
    if (toprow+(rows-1) > botrow) {
      toprow = botrow-(rows-1);
    }
    if (toprow < 0) {
      toprow = 0;
    }

    topcol = curcol - vcols/2;
    if (topcol < 0) {
      topcol = 0;
    }
    else if (topcol + vcols > m->m_cols) {
      topcol = m->m_cols - vcols;
      if (topcol < 0) {
	topcol = 0;
      }
    }
    m->m_topi = &m->m_items[topcol*rows + toprow];
    m->m_oldwidth = ws.width;
    m->m_oldheight = ws.height;
  }

  /* Compute how many columns we need to see.  This is normally the number
    of visible columns + 1 extra (it won't be totally visible, tho).  We
    have to be careful never to try to show more columns than exist. */

  dispcols = MIN(vcols+1, m->m_cols-topcol);

  i = 0;
  for (c=0 ; c<dispcols ; c++) {
    for (r=0 ; r<rows ; r++) {

      /* If we need to redisp, compute x & y address. */

      mi = &m->m_items[(c+topcol)*rows + r + toprow];
      if (mi >= &m->m_items[nitems]) {
        goto end;
      }

      flag = (mi->mi_flags & M_REDISP) || redisp;
      if (flag || (mi==m->m_curi)) {
        y = M_TBMARGIN + titheight + r;
        x = M_LRMARGIN + c*(maxwidth+M_CSPACE) - 1;
        if (mi == m->m_curi) {
          cursorx = x;
          cursory = y;
        }

	/* If we need to re-display, go do it.
	   Output a leading dash for marked entries, space otherwise.
	   If there are entries to the left, use a left-arrow in column 0.
	   If there are entries to the right, use a right arrow in the
	   last column. */

        if (flag) {
          self = (mi->mi_flags & M_MARKED) ||
          mi == m->m_curi;
          (void)wgoto(w,y,x);
          if (self) {
	    (void)attron (A_STANDOUT);
	    (void)wputc(w,'-');
          }
	  else {
	    (void)wputc(w,' ');
	  }
          dim = mi->mi_flags & M_DIMMED;
          if (dim) {	/* low intensity */
	    (void)attron (A_DIM);
          }
          (void)wputs(w,mi->mi_name);
          if (self || dim) {
	    (void)attroff (A_DIM);
	    (void)attroff (A_STANDOUT);
          }
        }
        mi->mi_flags &= ~M_REDISP;
      }
      i++;
    }
  }
end:
  *prows = rows;
  *pvcols = vcols;
  (void)wgoto(w,cursory,cursorx);
}			
