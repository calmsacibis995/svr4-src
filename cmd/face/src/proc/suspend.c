/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)face:src/proc/suspend.c	1.4"
/*LINTLIBRARY*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include "wish.h"

int
suspend(cmd)
char *cmd;
{
	char suspath[40];
	pid_t vpid;
	FILE *fp;
	static void sig_nothing();
	
	signal(SIGUSR1, sig_nothing);

	if ((vpid = (pid_t) atol(getenv("VPID"))) == 0) {
#ifdef _DEBUG
		_debug(stderr, "Unable to get VPID\n");
#endif
		return(FAIL);
	}

	sprintf(suspath, "/tmp/suspend%ld", vpid);
	if ((fp = fopen(suspath, "w")) == NULL) {
#ifdef _DEBUG
		_debug(stderr, "Unable to open suspend file %s\n", suspath);
#endif
		return(FAIL);
	}
	(void) fprintf(fp, "%ld\n%s\n", getpid(), cmd ? cmd : "");
	(void) fclose(fp);

	if (kill(vpid, SIGUSR1) == FAIL) {
#ifdef _DEBUG
		_debug(stderr, "Unable to send sigusr1 to face pid=%ld\n", vpid);
#endif
		return(FAIL);
	}
	pause();
	return(SUCCESS);
}

/*ARGSUSED*/
static void
sig_nothing(sig)
int sig;
{
	/* do nothing, just catch the signal and return */
	return;
}
