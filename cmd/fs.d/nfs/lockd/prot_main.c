/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/prot_main.c	1.3.4.1"
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

#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include "prot_time.h"
#include "prot_lock.h"
#include "priv_prot.h"

int debug;
XDR x;
extern int report_sharing_conflicts;
extern int klm, nlm, HASH_SIZE;
extern FILE *fp;

extern void xtimer();
extern void priv_prog(), klm_prog(), nlm_prog();

main(argc, argv)
	int argc;
	char ** argv;
{
	int c, t, ppid;
	FILE *fopen();
	extern char *optarg;

	LM_GRACE = LM_GRACE_DEFAULT;
	LM_TIMEOUT = LM_TIMEOUT_DEFAULT;
	HASH_SIZE = 29;
	report_sharing_conflicts = 0;

	while ((c = getopt(argc, argv, "s:t:d:g:h:")) != EOF)
		switch (c) {
		case 's':
			report_sharing_conflicts++;
			break;
		case 't':
			(void) sscanf(optarg, "%d", &LM_TIMEOUT);
			break;
		case 'd':
			(void) sscanf(optarg, "%d", &debug);
			break;
		case 'g':
			(void) sscanf(optarg, "%d", &t);
			LM_GRACE = 1 + t/LM_TIMEOUT;
			break;
		case 'h':
			(void) sscanf(optarg, "%d", &HASH_SIZE);
			break;
		default:
			fprintf(stderr, "rpc.lockd -t[timeout] -g[grace_period] -d[debug]\n");
			return (0);
		}
	if (debug)
		printf("lm_timeout = %d secs, grace_period = %d secs, hashsize = %d\n",
		 LM_TIMEOUT,  LM_GRACE, HASH_SIZE);

	if (!debug) {
		ppid = fork();
		if (ppid == -1) {
			(void) fprintf(stderr, "rpc.lockd: fork failure\n");
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
	else {
		setlinebuf(stderr);
		setlinebuf(stdout);
	}

	(void) signal(SIGALRM, xtimer);

	/* NLM declaration */
	if (!svc_create(nlm_prog, NLM_PROG, NLM_VERS, "netpath")) { 
		perror("svc_create");
 		fprintf(stderr, "unable to create (NLM_PROG, NLM_VERS) for netpath.");
		exit(1);
	}

	/* NLM VERSX declaration */
	if (!svc_create(nlm_prog, NLM_PROG, NLM_VERSX, "netpath")) {
		perror("svc_create");
		fprintf(stderr, "unable to create (NLM_PROG, NLM_VERSX) for netpath.");
		exit(1);
	}

	/* KLM declaration */
	if (!svc_create_local_service(klm_prog, KLM_PROG, KLM_VERS, "netpath", 
		"lockd")) {
                perror("svc_create");
                fprintf(stderr, "unable to create (KLM_PROG, KLM_VERS) for netpath.");          
                exit(1);
        }

	/* PRIV declaration */
	if (!svc_create(priv_prog, PRIV_PROG, PRIV_VERS, "netpath")) { 
                perror("svc_create");
                fprintf(stderr, "unable to create (PRIV_PROG, PRIV_VERS) for netpath.");        
                exit(1);
        }

	init();
	init_nlm_share();

	if (debug == 3) {
		printf("lockd create logfile\n");
		klm = KLM_PROG;
		nlm = NLM_PROG;
		if ((fp = fopen("logfile", "w+")) == NULL) {
			perror("logfile fopen:");
			exit(1);
		}
		xdrstdio_create(&x, fp, XDR_ENCODE);
	}
	(void) alarm(LM_TIMEOUT);
	svc_run();
	fprintf(stderr, "svc_run returned\n");
	exit(1);
	/* NOTREACHED */
}
