/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:temp.c	1.5.5.1"

#include "rcv.h"
#include <pwd.h>
#ifdef preSVr4
extern struct passwd *getpwnam();
extern struct passwd *getpwuid();
#endif

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Give names to all the temporary files that we will need.
 */

void
tinit()
{
	register int err = 0;
	register pid_t pid = mypid;
	struct passwd *pwd;

	sprintf(tempMail, "/tmp/Rs%-ld", pid);
	sprintf(tempQuit, "/tmp/Rm%-ld", pid);
	sprintf(tempEdit, "/tmp/Re%-ld", pid);
	sprintf(tempSet, "/tmp/Rx%-ld", pid);
	sprintf(tempMesg, "/tmp/Rx%-ld", pid);
	sprintf(tempZedit, "/tmp/Rz%-ld", pid);

	/* get the name associated with this uid */
	pwd = getpwuid(uid = myruid);
	if (!pwd) {
		copy("ubluit", myname);
		err++;
		if (rcvmode) {
			printf("Who are you!?\n");
			exit(1);
		}
	}
	else
		copy(pwd->pw_name, myname);
	endpwent();
	lockname = myname;

	strcpy(homedir, Getf("HOME"));
	findmail();
	assign("MBOX", Getf("MBOX"));
	assign("MAILRC", Getf("MAILRC"));
	assign("DEAD", Getf("DEAD"));
	assign("save", "");
	assign("asksub", "");
	assign("header", "");
}
