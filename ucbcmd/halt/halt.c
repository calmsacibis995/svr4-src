/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhalt:halt.c	1.2.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * Halt
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/uadmin.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>

#define RB_NOSYNC  1

main(argc, argv)
	int argc;
	char **argv;
{
	int fcn=0;
	int howto=0;
	char *ttyname(), *ttyn = ttyname(2);
	register unsigned i;
	register int qflag = 0;
	int needlog = 1;
	char *user, *getlogin();
	struct passwd *pw, *getpwuid();

	openlog("halt", 0, 4<<3);
	fcn = AD_HALT;
	argc--; argv++;
	while (argc > 0) {
		if (!strcmp(*argv, "-n")) {
			howto |= RB_NOSYNC;
		} else if (!strcmp(*argv, "-y")) {
			ttyn = 0;
		} else if (!strcmp(*argv, "-q")) {
			qflag++;
		} else if (!strcmp(*argv, "-l")) {
			needlog = 0;
		} else {
			fprintf(stderr, "usage: halt [ -lnqy ]\n");
			exit(1);
		}
		argc--, argv++;
	}

	if (ttyn && !strncmp(ttyn, "/dev/ttyd", strlen("/dev/ttyd"))) {
		fprintf(stderr, "halt: dangerous on a dialup; use ``halt -y'' if you are really sure\n");
		exit(1);
	}

	if (needlog) {
		user = getlogin();
		if (user == (char *)0 && (pw = getpwuid(getuid())))
			user = pw->pw_name;
		if (user == (char *)0)
			user = "root";
		syslog(2, "halted by %s", user);
	}

	(void) signal(SIGHUP, SIG_IGN);		/* for network connections */
	if (kill(1, SIGTSTP) == -1) {
		fprintf(stderr, "halt: can't idle init\n");
		exit(1);
	}
	sleep(1);
	signal(SIGTERM, SIG_IGN);
	(void) kill(-1, SIGTERM);	/* one chance to catch it */
	sleep(5);

	if (!qflag && (howto & RB_NOSYNC) == 0) {
		markdown();
		sync();
		setalarm(5);
		pause();
	}
	reboot(fcn);
	perror("reboot");
	exit(0);
	/* NOTREACHED */
}

reboot(fcn)
	int fcn;
{

        if ( getuid() != 0 ) {
                errno = EPERM;
                return -1;
        }
        (void) uadmin(A_SHUTDOWN, fcn, 0);
}  

void dingdong(i)
int i;
{
	/* RRRIIINNNGGG RRRIIINNNGGG */
}

setalarm(n)
	unsigned n;
{
	(void) signal(SIGALRM, dingdong);
	alarm(n);
}

#include <utmp.h>
#include <utmpx.h>
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
char	wtmpf[]	= "/var/adm/wtmp";
char	wtmpxf[] = "/var/adm/wtmpx";
struct utmp wtmp;
struct utmpx wtmpx;

markdown()
{
	off_t lseek();
	time_t time();
	int f;

	if ((f = open(wtmpxf, O_WRONLY)) >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmpx.ut_line, "~");
		SCPYN(wtmpx.ut_name, "shutdown");
		SCPYN(wtmpx.ut_host, "");
		time(&wtmpx.ut_tv.tv_sec);
		write(f, (char *)&wtmpx, sizeof(wtmpx));
		close(f);
	}
	if ((f = open(wtmpf, O_WRONLY)) >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "shutdown");
		wtmp.ut_time = wtmpx.ut_tv.tv_sec;
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
}
