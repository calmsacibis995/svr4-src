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
#ident	"@(#)fmli:qued/singleline.c	1.11"

#include <ctype.h>
#include <curses.h>
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "wish.h"

#define HALFLINE	(int)(Cfld->cols / 2)

static token _fixed();
static token _virtual();

/*
 * get single line of input
 */
singleline(tok)
token tok;
{
	if (Flags & I_SCROLL)
		return(_virtual(tok));
	else
		return(_fixed(tok));
}

static token
_fixed(tok)
token tok;
{
	register token rettok;
	register int col;
	static int lastwaswrap = FALSE;

	rettok = TOK_NOP;
	switch(tok) {
	case TOK_BACKSPACE:
	case TOK_LEFT:
	case TOK_RIGHT:
	case TOK_IL:
		beep();		/* do nothing and beep */
		break;
	case TOK_ENTER:
	case TOK_RETURN:
		if (!(Flags & I_NOEDIT))
			rettok = TOK_SAVE;
		else
			fgo(0, 0);
		break;
	case TOK_DL:
		Flags |= I_CHANGED;
		fgo(0, 0);
		fclearline();
		break;
	case TOK_WRAP:
		if (Flags & I_AUTOADV && !(Flags & I_NOEDIT))
		    rettok = TOK_SAVE;
		else
		    if (lastwaswrap == TRUE)
			beep();
		if (Cfld->curcol >= LASTCOL)
		    fgo(0, LASTCOL);
		break;
	case TOK_HOME:
	case TOK_BEG:
		fgo(0, 0);
		break;
	case TOK_SHOME:
	case TOK_END:
		col = LASTCOL;
		while (col >= 0 && freadchar(0, col) == ' ')
			col--;
		fgo(0, (col == LASTCOL || col == 0 ? col : col+1));
		break; 
	default:
		rettok = tok;
	}
	lastwaswrap = (tok == TOK_WRAP ? TRUE : FALSE);
	return(rettok);
}

static token
_virtual(tok)
token tok;
{
	register token rettok;
	register int col;

	rettok = TOK_NOP;
	switch(tok) {
	case TOK_IL:
		beep();
		break;
	case TOK_BACKSPACE:
		if (scroll_left(HALFLINE) == FALSE)
			beep();
		else {
			fgo(0, Cfld->curcol - 1);
			fdelchar();
		}
		break;
	case TOK_ENTER:
	case TOK_RETURN:
		while (scroll_left(Cfld->cols) == TRUE)   /* abs k14 */
		    ;
		if (!(Flags & I_NOEDIT))
			rettok = TOK_SAVE;
		else
			fgo(0, 0);
		break;
	case TOK_DL:
		Flags |= I_CHANGED;
		fgo(0, 0);
		fclearline();
		Buffoffset = 0;
		clearbuf();
		break;
	case TOK_LEFT:
		if (scroll_left(HALFLINE) == FALSE)
			beep();
		break;
	case TOK_RIGHT:
		if (scroll_right(HALFLINE, FALSE) == FALSE)
			beep();
		break;
	case TOK_WRAP:
		if (scroll_right(HALFLINE, FALSE) == FALSE) {

		        Bufflast += HALFLINE;
			growbuf(Bufflast);

/**			growbuf(Buffsize + HALFLINE);    abs k17 */
		        syncbuf(Buffoffset, 0, 0);    /* abs f15 */
			scroll_right(HALFLINE, TRUE);
		}
		fgo(0, Cfld->curcol + 1);
		break;
	case TOK_HOME:
	case TOK_BEG:
		while (scroll_left(Cfld->cols) == TRUE)
			;
		fgo(0, 0);
		break;
	case TOK_SHOME:
	case TOK_END:
		while (scroll_right(Cfld->cols, FALSE) == TRUE)
			;
		col = LASTCOL;
		while (col >= 0 && freadchar(0, col) == ' ')
			col--;
		fgo(0, (col == LASTCOL || col == 0 ? col : col+1));
		break; 
	default:
		rettok = tok;
	}
	return(rettok);
}
