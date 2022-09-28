/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:getpwinfo.c	2.8.3.1"

#include "uucp.h"

#include <pwd.h>
extern struct passwd *getpwuid(), *getpwnam();
extern char	*getlogin();


/*
 * get passwd file info for logname or uid
 *	uid	-> uid #	
 *	name	-> address of buffer to return ascii user name
 *		This will be set to pw->pw_name.
 *
 * return:
 *	0	-> success
 *	FAIL	-> failure (logname and uid not found)
 */
int
guinfo(uid, name)
uid_t uid;
char *name;
{
	register struct passwd *pwd;
	char	*login_name;

	/* look for this user as logged in utmp */
	if ((login_name = getlogin()) != NULL) {
		pwd = getpwnam(login_name);
		if (pwd != NULL && pwd->pw_uid == uid)
			goto uid_found;
	}

	/* no dice on utmp -- get first from passwd file */
	if ((pwd = getpwuid(uid)) == NULL) {
	    if ((pwd = getpwuid(UUCPUID)) == NULL)
		/* can not find uid in passwd file */
		return(FAIL);
	}

uid_found:
	(void) strcpy(name, pwd->pw_name);
	return(0);
}

/*
 * get passwd file info for name
 *	name	-> ascii user name
 *	uid	-> address of integer to return uid # in
 *	path	-> address of buffer to return working directory in
 * returns:
 *	0	-> success
 *	FAIL	-> failure
 */
int
gninfo(name, uid, path)
char *path, *name;
uid_t *uid;
{
	register struct passwd *pwd;

	if ((pwd = getpwnam(name)) == NULL) {
		/* can not find name in passwd file */
		*path = '\0';
		return(FAIL);
	}

	(void) strcpy(path, pwd->pw_dir);
	*uid = pwd->pw_uid;
	return(0);
}


