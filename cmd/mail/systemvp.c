/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:systemvp.c	1.4.3.1"
/*
    These routines are based on the standard UNIX stdio popen/pclose
    routines. This version takes an argv[][] argument instead of a string
    to be passed to the shell. The routine execvp() is used to call the
    program, hence the name popenvp() and the argument types.

    This routine avoids an extra shell completely, along with not having
    to worry about quoting conventions in strings that have spaces,
    quotes, etc. 
*/

#ident	"@(#)mail:systemvp.c	1.4.3.1"
/*	3.0 SID #	1.4	*/
/*LINTLIBRARY*/
#include <signal.h>

#ifdef __STDC__
# include <unistd.h>
# include <wait.h>
#else
extern int fork(), execl(), wait();
#endif

int
systemvp(file, argv, resetid)
char	*file;
char	**argv;
{
	int	status, pid, w;
	void (*istat)(), (*qstat)();

	if((pid = fork()) == 0) {
		if (resetid) {
			setgid(getgid());
			setuid(getuid());
		}
		(void) execvp(file, argv);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while((w = wait(&status)) != pid && w != -1)
		;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	return((w == -1)? w: status);
}
