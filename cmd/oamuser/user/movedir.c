/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/movedir.c	1.4.7.1"



#include <sys/types.h>
#include <stdio.h>
#include <userdefs.h>
#include "messages.h"

#define	SBUFSZ	256

extern int access(), rm_files();
extern void errmsg();

static char cmdbuf[ SBUFSZ ];	/* buffer for system call */

/*
	Move directory contents from one place to another
*/
int
move_dir( from, to, login )
char *from;			/* directory to move files from */
char *to;			/* dirctory to move files to */
char *login;			/* login id of owner */
{
	register rc = EX_SUCCESS;
	/******** THIS IS WHERE SUFFICIENT SPACE CHECK GOES */
	
	if( access( from, 00 ) == 0) {	/* home dir exists */
		/* move all files */
		(void) sprintf( cmdbuf,
			"cd %s && find . -user %s -print | cpio -pd %s", 
			from, login, to);

		if( system( cmdbuf ) != 0) {
			errmsg( M_NOSPACE, from, to );
			return( EX_NOSPACE );
		}

		/* Remove the files in the old place */
		rc = rm_files( from, login );

	}

	return( rc );
}

