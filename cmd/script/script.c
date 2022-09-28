/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*      Portions Copyright(c) 1988, Sun Microsystems, Inc.      */
/*      All Rights Reserved.                                    */

#ident	"@(#)script:script.c	1.1.2.1"

/*
 * script
 */
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stropts.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/termios.h>
#include <sys/file.h>

int 	grantpt();
int 	unlockpt();
char	*ptsname();

char	*getenv();
char	*ctime();
char	*shell;
FILE	*fscript;
int	master;			/* file descriptor for master pseudo-tty */
int	slave;			/* file descriptor for slave pseudo-tty */
int	child;
int	subchild;
char	*fname = "typescript";
void	sigwinch();
void	finish();

struct	termios b;
struct	winsize size;
int	lb;
int	l;
char	*mptname = "/dev/ptmx";	/* master pseudo-tty device */

int	aflg;

main(argc, argv)
	int argc;
	char *argv[];
{
	uid_t ruidt;
	gid_t gidt;

	shell = getenv("SHELL");
	if (shell == 0)
		shell = "/bin/sh";
	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		switch (argv[0][1]) {

		case 'a':
			aflg++;
			break;

		default:
			fprintf(stderr,
			    "usage: script [ -a ] [ typescript ]\n");
			exit(1);
		}
		argc--, argv++;
	}
	if (argc > 0)
		fname = argv[0];
	ruidt = getuid();
	gidt = getgid();
	if ((fscript = fopen(fname, aflg ? "a" : "w")) == NULL) {
		perror(fname);
		fail();
	}
	chown(fname, ruidt, gidt);
	getmaster();
	printf("Script started, file is %s\n", fname);
	fixtty();

	(void) signal(SIGCHLD, finish);
	child = fork();
	if (child < 0) {
		perror("fork");
		fail();
	}
	if (child == 0) {
		subchild = child = fork();
		if (child < 0) {
			perror("fork");
			fail();
		}
		if (child)
			dooutput();
		else
			doshell();
	}
	doinput();
}

doinput()
{
	char ibuf[BUFSIZ];
	int cc;

	(void) fclose(fscript);
	signal(SIGWINCH, sigwinch);
	while ((cc = read(0, ibuf, BUFSIZ)) > 0)
		(void) write(master, ibuf, cc);
	done();
}

void
sigwinch()
{
	struct winsize ws;

	if (ioctl(0, TIOCGWINSZ, &ws) == 0)
		(void) ioctl(master, TIOCSWINSZ, &ws);
}

#include <sys/wait.h>

void
finish()
{
	int status;
	register int pid;
	register int die = 0;

	while ((pid = wait(&status)) > 0)
		if (pid == child)
			die = 1;

	if (die)
		done();
}

dooutput()
{
	time_t tvec;
	char obuf[BUFSIZ];
	int cc;

	(void) close(0);
	tvec = time((time_t *)0);
	fprintf(fscript, "Script started on %s", ctime(&tvec));
	for (;;) {
		cc = read(master, obuf, sizeof (obuf));
		if (cc <= 0)
			break;
		(void) write(1, obuf, cc);
		(void) fwrite(obuf, 1, cc, fscript);
	}
	done();
}

doshell()
{

	setpgrp();	/* relinquish control terminal */
	getslave();
	(void) close(master);
	(void) fclose(fscript);
	(void) dup2(slave, 0);
	(void) dup2(slave, 1);
	(void) dup2(slave, 2);
	(void) close(slave);
	execl(shell, shell, NULL);
	perror(shell);
	fail();
}

fixtty()
{
	struct termios sbuf;

	sbuf = b;
	sbuf.c_iflag &= ~(INLCR|IGNCR|ICRNL|IUCLC|IXON);
	sbuf.c_oflag &= ~OPOST;
	sbuf.c_lflag &= ~(ICANON|ISIG|ECHO);
	sbuf.c_cc[VMIN] = 1;
	sbuf.c_cc[VTIME] = 0;
	(void) ioctl(0, TCSETSW, (char *)&sbuf);
}

fail()
{

	(void) kill(0, SIGTERM);
	done();
}

done()
{
	time_t tvec;

	if (subchild) {
		tvec = time((time_t *)0);
		fprintf(fscript,"\nscript done on %s", ctime(&tvec));
		(void) fclose(fscript);
		(void) close(master);
	} else {
		(void) ioctl(0, TCSETSW, (char *)&b);
		printf("Script done, file is %s\n", fname);
	}
	exit(0);
}

getmaster()
{
	struct stat stb;

	if ( (master=open(mptname, O_RDWR)) >= 0 ) {	/* a pseudo-tty is free */
		(void) ioctl(0, TCGETS, (char *)&b);
		(void) ioctl(0, TIOCGWINSZ, (char *)&size);
		return;
	} else {					/* out of pseudo-tty's */
		perror(mptname);
		fprintf(stderr, "Out of pseudo-tty's\n");
		fail();
	}
}

getslave()
{
	char *slavename;	/* name of slave pseudo-tty */

	grantpt(master);			/* change permissions of slave */
	unlockpt(master);			/* unlock slave */
	slavename = ptsname(master);		/* get name of slave */
	slave = open(slavename, O_RDWR);	/* open slave */
	if (slave < 0) {			/* error on opening slave */
		perror(slavename);
		fail();
	}
	ioctl(slave, I_PUSH, "ptem");		/* push pt hw emulation module */
	ioctl(slave, I_PUSH, "ldterm");		/* push line discipline */

	(void) ioctl(slave, TCSETSF, (char *)&b);
	(void) ioctl(slave, TIOCSWINSZ, (char *)&size);
}
