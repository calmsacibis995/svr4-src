/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:savehdrs.c	1.5.3.1"
#include "mail.h"
/*
 * Save info on each header line for possible generation
 * of MTA positive or negative delivery notification
 */
void savehdrs(s, hdrtype)
char *s;
int hdrtype;
{
	char		*q;
	int		rf;
	char		delim = ':';
	char		tbuf[HDRSIZ];
	static int	last_hdrtype = -1;

	if (hdrtype > H_CONT) {
		return;
	}
	if (hdrtype == H_CONT) {
		trimnl(s);
		pushlist(last_hdrtype, TAIL, s, TRUE);
		return;
	}

	last_hdrtype = hdrtype;

	if ((hdrtype == H_FROM) || (hdrtype == H_FROM1)) {
		delim = ' ';
	} else {
		if (fnuhdrtype == 0) {
			/* Save type of first non-UNIX header line */
			fnuhdrtype = hdrtype;
		}
	}
	switch (hdrtype) {
	   case H_FROM1:
		/* If first ">From " line, check for '...remote from...' */
		if (hdrlines[H_FROM1].head == (struct hdrs *)NULL) {
			if ((rf = substr(s, " remote from ")) >= 0) {
				trimnl(s + rf);
				sprintf(tbuf,"from %s by %s%s; %s",
						s+rf+13, thissys, maildomain(), RFC822datestring);
				pushlist (H_RECEIVED, HEAD, tbuf, FALSE);
			}
		}
		break;

	   /* Remember that these header line type were in orig. msg.  */
	   case H_AFWDFROM:
		orig_aff++;
		break;
	   case H_RECEIVED:
		orig_rcv++;
		break;
	   case H_TCOPY:
		orig_tcopy++;
		break;
	}
	q = strchr(s,delim) + 1;
	q = skipspace(q);
	trimnl(q);
	if ((hdrtype == H_UAID) || (hdrtype == H_MTSID)) {
		/* Check for enclosing '<' & '>', and remove if found */
		/* gendeliv() will replace them if necessary */
		if ((*q == '<') && (*(q+strlen(q)-1) == '>')) {
			q++;
			*(q+strlen(q)-1) = '\0';
		}
	}

	pushlist (hdrtype, TAIL, q, FALSE);
}
