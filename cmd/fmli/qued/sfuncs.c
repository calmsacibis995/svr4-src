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
#ident	"@(#)fmli:qued/sfuncs.c	1.7"

#include <stdio.h>
#include <curses.h>
#include "wish.h"
#include "vtdefs.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "attrs.h"

extern void acswinschar();

fdelchar()
{
	int saverow, savecol;

	saverow = Cfld->currow;
	savecol = Cfld->curcol;
	wdelchar();
	/*
	 * go to last column and insert a blank
	 */
	fgo(saverow, LASTCOL);
	winschar(' ', Fieldattr);
	fgo(saverow, savecol);
}

finsstr(buff)
char *buff;
{
	register char *bptr;

	for (bptr = buff; *bptr & A_CHARTEXT != '\0'; bptr++)
		;
	bptr--;
	while (bptr >= buff)
		finschar(*bptr--);
}

finschar(c)
char c;
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
	winschar(c, Fieldattr);
}
