/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)rpcsvc:rwall.c	1.2.1.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
#ifndef lint
static	char sccsid[] = "@(#)rwall.c 1.5 89/04/06 Copyr 1984 Sun Micro";
#endif

/*
 * rwall.c
 *	The client rwall program
 * 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <utmp.h>
#include <rpc/rpc.h>
#include <signal.h>
#include <setjmp.h>
#include <rpcsvc/rwall.h>

#define	USERS	128
char who[9] = "???";
char *path;

main(argc, argv)
	int argc;
	char **argv;
{
	int msize;
	char buf[BUFSIZ];
	register i;
	struct utmp utmp[USERS];
	FILE *f;
	int sline;
	char hostname[256];
	int hflag;
	
	if (argc < 2)
		usage();
	(void) gethostname(hostname, sizeof (hostname));

	if ((f = fopen("/etc/utmp", "r")) == NULL) {
		fprintf(stderr, "Cannot open /etc/utmp\n");
		exit(1);
	}
	sline = ttyslot(); /* 'utmp' slot no. of sender */
	fread((char *)utmp, sizeof(struct utmp), USERS, f);
	fclose(f);
	if (sline > 0)
		strncpy(who, utmp[sline].ut_name, sizeof(utmp[sline].ut_name));

	sprintf(buf, "From %s@%s:  ", who, hostname);
	msize = strlen(buf);
	while ((i = getchar()) != EOF) {
		if (msize >= sizeof buf) {
			fprintf(stderr, "Message too long\n");
			exit(1);
		}
		buf[msize++] = i;
	}

	buf[msize] = '\0';
	path = buf;
	hflag = 1;
	while (argc > 1) {
		if (argv[1][0] == '-') {
			switch (argv[1][1]) {
				case 'h':
					hflag = 1;
					break;
				case 'n':
					hflag = 0;
					break;
				default:
					usage();
					break;
			}
		} else if (hflag) {
			doit(argv[1]);
		}
#ifdef NETGROUP
		else {
			char *machine, *user, *domain;

			setnetgrent(argv[1]);
			while (getnetgrent(&machine, &user, &domain)) {
				if (machine)
					doit(machine);
				else
					doall();
			}
			endnetgrent();
		}
#endif
		argc--;
		argv++;
	}
	return (0);
}

#ifdef NETGROUP
/*
 * Saw a wild card, so do everything
 */
doall()
{
	fprintf(stderr, "writing to everyone not yet supported\n);
}
#endif

/*
 * clnt_call to a host that is down has a very long timeout
 * so we have only limited patience.
 *
 * We use setjmp() and all that machinery, because we want to return
 * after being patient to the main loop where we may have to rwall
 * to some other hosts also.
 */
#define PATIENCE 10

jmp_buf jmp;

scuttle()
{
	longjmp(jmp, 1);
}

doit(hostname)
	char *hostname;
{
	CLIENT *clnt;

#ifdef DEBUG
	fprintf(stderr, "sending message to %s\n%s\n", hostname, path);
	return;
#endif
	if (setjmp(jmp)) {
		fprintf(stderr,"rwall:can't sent to %s : timeout\n", hostname);
		return (-1);
	}
	signal(SIGALRM, scuttle);
	(void) alarm(PATIENCE);
	clnt = clnt_create(hostname, WALLPROG, WALLVERS, "netpath");
	if (clnt) {
		if (wallproc_wall_1(&path, clnt) == NULL) {
			clnt_perror(clnt, hostname);
		}
		clnt_destroy(clnt);
	} else {
		clnt_pcreateerror(hostname);
	}
	(void) alarm(0);
	return (0);
}

static
usage()
{
	fprintf(stderr,
		"Usage: rwall host .... [-n netgroup ....] [-h host ...]\n");
	exit(1);
}
