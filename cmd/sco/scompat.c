/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:scompat.c	1.1"

/* SCOMPAT command:
 *
 * Provided for compatibility of Santa Cruz UNIX System applications that
 * attempt KD ioctls that map to STREAMS ioctls in 4.0. Basically, the
 * scompat command forks a shell (default to /sbin/sh) if no arguments
 * are presented to it. Within the lifetime of that shell, the KD ioctls
 * are interpreted with the SCO meaning; upon exit of the shell, ioctls are
 * again interpreted with the STREAMS meaning. This is done on a per-virtual
 * terminal basis.
 *
 * If an argument is passed to the scompat command, that remaining argument line
 * is fork'ed and exec'd. While alive, the virtual terminal will interpret the
 * ioctls as SCO ioctls; upon exit of the forked child, the virtual terminal
 * is reset to the default interpretation of the ioctls.
 */

#include "stdio.h"
#include "sys/types.h"
#include "string.h"
#include "signal.h"
#include "fcntl.h"
#include "sys/kd.h"
#include "sys/termio.h"
#include "errno.h"

extern int errno;

main(argc, argv)
int argc;
char **argv;
{
	int	rv,fd, compat_stat = 0;
	unchar	i;
	char	*shell; /* points to shell name */

	if ( (fd=open("/dev/tty",O_RDWR)) == -1) {
		perror("Cannot open /dev/tty\n");
		exit(2);
	}

	/* if the WS_GETXXCOMPAT ioctl does not work, we are not on a VT */
	if ( (compat_stat = ioctl(fd,WS_GETXXCOMPAT,0)) < 0) {
		perror("not a virtual TTY -- WS_GETXXCOMPAT ioctl fails");
		exit(1);
	}
	/* see if SHELL environment variable set */
	if ((shell = (char *)getenv("SHELL")) == (char *) NULL)
		shell = "/sbin/sh"; /* default to shell */

	if (!compat_stat) /* XENIX compatibility was not turned on */
		set_compat_on(fd);

	if (argc == 1) /* simply fork shell if no args */
		rv = fork_shell(shell,fd);
	else /* otherwise fork and exec args */
		rv = fork_argv1(&argv[1],fd);

	if (!compat_stat)
		set_compat_off(fd);
	exit(rv);
}


/* 
 * Turn XENIX ioctl compatibility mode on 
 */
set_compat_on(fd)
int fd;
{
	if (ioctl(fd,WS_SETXXCOMPAT,0) < 0) {
		perror("WS_SETXXCOMPAT ioctl failed");
		exit(6);
	}
}

/*
 * Turn compatibility mode off. If ioctl fails, not much we can do except
 * squawk and exit
 */

set_compat_off(fd)
int fd;
{
	if (ioctl(fd,WS_CLRXXCOMPAT,0) < 0) {
		perror("WS_CLRXXCOMPAT ioctl failed");
		exit(7);
	}
}


/*
 * Fork a shell. Child should close fd to /dev/tty. Parent should wait for
 * death of child. Return child's exit status.
 */

fork_shell(shell,fd)
char *shell;
int fd;
{
	int childstat;

	switch (fork()) {

	case 0: /* child */
		close(fd);
		if (execl(shell,shell,0) < 0)
			fprintf(stderr,"Could not exec %s\n",shell);
		return(1);

	case -1:
		perror("Could not fork sub-shell");
		return(0);

	default: /* parent */
		wait(&childstat);
		return (childstat>>8);
	}
}

/*
 * Fork a child and have it exec the command line given by argv.
 * Parent should wait, child should close fd to /dev/tty. Parent
 * should return exit status.
 */

fork_argv1(argv,fd)
char **argv;
int fd;
{
	int childstat;

	switch (fork()) {

	case 0: /* child */
		close(fd);
		if (execvp(*argv,argv) < 0)
			fprintf(stderr,"Could not exec %s\n",*argv);
		return(1);

	case -1:
		perror("Could not fork process");
		return(0);

	default: /* parent */
		wait(&childstat);
		return (childstat>>8);
	}
}
