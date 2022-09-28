/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)vtlmgr:newvt.c	1.3"

#include <stdio.h>
#include "sys/types.h"
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include "sys/at_ansi.h"
#include "sys/kd.h"
#include "sys/vt.h"
#include "sys/termio.h"
#include "errno.h"

extern int errno;

main(argc, argv)
int argc;
char *argv[];
{
	extern int optind;
	extern char *optarg;

	int	fd, option, wantvt = -1, errcnt = 0;
	long	vtno;
	char	*comm, *bname, *av0, prompt[11];
	char	name[VTNAMESZ], vtpref[VTNAMESZ], vtname[VTNAMESZ];
	ushort	ttype;
	struct termio	term;
	struct vt_stat	vtinfo;
	
	comm = (char *)NULL;
	bname = (char *)NULL;
	av0 = argv[0];
	while ((option = getopt(argc, argv, "e:n:")) != EOF) {
		switch (option) {
		case 'e':
			if (comm != (char *)NULL) {
				fprintf(stderr, "Multiple use of -e\n");
				errcnt++;
			}
			comm = optarg;
			break;
		case 'n':
			if (wantvt != -1) {
				fprintf(stderr, "Multiple use of -n\n");
				errcnt++;
			}
			wantvt = atoi(optarg);
			break;
		default:
			errcnt++;
			break;
		}
	}
	argc += optind;
	argv += optind;
	if (errcnt) {
		fprintf(stderr, "usage: newvt [-e command] [-n vt number]\n");
		exit(errcnt);
	}
	errno = 0;
	if ((fd = open("/dev/tty", O_RDONLY)) == -1)
		exit(1);
	while ((ttype = ioctl(fd, KIOCINFO, 0)) == (ushort)-1) {
		if (ioctl(fd, TIOCVTNAME, name) == -1)
			break;
		sprintf(vtname, "/dev/%s", name);
		if ((fd = open(vtname, O_RDONLY)) == -1)
			break;
	}
	if (ttype == (ushort)-1) {
		fprintf(stderr, "cannot execute %s on remote terminals\n", av0);
		exit(1);
	}
	if (ttype == 0x6b64) /* "kd" */
		strcpy(vtpref, "/dev/");
	else if (((ttype & 0xff00) >> 8) == 0x73) /* SunRiver */
		sprintf(vtpref, "/dev/s%d", (ttype & 0xff));
	ioctl(fd, TCGETA, &term);
	if (wantvt < 0) {
		ioctl(fd, VT_OPENQRY, &vtno);
		if (vtno < 0) {
			fprintf(stderr, "No vts available\n");
			exit(1);
		}
	} else {
		vtno = wantvt;
		ioctl(fd, VT_GETSTATE, &vtinfo);
		if (vtinfo.v_state & (1 << vtno)) {
			fprintf(stderr, "%svt%02d is not available\n", vtpref, vtno);
			exit(1);
		}
	}
	sprintf(vtname, "%svt%02d", vtpref, vtno);
	close(fd);
	close(2);
	close(1);
	close(0);
	if (fork())
		exit(0);	/* parent */
	setpgrp();   
	if (open(vtname, O_RDWR) == -1)
		exit(1);
	ioctl(0, TCSETA, &term);
	dup(0);
	dup(0);
	sprintf(prompt,"PS1=VT %d> ", vtno);
	putenv(prompt);
	fputs("\033c", stdout);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	if (comm != (char *)NULL) {
		system(comm);
	} else if ((comm = (char *)getenv("SHELL")) != (char *)NULL) {
		if ((bname = strrchr(comm, '/')) == (char *) NULL)
			bname = comm;
		else	/* skip past '/' */
			bname++;
		if (execl(comm, bname, 0) == -1)
			fprintf(stderr, "exec of %s failed\n", comm);
	} else if (execl("/bin/sh", "sh", 0) == -1)
		fprintf(stderr, "exec of /bin/sh failed\n");
	sleep(5);
	exit(1);
}
