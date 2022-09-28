/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/exec.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)exec.c	3.13	LCC);	/* Modified: 19:57:06 12/1/89 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/
/*
 *	MODIFICATION HISTORY
 *	12/16/87 Jeremy Daw SPR # 2212
 *		Removed close_all() call from resetEnv() because what it
 *	did was close all the files that the child inherited from the parent.
 *	Since shared memory is not process sensitive it would delete these
 *	files' entries from the shared memory table as well. This left the
 *	parent, *dossvr*, without access to the files that were open before
 *	the mkdir or rmdir calls that call fork and exec children that call
 *	resetEnv() on completion. TBD, perhaps we should do system mkdir
 *	and rmdir calls rather than forking "/bin/mkdir" and "/bin/rmdir".
 */


#include "const.h"
#include	<errno.h>
#include	<fcntl.h>

extern int errno;

#ifndef SIGNAL_KNOWN
extern	int
	   (*signal())();
#endif

void
	   exit();

/*
 * exec_cmd:	Executes a command and returns 1 if successful and 0
 *		otherwise.
 *		When descriptor is set to -1 all file descriptors will
 *		be closed, otherwise the specified file descriptor will
 *		remain open.
 */

int
exec_cmd(cmd, args, descriptor)
char *cmd;			/* Pointer to command to be executed */
char *args[];			/* pointer to arguements */
int  descriptor;
{
    register int
	i,			/* Loop counter */
	pid,			/* Process id from wait() */
	child_pid;		/* Process id of child from fork() */

    int
	cstat;			/* Status of process from wait() */

    void	(*saved)() = signal(SIG_CHILD, SIG_DFL);
    child_pid = fork();

    if (child_pid == 0) {
	resetEnv(descriptor);
	execvp(cmd, args);
	exit(255);
	/*NOTREACHED*/
    }

/*
 * Wait for signal from child and return the completion code of program.
 */
    else {
	for (;;) {
	    pid = u_wait(&cstat);

	    if (child_pid == pid) {
		signal(SIG_CHILD, saved);
		return cstat == 0;
	    } else
	        childExit(child_pid, cstat);
	}
    }
}

/*  
    reset environment after fork; close all currently opened Unix 
    file descriptors and open /dev/null for descriptors 0,1,and 2.
    NOTE: When descriptor is valid, the corresponding file will remain
    open.
*/

resetEnv(descriptor)
int	descriptor;
{
int	maxFiles;		/* maximum possible number of open files */
int	i,
	dummyDesc;
				/* 12/16/87 JD used to do a close_all() here
				 * but it unecessaryly closed all record locking
				 * entries too. Which included for the parent
				 * which in this case is the dossvr!
				 */

	maxFiles = uMaxDescriptors();
	for (i=0; i < maxFiles; i++)
		if (i != descriptor)
			close(i);

	do
		dummyDesc = open("/dev/null", O_RDONLY);
	while (dummyDesc == -1 && errno == EINTR);
	do
		dummyDesc = fcntl(dummyDesc, F_DUPFD, 0);
	while (dummyDesc == -1 && errno == EINTR);
	do
		dummyDesc = fcntl(dummyDesc, F_DUPFD, 0);
	while (dummyDesc == -1 && errno == EINTR);
}

/*
 * fork_wait:	Forks a child.  Parent wiaits for child to exit and returns
 *		pid of child or -1 if fork failed.  Returns 0 to child.
 *
 */
int 
fork_wait(status)
int *status;

{
    register int
	pid,			/* Process id from wait() */
	child_pid;		/* Process id of child from fork() */

    int
	cstat;			/* Status of process from wait() */

    void	(*saved)() = signal(SIG_CHILD, SIG_DFL);
    child_pid = fork();

    if (child_pid == 0) return 0;

/*
 * Wait for signal from child and return the completion code of program.
 */
    if (child_pid == -1) return -1;
    else {
	for (;;) {
	    pid = u_wait(&cstat);

	    if (child_pid == pid) {
		signal(SIG_CHILD, saved);
		if (status != (int *)NULL) *status = cstat;
		return child_pid;
	    } else
	        childExit(child_pid, cstat);
	}
    }
}
