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

#ident	"@(#)librpc:svc_simple.c	1.4.1.1"

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
static char sccsid[] = "@(#)svc_simple.c 1.40 89/02/28 Copyr 1984 Sun Micro";
#endif

/* 
 * svc_simple.c
 * Simplified front end to rpc.
 *
 */

/*
 * This interface creates a virtual listener for all the services
 * started thru registerrpc. It listens on the same endpoint for
 * all the services and then executes the corresponding service
 * for the given prognum and procnum.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_ERR 3
#endif /* SYSLOG */
#include <rpc/nettype.h>

static struct proglst {
	char *(*p_progname)();
	u_long p_prognum;
	u_long p_versnum;
	u_long p_procnum;
	SVCXPRT *p_transp;
	char *p_netid;
	char *p_xdrbuf;
	int p_recvsz;
	xdrproc_t p_inproc, p_outproc;
	struct proglst *p_nxt;
} *proglst;

static void universal();
extern char *malloc(), *strdup();

/*
 * For simplified, easy to use kind of rpc interfaces. nettype indicates
 * the type of transport on which the service will be
 * listening. Used for conservation of the system resource. Only one
 * handle is created for all the services (actually one of each netid)
 * and same xdrbuf is used for same netid. The size of the arguments
 * is also limited by the recvsize for that transport, even if it is
 * a COTS transport. This may be wrong, but for cases like these, they
 * should not use the simplified interfaces like this.
 *
 * Should be used only by those applications where they have only
 * one PROCEDURE per program/version number.
 *
 */
rpc_reg(prognum, versnum, procnum, progname, inproc, outproc, nettype)
	u_long prognum;			/* program number */
	u_long versnum;			/* version number */
	u_long procnum;			/* procedure number */
	char *(*progname)();		/* Server routine */
	xdrproc_t inproc, outproc;	/* in/out XDR procedures */
	char *nettype;			/* nettype */
{
	struct netconfig *nconf;
	int done = FALSE;
	int net;

	if (procnum == NULLPROC) {
		(void) syslog(LOG_ERR,
		 "rpc_reg: can't reassign procedure number %d", NULLPROC);
		return (-1);
	}

	if (nettype == NULL)
		nettype = "netpath";		/* The default behavior */
	if ((net = _rpc_setconf(nettype)) == 0) {
		(void) syslog(LOG_ERR, "rpc_reg: can't find appropriate transport");
		return (-1);
	}
	while (nconf = _rpc_getconf(net)) {
		struct proglst *pl;
		SVCXPRT *svcxprt;
		int madenow;
		u_int recvsz;
		char *xdrbuf;
		char *netid;

		madenow = FALSE;
		svcxprt = (SVCXPRT *)NULL;
		for (pl = proglst; pl; pl = pl->p_nxt)
			if (strcmp(pl->p_netid, nconf->nc_netid) == 0) {
				svcxprt = pl->p_transp;
				xdrbuf = pl->p_xdrbuf;
				recvsz = pl->p_recvsz;
				netid = pl->p_netid;
				break;
			}

		if (svcxprt == (SVCXPRT *)NULL) {
			struct t_info tinfo;

			svcxprt = svc_tli_create(RPC_ANYFD, nconf,
					(struct t_bind *)NULL, 0, 0);
			if (svcxprt == (SVCXPRT *)NULL)
				continue;
			if (t_getinfo(svcxprt->xp_fd, &tinfo) == -1)
				continue;
			recvsz = _rpc_get_t_size(0, tinfo.tsdu);
			if (((xdrbuf = malloc((unsigned)recvsz)) == NULL) ||
				((netid = strdup(nconf->nc_netid)) == NULL)) {
				(void) syslog(LOG_ERR, "rpc_reg: out of memory");
				SVC_DESTROY(svcxprt);
				break;
			}
			madenow = TRUE;
		}
		(void) rpcb_unset(prognum, versnum, nconf);
		if (!svc_reg(svcxprt, prognum, versnum, universal, nconf)) {
		    	(void) syslog(LOG_ERR, "couldn't register prog %d vers %d for %s",
					prognum, versnum, nconf->nc_netid);
			if (madenow)
				SVC_DESTROY(svcxprt);
			continue;
		}

		pl = (struct proglst *)malloc(sizeof(struct proglst));
		if (pl == (struct proglst *)NULL) {
			(void) syslog(LOG_ERR, "rpc_reg: out of memory");
			if (madenow)
				SVC_DESTROY(svcxprt);
			break;
		}
		pl->p_progname = progname;
		pl->p_prognum = prognum;
		pl->p_versnum = versnum;
		pl->p_procnum = procnum;
		pl->p_inproc = inproc;
		pl->p_outproc = outproc;
		pl->p_transp = svcxprt;
		pl->p_xdrbuf = xdrbuf;
		pl->p_recvsz = recvsz;
		pl->p_netid = netid;
		pl->p_nxt = proglst;
		proglst = pl;
		done = TRUE;
	}
	_rpc_endconf();

	if (done == FALSE) {
		(void) syslog(LOG_ERR,
		"rpc_reg: cant find suitable transport for %s", nettype);
		return (-1);
	}
	return (0);
}

/*
 * The universal handler for the services registered using registerrpc.
 * It handles both the connectionless and the connection oriented cases.
 */
static void
universal(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	u_long prog, vers, proc;
	char *outdata;
	char *xdrbuf;
	struct proglst *pl;

	/* 
	 * enforce "procnum 0 is echo" convention
	 */
	if (rqstp->rq_proc == NULLPROC) {
		if (svc_sendreply(transp, xdr_void, (char *)NULL) == FALSE) {
			(void) syslog(LOG_ERR, "svc_sendreply failed");
		}
		return;
	}
	prog = rqstp->rq_prog;
	vers = rqstp->rq_vers;
	proc = rqstp->rq_proc;
	for (pl = proglst; pl; pl = pl->p_nxt)
		if (pl->p_prognum == prog && pl->p_procnum == proc &&
			pl->p_versnum == vers &&
			(strcmp(pl->p_netid, transp->xp_netid) == 0)) {
			/* decode arguments into a CLEAN buffer */
			xdrbuf = pl->p_xdrbuf;
			 /* Zero the arguments: reqd ! */
			(void) memset(xdrbuf, 0, sizeof(pl->p_recvsz));
			/*
			 * Assuming that sizeof(xdrbuf) would be enough
			 * for the arguments; if not then the program
			 * may bomb. BEWARE!
			 */
			if (!svc_getargs(transp, pl->p_inproc, xdrbuf)) {
				svcerr_decode(transp);
				return;
			}
			outdata = (*(pl->p_progname))(xdrbuf);
			if (outdata == NULL && pl->p_outproc != xdr_void)
				/* there was an error */
				return;
			if (!svc_sendreply(transp, pl->p_outproc, outdata)) {
				(void) syslog(LOG_ERR,
				"trouble replying to prog %d vers %d",
					prog, vers);
				return;
			}
			/* free the decoded arguments */
			(void) svc_freeargs(transp, pl->p_inproc, xdrbuf);
			return;
		}
	/* This should never happen */
	(void) syslog(LOG_ERR, "never registered prog %d vers %d", prog, vers);
	return;
}
