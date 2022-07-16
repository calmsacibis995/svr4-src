/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/opendir.c	1.13"
/*
	opendir -- C library extension routine

*/

#ifdef __STDC__
	#pragma weak opendir = _opendir
#endif
#include	"synonyms.h"
#include	"shlib.h"
#include	<sys/types.h>
#include	<dirent.h>
#include   	<sys/fcntl.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<stdlib.h>
#include	<errno.h>

extern int	open(), close(), fstat();

#define NULL	0

DIR *
opendir( filename )
const char		*filename;	/* name of directory */
{
	register DIR	*dirp = NULL;	/* -> malloc'ed storage */
	register int	fd;		/* file descriptor for read */
	struct stat	sbuf;		/* result of fstat() */

	if ( (fd = open( filename, 0 )) < 0 )
		return NULL;
	/* POSIX mandated behavior
	 * close on exec if using file descriptor 
 	 */
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) 
		return NULL;
	if ( (fstat( fd, &sbuf ) < 0)
	  || ((sbuf.st_mode & S_IFMT) != S_IFDIR)
	  || ((dirp = (DIR *)malloc( sizeof(DIR) )) == NULL)
	  || ((dirp->dd_buf = malloc(DIRBUF)) == NULL)
	   )	{
		if ((sbuf.st_mode & S_IFMT) != S_IFDIR)
			errno = ENOTDIR;
		if (dirp)
			free(dirp); 	/* free first malloc, if needed */
		
		(void)close( fd );
		return NULL;		/* bad luck today */
		}

	dirp->dd_fd = fd;
	dirp->dd_loc = dirp->dd_size = 0;	/* refill needed */

	return dirp;
}
