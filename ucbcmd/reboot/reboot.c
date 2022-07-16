/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbreboot:reboot.c	1.2.1.1"

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
 * Reboot
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uadmin.h>

#define RB_ASKNAME      0x001   /* ask for file name to reboot from */
#define RB_NOSYNC       0x004   /* dont sync before reboot */
#define RB_HALT         0x008   /* don't reboot, just halt */
#define RB_DUMP         0x080   /* dump system core */
#define RB_STRING       0x200   /* pass boot args to prom monitor */
 
main(argc, argv)
	int argc;
	char **argv;
{
	int howto = 0;
	register char *argp;
	register i;
	register ok = 0;
	register qflag = 0;
	int needlog = 1;
	char *user, *getlogin();
	struct passwd *pw, *getpwuid();
	extern char *optarg;
	extern int optind, opterr;
	int c;

	openlog("reboot", 0, 4<<3 );

	while ((c = getopt(argc, argv, "qndl")) != EOF) {
		switch((char)c) {
		case 'q':
			qflag++;
			break;
		case 'n':
			howto |= RB_NOSYNC;
			break;
		case 'd':
			howto |= RB_DUMP;
			break;
		case 'l':
			needlog = 0;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 1)
		howto |= RB_STRING;
	else if (argc != 0) 
		usage();

	if (needlog) {
		user = getlogin();
		if (user == (char *)0 && (pw = getpwuid(getuid())))
			user = pw->pw_name;
		if (user == (char *)0)
			user = "root";
		syslog(2, "rebooted by %s", user);
	}

	signal(SIGHUP, SIG_IGN);	/* for remote connections */
	if (kill(1, SIGTSTP) == -1) {
		fprintf(stderr, "reboot: can't idle init\n");
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
	reboot(howto, argv[0]);
	perror("reboot");
	kill(1, SIGHUP);
	exit(1);
}

reboot(howto, bootargs)
        int     howto;
        char    *bootargs;
{
        int     cmd;
        int     fcn;
        int     mdep;
 
        if ( getuid() != 0 ) {
                errno = EPERM;
                return -1;
        }
 
        if ( howto & RB_HALT ) {
                cmd = A_SHUTDOWN;
                fcn = AD_HALT;
        } else if ( howto & RB_ASKNAME ) {
                cmd = A_SHUTDOWN;
                fcn = AD_IBOOT;
        } else {                /* assume RB_AUTOBOOT */
                cmd = A_SHUTDOWN;
                fcn = AD_BOOT;
        }
 
        (void) uadmin(cmd, fcn, mdep);
}

void dingdong(i)
int i;
{
	/* RRRIIINNNGGG RRRIIINNNGGG */
}

setalarm(n)
{
	signal(SIGALRM, dingdong);
	alarm(n);
}

#include <utmp.h>
#include <utmpx.h>
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
char	wtmpf[]	= "/var/adm/wtmp";
char	wtmpxf[] = "/var/adm/wtmpx";
struct utmpx wtmpx;
struct utmp wtmp;	

markdown()
{
	register int f;

	if ((f = open(wtmpxf, 1)) >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmpx.ut_line, "~");
		SCPYN(wtmpx.ut_name, "shutdown");
		SCPYN(wtmpx.ut_host, "");
		time(&wtmpx.ut_tv.tv_sec);
		write(f, (char *)&wtmpx, sizeof(wtmpx));
		close(f);
	}
	if ((f = open(wtmpf, 1)) >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "shutdown");
		wtmp.ut_time = wtmpx.ut_tv.tv_sec;
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
}

usage()
{
	fprintf(stderr,
	    "usage: reboot [ -dnql ] [ boot args ]\n");
	exit(1);
}
