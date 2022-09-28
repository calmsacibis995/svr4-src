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
#ident	"@(#)fmli:qued/scrollbuf.c	1.14"

#include <stdio.h>
#include <curses.h>
#include "wish.h"
#include "token.h"
#include "winp.h"
#include "fmacs.h"
#include "terror.h"
 
extern int    *acsreadline();
extern void    acswputchar();
extern void    acsputstring();
extern char   *fputstring();

/*
 * GROWBUF will increase the scroll buffer by size 
 */
void 
growbuf(size)
int size;
{
    unsigned oldbuffsize;

    oldbuffsize = Buffsize;
    if (Scrollbuf == NULL) {
	if ((Scrollbuf =	/* added +1 abs k15 */
	     (chtype *)malloc((size +1) * sizeof(*Scrollbuf))) == NULL)
	    fatal(NOMEM, "");
	Buffoffset = 0;
	Bufflast = size;
    }	
    else if (Buffsize != size) {
	if ((Scrollbuf = (chtype *) /* added +1  abs k15 */
	     realloc(Scrollbuf, (size +1) * sizeof(*Scrollbuf))) == NULL)
	    fatal(NOMEM, "");
    }
    if ((Buffsize = size)  > oldbuffsize) /* initialize new block */
	memset((char *)(Scrollbuf + oldbuffsize), 0,
	       (Buffsize - oldbuffsize +1) * sizeof(*Scrollbuf)); /* +1 abs k15 */
}

free_scroll_buf(fld)
ifield *fld;
{
	if (fld->scrollbuf)
		free(fld->scrollbuf);
	fld->scrollbuf = NULL;
	fld->buffsize = 0;
	fld->buffoffset = 0;
	fld->bufflast = 0;
}

/*
 * SYNCBUF will syncronize the window map with the scroll buffer
 */
void 
syncbuf(offset, start, end)
unsigned offset;
int start;
int end;
{
    register chtype *currptr;
    register int i;
    int saverow, savecol;

    saverow = Cfld->currow;
    savecol = Cfld->curcol;
    if (Scrollbuf == NULL)
	growbuf(FIELDBYTES);	/* initialize scroll buffer */
    currptr = Scrollbuf + offset;
    if (Currtype == SINGLE) {
	if ((offset + Cfld->cols) >= Bufflast)
	    Bufflast = offset + (unsigned)acsreadline(0, currptr, TRUE);
	else
	    acsreadline(0, currptr, FALSE);
    }
    else {
	for (i = start; i <= end; i++, currptr += LINEBYTES)
	    acsreadline(i, currptr, TRUE);
    }
    fgo(saverow, savecol);
}

/*
 * CLEARBUF will clear the scroll buffer
 */
void
clearbuf()
{
    /*
     * clear scroll buffer from current cursor position to end of field
     */
    syncbuf(Buffoffset, 0, Fieldrows - 1); /* added +1 on next line. abs f15 */
    Bufflast = Buffoffset + (Currtype == SINGLE ? Cfld->cols  : FIELDBYTES); 
    /* actually shrinks scroll buffer... was growbuf(Buffsize)  abs k15 */
    growbuf(Buffoffset + (Currtype == SINGLE ? Cfld->cols : FIELDBYTES));
    setarrows();
}

/*
 * SHIFTBUF will shift the scroll buffer in the specified direction
 */
void 
shiftbuf(direction)
int direction;
{
    register chtype *sptr;
    int startcol, startrow;
    int saverow, savecol;

    saverow = Cfld->currow;
    savecol = Cfld->curcol;
    if (Scrollbuf == NULL)
	growbuf(FIELDBYTES);
    switch(direction) {
    case UP:
	if ((startrow = Buffoffset + FIELDBYTES) >= Bufflast) {
	    /* end of scroll buffer, see if there is more text */
	    if (Valptr) {
		fgo(LASTROW, 0);
		Valptr = (char *) fputstring(Valptr);
	    }
	}
	else {
	    Bufflast -= LINEBYTES;
	    sptr = Scrollbuf + startrow;
	    fgo(LASTROW, 0);
	    acsputstring(sptr);
	    memshift((char *)sptr, (char *)(sptr + LINEBYTES)
		     , (Bufflast - startrow) * sizeof(*sptr));
	}
	break;
    case DOWN:
	startrow = Buffoffset + FIELDBYTES;
	if ((Bufflast + LINEBYTES) > Buffsize)
	    growbuf(Buffsize + FIELDBYTES);
	sptr = Scrollbuf + startrow;
	memshift((char *)(sptr + LINEBYTES), (char *)sptr,
		 (Bufflast - startrow) * sizeof(*sptr));
	Bufflast += LINEBYTES;
	acsreadline(LASTROW, sptr, TRUE);
	break;
    case LEFT:
	/*
	 * If this is the last page then there is no "off-screen"
	 * text to shift.
	 */
	if ((startcol = Buffoffset + Cfld->cols) >= Bufflast)
	    return;
	sptr = Scrollbuf + startcol; 
	fgo(0, LASTCOL);
	acswputchar(*sptr);	/* print character shifted in */
	memshift((char *)sptr, (char *)(sptr + 1),
		 (Bufflast - startcol) * sizeof(*sptr));
	Bufflast--;
	break;
    case RIGHT:
	/*
	 * If this is the last page then there is no "off-screen"
	 * text to shift.
	 */
	if ((startcol = Buffoffset + Cfld->cols) > ++Bufflast)
	    return;	
	if (Bufflast >= Buffsize)
	    growbuf(Buffsize + Cfld->cols);
	sptr = Scrollbuf + startcol; 
	memshift((char *)(sptr + 1), (char *)sptr,
		 (Bufflast - startcol) * sizeof(*sptr));
	*sptr = acsreadchar(0, LASTCOL); /* character shifted out */
    }
    setarrows();
    fgo(saverow, savecol);
}
