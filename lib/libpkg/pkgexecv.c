/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:pkgexecv.c	1.8.5.1"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

extern char	**environ;
extern int	errno;

extern pid_t	fork(),
		waitpid();
#ifdef PRESVR4
extern int	wait();
#endif
extern int	execve(),
		ds_close();
extern void	exit(),
		progerr();

extern int ds_curpartcnt;
/*VARARGS*/
int
pkgexecv(filein, fileout, arg)
char	*filein, *fileout;
char	*arg[];
{
	int	n, status, upper, lower;
	pid_t	pid;
	void	(*func)();

	pid = fork();
	if(pid < 0) {
		progerr("bad fork(), errno=%d", errno);
		return(-1);
	} else if(pid) {
		/* parent */
		func = signal(SIGINT, SIG_IGN);
		if(ds_curpartcnt >= 0) {
			if(n = ds_close(0))
				return -1;
		}
#ifndef PRESVR4
		n = waitpid(pid, &status, 0);
#else /* SVR3 */
		while ( ((n = wait(&status)) != pid) && (n > 0) )
			;
#endif
		if(n != pid) {
			progerr("wait for %d failed, pid=%d errno=%d", 
				pid, n, errno);
			return(-1);
		}
		upper = (status>>8) & 0177;
		lower = status & 0177;
		(void) signal(SIGINT, func);
		return(lower ? (-1) : upper);
	}
	/* child */
	if(filein)
		(void) freopen(filein, "r", stdin);
	if(fileout) {
		(void) freopen(fileout, "a", stdout);
		(void) freopen(fileout, "a", stderr);
	}

	(void) execve(arg[0], arg, environ);
	progerr("exec of %s failed, errno=%d", arg[0], errno);
	exit(99);
	/*NOTREACHED*/
}
