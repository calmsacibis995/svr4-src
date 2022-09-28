/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:vt/wputchar.c	1.5"

#include	<curses.h>
#include	"wish.h"
#include	"vtdefs.h"
#include	"vt.h"
/*
  --------------------------------------------------------------------------------
  wputchar
          Output character `ch' to window `w'  with video attributes `attr'.
          If w is NULL, output goes to window associated with the current frame
          (VT_curid).
  --------------------------------------------------------------------------------
*/
void
wputchar(ch, attr, w)
char	ch;
chtype  attr;
WINDOW  *w;
{
	register chtype	c;
	register WINDOW   *win;
	register struct vt	*v;
	int      row, col;

	c = ch;

	if ((win=w) == NULL)
	{
 	   v = &VT_array[VT_curid];
	   v->flags |= VT_DIRTY;
	   win = v->win;
	}

	if ( ch > 037 && ch < 0177 )
	{
		if (attr & A_ALTCHARSET)  /* map input into graphics chars */
		{                         /* as defined in FMLI manual     */
		    switch (ch)	          /*           1                   */
		    {                     /*      d--------a               */
		    case 'a':             /*      |    |   |               */
			c = ACS_URCORNER; /*     4|----+---|2              */
			break;            /*      |    |   |               */
		    case 'b':             /*      c--------b               */
			c = ACS_LRCORNER; /*           3                   */
			break;
		    case 'c':
			c = ACS_LLCORNER;
			break;
		    case 'd':
			c = ACS_ULCORNER;
			break;
		    case '1':
			c = ACS_TTEE;
			break;
		    case '2':
			c = ACS_RTEE;
			break;
		    case '3':
			c = ACS_BTEE;
			break;
		    case '4':
			c = ACS_LTEE;
			break;
		    case '-':
			c = ACS_HLINE;
			break;
		    case '|':
			c = ACS_VLINE;
			break;
		    case '+':
			c = ACS_PLUS;
			break;
		    case '<':
			c = ACS_LARROW;
			break;
		    case '>':
			c = ACS_RARROW;
			break;
		    case 'v':
			c = ACS_DARROW;
			attr &= ~A_ALTCHARSET; /* kluge to avoid curses bug */
			break;
		    case '^':
			c = ACS_UARROW;
			break;
		    /* the following characters are not found in the fmli  */
		    /* documentation but except for # conform to the vt100 */
		    /* alternate charset.  see terminfo(4)                 */
		    case '0':
			c = ACS_BLOCK;
			break;
		    case 'I':
			c = ACS_LANTERN;
			break;
		    case '\'':
			c = ACS_DIAMOND;
			break;
		    case '#':
			c = ACS_CKBOARD;
			break;
		    case 'f':
			c = ACS_DEGREE;
			break;
		    case 'g':
			c = ACS_PLMINUS;
			break;
		    case 'h':
			c = ACS_BOARD;
			break;
		    case 'o':
			c = ACS_S1;
			break;
		    case 's':
			c = ACS_S9;
			break;
		    case '~':
			c = ACS_BULLET;
			break;
		    default:	/* turn off alt char set for unrecognized chars */
			attr &= ~A_ALTCHARSET;
			break;
		    }
		}  
                waddch(win, c | attr);
		return;
	}

	getyx(win, row, col);

	switch (c) {
	case MENU_MARKER:
		c = ACS_RARROW;
/* les */
		waddch(win, c | attr);
/***/
		return;
	case '\n':
		wmove(win, row + 1, 1);
		return;
	case '\b':
	  	wmove(win, row, col - 1);
		return;
	case '\t':
		wmove(win, row, (col + 8) & ~7);
		return;
	case '\r':
		wmove(win, row, 1);
		return;
	default:
		if (c < ' ')
			return;
		break;
	}
}
/* EVERYTHING BELOW here is COMMENTED OUT */
/* abs: removed dependency on VT_NOBORDER and moved \n\b\t\r code into above case stmt 

	if (!(v->flags & VT_NOBORDER)) {
		int	row, col;
		int	mr, mc;

		getyx(win, row, col);
*/
/*  les: not used
		getmaxyx(win, mr, mc);
*/
/* abs: same as above
		switch (c) {
		case '\n':
			wmove(win, row + 1, 1);
			return;
		case '\b':
		  	wmove(win, row, col - 1);
			return;
		case '\t':
			wmove(win, row, (col + 8) & ~7);
			return;
		case '\r':
			wmove(win, row, 1);
			return;
		}
	}
*/
/* les: move to top
	waddch(v->win, c | highlights(attr));
}
*/
