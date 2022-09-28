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
#ident	"@(#)fmli:qued/getfield.c	1.12"

#include <stdio.h>
#include <curses.h>
#include "token.h"
#include "wish.h"
#include "winp.h"
#include "fmacs.h"
#include "terror.h"

static char *getfixedval();
static char *getscrollval();

/*
 * GETFIELD will return the contents of the current field.  It is not
 * stable as of yet and does not account for scrolling field.
 */
char *
getfield(fld, buff)
ifield *fld;
char *buff;
{
	register char *fbuf, *val;
	ifield *savefield;

	savefield = Cfld;
	if (fld != NULL)
		Cfld = fld;
	if (Flags & I_INVISIBLE) {
		if (Value == NULL)
			val = nil;
		else
			val = Value;
	}
	else if (Flags & I_SCROLL)
		val = getscrollval();
	else
		val = getfixedval();
	if (buff == NULL)
		fbuf = val;
	else {
		strcpy(buff, val);
		fbuf = buff;
	}
	Cfld = savefield;
	return(fbuf);
}

static char *
getfixedval()
{
	register int row;
	register char *bptr;

	if (!(Flags & I_CHANGED))
		return(Value);
	Flags &= ~(I_CHANGED);

	/*
	 * If this field does not already have a value then
	 * allocate space equal to the size of the field dimensions
	 * (Buffer is guarenteed to be at least this size if there
	 * already is a field value)
	 */
	if (!Value && (Value = malloc(FIELDBYTES +1)) == NULL) /* +1 abs k15 */
		fatal(NOMEM, nil);

	/*
	 * Read the field value from the window map and eat
	 * trailing new-line characters
	 */
	for (bptr = Value, row = 0; row <= LASTROW; row++) {
		bptr += freadline(row, bptr, TRUE);
		*bptr++ = '\n';
	}
	while (--bptr >= Value && *bptr == '\n')
		*bptr = '\0';
	return(Value);
}

static char *
getscrollval()
{
	register char *dptr;
	register chtype *sptr, *lastptr, *saveptr;
	unsigned buflength, lenvalptr;
	char *dest;

	if (!(Flags & I_CHANGED))
		return(Value);
	Flags &= ~(I_CHANGED);
	/*
	 *	HORIZONTAL SCROLL FIELD 
	 *
	 *	- syncronize the window map with the scroll buffer.
	 *	- set Value to the result
	 *
	 */
	if (Currtype == SINGLE) {
		syncbuf(Buffoffset, 0, 0);		/* syncronize buffer */
		if ((dest = malloc(Buffsize + 1)) == NULL)
			fatal(NOMEM, nil);
		dptr = dest;
		sptr = Scrollbuf;
		while ((*dptr++ = (char)(*(sptr++) & A_CHARTEXT)) != '\0')
			;
		if (Value)
			free(Value);
		Value = dest;
		return(Value);
	}

	/*
	 *	VERTICAL SCROLL FIELD 
	 *
	 *	- syncronize the window map with the scroll buffer.
	 *	- "pack" the scoll buffer (remove trailing blanks).
	 * 	- append the remaining (unprocessed) text pointed to by Valptr.
	 *	- eat trailing new-lines
	 * 	- set Value to the result. 
	 *
	 */
	syncbuf(Buffoffset, 0, Fieldrows - 1);	/* syncronize buffer */
	if ((dest = malloc(Buffsize + 1)) == NULL)
		fatal(NOMEM, nil);
	lastptr = Scrollbuf + Bufflast;
	saveptr = sptr = Scrollbuf;
	dptr = dest;
	while (sptr < lastptr) {  /* pack Scrollbuf */  
		if ((*dptr++ = (char)(*(sptr++) & A_CHARTEXT)) == '\0') {
			saveptr += LINEBYTES;
			sptr = saveptr;
			*(dptr - 1) = '\n';
		}
	} 
	*dptr = '\0';

	buflength = strlen(dest);
	if (Valptr) {				/* append unprocessed text */
		lenvalptr = strlen(Valptr);
		if ((dest = realloc(dest, buflength + lenvalptr + 1)) == NULL)
			fatal(NOMEM, nil);
		strcat(dest, Valptr);
		Valptr = dest + buflength;
		buflength += lenvalptr;
	}
	if (Value)
		free(Value);
	Value = dest;
	for (dptr = dest + buflength - 1; --dptr >= dest && *dptr == '\n'; )
		*dptr = '\0';			/* eat trailing new-lines */
	return(Value);
}

