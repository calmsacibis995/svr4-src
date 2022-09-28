/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:stamp.c	1.5.3.1"
#include "mail.h"
/*
	If the mailfile still exists (it may have been deleted),
	time-stamp it; so that our re-writing of mail back to the
	mailfile does not make shell think that NEW mail has arrived
	(by having the file times change).
*/
void stamp()
{
	if ((access(mailfile, A_EXIST) == A_OK) && (utimep->modtime != -1))
		if (utime(mailfile, utimep) != A_OK)
			errmsg(E_FILE,"Cannot time-stamp mailfile");
}
