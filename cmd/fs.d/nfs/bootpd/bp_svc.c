/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/bootpd/bp_svc.c	1.2.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 *     Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 *
 *  (c) 1986,1987,1988.1989  Sun Microsystems, Inc
 *  (c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *            All rights reserved.
 *
 */
/*
 * Main program of the bootparam server.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <rpc/rpc.h>
#include <netinet/in.h>
#include "bootparam.h"

extern int debug;

static void background();
static void bootparamprog_1();

main(argc, argv)
	int argc;
	char **argv;
{
	struct netconfig *nconf;

	if (argc > 1)  {
		if (strncmp(argv[1], "-d", 2) == 0) {
			debug++;
			(void) fprintf(stderr, "In debug mode\n");
		} else {
			(void) fprintf(stderr, "usage: %s [-d]\n", argv[0]);
			exit(1);
		}
	}

	if (! issock(0)) {	/* started by user */
		if (!debug)
			background();

		nconf = getnetconfigent("udp");
		if (nconf == (struct netconfig *) 0) {
			(void) fprintf(stderr, "no netconfig entry for udp");
			exit(1);
		}
		rpcb_unset(BOOTPARAMPROG, BOOTPARAMVERS, nconf);
	}

	if (!svc_create(bootparamprog_1, BOOTPARAMPROG,
		BOOTPARAMVERS, "udp")) {
		(void) fprintf(stderr, "unable to register (BOOTPARAMPROG, BOOTPARAMVERS, udp).\n");
		exit(1);
	}

	/*
	 * Start serving
	 */
	svc_run();
	(void) fprintf(stderr, "svc_run returned\n");
	exit(1);

	/* NOTREACHED */
}

static void
background()
{
#ifndef DEBUG
	if (fork())
		exit(0);
	{ int s;
	for (s = 0; s < 10; s++)
		(void) close(s);
	}
	(void) open("/", O_RDONLY);
	(void) dup2(0, 1);
	(void) dup2(0, 2);
#endif
	setsid();
}

static void
bootparamprog_1(rqstp, xprt)
	struct svc_req *rqstp;
	SVCXPRT *xprt;
{
	union {
		bp_whoami_arg bootparamproc_whoami_1_arg;
		bp_getfile_arg bootparamproc_getfile_1_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();
	extern bp_whoami_res *bootparamproc_whoami_1();
	extern bp_getfile_res *bootparamproc_getfile_1();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		svc_sendreply(xprt, xdr_void, NULL);
		return;

	case BOOTPARAMPROC_WHOAMI:
		xdr_argument = xdr_bp_whoami_arg;
		xdr_result = xdr_bp_whoami_res;
		local = (char *(*)()) bootparamproc_whoami_1;
		break;

	case BOOTPARAMPROC_GETFILE:
		xdr_argument = xdr_bp_getfile_arg;
		xdr_result = xdr_bp_getfile_res;
		local = (char *(*)()) bootparamproc_getfile_1;
		break;

	default:
		svcerr_noproc(xprt);
		return;
	}
	memset(&argument, 0, sizeof(argument));
	if (! svc_getargs(xprt, xdr_argument, &argument)) {
		svcerr_decode(xprt);
		return;
	}
	if ((result = (*local)(&argument)) != NULL) {
		if (! svc_sendreply(xprt, xdr_result, result)) {
			svcerr_systemerr(xprt);
		}
	}
	if (! svc_freeargs(xprt, xdr_argument, &argument)) {
		(void) fprintf(stderr,"unable to free arguments\n");
		exit(1);
	}
}
