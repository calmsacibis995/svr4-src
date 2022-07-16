/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)stitam:wind.c	1.3"
#include "tam.h"
#include "wind.h"
#include "sys/window.h"
#include "track.h"

#define	H_STEP	6
#define	V_STEP	3

#define PT_HSPACE 1
#define PT_VSPACE 2

int tryrow, trycol, lines, cols;
char winmap[100][150];

/**************************************************************************

  w = WIND(type,height,width,flags,pfont)

  Creates a window of height by width characters and loads it
  with the fonts listed in pfont.  The window's placement is
  determined by the type as follows:

  W_POPUP makes the new window live "inside" the window
  wncur.  Inside is defined as completely within,
  and centered.  Overflow goes down and to the right if
  possible.

  W_SON makes the new window on the lower-right corner if
  possible.  Then the lower left, then upper corners.  The
  overlap is determined based on the size of the window
  wncur.

  W_NEW creates a window in a new part of the display, avoiding
  existing windows.  This is very slow and should only be used
  where absolutely necessary.

**************************************************************************/

/*ARGSUSED*/
int
wind(type,height,width,flags,pfont)
int type,height,width;
short flags;
char *pfont[];
{
  int ret = WERR_OK;
  WSTAT ws;
  int	first_row;


  ws.begx = 0;
  ws.begy = 0;

  lines = LINES-2;	/* LINES and COLS are  */
  cols  = COLS-3;	/* extern in tam.h     */
  first_row = 0;


  ws.width = cols;
  ws.height = lines;
  
  if (type == W_SON || type == W_POPUP) {
    if (wncur >= 0) {
      (void)wgetstat(wncur,&ws);
    }
    else {
      ws.begy = 0;	/* 1 for bitmap, 0 for remote */
      ws.begx = 0;
      ws.height = 10;
      ws.width = 10;
    }
  }

  switch (type) {
    case W_SON: {
      tryrow = ws.begy + ws.height/2;
      trycol = ws.begx + ws.width/2;
      if (tryrow+height >= lines) {	/* if too low */
        tryrow -= height;	/* move to uppers */
      }
      if (trycol+width >= cols) {	/* if too right */
        trycol -= width;	/* move to left */
      }
      break;
    }

    case W_POPUP: {
      tryrow = ws.begy + ws.height/2 - height/2;
      trycol = ws.begx + ws.width/2 - width/2;
      break;
    }

    case W_NEW: {
      do_w_new(height,width);
      break;
    }

    default: {
      tryrow = first_row;
      trycol = 0;
      break;
    }
  }

  if (tryrow < first_row || tryrow >= lines) {
    tryrow = first_row;
  }
  if (trycol < 0 || trycol >= cols) {
    trycol = 0;
  }

/*  Now that we have a guess at trycol and tryrow, try to fit it.
    If we cannot, slide towards the upper left until we hit 1,0 in case of
    bit-map and 0,0 in case of remote terminal.
    The '1' is to keep off the status line.  */

  while (tryrow + height > lines) {
    tryrow--;
    if (tryrow < 0) {
      return(WERR_TOOBIG);
    }
  }

  while (trycol + width > cols) {
    trycol--;
    if (trycol < 0) {
      return(WERR_TOOBIG);
    }
  }

  ret = wcreate(tryrow,trycol,height,width,(unsigned short)flags);

  return(ret);
}

/*****************************************************************************

  do_w_new(height,width);

  Based on the supplied height and width of the wanted window, find 
  the best tryrow and trycol that will give the most empty space

*****************************************************************************/

do_w_new(height,width)
int height,width;
{
  WSTAT ws;

  int nwin;
  int iw, jw;
  int row, col, marks;
  int min_mark;
  int hspace, vspace;
  int rhv, cwh;
  char better;
  static int lastrow, lastcol = 2;



/*  Clear out winmap array and mark the slots occupied by existing windows
    so we can figure out where to put new window in case of W_NEW. 
    In case of remote, we just wgetstat on all windows from 0 to NWINDOW
    and look at the valid ones. In case of bit-map, we have to open /dev
    and search through it.	*/

  for (iw=0 ; iw < lines ; iw ++) {
    for (jw=0 ; jw < cols ; jw ++) {
      winmap[iw][jw] = FALSE;
    }
  }

  hspace = PT_HSPACE;
  vspace = PT_VSPACE;

  for (nwin=0 ; (nwin < NWINDOW) ; nwin++) {
    if (wgetstat (nwin, &ws) == 0) {
      for (iw=ws.begy;iw < ws.begy+ws.height+PT_VSPACE;iw++) {
	if (iw < lines) {
	  for (jw=ws.begx ; jw < ws.begx+ws.width+PT_HSPACE; jw++) {
	    if (jw < cols) {
	      winmap[iw][jw] = TRUE;
	    }
	  }
	}
      }
    }
  }

  row = 0;			/* 0 for remote, 1 for bitmap	*/
  col = 0;			/* left most usable column	*/
  min_mark = 1000;		/* impossibly big number	*/

  while (1) {
    marks = 0;
    rhv = row + height + vspace;
    cwh = col + width + hspace;
    for (iw=row; iw < rhv ; iw ++) {
      for (jw=col ; jw < cwh ; jw ++) {
        if (winmap[iw][jw]) {
          marks++;
	}
      }
    }
    better = 0;
    if ((!marks ) || ( marks < min_mark)) {
      better = 1;
      min_mark = marks;
      tryrow = row;
      trycol = col;
      if (!marks) {		/* if we find an empty space */
        break;		/* take it no matter where  */
      }
    }

    col += H_STEP;		/* move to the right */
    if ( col >= cols - width) {	/* if overshoot */
      row += V_STEP;
      if (row >= lines - height) {
	if ((marks == min_mark) && (!better)) {
	  if ((lastcol + 6) < cols-width) {
	    trycol = lastcol + 6;
	  }
	  else {
	    trycol = 0;
	    tryrow = ((lastrow + 1) < lines-height) ? lastrow+1 : 0;
	  }
	}
        break;		/* done for loop */
      }
      col = 0;		/* back to left */
    }
  }
}
