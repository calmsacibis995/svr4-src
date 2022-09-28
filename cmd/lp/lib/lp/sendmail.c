/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/lp/sendmail.c	1.7.3.1"
/* sendmail(user, msg) -- send msg to user's mailbox */

#include "stdio.h"
#include "stdlib.h"

#include "lp.h"

void
#if	defined(__STDC__)
sendmail (
	char *			user,
	char *			msg
)
#else
sendmail (user, msg)
	char			*user,
				*msg;
#endif
{
	FILE			*pfile;

	char			*mailcmd;

	if (isnumber(user))
		return;

	if ((mailcmd = Malloc(strlen(MAIL) + 1 + strlen(user) + 1))) {
		sprintf (mailcmd, "%s %s", MAIL, user);
		if ((pfile = popen(mailcmd, "w"))) {
			(void)fprintf (pfile, "%s\n", msg);
			pclose (pfile);
		}
		Free (mailcmd);
	}
	return;
}
