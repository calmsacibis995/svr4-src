/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/mypopen.c	1.1"
/* static char Sccsid[] = "@(#)popen.c	5.3 u370 source"; */
/*	@(#)popen.c	1.3	*/
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include "global.h"	/* pid_t, SIGTYPE, shell, and basename() */

#define	tst(a,b) (*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

extern FILE *fdopen();
extern void _exit();
static pid_t popen_pid[20];

FILE *
mypopen(cmd, mode)
char	*cmd, *mode;
{
	int	p[2];
	register pid_t *poptr;
	register int myside, yourside;
	register pid_t pid;

	if(pipe(p) < 0)
		return(NULL);
	myside = tst(p[WTR], p[RDR]);
	yourside = tst(p[RDR], p[WTR]);
	if((pid = fork()) == 0) {
		/* myside and yourside reverse roles in child */
		int	stdio;

		/* close all pipes from other popen's */
		for (poptr = popen_pid; poptr < popen_pid+20; poptr++) {
			if(*poptr)
				(void) close(poptr - popen_pid);
		}
		stdio = tst(0, 1);
		(void) close(myside);
		(void) close(stdio);
#if V9
		(void) dup2(yourside, stdio);
#else
		(void) fcntl(yourside, F_DUPFD, stdio);
#endif
		(void) close(yourside);
		(void) execlp(shell, basename(shell), "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return(NULL);
	popen_pid[myside] = pid;
	(void) close(yourside);
	return(fdopen(myside, mode));
}

int
pclose(ptr)
FILE	*ptr;
{
	register int f;
	register pid_t r;
	int status;
	SIGTYPE (*hstat)(), (*istat)(), (*qstat)();

	f = fileno(ptr);
	(void) fclose(ptr);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	while((r = wait(&status)) != popen_pid[f] && r != -1)
		;
	if(r == -1)
		status = -1;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	(void) signal(SIGHUP, hstat);
	/* mark this pipe closed */
	popen_pid[f] = 0;
	return(status);
}
