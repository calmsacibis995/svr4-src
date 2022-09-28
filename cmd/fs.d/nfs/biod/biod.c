/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/biod/biod.c	1.3.2.1"

/*
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/*
 * This is the NFS asynchronous block I/O daemon
 */

main(argc, argv)
	int argc;
	char *argv[];
{
	extern int errno;
	int pid;
	int count;

	if (argc > 2) {
		usage(argv[0]);
	}

	if (argc == 2) {
		count = atoi(argv[1]);
		if (count < 0) {
			usage(argv[0]);
		}
	} else {
		count = 1;
	}

	setsid();
	while (count--) {
		pid = fork();
		if (pid == 0) {
			async_daemon();		/* Should never return */
			fprintf(stderr, "%s: async_daemon ", argv[0]);
			perror("");
			exit(1);
		}
		if (pid < 0) {
			fprintf(stderr, "%s: cannot fork", argv[0]);
			perror("");
			exit(1);
		}
	}
	exit(0);
	/* NOTREACHED */
}

usage(name)
	char	*name;
{

	fprintf(stderr, "usage: %s [<count>]\n", name);
	exit(1);
}
