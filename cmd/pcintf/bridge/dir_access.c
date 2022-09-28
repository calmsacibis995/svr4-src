#ident	"@(#)dir_access.c	1.2	92/07/24	JPB"

/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/dir_access.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)dir_access.c	3.9	LCC);	/* Modified: 16:23:25 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#if !defined(SYS5_3) && !defined(SYS5_4)

#if	!defined(BERKELEY42) && !defined(LOCUS) && !defined(DGUX)
#ifndef	PARAM_GETS_TYPES
#include	<sys/types.h>
#endif	/* ~PARAM_GETS_TYPES */
#include	<sys/param.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<string.h>
#include	<xdir.h>

#include	<log.h>

extern int errno;

extern char
	*memory();

void
	free();

extern long
	lseek();


/*			Directory Access Routines 			*/

/*
 * Opendir() - opens a directory.
 */

DIR *
opendir(name)
    char *name;
{
    register DIR *dirp;
    register int fd;

    do
	fd = open(name, O_RDONLY);
    while (fd == -1 && errno == EINTR);
    if (fd < 0) {
	debug(0, ("opendir(%s) open failed\n", name));
	return NULL;
    }
    if ((dirp = (DIR *)memory(sizeof(DIR))) == NULL) {
	debug(0, ("opendir(%s) malloc failed\n", name));
        close(fd);
	return NULL;
    }
    dirp->dd_fd = fd;
    dirp->dd_loc = 0;
    dirp->dd_size = 0;
    return(dirp);
}


/*
 * Readdir() - get next entry in a directory.
 */

struct direct *
readdir(dirp)
    register DIR *dirp;
{
    register struct direct *dp;

    for (;;) {
    	if (dirp->dd_loc == 0) {
 	    do
		dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, BLKSIZ);
	    while (dirp->dd_size == -1 && errno == EINTR);
 	    if (dirp->dd_size <= 0)
    		return NULL;
	}
    	if (dirp->dd_loc >= dirp->dd_size) {
    	    dirp->dd_loc = 0;
    	    continue;
    	}
    	dp = (struct direct *)(dirp->dd_buf + dirp->dd_loc);
    	dirp->dd_loc += sizeof(struct direct);
    	if (dp->d_ino == 0)
    	   continue;
    	dirp->dd_dir.d_ino = dp->d_ino;
    	(void) strncpy(dirp->dd_dir.d_name, dp->d_name, DIRSIZ);
	dirp->dd_dirchars[sizeof (struct direct)] = '\0';
    	return(&dirp->dd_dir);
    }
}




/*
 * Telldir - returns current offset into a directory.
 */

long 
telldir(dirp)
    DIR *dirp;
{
    return (lseek(dirp->dd_fd, 0L, 1) - dirp->dd_size + dirp->dd_loc);
}




/*
 * Seekdir - seek to an entry in a directory.
 */

void 
seekdir(dirp, loc)
    register DIR *dirp;
    long loc;
{
    long curloc, base, offset;

    curloc = telldir(dirp);
    if (loc == curloc)
    	return;
    base = loc & ~(BLKSIZ - 1);
    offset = loc & (BLKSIZ - 1);
    lseek(dirp->dd_fd, base, 0);
    dirp->dd_loc = 0;
    if (offset != 0) {
	readdir(dirp);
	dirp->dd_loc = offset;
    }
}



/*
 * Closedir - close a directory.
 */

void 
closedir(dirp)
    register DIR *dirp;
{
    close(dirp->dd_fd);
    dirp->dd_fd = -1;
    dirp->dd_loc = 0;
    free(dirp);
}

#endif	/* !defined(BERKELEY42) && !(defined(LOCUS)) */

#endif	/* !defined(SYS5_3) && !defined(SYS5_4) */
