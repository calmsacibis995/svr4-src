/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/telldir.c	1.6"
/*
	telldir -- C library extension routine

*/

#ifdef __STDC__
	#pragma weak telldir = _telldir
#endif
#include	"synonyms.h"
#include	<sys/types.h>
#include 	<dirent.h>

extern long	lseek();

long
telldir( dirp )
DIR	*dirp;			/* stream from opendir() */
{
	struct dirent *dp;
	if (lseek(dirp->dd_fd, 0, 1) == 0)	/* if at beginning of dir */
		return(0);			/* return 0 */
	dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
	return(dp->d_off);
}
