/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libyp:yp_b_svc.c	1.6.2.1"

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

#include <stdio.h>
#include <signal.h>
#include <rpc/rpc.h>
#include <memory.h>
#include <netconfig.h>
#ifdef SYSLOG
#include <syslog.h>
#else
#define LOG_ERR 1
#define openlog(a, b, c)
#endif
#include "yp_b.h"
#include <sys/resource.h>


#ifdef DEBUG
#define RPC_SVC_FG
#endif

#define _RPCSVC_CLOSEDOWN 120
#define YPBIND_ERR_ERR 1		/* Internal error */
#define YPBIND_ERR_NOSERV 2		/* No bound server for passed domain */
#define YPBIND_ERR_RESC 3		/* System resource allocation failure */
#define YPBIND_ERR_NODOMAIN 4		/* Domain doesn't exist */

#ifdef RPC_SVC_FG
static int _rpcpmstart;         /* Started by a port monitor ? */
#endif
static int _rpcsvcdirty;	/* Still serving ? */
int setok;			/* indicates who is allowed to do ypset */

static void _msgout();
static void closedown();
void ypbindprog_3();

extern unsigned int alarm();
extern int close();
extern void exit();
extern long fork();
extern int setsid();
extern int strcmp();
extern int syslog();
extern int t_getinfo();
extern int t_sync();

main(argc, argv)
	int argc;
	char **argv;
{
	int pid, i;

	if (geteuid()!= 0) {
		(void) fprintf(stderr, "must be root to run %s\n", argv[0]);
		exit(1);
	}

	argc--;
	argv++;

	while (argc > 0) {
		if (!strcmp(*argv,"-ypset")) {
			setok = TRUE;
		} else if (!strcmp(*argv,"-ypsetme")) {
			setok = YPSETLOCAL;
		} else {
			fprintf(stderr, "usage: ypbind [-ypset] [-ypsetme]\n");
			exit(1);
		}
		argc--,
		argv++;
	}

	if (setok==TRUE) {
		fprintf(stderr, 
		    "ypbind -ypset: allowing ypset! (this is insecure)\n");
	}
	if (setok==YPSETLOCAL) {
		fprintf(stderr, 
		    "ypbind -ypsetme: allowing local ypset! (this is insecure)\n");
	}
	if (t_sync(0) != -1) {
		char *netid;
		struct netconfig *nconf;
		SVCXPRT *transp;
		extern char *getenv();

#ifdef RPC_SVC_FG
		_rpcpmstart = 1;
#endif
		if ((netid = getenv("NLSPROVIDER")) == NULL) {
			_msgout("cannot get transport name");
			exit(1);
		}
		if ((nconf = getnetconfigent(netid)) == NULL) {
			_msgout("cannot get transport info");
			exit(1);
		}
		if ((transp = svc_tli_create(0, nconf, NULL, 0, 0)) == NULL) {
			_msgout("cannot create server handle");
			exit(1);
		}
		if (!svc_reg(transp, YPBINDPROG, YPBINDVERS, ypbindprog_3, 0)) {
			_msgout("unable to register (YPBINDPROG, YPBINDVERS).");
			exit(1);
		}
		(void) signal(SIGALRM, closedown);
		(void) alarm(_RPCSVC_CLOSEDOWN);
		svc_run();
		_msgout("svc_run returned");
		exit(1);
		/* NOTREACHED */
	}
#ifndef RPC_SVC_FG
	pid = fork();
	if (pid < 0) {
		perror("cannot fork");
		exit(1);
	}
	if (pid)
		exit(0);
	for (i = 0 ; i < 20; i++)
		(void) close(i);
	(void) setsid();
	openlog("ypbind", LOG_PID, LOG_DAEMON);
#endif
	if (!svc_create(ypbindprog_3, YPBINDPROG, YPBINDVERS, "netpath")) {
 		_msgout("unable to create (YPBINDPROG, YPBINDVERS) for netpath.");
		exit(1);
	}

	svc_run();
	_msgout("svc_run returned");
	exit(1);
	/* NOTREACHED */
}

void
ypbindprog_3(rqstp, transp)
	struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	union {
		ypbind_domain ypbindproc_domain_3_arg;
		ypbind_setdom ypbindproc_setdom_3_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	_rpcsvcdirty = 1;
	switch (rqstp->rq_proc) {
	case YPBINDPROC_NULL:
		xdr_argument = xdr_void;
		xdr_result = xdr_void;
		local = (char *(*)()) ypbindproc_null_3;
		break;

	case YPBINDPROC_DOMAIN:
		xdr_argument = xdr_ypbind_domain;
		xdr_result = xdr_ypbind_resp;
		local = (char *(*)()) ypbindproc_domain_3;
		break;

	case YPBINDPROC_SETDOM:
		xdr_argument = xdr_ypbind_setdom;
		xdr_result = xdr_void;
		local = (char *(*)()) ypbindproc_setdom_3;
		break;

	default:
		svcerr_noproc(transp);
		_rpcsvcdirty = 0;
		return;
	}
	(void) memset((char *)&argument, 0, sizeof(argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		_rpcsvcdirty = 0;
		return;
	}
	if (rqstp->rq_proc == YPBINDPROC_SETDOM)
		result = (*local)(&argument, rqstp, transp);
	else
		result = (*local)(&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		_msgout("unable to free arguments");
		exit(1);
	}
	_rpcsvcdirty = 0;
	return;
}

void
_msgout(msg)
	char *msg;
{
#ifdef RPC_SVC_FG
	if (_rpcpmstart)
		syslog(LOG_ERR, msg);
	else
		(void) fprintf(stderr, "%s\n", msg);
#else
	syslog(LOG_ERR, msg);
#endif
}

void
closedown()
{
	if (_rpcsvcdirty == 0) {
		extern fd_set svc_fdset;
		static struct rlimit	rl;
		int i, openfd;
		struct t_info tinfo;

		if (t_getinfo(0, tinfo) || (tinfo.servtype == T_CLTS))
			exit(0);
		if (rl.rlim_max == 0)
			getrlimit(RLIMIT_NOFILE, &rl);
		for (i = 0, openfd = 0; i < rl.rlim_max && openfd < 2; i++)
			if (FD_ISSET(i, &svc_fdset))
				openfd++;
		if (openfd <= 1)
			exit(0);
	}
	(void) alarm(_RPCSVC_CLOSEDOWN);
}
