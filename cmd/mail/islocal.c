/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:islocal.c	1.4.3.1"
#include "mail.h"

/*
 * islocal (char *user, uid_t *puid) - see if user exists on this system
 */
islocal(user, puid)
char *user;
uid_t *puid;
{
	char	fname[80];
	struct stat statb;
	struct passwd *pwd_ptr;

	/* Check for existing mailfile first */
	sprintf(fname,"%s%s", maildir, user);
	if (stat(fname,&statb) == 0) {
		*puid = statb.st_uid;
		return (TRUE);
	}

	/* If no existing mailfile, check passwd file */
	setpwent();	
	if ((pwd_ptr = getpwnam(user)) == NULL) {
		return(FALSE);
	}
	*puid = pwd_ptr->pw_uid;
	return (TRUE);
}
