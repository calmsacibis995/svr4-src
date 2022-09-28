/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:qued/acs_io.c	1.4"

#include <curses.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "vtdefs.h"
#include "vt.h"

#define STR_SIZE	256

/* acsreadline is identical to freadline except it does NOT
 * strip off the curses attribute bits, ie. it deals with
 * a line of chtype's intead of chars.
 */

int
acsreadline(row, buff, terminate)
int row;
chtype *buff;
int terminate;
{
	register int len, size = 0;
	chtype ch_string[STR_SIZE];

	fgo (row, 0);
	len = winchnstr((&VT_array[VT_curid])->win, ch_string, LASTCOL + 1) - 1;

	/* extract characters from the ch_string and copy them into buff */

	while (len >= 0 && ((ch_string[len] & A_CHARTEXT) == ' '))
		len--;

	if (len >= 0) {		/* if there is text on this line */
		size = ++len;
		len = 0;
		while (len < size)
			*buff++ = ch_string[len++];
	}
	if (terminate)
		*buff = (chtype)'\0';
	return(size);
}


void
acswinschar(ch)
chtype ch;
{
	register struct	vt	*v;

	v = &VT_array[VT_curid];
	v->flags |= VT_DIRTY;
	winsch(v->win, ch);
}


/* this routine is the same as finschar except it deals with
 * a chtype instead of a char
 */
acsinschar(ch)
chtype ch;
{
	int saverow, savecol;

	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	/* 
	 * delete last character, re-position cursor and insert
	 * a character
	 */
	fgo(saverow, LASTCOL);
	wdelchar();
	fgo(saverow, savecol);
	acswinschar(ch);
}

/* this routine is the same as finsstr except it deals with
 * chtype's instead of a char's
 */
acsinsstr(buff)
chtype *buff;
{
	register chtype *bptr;

	for (bptr = buff; (*bptr & A_CHARTEXT) != 0 ; bptr++)
		;
	bptr--;
	while (bptr >= buff)
		acsinschar(*bptr--);
}

/* this routine is the same as wreadchar except it does NOT
 * strip the curses attribute bits, ie. it deals with a chtype
 * instead of a char
 */
chtype
acswreadchar(row, col)
unsigned row;
unsigned col;
{
	register struct	vt	*v;
	int savey, savex;
	register chtype ch;

	v = &VT_array[VT_curid];
	getyx(v->win, savey, savex);
	if (!(v->flags & VT_NOBORDER)) {
		row++;
		col++;
	}
	ch = mvwinch(v->win, row, col);
	wmove(v->win, savey, savex);		/* return cursor */
	return(ch);
}

/*
  -----------------------------------------------------------------------------
acswputchar
          Output character `ch' to current window
	  Used when character was already on the screen once, and
	  thus we know character is printable and any special proccesing
	  was previously done.
  -----------------------------------------------------------------------------
*/
void
acswputchar(ch)
chtype	ch;
{
    register WINDOW   *win;
    register struct vt	*v;

    v = &VT_array[VT_curid];
    v->flags |= VT_DIRTY;
    win = v->win;

    waddch(win, ch);
    return;
}


/*
 * ACSPUTSTRING is used  in place of fputstring when outputing from the 
 * scroll buffer. since all the special character and output attribute
 * proccessing was already done when fputstring wrote into the scrollbuffer
 * none of that processing is needed here.
 */
void
acsputstring(str)
chtype *str;
{
    register chtype   *sptr;
    register int row, col, done; 
    register WINDOW   *win;
    struct vt	      *v;
    int i, numspaces, pos;



    v = &VT_array[VT_curid];
    v->flags |= VT_DIRTY;
    win = v->win;

    v = &VT_array[VT_curid];
    v->flags |= VT_DIRTY;
    win = v->win;
    col = Cfld->curcol;
    row = Cfld->currow;
    done = FALSE;
    sptr = str;
    while (!done)
    {
	if ((*sptr) & A_CHARTEXT)
	{
	    waddch(win, *sptr++);
	    col++;
	}
	else
	{
	    done = TRUE;
	    continue;
	}
	if (col > LASTCOL)
	{
	    if (row == LASTROW) 
		done = TRUE;
	    else
		fgo(++row, col = 0);
	}
    }
    Cfld->curcol = col;
    Cfld->currow = row;
    return;
}
