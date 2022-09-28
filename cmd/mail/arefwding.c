/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:arefwding.c	1.3.3.1"
#include "mail.h"
/*
    NAME
	areforwarding - check to see if mail is being forwarded

    SYNOPSIS
	int areforwarding(char *mailfile)

    DESCRIPTION
	areforwarding() looks in the mail file for the
	"Forward to " string and returns true if it is
	found. The global string sendto is used as the
	work area. If mail is being forwarded, then
	sendto will return with the list of users.

    RETURNS
	TRUE	-> forwarding
	FALSE	-> local
*/
areforwarding(s)
char *s;
{
	FILE *fp;
	int flen;

	if ((fp = fopen(s, "r")) == NULL)
		return(FALSE);

	flen = strlen(frwrd);
	fread(sendto, (unsigned)flen, 1, fp);
	if (strncmp(sendto, frwrd, flen) == SAME) {
		fgets(sendto, sizeof(sendto), fp);
		fclose(fp);
		return(TRUE);
	}
	sendto[0] = '\0';	/* let's be nice */
	fclose(fp);
	return(FALSE);
}
