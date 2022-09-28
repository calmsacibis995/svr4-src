/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rmount:mntlock.c	1.1.6.1"


/*
	lock -- set a semaphore lock file for rmnttab
	unlock -- delete the lock file
*/

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define RSEM_FILE "/etc/rfs/.rmnt.lock"

extern int errno;
extern char *cmd;

static int lfd = -1;

void
lock()
{
	while ((lfd = creat(RSEM_FILE, (S_IRUSR|S_IWUSR))) < 0 && errno == EAGAIN)
		sleep(1);
	if (lfd < 0)
		fprintf(stderr, "%s: warning: cannot create temp file %s\n",
			cmd, RSEM_FILE);
	if (lockf(lfd, F_LOCK, 0L) < 0)
		fprintf(stderr, "%s: warning: cannot lock temp file %s\n",
			cmd, RSEM_FILE);
}

void
unlock()
{
	if (lfd >= 0)
		close(lfd);
}
