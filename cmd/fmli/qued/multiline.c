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
#ident	"@(#)fmli:qued/multiline.c	1.11"

#include <stdio.h>
#include <ctype.h>
#include <curses.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"

static token _fixed();
static token _virtual();

#define SCROLLSIZE	((Cfld->rows - 2 <= 0) ? 1 : Cfld->rows - 2)
#define HALFSIZE	(int)(Cfld->rows / 2)

token
multiline(tok)
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
	static int emptyrow();

	rettok = TOK_NOP;
	switch (tok) {
	case TOK_BACKSPACE:
	case TOK_RETURN:
	case TOK_WRAP:
	    if (Flags & I_AUTOADV)
		rettok = TOK_NEXT;
	    else
		beep();
	    break;
	case TOK_IL:
		if (Flags & I_NOEDIT)
			beep();
		else if (emptyrow(LASTROW) == FALSE)
			beep();
		else {
			finsline(1, TRUE);
			Flags |= I_CHANGED;
		}
		break;
	case TOK_DL:
		if (Flags & I_NOEDIT)
			beep();
		else {
			fdelline(1);
			Flags |= I_CHANGED;
		}
		break;
	case TOK_HOME:
	case TOK_BEG:
		fgo(0, 0);
		break;
	case TOK_SHOME:
	case TOK_END:
		col = LASTCOL;
		while (col >= 0 && freadchar(LASTROW, col) == ' ')
			col--;
		fgo(LASTROW, (col == LASTCOL || col == 0 ? col : col+1));
		break; 
	default:
		rettok = tok;
	}
	return(rettok);
}

static token
_virtual(tok)
token tok;
{
	register token rettok;
	register int col;

	rettok = TOK_NOP;
	switch (tok) {
	case TOK_DL:
		if (Flags & I_NOEDIT) {
			beep();
			break;
		}
		Flags |= I_CHANGED;
		fdelline(1);
		shiftbuf(UP);
		break;
	case TOK_IL:
		if (Flags & I_NOEDIT) {
			beep();
			break;
		}
		if (Cfld->currow == LASTROW) {
			if (scroll_down(1) == FALSE) {
				Bufflast += FIELDBYTES;
				growbuf(Bufflast);
				scroll_down(1);
			}
			fgo(LASTROW - 1, 0);
		}
		Flags |= I_CHANGED;
		shiftbuf(DOWN);
		finsline(1, TRUE);
		break;
	case TOK_NPAGE:
		if (Flags & I_NOPAGE)
			rettok = tok;	/* paging not permitted */
		else {
			if (scroll_down(SCROLLSIZE) == FALSE)
				beep();
		}
		break;
	case TOK_PPAGE:
		if (Flags & I_NOPAGE)
			rettok = tok;	/* paging not permitted */
		else {
			if (scroll_up(SCROLLSIZE) == FALSE)
				beep();
		}
		break;
	case TOK_UP:
	case TOK_SR:
		if (scroll_up(1) == FALSE)
			rettok = TOK_UP;
		break;
	case TOK_RETURN:
		if (scroll_down(HALFSIZE) == FALSE) {
			if (Flags & I_NOEDIT)
				beep();	
			else {	
				Bufflast += FIELDBYTES ;     /* abs k17 */
				growbuf(Bufflast);           /* abs k17 */
				scroll_down(HALFSIZE);
			}
		}
		fgo(Cfld->currow == LASTROW ? Cfld->currow : Cfld->currow + 1, 0);
		break;
	case TOK_DOWN:
	case TOK_SF:
		if (scroll_down(1) == FALSE)
			rettok = TOK_DOWN;	
		break;
	case TOK_WRAP:
		if (scroll_down(1) == FALSE) {
			Bufflast += FIELDBYTES; /* abs k17 */
			growbuf(Bufflast);	/* abs k17 */
			scroll_down(1);
		}
		fgo(LASTROW - 1, Cfld->curcol);
		if ((col = do_wrap()) < 0) {
			col = 0;
			beep();
		}
		if (Cfld->curcol == LASTCOL)  	/* if cursor on last col */ 
			fgo(Cfld->currow + 1, col);
		break;
	case TOK_BACKSPACE:
		if (scroll_up(1) == FALSE)
			beep();
		else {
			fgo(Cfld->currow, LASTCOL);
			fputchar(' ');
		}
		break;
	case TOK_HOME:
	case TOK_BEG:
		while (scroll_up(SCROLLSIZE) == TRUE)
			;
		fgo(0, 0);
		break;
	case TOK_SHOME:
	case TOK_END:
		while (scroll_down(SCROLLSIZE) == TRUE)
			;
		col = LASTCOL;
		while (col >= 0 && freadchar(LASTROW, col) == ' ')
			col--;
		fgo(LASTROW, (col == LASTCOL || col == 0 ? col : col+1));
		break;
	default:
		rettok = tok;
	}
	return(rettok);
}

static int
emptyrow(row)
register int row;
{
	register int col;
	int saverow, savecol;

	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	fgo(row, col = LASTCOL);
	while (col >= 0 && freadchar(row, col) == ' ')
		col--;
	fgo(saverow, savecol);
	return(col < 0 ? TRUE : FALSE);
}
