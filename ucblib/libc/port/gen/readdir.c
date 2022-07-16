/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/readdir.c	1.1.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
	readdir -- C library extension routine

*/

#include	<sys/types.h>
#include	<sys/dir.h>
#include 	<sys/dirent.h>

extern char	*strncpy();
extern int	getdents(), strlen();

struct 		direct dc;

#define NULL 0
struct direct *
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

	/* Copy dirent into direct */
	dc.d_ino = dp->d_ino;
	dc.d_reclen = dp->d_reclen;
	dc.d_name = dp->d_name;
	dc.d_namlen = strlen(dc.d_name);
	
	return(&dc);
}
