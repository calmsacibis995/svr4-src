/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*LINTLIBRARY*/
#ident	"@(#)libpkg:pkgexecl.c	1.8.6.1"

#include <stdio.h>
#include <string.h>
#include <varargs.h>
#include <signal.h>
#include <sys/types.h>

extern char	**environ;
extern int	errno;

extern void	exit(),
		progerr();
extern pid_t	fork();
#ifndef PRESVR4
extern pid_t waitpid();
#else
extern int wait();
#endif
extern int	execve();

#define MAXARGS	16

/*VARARGS*/
int
pkgexecl(va_alist)
va_dcl
{
	va_list ap;
	char	*pt, *arg[MAXARGS+1];
	char	*filein, *fileout;
	int	n, status, upper, lower;
	pid_t	pid;
	void	(*func)();

	va_start(ap);
	filein = va_arg(ap, char *);
	fileout = va_arg(ap, char *);

	n = 0;
	while(pt = va_arg(ap, char *)) {
		arg[n++] = pt;
	}
	arg[n] = NULL;
	va_end(ap);

	pid = fork();
	if(pid < 0) {
		progerr("bad fork(), errno=%d", errno);
		return(-1);
	} else if(pid) {
		/* parent process */
		func = signal(SIGINT, SIG_IGN);
#ifndef PRESVR4
		n = waitpid(pid, &status, 0);
#else
		{
		int opid;
		opid=pid;
		while ( ((pid=wait(&status)) != opid) && pid > 0 )
			;
		n=pid;
		}
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
	for (status = 3; status < 101; status++)
		close (status);
	if(filein)
		(void) freopen(filein, "r", stdin);
	if(fileout) {
		(void) freopen(fileout, "w", stdout);
		/*
		(void) fclose(stderr);
		(void) dup(fileno(stdout));
		*/
	}
	(void) execve(arg[0], arg, environ);
	progerr("exec of %s failed, errno=%d", arg[0], errno);
	exit(99);
	/*NOTREACHED*/
}
