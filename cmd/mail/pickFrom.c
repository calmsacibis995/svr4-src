/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:pickFrom.c	1.5.3.1"
#include "mail.h"
/*
 * pickFrom (line) - scans line, ASSUMED TO BE of the form
 *	[>]From <fromU> <date> [remote from <fromS>]
 * and fills fromU and fromS global strings appropriately.
 */

void pickFrom (lineptr)
register char *lineptr;
{
	register char *p;
	static char rf[] = "remote from ";
	int rfl;

	if (*lineptr == '>')
		lineptr++;
	lineptr += 5;
	for (p = fromU; *lineptr; lineptr++) {
		if (isspace (*lineptr))
			break;
		*p++ = *lineptr;
	}
	*p = '\0';
	rfl = strlen (rf);
	while (*lineptr && strncmp (lineptr, rf, rfl))
		lineptr++;
	if (*lineptr == '\0') {
		fromS[0] = 0;
	} else {
		lineptr += rfl;
		for (p = fromS; *lineptr; lineptr++) {
			if (isspace (*lineptr))
				break;
			*p++ = *lineptr;
		}
		*p = '\0';
	}
}
