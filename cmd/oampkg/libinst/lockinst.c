/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:libinst/lockinst.c	1.2.3.1"

#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <pkglocs.h>

extern int	creat(),
		lockf();
extern void	progerr(),
		logerr(),
		quit();

#define WAITMSG	"Waiting for exclusive access to installation service ..."

void
lockinst()
{
	char	path[PATH_MAX];
	int	fd;

	(void) sprintf(path, "%s/.lockfile", PKGADM);
	fd = creat(path, 0444);
	if(fd < 0) {
		progerr("unable to create lockfile <%s>", path);
		quit(99);
	}

	if(lockf(fd, F_TLOCK, 0)) {
		logerr(WAITMSG);
		if(lockf(fd, F_LOCK, 0)) {
			progerr("unable to gain exclusive lock on <%s>", path);
			quit(99);
		}
	}
}
