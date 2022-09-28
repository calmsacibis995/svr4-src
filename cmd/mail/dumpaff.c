/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:dumpaff.c	1.6.3.1"
#include "mail.h"
/*
 * Put out H_AFWDFROM and H_AFWDCNT lines if necessary, or
 * suppress their printing from the calling routine.
 */
void dumpaff (type, htype, didafflines, suppress, f)
register int	type;
register int	htype;
register int	*didafflines;
register int	*suppress;
register FILE	*f;
{
	int		affspot;	/* Place to put H_AFWDFROM lines */
	struct hdrs	*hptr;
	char		*pn = "dumpaff";

	Dout(pn, 15, "type=%d, htype=%d/%s, *didafflines=%d, *suppress=%d\n", type, htype, htype >= 0 ? header[htype].tag : "None", *didafflines, *suppress);

	affspot = pckaffspot();
	if (affspot == -1) {
		Dout(pn, 15, "\taffspot==-1\n");
		return;
	}

	switch (htype) {
	case H_AFWDCNT:
		*suppress = TRUE;
		Dout(pn, 15, "\tAuto-Forward-Count found\n");
		return;
	case H_AFWDFROM:
		*suppress = TRUE;
		break;
	}

	if (*didafflines == TRUE) {
		Dout(pn, 15, "\tdidafflines == TRUE\n");
		return;
	}

	if ((htype >= 0) && (affspot != htype)) {
		Dout(pn, 15, "\thtype < 0 || affspot != htype, *suppress=%d\n", *suppress);
		return;
	}

	*didafflines = TRUE;
	for (hptr = hdrlines[H_AFWDFROM].head;
	     hptr != (struct hdrs *)NULL;
	     hptr = hptr->next) {
		printhdr(type, H_AFWDFROM, hptr, f);
	}
	fprintf(f,"%s %d\n", header[H_AFWDCNT].tag, affcnt);
	Dout(pn, 15, "\t*didafflines=%d, *suppress=%d\n", *didafflines, *suppress);
}
