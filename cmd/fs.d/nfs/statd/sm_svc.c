/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/statd/sm_svc.c	1.2.2.1"
/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */
#include <stdio.h>
#include <signal.h>
#include <netconfig.h>
#include <rpc/rpc.h>
#include <netinet/in.h>
#include <sys/param.h>
#include "sm_inter.h"
#include "sm_statd.h"

#define current0	"/etc/sm"
#define backup0		"/etc/sm.bak"
#define state0		"/etc/state"

#define current1	"sm"
#define backup1		"sm.bak"
#define state1		"state"

char STATE[MAXHOSTNAMELEN], CURRENT[MAXHOSTNAMELEN], BACKUP[MAXHOSTNAMELEN];

int debug;
extern crash_notice(), recovery_notice();
void sm_try();
extern char *strcpy();

static void
sm_prog_1(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		struct sm_name sm_stat_1_arg;
		struct mon sm_mon_1_arg;
		struct mon_id sm_unmon_1_arg;
		struct my_id sm_unmon_all_1_arg;
		struct stat_chge ntf_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();
	extern struct sm_stat_res *sm_stat_1();
	extern struct sm_stat_res *sm_mon_1();
	extern struct sm_stat *sm_unmon_1();
	extern struct sm_stat *sm_unmon_all_1();
	extern void *sm_simu_crash_1();
	extern void *sm_notify();
	extern bool_t xdr_notify();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(transp, xdr_void, (caddr_t)NULL);
		return;

	case SM_STAT:
		xdr_argument = xdr_sm_name;
		xdr_result = xdr_sm_stat_res;
		local = (char *(*)()) sm_stat_1;
		break;

	case SM_MON:
		xdr_argument = xdr_mon;
		xdr_result = xdr_sm_stat_res;
		local = (char *(*)()) sm_mon_1;
		break;

	case SM_UNMON:
		xdr_argument = xdr_mon_id;
		xdr_result = xdr_sm_stat;
		local = (char *(*)()) sm_unmon_1;
		break;

	case SM_UNMON_ALL:
		xdr_argument = xdr_my_id;
		xdr_result = xdr_sm_stat;
		local = (char *(*)()) sm_unmon_all_1;
		break;

	case SM_SIMU_CRASH:
		xdr_argument = xdr_void;
		xdr_result = xdr_void;
		local = (char *(*)()) sm_simu_crash_1;
		break;

	case SM_NOTIFY:
		xdr_argument = xdr_notify;
		xdr_result = xdr_void;
		local = (char *(*)()) sm_notify;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	(void) memset (&argument, 0, sizeof (argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local)(&argument);
	if (!svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (rqstp->rq_proc != SM_MON)
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		fprintf(stderr, "rpc.statd: unable to free arguments\n");
		exit(1);
	}
}

main(argc, argv)
	int argc;
	char **argv;
{
	int t;
	int c;
	int ppid;
	extern int optind;
	extern char *optarg;
	int choice = 0;
	SVCXPRT *transp, *svc_create_statd();

	(void) signal(SIGALRM, sm_try);
	while ((c = getopt(argc, argv, "Dd:")) != EOF)
		switch (c) {
		case 'd':
			(void) sscanf(optarg, "%d", &debug);
			break;
		case 'D':
			choice = 1;
			break;
		default:
			fprintf(stderr, "rpc.statd -d[debug] -D\n");
			return (1);
		}
	if (choice == 0) {
		(void) strcpy(CURRENT, current0);
		(void) strcpy(BACKUP, backup0);
		(void) strcpy(STATE, state0);
	}
	else {
		(void) strcpy(CURRENT, current1);
		(void) strcpy(BACKUP, backup1);
		(void) strcpy(STATE, state1);
	}
	if (debug)
		printf("debug is on, create entry: %s, %s, %s\n", CURRENT, BACKUP, STATE);

	if (!debug) {
		ppid = fork();
		if (ppid == -1) {
			(void) fprintf(stderr, "rpc.statd: fork failure\n");
			(void) fflush(stderr);
			abort();
		}
		if (ppid != 0) {
			exit(0);
		}
		for (t = 0; t< 20; t++) {
			(void) close(t);
		}

		(void) open("/dev/console", 2);
		(void) open("/dev/console", 2);
		(void) open("/dev/console", 2);
		(void) setpgrp(0, 0);
	}

        if ((transp = svc_create_statd(sm_prog_1, SM_PROG, SM_VERS, "netpath", 
		"statd")) == (SVCXPRT *)NULL) {    
                perror("svc_create_statd");
                fprintf(stderr, "unable to create (SM_PROG, SM_VERS) for netpath.");        
                exit(1);
        }

	statd_init();
	svc_run();
	fprintf(stderr, "rpc.statd: svc_run returned\n");
	exit(1);
	/* NOTREACHED */
}
