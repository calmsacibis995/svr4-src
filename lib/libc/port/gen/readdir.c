/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/readdir.c	1.11"
/*
	readdir -- C library extension routine

*/

#ifdef __STDC__
	#pragma weak readdir = _readdir
#endif
#include	"synonyms.h"
#include	<sys/types.h>
#include	<dirent.h>

extern char	*strncpy();
extern int	getdents(), strlen();

#define NULL 0
struct dirent *
readdir(dirp)
	register DIR		*dirp;	/* stream from opendir() */
	{
	register struct dirent	*dp;	/* -> directory data */
	int saveloc = 0;

	if (dirp->dd_size != 0) {
		dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
		saveloc = dirp->dd_loc;   /* save for possible EOF */
		dirp->dd_loc += dp->d_reclen;
	}
	if (dirp->dd_loc >= dirp->dd_size)
		dirp->dd_loc = dirp->dd_size = 0;

	if (dirp->dd_size == 0 	/* refill buffer */
	  && (dirp->dd_size = getdents(dirp->dd_fd,
			(struct dirent *)dirp->dd_buf, DIRBUF)
	     ) <= 0
	   ) {
		if (dirp->dd_size == 0)	/* This means EOF */
			dirp->dd_loc = saveloc;  /* EOF so save for telldir */
		return NULL;	/* error or EOF */
	}

	dp = (struct dirent *)&dirp->dd_buf[dirp->dd_loc];
	return(dp);

	}
