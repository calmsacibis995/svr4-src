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
#ident	"@(#)fmli:qued/fput.c	1.11"

#include <stdio.h>
#include <curses.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "attrs.h"


extern char *attr_on();
extern char *attr_off();


/*
 * FPUTSTRING will return NULL if the entire string fits in the field
 * otherwise it returns a pointer to the beginning of the substring that
 * does not fit
 */
char *
fputstring(str)
char *str;
{
	register char *sptr;
	register int row, col, done; 
	int i, numspaces, pos;
	chtype attrs;

	col = Cfld->curcol;
	row = Cfld->currow;
	attrs = Lastattr;
	done = FALSE;
	sptr = str;
	while (!done) {
		if (*sptr == '\\') {
			switch(*(++sptr)) {
			case 'b':
				*sptr = '\b';
				break;
                        case '-':
				if (Cfld->flags & I_TEXT)
				    sptr = attr_off(sptr, &attrs, NULL) + 1;
                                continue;   /* don't need to wputchar */
                                break;
			case 'n':
				*sptr = '\n';
				break;
                        case '+':
				if (Cfld->flags & I_TEXT)
				    sptr = attr_on(sptr, &attrs, NULL) + 1;
                                continue;   /* don't need to wputchar */
                                break;
			case 't':
				*sptr = '\t';
				break;
			case 'r':
				*sptr = '\r';
				break;
			}
		}
		switch(*sptr) {
		case '\n':
			fgo(row, col);
			fclearline();
			if (row == LASTROW)
				done = TRUE;
			else
				fgo(++row, col = 0);
			sptr++;
			break;
		case '\b':
			if (col != 0)
				fgo(row, --col);
			sptr++;
			break;
		case '\t':
			numspaces = ((col + 8) & ~7) - col;
			for (i = 0; i < numspaces && col <= LASTCOL; i++, col++)
				wputchar(' ', attrs, NULL);
			sptr++;
			break;
		case '\0':
			done = TRUE;
			sptr = NULL;
			continue;
		default:
			wputchar(*sptr++, attrs, NULL);
			col++;
			break;
		}
		if (col > LASTCOL) {
			if (row == LASTROW) {
				if ((Flags & I_SCROLL) && (Flags & I_WRAP)) {
					/*
					 * If the word is not longer then
					 * the length of the field then
					 * clear away the word to wrap...
					 * and adjust the string pointer to
					 * "unput" the word ....
					 */
					pos = prev_bndry(row, ' ', TRUE);
					if (pos >= 0) {
						fgo(row, pos);
						fclearline();
						sptr -= (LASTCOL - pos);
					}
				}
				else
					sptr++;
				done = TRUE;
			}
			else if ((Flags & I_WRAP) && (wrap() == TRUE)) {
				if ((col = do_wrap()) < 0)
					col = 0;
				fgo(++row, col);
			}
			else {
				if (*sptr != '\n')
					fgo(++row, col = 0);
			}
		}
	}
	Lastattr = attrs;
	Cfld->curcol = col;
	Cfld->currow = row;
	return(sptr);
}
