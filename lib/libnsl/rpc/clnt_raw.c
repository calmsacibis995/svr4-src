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

#ident	"@(#)librpc:clnt_raw.c	1.3.1.1"

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
static char sccsid[] = "@(#)clnt_raw.c 1.34 89/02/28 Copyr 1984 Sun Micro";
#endif

/*
 * clnt_raw.c
 *
 * Memory based rpc for simple testing and timing.
 * Interface to create an rpc client and server in the same process.
 * This lets us similate rpc and get round trip overhead, without
 * any interference from the kernel.
 */

#include <rpc/rpc.h>
#include <rpc/raw.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_ERR 3
#endif /* SYSLOG */

#define MCALL_MSG_SIZE 24
#ifndef UDPMSGSIZE
#define UDPMSGSIZE 8800
#endif

/*
 * This is the "network" we will be moving stuff over.
 */
static struct clnt_raw_private {
	CLIENT	client_object;
	XDR	xdr_stream;
	char	*raw_buf;	/* should be shared with server handle */
	char	mashl_callmsg[MCALL_MSG_SIZE];
	u_int	mcnt;
} *clnt_raw_private;

static struct clnt_ops *clnt_raw_ops();

extern char	*calloc();

/*
 * Create a client handle for memory based rpc.
 */
CLIENT *
clnt_raw_create(prog, vers)
	u_long prog;
	u_long vers;
{
	register struct clnt_raw_private *clp = clnt_raw_private;
	struct rpc_msg call_msg;
	XDR *xdrs;
	CLIENT *client;

	if (clp == NULL) {
		clp = (struct clnt_raw_private *)calloc(1, sizeof (*clp));
		if (clp == NULL)
			return ((CLIENT *)NULL);
		if (_rawcombuf == NULL)
			_rawcombuf = (char *)calloc(UDPMSGSIZE, sizeof(char));
		clp->raw_buf = _rawcombuf; /* Share it with the server */
		clnt_raw_private = clp;
	}
	xdrs = &clp->xdr_stream;
	client = &clp->client_object;

	/*
	 * pre-serialize the static part of the call msg and stash it away
	 */
	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = prog;
	call_msg.rm_call.cb_vers = vers;
	xdrmem_create(xdrs, clp->mashl_callmsg, MCALL_MSG_SIZE, XDR_ENCODE); 
	if (! xdr_callhdr(xdrs, &call_msg)) {
		(void) syslog(LOG_ERR, "clnt_raw_create - Fatal header serialization error.");
	}
	clp->mcnt = XDR_GETPOS(xdrs);
	XDR_DESTROY(xdrs);

	/*
	 * Set xdrmem for client/server shared buffer
	 */
	xdrmem_create(xdrs, clp->raw_buf, UDPMSGSIZE, XDR_FREE);

	/*
	 * create client handle
	 */
	client->cl_ops = clnt_raw_ops();
	client->cl_auth = authnone_create();
	return (client);
}

static enum clnt_stat 
clnt_raw_call(h, proc, xargs, argsp, xresults, resultsp, timeout)
	CLIENT *h;
	u_long proc;
	xdrproc_t xargs;
	caddr_t argsp;
	xdrproc_t xresults;
	caddr_t resultsp;
	struct timeval timeout;
{
	register struct clnt_raw_private *clp = clnt_raw_private;
	register XDR *xdrs = &clp->xdr_stream;
	struct rpc_msg msg;
	enum clnt_stat status;
	struct rpc_err error;

	if (clp == NULL)
		return (RPC_FAILED);
call_again:
	/*
	 * send request
	 */
	xdrs->x_op = XDR_ENCODE;
	XDR_SETPOS(xdrs, 0);
	((struct rpc_msg *)clp->mashl_callmsg)->rm_xid++ ;
	if ((! XDR_PUTBYTES(xdrs, clp->mashl_callmsg, clp->mcnt)) ||
	    (! XDR_PUTLONG(xdrs, (long *)&proc)) ||
	    (! AUTH_MARSHALL(h->cl_auth, xdrs)) ||
	    (! (*xargs)(xdrs, argsp))) {
		return (RPC_CANTENCODEARGS);
	}
	(void) XDR_GETPOS(xdrs);  /* called just to cause overhead */

	/*
	 * We have to call server input routine here because this is
	 * all going on in one process.
	 */
	svc_getreq(1);

	/*
	 * get results
	 */
	xdrs->x_op = XDR_DECODE;
	XDR_SETPOS(xdrs, 0);
	msg.acpted_rply.ar_verf = _null_auth;
	msg.acpted_rply.ar_results.where = resultsp;
	msg.acpted_rply.ar_results.proc = xresults;
	if (! xdr_replymsg(xdrs, &msg))
		return (RPC_CANTDECODERES);
	_seterr_reply(&msg, &error);
	status = error.re_status;

	if (status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(h->cl_auth, &msg.acpted_rply.ar_verf)) {
			status = RPC_AUTHERROR;
		}
		/* end successful completion */
	} else {
		if (AUTH_REFRESH(h->cl_auth))
			goto call_again;
		/* end of unsuccessful completion */
	}

	if (status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(h->cl_auth, &msg.acpted_rply.ar_verf)) {
			status = RPC_AUTHERROR;
		}
		if (msg.acpted_rply.ar_verf.oa_base != NULL) {
			xdrs->x_op = XDR_FREE;
			(void) xdr_opaque_auth(xdrs, &(msg.acpted_rply.ar_verf));
		}
	}
	return (status);
}

static void
clnt_raw_geterr()
{
}

static bool_t
clnt_raw_freeres(cl, xdr_res, res_ptr)
	CLIENT *cl;
	xdrproc_t xdr_res;
	caddr_t res_ptr;
{
	register struct clnt_raw_private *clp = clnt_raw_private;
	register XDR *xdrs = &clp->xdr_stream;

	if (clp == NULL)
		return (FALSE);
	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr));
}

static void
clnt_raw_abort()
{
}

static bool_t
clnt_raw_control()
{
	return (FALSE);
}

static void
clnt_raw_destroy()
{
}

static struct clnt_ops *
clnt_raw_ops()
{
	static struct clnt_ops ops;

	if (ops.cl_call == NULL) {
		ops.cl_call = clnt_raw_call;
		ops.cl_abort = clnt_raw_abort;
		ops.cl_geterr = clnt_raw_geterr;
		ops.cl_freeres = clnt_raw_freeres;
		ops.cl_destroy = clnt_raw_destroy;
		ops.cl_control = clnt_raw_control;
	}
	return (&ops);
}
