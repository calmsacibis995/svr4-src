/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:proc/suspend.c	1.5"

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>	/* EFT abs k16 */
#include "wish.h"

int
suspend(cmd)
char *cmd;
{
    char suspath[40];
    pid_t vpid;			/* EFT abs k16 */
    FILE *fp;
    void sig_nothing();
	
    sigset(SIGUSR1, sig_nothing);

    if ((vpid = strtol(getenv("VPID"), (char **)NULL, 0)) == 0) /* EFT k16 */
    {
#ifdef _DEBUG
	_debug(stderr, "Unable to get VPID\n");
#endif
	return(FAIL);
    }

    sprintf(suspath, "/tmp/suspend%d", vpid);
    if ((fp = fopen(suspath, "w")) == NULL) {
#ifdef _DEBUG
	_debug(stderr, "Unable to open suspend file %s\n", suspath);
#endif
	return(FAIL);
    }
    (void) fprintf(fp, "%d\n%s\n", getpid(), cmd ? cmd : "");
    (void) fclose(fp);

    if (kill(vpid, SIGUSR1) == FAIL) {
#ifdef _DEBUG
	_debug(stderr, "Unable to send sigusr1 to face pid=%d\n", vpid);
#endif
	return(FAIL);
    }
    pause();
    return(SUCCESS);
}


static void
sig_nothing(sig)
int sig;
{
	/* do nothing, just catch the signal and return */
	return;
}
