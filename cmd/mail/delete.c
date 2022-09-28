/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:delete.c	1.5.3.1"
#include "mail.h"
/*
	signal catching routine --- reset signals on quits and interupts
	exit on other signals
		i	-> signal #
*/
void delete(i)
register int i;
{
	static char pn[] = "delete";
	setsig(i, delete);

	if (i > SIGQUIT) {
		fprintf(stderr, "%s: ERROR signal %d\n",program,i);
		Dout(pn, 0, "caught signal %d\n", i);
	} else {
		fprintf(stderr, "\n");
	}

	if (delflg && (i==SIGINT || i==SIGQUIT)) {
		longjmp(sjbuf, 1);
	}
	done(0);
}
