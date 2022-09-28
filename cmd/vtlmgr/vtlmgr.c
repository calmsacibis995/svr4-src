/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)vtlmgr:vtlmgr.c	1.3.2.1"

#include <stdio.h>
#include "sys/types.h"
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "sys/at_ansi.h"
#include "sys/kd.h"
#include "sys/vt.h"
#include "sys/termio.h"
#include "errno.h"
#include "sys/stropts.h"

extern int errno;

#define	VTLRCLEN	512

char	sendhup = 0,
	prompt[20],
	vtpref[10],
	vtdev[11];
int	kd,
	old_keymap,
	chld[15];
ushort	vtq;

struct kbentry	kbent;
struct termio	term;
int ppid;
int pgid;


cleanup()
{
	int i;
	int fd;


	/* Put back Alt-! mapping */
	kbent.kb_table = K_ALTSHIFTTAB;
	kbent.kb_index = 2;	/* Scan code for "1" key */
	kbent.kb_value = old_keymap;
	ioctl(kd, KDSKBENT, &kbent);
	if (sendhup) {
		for (i = 0; i < 15; i++)
			if (chld[i] != 0)
				kill(chld[i], SIGHUP);
	}
	exit(0);
}

childsig()
{
	int	stat;

	wait(&stat);
}


main(argc, argv)
int argc;
char **argv;
{
	extern int optind;
	extern char *optarg;

	int	fd, con, option, compatflg, errcnt = 0;
	unchar	i;
	ushort	ttype;
	long	vtno;
	char	*shell, *basename, *home, *cp, *av0, vtmon_p[20];
	char	vtlrc[VTLRCLEN];

	if ( (ppid = getppid()) < 0) {
		perror("vtlmgr: Could not obtain parent process ID\n");
		exit(1);
	}
	if ( (pgid = getsid(ppid)) < 0) {
		perror("vtlmgr: Could not obtain process group ID for parent process\n");
		exit(2);
	}
	av0 = argv[0];
	while ((option = getopt(argc, argv, "k")) != EOF) {
		switch (option) {
		case 'k':
			if (sendhup) {
				fprintf(stderr, "Multiple use of -k\n");
				errcnt++;
			}
			sendhup = 1;
			break;
		default:
			errcnt++;
			break;
		}
	}
	argc += optind;
	argv += optind;
	if (errcnt) {
		fprintf(stderr, "usage: vtlmgr [-k]\n");
		exit(errcnt);
	}
	errno = 0;
	if ((fd = open("/dev/tty", O_RDONLY)) == -1) {
		fprintf(stderr, "open of /dev/tty failed, errno = %d\n", errno);
		exit(1);
	}
	if ((ttype = ioctl(fd, KIOCINFO, 0)) == (ushort)-1) {
		fprintf(stderr, "cannot execute %s on remote terminals\n", av0);
		exit(1);
	} 
	ioctl(fd, TCGETA, &term);
	if (ttype == 0x6b64) /* "kd" */
		strcpy(vtpref, "/dev/");
	else if (((ttype & 0xff00) >> 8) == 0x73) /* SunRiver "s#" */
		sprintf(vtpref, "/dev/s%d", (ttype & 0xff));
	sprintf(vtmon_p, "%svtmon", vtpref);
	errno = 0;
	if ((kd = open(vtmon_p, O_RDWR | O_EXCL)) == -1) {
		fprintf(stderr, "open of %s failed, errno %d\n", vtmon_p, errno);
		exit(1);
	}
	close(fd);
	close(2);
	if (fork())
     		exit(0);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	sigset(SIGTERM, cleanup);
	sigset(SIGHUP, cleanup);
	sigset(SIGCLD, childsig);
	if (setpgid(getpid(),pgid) < 0) {
		perror("Could not join process group of parent");
		exit(3);
	}
	close(1);


	/* Add key mapping for "open next VT" */
	/* Put this on Alt-! */
	kbent.kb_table = K_ALTSHIFTTAB;
	kbent.kb_index = 2;	/* Scan code for "1" key */
	ioctl(kd, KDGKBENT, &kbent);
	old_keymap = kbent.kb_value;
	kbent.kb_value = SPECIALKEY | K_MGRF;
	ioctl(kd, KDSKBENT, &kbent);

	for (;;) {
		gid_t gid;
		if (read(kd, &i, 1) == -1)
			continue;
		if (i == K_MGRF) {
			ioctl(kd, VT_OPENQRY, &vtno);
			i = K_VTF + vtno;
		}
		if (i < K_VTF || i > K_VTL)
			continue;
		if ((chld[i-K_VTF] = fork()) != 0)	/* parent process */
			continue;
		setpgrp();   
		/* set GID to real user id */
		gid = getgid();
		sprintf(vtdev, "%svt%02d", vtpref, i - K_VTF);
		close(kd);
		close(0);
		open(vtdev, O_RDWR);
		ioctl(0, TCSETA, &term);
		dup(0);
		dup(0);
		if ( setgid(getgid()) == -1) {
				fprintf(stderr, "invalid group ID for attempted operation, errno = %d\n", errno);
				sleep(5);
				exit(1);
		}
		sprintf(prompt,"PS1=VT %d> ", i - K_VTF);
		putenv(prompt);
		fputs("\033c", stdout);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		home = getenv("HOME");
		cp = (char *) NULL;
		if (home != (char *)NULL && ((strlen(home) + strlen("/.vtlrc")) < VTLRCLEN)) {
			(void) strcpy(vtlrc,home);
			cp = vtlrc + strlen(vtlrc);
			(void) strcpy(cp,"/.vtlrc");
		}
		if (cp != (char *) NULL && (access(vtlrc,04) == 0))
			(void) system(". $HOME/.vtlrc");

		if ((shell = (char *)getenv("SHELL")) == (char *) NULL) {
			errno = 0;
			if (execl("/sbin/sh", "sh", 0) == -1) {
				fprintf(stderr, "exec of /sbin/sh failed, errno = %d\n", errno);
				sleep(5);
				exit(1);
			}
		} else {
			if ((basename = strrchr(shell, '/')) == (char *) NULL)
				basename = shell;
			else	/* skip past '/' */
				basename++;
			errno = 0;
			if (execl(shell, basename, 0) == -1) {
				fprintf(stderr, "exec of %s failed, errno = %d\n", shell, errno);
				sleep(5);
				exit(1);
			}
		}
	} /* ;; */
}
