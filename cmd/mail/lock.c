/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:lock.c	1.6.3.1"
#include "mail.h"

void lock (user)
char	*user;
{
	char	tbuf[80];

	switch (maillock(user,10)) {
	case L_SUCCESS:
	    return;
	case L_NAMELEN:
	    sprintf(tbuf,
		"%s: Cannot create lock file. Username '%s' is > 13 chars\n",
		program,user);
	    break;
	case L_TMPLOCK:
	    strcpy(tbuf, "Cannot create temp lock file\n");
	    break;
	case L_TMPWRITE:
	    strcpy(tbuf, "Error writing pid to lock file\n");
	    break;
	case L_MAXTRYS:
	    strcpy(tbuf, "Creation of lockfile failed after 10 tries");
	    break;
	case L_ERROR:
	    strcpy(tbuf, "Cannot link temp lockfile to lockfile\n");
	    break;
	case L_MANLOCK:
	    strcpy(tbuf, "Cannot set mandatory file lock on temp lockfile\n");
	    break;
	}
	errmsg(E_LOCK,tbuf);
	if (sending) {
		goback(0);
	}
	done(0);
}

void unlock() {
	mailunlock();
}
