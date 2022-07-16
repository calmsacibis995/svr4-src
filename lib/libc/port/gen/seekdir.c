/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/seekdir.c	1.9"
/*
	seekdir -- C library extension routine

*/

#ifdef __STDC__
	#pragma weak seekdir = _seekdir
#endif
#include	"synonyms.h"
#include	<sys/types.h>
#include	<dirent.h>

extern long	lseek(), telldir();

#define NULL	0

void
seekdir(dirp, loc)
register DIR	*dirp;		/* stream from opendir() */
long		loc;		/* position from telldir() */
{

	if (telldir(dirp) == loc)
		return; 		/* save time */
	dirp->dd_loc = 0;
	lseek(dirp->dd_fd, loc, 0);
	dirp->dd_size = 0;
}
