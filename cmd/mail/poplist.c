/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:poplist.c	1.4.3.1"
#include "mail.h"
/*
 * Remove an entry from its linked list and free any malloc'd memory..
 */
void poplist (hdrtype, where)
register int	hdrtype;
register int	where;
{
	struct	hdrs	*hdr2rm, *cont2rm, *nextcont;

	/* Remove first/last entry from list */

	hdr2rm = (where == HEAD ?
			hdrlines[hdrtype].head : hdrlines[hdrtype].tail);

	if (hdr2rm == (struct hdrs *)NULL) {
		return;
	}
	if (where == HEAD) {
		if (hdr2rm->next == (struct hdrs *)NULL) {
			/* Only 1 entry in list */
			hdrlines[hdrtype].head = hdrlines[hdrtype].tail =
							(struct hdrs *)NULL;
		} else {
			hdrlines[hdrtype].head = hdr2rm->next;
			hdr2rm->next->prev = (struct hdrs *)NULL;
		}
	} else {
		if (hdr2rm->prev == (struct hdrs *)NULL) {
			/* Only 1 entry in list */
			hdrlines[hdrtype].head = hdrlines[hdrtype].tail =
							(struct hdrs *)NULL;
		} else {
			hdrlines[hdrtype].tail = hdr2rm->prev;
			hdr2rm->prev->next = (struct hdrs *)NULL;
		}
	}
	/* Keep track of total bytes added to message due to    */
	/* selected lines in case non-delivery                  */
	/* notification needs to be sent. (See also copylet())  */
	if (hdrtype == H_AFWDFROM) {
	    affbytecnt -=
		(strlen(header[H_AFWDFROM].tag) + strlen(hdr2rm->value) + 2);
	    affcnt--;
	}
	if (hdrtype == H_RECEIVED) {
	    rcvbytecnt -=
		(strlen(header[H_RECEIVED].tag) + strlen(hdr2rm->value) + 2);
	}

	cont2rm = hdr2rm->cont;
	while (cont2rm != (struct hdrs *)NULL) {
		nextcont = cont2rm->next;
		if (hdrtype == H_AFWDFROM) {
		    affbytecnt -= (strlen(cont2rm->value) + 1);
		    affcnt--;
		}
		if (hdrtype == H_RECEIVED) {
		    rcvbytecnt -= (strlen(cont2rm->value) + 1);
		}
		free ((char *)cont2rm);
		cont2rm = nextcont;
	}
	free ((char *)hdr2rm);
}
