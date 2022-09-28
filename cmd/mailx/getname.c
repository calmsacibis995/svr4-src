/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:getname.c	1.3.5.1"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 */

#include "rcv.h"
#include <pwd.h>
#ifdef preSVr4
extern struct passwd *getpwnam();
extern struct passwd *getpwuid();
#endif

/*
 * Search the passwd file for a uid.  Return name through ref parameter
 * if found, indicating success with 0 return.  Return -1 on error.
 * If -1 is passed as the user id, close the passwd file.
 */

getname(uid, namebuf)
	uid_t uid;
	char namebuf[];
{
	struct passwd *pw;

	if (uid == (uid_t)-1) {
		endpwent();
		return(0);
	}
	setpwent();
	pw = getpwuid(uid);
	if (pw) {
		strcpy(namebuf, pw->pw_name);
		return(0);
	}
	return(-1);
}

/*
 * Convert the passed name to a user id and return it.  Return -1
 * on error.  Iff the name passed is 0, close the passwd file.
 */

uid_t
getuserid(name)
	char name[];
{
	struct passwd *pw;

	if (name == 0) {
		endpwent();
		return(0);
	}
	setpwent();
	pw = getpwnam(name);
	return pw ? pw->pw_uid : (uid_t)-1;
}
