/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbrenice:renice.c	1.3.1.1"

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

	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#ifndef lint
static	char *sccsid = "@(#)renice.c 1.7 88/08/04 SMI"; /* from UCB 4.6 83/07/24 */
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <pwd.h>

#define PRIO_MAX	20
#define PRIO_MIN	-20

/*
 * Change the priority (nice) of processes
 * or groups of processes which are already
 * running.
 */
main(argc, argv)
	char **argv;
{
	int which = PRIO_PROCESS;
	int who = 0, prio, errs = 0;
	struct passwd *pwd;
	char *c;

	argc--, argv++;
	if (argc < 2) {
		fprintf(stderr, "usage: renice priority [ [ -p ] pids ] ");
		fprintf(stderr, "[ [ -g ] pgrps ] [ [ -u ] users ]\n");
		exit(1);
	}
	prio = atoi(*argv);
	argc--, argv++;
	if (prio > PRIO_MAX)
		prio = PRIO_MAX;
	if (prio < PRIO_MIN)
		prio = PRIO_MIN;
	for (; argc > 0; argc--, argv++) {
		if (strcmp(*argv, "-g") == 0) {
			which = PRIO_PGRP;
			continue;
		}
		if (strcmp(*argv, "-u") == 0) {
			which = PRIO_USER;
			continue;
		}
		if (strcmp(*argv, "-p") == 0) {
			which = PRIO_PROCESS;
			continue;
		}
		if (which == PRIO_USER) {
                      if ((who = getint(*argv)) < 0) {
				pwd = getpwnam(*argv);
				if (pwd == NULL) {
					fprintf(stderr, "renice: %s: unknown user\n",
					*argv);
					continue;
				} else 
					who = pwd->pw_uid;
			}
		} else {
                      if ((who = getint(*argv)) < 0) {
				fprintf(stderr, "renice: %s: bad value\n",
					*argv);
				continue;
			}
		}
		errs += donice(which, who, prio);
	}
	exit(errs != 0);
	/* NOTREACHED */
}

donice(which, who, prio)
	int which, who, prio;
{
	int oldprio;
	extern int errno;

	errno = 0, oldprio = getpriority(which, who);
	if (oldprio == -1 && errno) {
		fprintf(stderr, "renice: %d: ", who);
		perror("getpriority");
		return (1);
	}
	if (setpriority(which, who, prio) < 0) {
		fprintf(stderr, "renice: %d: ", who);
		perror("setpriority");
		return (1);
	}
	printf("%d: old priority %d, new priority %d\n", who, oldprio, prio);
	return (0);
}


getint(arg)
char *arg;
{
	char *c;

        c = arg;
        while (*c != '\0') {
		if (!isdigit(*c)) 
        		break;
        	else c++;
        }
        if (*c == '\0')
        	return(atoi(arg));
	else 
		return(-1);
}
