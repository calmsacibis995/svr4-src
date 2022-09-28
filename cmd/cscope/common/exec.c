/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/exec.c	1.4.2.1"
/*	cscope - interactive C symbol cross-reference
 *
 *	process execution functions
 */

#include "global.h"
#include <varargs.h>

#if !BSD
#define getdtablesize()	_NFILE
#endif	

static	SIGTYPE	(*oldsigquit)();	/* old value of quit signal */
static	SIGTYPE	(*oldsighup)();		/* old value of hangup signal */

/* execute forks and executes a program or shell script, waits for it to
 * finish, and returns its exit code.
 */

/*VARARGS1*/
execute(a, va_alist)	/* note: "exec" is already defined on u370 */
char	*a;
va_dcl
{
	va_list	ap;
	int	exitcode;
	char	*argv[BUFSIZ];
	pid_t	p;
	pid_t	myfork();

	/* fork and exec the program or shell script */
	endwin();	/* restore the terminal modes */
	mousecleanup();
	fflush(stdout);
	va_start(ap);
	for (p = 0; (argv[p] = va_arg(ap, char *)) != 0; p++)
		;
	if ((p = myfork()) == 0) {
		myexecvp(a, argv);	/* child */
	}
	else {
		exitcode = join(p);	/* parent */
	}
	/* the menu and scrollbar may be changed by the command executed */
#if UNIXPC || !TERMINFO
	nonl();
	cbreak();	/* endwin() turns off cbreak mode so restore it */
	noecho();
#endif
	mousemenu();
	drawscrollbar(topline, nextline);
	va_end(ap);
	return(exitcode);
}

/* myexecvp is an interface to the execvp system call to
 * close all files except stdin, stdout, and stderr; and
 * modify argv[0] to reference the last component of its path-name.
 */

myexecvp(a, args)
char	*a;
char	**args;
{
	register int	i;
	char	msg[MSGLEN + 1];
	
	/* close files */
	for (i = 3; i < getdtablesize() && close(i) == 0; ++i) {
		;
	}
	/* modify argv[0] to reference the last component of its path name */
	args[0] = basename(args[0]);

	/* execute the program or shell script */
	execvp(a, args);	/* returns only on failure */
	(void) sprintf(msg, "\nCannot exec %s", a);
	(void) perror(msg);		/* display the reason */
	askforreturn();		/* wait until the user sees the message */
	exit(1);		/* exit the child */
	/* NOTREACHED */
}

/* myfork acts like fork but also handles signals */

pid_t
myfork() 
{
	pid_t	p;		/* process number */

	p = fork();
	
	/* the parent ignores the interrupt, quit, and hangup signals */
	if (p > 0) {
		oldsigquit = signal(SIGQUIT, SIG_IGN);
		oldsighup = signal(SIGHUP, SIG_IGN);
	}
	/* so they can be used to stop the child */
	else if (p == 0) {
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
	}
	/* check for fork failure */
	if (p == -1) {
		myperror("Cannot fork");
	}
	return p;
}

/* join is the compliment of fork */

join(p) 
pid_t	p; 
{
	int	status;  
	pid_t	w;

	/* wait for the correct child to exit */
	do {
		w = wait(&status);
	} while (p != -1 && w != p);

	/* restore signal handling */
	signal(SIGQUIT, oldsigquit);
	signal(SIGHUP, oldsighup);

	/* return the child's exit code */
	return(status >> 8);
}
