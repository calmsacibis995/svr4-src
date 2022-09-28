/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uadmin:uadmin.c	1.4.1.1"

#include <stdio.h>
#include <signal.h>
#include <sys/uadmin.h>

char *Usage = "Usage: %s cmd fcn\n";

#ifdef i386
#include <errno.h>
#endif

main(argc, argv)
char *argv[];
{
	register cmd, fcn;
	sigset_t set, oset;

#ifdef i386
	int ret = 0;
	extern errno;
#endif

	if (argc != 3) {
		fprintf(stderr, Usage, argv[0]);
		exit(1);
	}

	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, &oset);
	
	cmd = atoi(argv[1]);
	fcn = atoi(argv[2]);

#ifdef i386
	if((cmd != A_SHUTDOWN && cmd != A_REBOOT && cmd != A_REMOUNT
	    && cmd != A_SETCONFIG)
	    || (fcn < 0 || fcn > 2)) {
		fprintf(stderr,"uadmin: invalid option\n");
		exit(1);
	}
	if(cmd == A_SETCONFIG && fcn != AD_PANICBOOT) {
		fprintf(stderr,"uadmin: invalid option\n");
		exit(1);
	}
	errno = 0;
	ret = nice(-39);
	if(ret == -1 && errno != 0) {
		fprintf(stderr,"uadmin: Can only be exectuted by root.\n");
		exit(1);
	}
#endif

	if (uadmin(cmd, fcn, 0) < 0)
		perror("uadmin");
	sigprocmask(SIG_BLOCK, &oset, (sigset_t *)0);
}
