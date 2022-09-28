/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/file.c	1.3.5.1"



#include	<sys/types.h>
#include	<sys/stat.h>

int
check_perm( statbuf, uid, gid, perm )
struct stat statbuf;
uid_t uid;
gid_t gid;
mode_t perm;
{
	int fail = -1;	/* assume no permission at onset */

	/* Make sure we're dealing with a directory */
	if( S_ISDIR( statbuf.st_mode )) {

		/*
		 * Have a directory, so make sure user has permission
		 * by the various possible methods to this directory.
		 */
		if( (statbuf.st_uid == uid) &&
		    (statbuf.st_mode & (perm << 6)) == (perm << 6) )
			fail = 0;
		else
		if( (statbuf.st_gid == gid) &&
		    (statbuf.st_mode & (perm << 3)) == (perm << 3) )
			fail = 0;
		else
		if( (statbuf.st_mode & perm) == perm )
			fail = 0;
	}

	return( fail );
}
