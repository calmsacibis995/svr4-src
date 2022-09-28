/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:user/rmfiles.c	1.6.5.1"



#include <sys/types.h>
#include <stdio.h>
#include <userdefs.h>
#include <errno.h>
#include "messages.h"

#define 	SBUFSZ	256

extern void errmsg();
extern int rmdir();
extern char *prerrno();

static char sptr[SBUFSZ];	/* buffer for system call */

int
rm_files(homedir, user)
char *homedir;			/* home directory to remove */
char *user;
{
	/* delete all files belonging to owner */
	(void) sprintf( sptr,"rm -rf %s", homedir );
	if( system(sptr) != 0 ) {
		errmsg( M_RMFILES );
		return( EX_HOMEDIR );
	}

	return( EX_SUCCESS );
}

