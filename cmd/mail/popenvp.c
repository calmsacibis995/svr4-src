/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:popenvp.c	1.3.3.1"

/*
    These routines are based on the standard UNIX stdio popen/pclose
    routines. This version takes an argv[][] argument instead of a string
    to be passed to the shell. The routine execvp() is used to call the
    program, hence the name popenvp() and the argument types.

    This routine avoids an extra shell completely, along with not having
    to worry about quoting conventions in strings that have spaces,
    quotes, etc. 
*/

/*	@(#)popen.c	1.2	*/
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define	tst(a,b) (*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

#ifdef SVR4
#include <unistd.h>
#else
extern FILE *fdopen();
extern int execvp(), fork(), pipe(), close(), fcntl();
#endif
static int popen_pid[20];

FILE *
popenvp(file, argv, mode, resetid)
char	*file;
char	**argv;
char	*mode;
{
	int	p[2];
	register int myside, yourside, pid;

	if(pipe(p) < 0)
		return(NULL);
	myside = tst(p[WTR], p[RDR]);
	yourside = tst(p[RDR], p[WTR]);
	if((pid = fork()) == 0) {
		/* myside and yourside reverse roles in child */
		int	stdio;

		if (resetid) {
			setgid(getgid());
			setuid(getuid());
		}
		stdio = tst(0, 1);
		(void) close(myside);
		(void) close(stdio);
		(void) fcntl(yourside, F_DUPFD, stdio);
		(void) close(yourside);
		(void) execvp (file, argv);
		fprintf(stderr,"exec failed. errno=%d.\n", errno);
		fflush(stderr);
		_exit(1);
	}
	if(pid == -1)
		return(NULL);
	popen_pid[myside] = pid;
	(void) close(yourside);
	return(fdopen(myside, mode));
}

int
pclosevp(ptr)
FILE	*ptr;
{
	register int f, r;
	int status;
	void (*hstat)(), (*istat)(), (*qstat)();

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
	return(status);
}
