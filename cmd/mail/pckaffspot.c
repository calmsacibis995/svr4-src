/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:pckaffspot.c	1.4.3.2"
#include "mail.h"
/*
 * If any H_AFWDFROM lines in msg, decide where to put them.
 * Returns :
 *	-1 ==> No H_AFWDFROM lines to be printed.
 *	> 0 ==> Header line type after (before) which to place H_AFWDFROM
 *              lines and H_AFWDCNT
 */
pckaffspot()
{
	static char pn[] = "pckaffspot";
	register int	rc;	

	if (hdrlines[H_AFWDFROM].head == (struct hdrs *)NULL) {
		rc = -1;
	} else if (orig_aff) {
		rc = H_AFWDFROM;
	} else if (fnuhdrtype == H_RVERS) {
		if (hdrlines[H_EOH].head != (struct hdrs *)NULL) {
			if (hdrlines[H_DATE].head != (struct hdrs *)NULL) {
				rc = H_DATE;
			} else {
				rc = H_EOH;
			}
		} else
			rc = H_RVERS;
	} else if ((fnuhdrtype == H_MVERS) &&
	    (hdrlines[H_EOH].head != (struct hdrs *)NULL)) {
		rc = H_EOH;
	} else {
		rc = H_CTYPE;
	}
	Dout(pn, 3, "'%s'\n", (rc == -1 ? "No Auto-Forward-From lines" : header[rc].tag));
	return (rc);
}
