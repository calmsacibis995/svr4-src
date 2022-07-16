/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)librpc:pmap_clnt.c	1.4.1.1"

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
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)pmap_clnt.c 1.56 89/04/03 Copyr 1984 Sun Micro";
#endif

#ifdef PORTMAP
/*
 * pmap_clnt.c
 * interface to pmap rpc service.
 *
 */

#include <rpc/rpc.h>
#include <rpc/nettype.h>
#include <netdir.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_rmt.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_ERR 3
#endif /* SYSLOG */
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/byteorder.h>

static struct timeval timeout = { 5, 0 };
static struct timeval tottimeout = { 60, 0 };
static struct timeval rmttimeout = { 3, 0 };

extern char *strdup();

/*
 * Set a mapping between program,version and port.
 * Calls the pmap service remotely to do the mapping.
 */
bool_t
pmap_set(program, version, protocol, port)
	u_long program;
	u_long version;
	int protocol;
	u_short port;
{
	bool_t rslt;
	struct netbuf *na;
	struct netconfig *nconf;
	char buf[32];

	if ((protocol != IPPROTO_UDP) && (protocol != IPPROTO_TCP))
		return (FALSE);
	nconf = _rpc_getconfip(protocol == IPPROTO_UDP ? "udp" : "tcp");
	if (! nconf) {
		return (FALSE);
	}
	sprintf(buf, "0.0.0.0.%d.%d", port >> 8 & 0xff, port & 0xff);
	na = uaddr2taddr(nconf, buf);
	if (! na) {
		freenetconfigent(nconf);
		return (FALSE);
	}
	rslt = rpcb_set(program, version, nconf, na);
	netdir_free((char *)na, ND_ADDR);
	freenetconfigent(nconf);
	return (rslt);
}

/*
 * Remove the mapping between program,version and port.
 * Calls the pmap service remotely to do the un-mapping.
 */
bool_t
pmap_unset(program, version)
	u_long program;
	u_long version;
{
	struct netconfig *nconf;
	bool_t rslt = FALSE;

	nconf = _rpc_getconfip("udp");
	if (nconf) {
		rslt = rpcb_unset(program, version, nconf);
		freenetconfigent(nconf);
	}
	nconf = _rpc_getconfip("tcp");
	if (nconf) {
		rslt = rpcb_unset(program, version, nconf);
		freenetconfigent(nconf);
	}
	return (rslt);
}

/*
 * Find the mapped port for program,version.
 * Calls the pmap service remotely to do the lookup.
 * Returns 0 if no map exists.
 */
u_short
pmap_getport(address, program, version, protocol)
	struct sockaddr_in *address;
	u_long program;
	u_long version;
	u_int protocol;
{
	u_short port = 0;
	int fd = RPC_ANYFD;
	register CLIENT *client;
	struct pmap parms;

	address->sin_port = htons(PMAPPORT);
	client = clntudp_bufcreate(address, PMAPPROG, PMAPVERS, timeout,
				&fd, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client != (CLIENT *)NULL) {
		parms.pm_prog = program;
		parms.pm_vers = version;
		parms.pm_prot = protocol;
		parms.pm_port = 0;	/* not needed or used */
		if (CLNT_CALL(client, PMAPPROC_GETPORT, xdr_pmap, &parms,
			xdr_u_short, &port, tottimeout) != RPC_SUCCESS) {
			rpc_createerr.cf_stat = RPC_PMAPFAILURE;
			clnt_geterr(client, &rpc_createerr.cf_error);
		} else if (port == 0) {
			rpc_createerr.cf_stat = RPC_PROGNOTREGISTERED;
		}
		CLNT_DESTROY(client);
	}
	address->sin_port = 0;
	return (port);
}

/*
 * Get a copy of the current port maps.
 * Calls the pmap service remotely to do get the maps.
 */
struct pmaplist *
pmap_getmaps(address)
	 struct sockaddr_in *address;
{
	struct pmaplist *head = (struct pmaplist *)NULL;
	int fd = RPC_ANYFD;
	struct timeval minutetimeout;
	register CLIENT *client;

	minutetimeout.tv_sec = 60;
	minutetimeout.tv_usec = 0;
	address->sin_port = htons(PMAPPORT);
	client = clnttcp_create(address, PMAPPROG, PMAPVERS, &fd, 50, 500);
	if (client != (CLIENT *)NULL) {
		if (CLNT_CALL(client, PMAPPROC_DUMP, xdr_void, NULL,
			xdr_pmaplist, &head, minutetimeout) != RPC_SUCCESS) {
			(void) syslog(LOG_ERR, clnt_sperror(client, "pmap_getmaps rpc problem"));
		}
		CLNT_DESTROY(client);
	}
	address->sin_port = 0;
	return (head);
}

/*
 * pmapper remote-call-service interface.
 * This routine is used to call the pmapper remote call service
 * which will look up a service program in the port maps, and then
 * remotely call that routine with the given parameters. This allows
 * programs to do a lookup and call in one step.
*/
enum clnt_stat
pmap_rmtcall(addr, prog, vers, proc, xdrargs, argsp, xdrres, resp, tout, port_ptr)
	struct sockaddr_in *addr;
	u_long prog, vers, proc;
	xdrproc_t xdrargs, xdrres;
	caddr_t argsp, resp;
	struct timeval tout;
	u_long *port_ptr;
{
	int fd = RPC_ANYFD;
	register CLIENT *client;
	struct rmtcallargs a;
	struct rmtcallres r;
	enum clnt_stat stat;
	short tmp = addr->sin_port;

	addr->sin_port = htons(PMAPPORT);
	client = clntudp_create(addr, PMAPPROG, PMAPVERS, rmttimeout, &fd);
	if (client != (CLIENT *)NULL) {
		a.prog = prog;
		a.vers = vers;
		a.proc = proc;
		a.args_ptr = argsp;
		a.xdr_args = xdrargs;
		r.port_ptr = port_ptr;
		r.results_ptr = resp;
		r.xdr_results = xdrres;
		stat = CLNT_CALL(client, PMAPPROC_CALLIT, xdr_rmtcall_args, &a,
					xdr_rmtcallres, &r, tout);
		CLNT_DESTROY(client);
	} else {
		stat = RPC_FAILED;
	}
	addr->sin_port = tmp;
	return (stat);
}
#endif /* PORTMAP */
