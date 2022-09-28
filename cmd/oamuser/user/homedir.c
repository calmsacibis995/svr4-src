/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/homedir.c	1.3.7.1"



#include <sys/types.h>
#include <stdio.h>
#include <userdefs.h>
#include <errno.h>
#include "messages.h"

#define 	SBUFSZ	256

extern int mkdir(), chown(), rm_homedir();
extern void errmsg();
extern char *prerrno();

static char cmdbuf[ SBUFSZ ];	/* buffer for system call */

/*
	Create a home directory and populate with files from skeleton
	directory.
*/
int
create_home( homedir, skeldir, uid, gid)
char *homedir;			/* home directory to create */
char *skeldir;			/* skel directory to copy if indicated */
uid_t uid;			/* uid of new user */
gid_t gid;			/* group id of new user */
{
	if( mkdir(homedir, 0775) != 0 ) {
		errmsg( M_OOPS, "create the home directory", prerrno( errno ) );
		return( EX_HOMEDIR );
	}

	if( chown(homedir, uid, gid) != 0 ) {
		errmsg( M_OOPS, "change ownership of home directory", 
			prerrno( errno ) );
		return( EX_HOMEDIR );
	}

	if(skeldir) {
		/* copy the skel_dir into the home directory */
		(void) sprintf( cmdbuf, "cd %s && find . -print | cpio -pd %s",
			skeldir, homedir);

		if( system( cmdbuf ) != 0 ) {
			errmsg( M_OOPS, "copy skeleton directory into home directory",
				prerrno( errno ) );
			(void) rm_homedir( homedir );
			return( EX_HOMEDIR );
		}

		/* make sure contents in the home dirctory have correct owner */
		(void) sprintf( cmdbuf,"cd %s && find . -exec chown %ld {} \\;",
			homedir, uid );
		if( system( cmdbuf ) != 0) {
			errmsg( M_OOPS, "change owner of files home directory",
				prerrno( errno ) );

			(void) rm_homedir( homedir );
			return( EX_HOMEDIR );
		}

		/* and group....... */
		(void) sprintf( cmdbuf, "cd %s && find . -exec chgrp %ld {} \\;",
			homedir, gid );
		if( system( cmdbuf ) != 0) {
			errmsg( M_OOPS, "change group of files home directory",
				prerrno( errno ) );
			(void) rm_homedir( homedir );
			return( EX_HOMEDIR );
		}
	}
	return( EX_SUCCESS );
}

/* Remove a home directory structure */
rm_homedir( dir )
char *dir;
{
	(void) sprintf( cmdbuf, "rm -rf %s", dir );
		
	return( system( cmdbuf ) );
}
