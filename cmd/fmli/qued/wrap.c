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
#ident	"@(#)fmli:qued/wrap.c	1.4"

#include <stdio.h>
#include <curses.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "terror.h"

#define MAXBOUND	3

extern         acsinsstr();

/*
 * PREV_BNDRY returns the position within the line of the previous blank
 * (or nonblank) starting from the last column of the current row
 */
prev_bndry(row, ch, val)
int row;
char ch;
int val;
{
	register int pos;

	for (pos = LASTCOL; pos >= 0; pos--)
		if ((freadchar(row, pos) == ch) == val)
			break;
	return(pos);
}

/*
 * WRAP returns TRUE if there is a character in the last line of the current
 * row and FALSE otherwise.
 */
wrap()
{
	if (freadchar(Cfld->currow, LASTCOL) != ' ')
		return(TRUE);
	else
		return(FALSE);
}

/*
 * DO_WRAP performs the word wrap ... It returns the number of characters
 * that were wrapped to the next line.
 */
do_wrap()
{
	register int i, need, pos, row;
	register chtype *bptr;
	int saverow, savecol;
	int numblanks;
	chtype *buff;
	int 	retval, maxlength, totallength, lastnonblank;

	if ((row = Cfld->currow) >= LASTROW)
		return(-1);		/* can't wrap on last line */

	saverow = row; 
	savecol = Cfld->curcol;

	/*
	 * see if wrap word fits on the next line
	 */
	pos = prev_bndry(row, ' ', TRUE) + 1;
	need = LASTCOL - pos + 1;
	numblanks = padding(freadchar(row, LASTCOL));
	totallength = need + numblanks;
	lastnonblank = prev_bndry(row + 1, ' ', FALSE);
	maxlength = (LASTCOL - MAXBOUND + 1) - (lastnonblank + 1);
	if (totallength > maxlength)
		return(-1);

	/*
	 * clear the word from the current line
	 */
	fgo(row, pos);
	if ((buff = (chtype *)malloc((totallength + 1) * sizeof(*buff))) == NULL)
		fatal(NOMEM, "");
	bptr = buff;
	for (i = 0; i < need; i++) {
		*bptr++ = acsreadchar(row, pos++);
/*>>ATTR<<*/		fputchar(' ');
	}
	for (i = 0; i < numblanks; i++)
		*bptr++ = ' ';
	*bptr = '\0';

	/*
	 * .. and place it on the next row
	 */
	fgo(row + 1, 0);
	acsinsstr(buff);
	free(buff);

	/*
	 * replace the cursor and let the calling routine move it if
	 * necessary
	 */
	fgo(saverow, savecol);

	return(totallength - numblanks);
}

padding(lastchar)
int lastchar;
{
	register int numblanks;

	/*
	 * compute number of blanks that must follow the wrapped word
	 */
	if (lastchar == '"' || lastchar == '\'') 
		lastchar = freadchar(Cfld->currow, LASTCOL - 1);
	if (lastchar == '.' || lastchar == '?' || lastchar == ':' || lastchar == '!')
		numblanks = 2;
	else
		numblanks = 1;
	return(numblanks);
}
