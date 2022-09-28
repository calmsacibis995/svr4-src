/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:getmajors.c	1.3"

/* This routine parses an entry from the master file and
 * gets the block and character majors. The multiple majors
 * feature allows the specification of more than one major
 * per device. One or more than one majors may be specified
 * in the fifth and sixth fields of the master file. If more
 * than one major is specified, a "range" notation is used
 * as in "f-l", where 'f' is the first major number in the
 * range and 'l' is the last.
 */

#include <stdio.h>
#include "defines.h"

static char buf[100];		/* used for error messages */

int getmajors(mstring, start, end)
char *mstring;
short *start;
short *end;
{
	register char *p;
	char savestring[20];
	int dash = 0;

	strcpy(savestring, mstring);
	for(p = mstring; *p != 0; p++) {
		if(!isdigit(*p) && *p != '-') {
			sprintf(buf,MMRANGE, savestring);
			fprintf(stderr, "getmajors: %s\n",buf);
			return(0);
		}
		if (*p == '-') {
			*p++ = 0;
			dash++;
			break;
		}
	}

	if(!isdigit(*mstring) || (dash && !isdigit(*p))) {
		sprintf(buf,MMSYNTAX, savestring);
		fprintf(stderr, "getmajors: %s\n",buf);
		return(0);
	}

	*start = atoi(mstring);

	if (!dash)
		*end = *start;
	else
		*end   = atoi(p);

	if (*end < *start) {
		sprintf(buf,MMIRANGE,savestring);
		fprintf(stderr, "getmajors: %s\n",buf);
		return(0);
	}

	return(1);
}

