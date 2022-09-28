/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rpcbind:pmap_svc.c	1.4.2.1"

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
static	char sccsid[] = "@(#)pmap_svc.c 1.23 89/04/05 Copyr 1984 Sun Micro";
#endif

/*
 * pmap_svc.c
 * The server procedure for the version 2 rpcbinder.
 * All the portmapper related interface from the portmap side.
 */

#ifdef PORTMAP
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "rpcbind.h"

#ifndef INADDR_LOOPBACK		/* Some <netinet/in.h> files do not have this */
#define	INADDR_LOOPBACK		(u_long)0x7F000001
#endif

static bool_t xdr_opaque_parms();
static bool_t xdr_len_opaque_parms();
static PMAPLIST *find_service();

/* 
 * 1 OK, 0 not
 * Called for all the version 2 inquiries.
 */
int
pmap_service(rqstp, xprt)
	register struct svc_req *rqstp;
	register SVCXPRT *xprt;
{
	PMAP reg;
	PMAPLIST *fnd;
	int ans, port;

	if (debugging)
		printf("rpcbind: request for proc %d\n", rqstp->rq_proc);

	switch (rqstp->rq_proc) {
	case PMAPPROC_NULL:
		/*
		 * Null proc call
		 */
#ifdef DEBUG
		printf("PMAPPROC_NULL\n");
#endif
		if ((!svc_sendreply(xprt, xdr_void, NULL)) && debugging) {
			abort();
		}
		break;

	case PMAPPROC_SET:
		/*
		 * Set a program,version to port mapping
		 */
#ifdef DEBUG
		printf("PMAPPROC_SET\n");
#endif
		/* The user should be using RPCBPROC_SET */
		ans = 0;
		if ((!svc_sendreply(xprt, xdr_long, (caddr_t)&ans)) &&
				debugging) {
			(void) fprintf(stderr, "rpcbind: svc_sendreply\n");
			abort();
		}
		break;

	case PMAPPROC_UNSET:
		/*
		 * Remove a program,version to port mapping.
		 */
#ifdef DEBUG
		printf("PMAPPROC_UNSET \n");
#endif
		/* The user should be using RPCBPROC_UNSET */
		ans = 0;
		if ((!svc_sendreply(xprt, xdr_long, (caddr_t)&ans)) &&
				debugging) {
			(void) fprintf(stderr, "rpcbind: svc_sendreply\n");
			abort();
		}
		break;

	case PMAPPROC_GETPORT:
		/*
		 * Lookup the mapping for a program,version and return its
		 * port number.
		 */
#ifdef DEBUG
		printf("PMAPPROC_GETPORT\n");
#endif
		if (!svc_getargs(xprt, xdr_pmap, &reg))
			svcerr_decode(xprt);
		else {
			fnd = find_service(reg.pm_prog, reg.pm_vers, reg.pm_prot);
			if (fnd) {
				char serveuaddr[32], *ua;
				int h1, h2, h3, h4, p1, p2;
				char *netid;

				if (reg.pm_prot == IPPROTO_UDP) {
					ua = udp_uaddr;
					netid = udptrans;
				} else {
					ua = tcp_uaddr; /* To get the len */
					netid = tcptrans;
				}
				sscanf(ua, "%d.%d.%d.%d.%d.%d", &h1, &h2, &h3,
						&h4, &p1, &p2);
				p1 = (fnd->pml_map.pm_port >> 8) & 0xff;
				p2 = (fnd->pml_map.pm_port) & 0xff;
				sprintf(serveuaddr, "%d.%d.%d.%d.%d.%d",
						h1, h2, h3, h4, p1, p2);
				if (is_bound(netid, serveuaddr))
					port = fnd->pml_map.pm_port;
				else
					/*
					 * XXX: This registration should be 
					 * deleted
					 */
					port = 0;
			} else
				port = 0;
			if ((!svc_sendreply(xprt, xdr_long, (caddr_t)&port)) &&
					debugging) {
				(void) fprintf(stderr, "rpcbind: svc_sendreply\n");
				abort();
			}
		}
		break;

	case PMAPPROC_DUMP:
		/*
		 * Return the current set of mapped program,version
		 */
#ifdef DEBUG
		printf("PMAPPROC_DUMP\n");
#endif
		if (!svc_getargs(xprt, xdr_void, NULL))
			svcerr_decode(xprt);
		else {
			if ((!svc_sendreply(xprt, xdr_pmaplist,
					(caddr_t)&list_pml)) && debugging) {
				(void) fprintf(stderr, "rpcbind: svc_sendreply\n");
				abort();
			}
		}
		break;

	case PMAPPROC_CALLIT:
		/*
		 * Calls a procedure on the local machine. If the requested
		 * procedure is not registered this procedure does not return
		 * error information!!
		 * This procedure is only supported on rpc/udp and calls via 
		 * rpc/udp. It passes null authentication parameters.
		 */
#ifdef DEBUG
		printf("PMAPPROC_CALLIT\n");
#endif
		callit(rqstp, xprt);
		break;

	default:
		svcerr_noproc(xprt);
		break;
	}
}

/*
 * Stuff for the rmtcall service
 */
#define ARGSIZE 9000

struct encap_parms {
	u_long arglen;
	char *args;
};

static bool_t
xdr_encap_parms(xdrs, epp)
	XDR *xdrs;
	struct encap_parms *epp;
{

	return (xdr_bytes(xdrs, &(epp->args), &(epp->arglen), ARGSIZE));
}

/*
 * Similar definitions and calls exist in <rpc/pmap_rmt.h> and
 * <rpc/pmap_prot.c>; but these calls are different.
 */
struct rmtcall_args {
	u_long	rmt_prog;
	u_long	rmt_vers;
	u_long	rmt_port;
	u_long	rmt_proc;
	struct encap_parms rmt_args;
};

static bool_t
xdr_rmtcall_args(xdrs, cap)
	register XDR *xdrs;
	register struct rmtcall_args *cap;
{
	/* does not get a port number */
	if (xdr_u_long(xdrs, &(cap->rmt_prog)) &&
	    xdr_u_long(xdrs, &(cap->rmt_vers)) &&
	    xdr_u_long(xdrs, &(cap->rmt_proc))) {
		return (xdr_encap_parms(xdrs, &(cap->rmt_args)));
	}
	return (FALSE);
}

static bool_t
xdr_rmtcall_result(xdrs, cap)
	register XDR *xdrs;
	register struct rmtcall_args *cap;
{
	if (xdr_u_long(xdrs, &(cap->rmt_port)))
		return (xdr_encap_parms(xdrs, &(cap->rmt_args)));
	return (FALSE);
}

/*
 * only worries about the struct encap_parms part of struct rmtcallargs.
 * The arglen must already be set!!
 */
static bool_t
xdr_opaque_parms(xdrs, cap)
	XDR *xdrs;
	struct rmtcall_args *cap;
{

	return (xdr_opaque(xdrs, cap->rmt_args.args, cap->rmt_args.arglen));
}

/*
 * This routine finds and sets the length of incoming opaque paraters
 * and then calls xdr_opaque_parms.
 */
static bool_t
xdr_len_opaque_parms(xdrs, cap)
	register XDR *xdrs;
	struct rmtcall_args *cap;
{
	register u_int beginpos, lowpos, highpos, currpos, pos;

	beginpos = lowpos = pos = xdr_getpos(xdrs);
	highpos = lowpos + ARGSIZE;
	while ((int)(highpos - lowpos) >= 0) {
		currpos = (lowpos + highpos) / 2;
		if (xdr_setpos(xdrs, currpos)) {
			/* Within the range */
			pos = currpos;
			lowpos = currpos + 1;
		} else {
			/* Out of the range */
			highpos = currpos - 1;
		}
	}
	xdr_setpos(xdrs, beginpos);
	cap->rmt_args.arglen = pos - beginpos;
	return (xdr_opaque_parms(xdrs, cap));
}

/*
 * Call a remote procedure service
 * This procedure is very quiet when things go wrong.
 * The proc is written to support broadcast rpc. In the broadcast case,
 * a machine should shut-up instead of complain, less the requestor be
 * overrun with complaints at the expense of not hearing a valid reply ...
 *
 * This now forks so that the program & process that it calls can call 
 * back to the rpcbinder.
 */
static
callit(rqstp, xprt)
	struct svc_req *rqstp;
	SVCXPRT *xprt;
{
	struct rmtcall_args a;
	PMAPLIST *pml;
	u_short port;
	struct sockaddr_in me;
	pid_t pid;
	int socket = RPC_ANYSOCK;
	CLIENT *client;
	struct authsys_parms *au = (struct authsys_parms *)rqstp->rq_clntcred;
	struct timeval timeout;
	char buf[ARGSIZE];

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	a.rmt_args.args = buf;
	if (!svc_getargs(xprt, xdr_rmtcall_args, &a))
		return;
	if ((pml = find_service(a.rmt_prog, a.rmt_vers, (u_long)IPPROTO_UDP)) == NULL)
		return;
	/*
	 * fork a child to do the work. Parent immediately returns.
	 * Child exits upon completion.
	 */
	if ((pid = fork()) != 0) {
		if (debugging && (pid < (pid_t)0)) {
			(void) fprintf(stderr, "rpcbind CALLIT: cannot fork.\n");
		}
		return;
	}
	port = pml->pml_map.pm_port;
	get_myaddress(&me);
	me.sin_port = htons(port);
	client = clntudp_create(&me, a.rmt_prog, a.rmt_vers, timeout, &socket);
	if (client != (CLIENT *)NULL) {
		if (rqstp->rq_cred.oa_flavor == AUTH_UNIX) {
			client->cl_auth = authsys_create(au->aup_machname,
						au->aup_uid, au->aup_gid,
						au->aup_len, au->aup_gids);
		}
		a.rmt_port = (u_long)port;
		if (clnt_call(client, a.rmt_proc, xdr_opaque_parms, 
				&a, xdr_len_opaque_parms, 
				&a, timeout) == RPC_SUCCESS) {
			svc_sendreply(xprt, xdr_rmtcall_result, &a);
		}
		AUTH_DESTROY(client->cl_auth);
		CLNT_DESTROY(client);
	}
	exit(0);
}

/*
 * returns the item with the given program,version number. If that version
 * number is not found, it returns the item with that program number, so that
 * the port number is now returned to the caller. The caller when makes a
 * call to this program, version number, the call will fail and it will
 * return with PROGVERS_MISMATCH. The user can then determine the highest
 * and the lowest version number for this program using clnt_geterr() and
 * use those program version numbers.
 */
static PMAPLIST *
find_service(prog, vers, prot)
	u_long prog;
	u_long vers;
	u_long prot;
{
	register PMAPLIST *hit = NULL;
	register PMAPLIST *pml;

	for (pml = list_pml; pml != NULL; pml = pml->pml_next) {
		if ((pml->pml_map.pm_prog != prog) ||
			(pml->pml_map.pm_prot != prot))
			continue;
		hit = pml;
		if (pml->pml_map.pm_vers == vers)
			break;
	}
	return (hit);
}
#endif PORTMAP
