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
#ident	"@(#)fmli:qued/fread.c	1.9"

#include <curses.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "vtdefs.h"
#include "vt.h"
#include "string.h"

#define STR_SIZE	256

int
freadline(row, buff, terminate)
int row;
char *buff;
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
			*buff++ = ch_string[len++] & A_CHARTEXT;
	}
	if (terminate)
		*buff = '\0';
	return(size);
}
