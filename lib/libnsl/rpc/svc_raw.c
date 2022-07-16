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


#ident	"@(#)librpc:svc_raw.c	1.4.2.1"

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
static char sccsid[] = "@(#)svc_raw.c 1.25 89/01/31 Copyr 1984 Sun Micro";
#endif

/*
 * svc_raw.c,   This a toy for simple testing and timing.
 * Interface to create an rpc client and server in the same UNIX process.
 * This lets us similate rpc and get rpc (round trip) overhead, without
 * any interference from the kernal.
 *
 */

#include <rpc/rpc.h>
#include <rpc/raw.h>
#ifndef UDPMSGSIZE
#define	UDPMSGSIZE 8800
#endif

/*
 * This is the "network" that we will be moving data over
 */
static struct svc_raw_private {
	char	*raw_buf;	/* should be shared with the cl handle */
	SVCXPRT	server;
	XDR	xdr_stream;
	char	verf_body[MAX_AUTH_BYTES];
} *svc_raw_private;

static struct xp_ops *svc_raw_ops();
extern char *calloc();

SVCXPRT *
svc_raw_create()
{
	register struct svc_raw_private *srp = svc_raw_private;

	if (srp == NULL) {
		srp = (struct svc_raw_private *)calloc(1, sizeof (*srp));
		if (srp == NULL)
			return ((SVCXPRT *)NULL);
		if (_rawcombuf == NULL)
			_rawcombuf = (char *)calloc(UDPMSGSIZE, sizeof (char));
		srp->raw_buf = _rawcombuf; /* Share it with the client */
		svc_raw_private = srp;
	}
	srp->server.xp_fd = 0;
	srp->server.xp_port = 0;
	srp->server.xp_p3 = NULL;
	srp->server.xp_ops = svc_raw_ops();
	srp->server.xp_verf.oa_base = srp->verf_body;
	xdrmem_create(&srp->xdr_stream, srp->raw_buf, UDPMSGSIZE, XDR_FREE);
	xprt_register(&srp->server);
	return (&srp->server);
}

static enum xprt_stat
svc_raw_stat()
{
	return (XPRT_IDLE);
}

static bool_t
svc_raw_recv(xprt, msg)
	SVCXPRT *xprt;
	struct rpc_msg *msg;
{
	register struct svc_raw_private *srp = svc_raw_private;
	register XDR *xdrs;

	if (srp == NULL)
		return (FALSE);
	xdrs = &srp->xdr_stream;
	xdrs->x_op = XDR_DECODE;
	(void) XDR_SETPOS(xdrs, 0);
	if (! xdr_callmsg(xdrs, msg))
		return (FALSE);
	return (TRUE);
}

static bool_t
svc_raw_reply(xprt, msg)
	SVCXPRT *xprt;
	struct rpc_msg *msg;
{
	register struct svc_raw_private *srp = svc_raw_private;
	register XDR *xdrs;

	if (srp == NULL)
		return (FALSE);
	xdrs = &srp->xdr_stream;
	xdrs->x_op = XDR_ENCODE;
	(void) XDR_SETPOS(xdrs, 0);
	if (! xdr_replymsg(xdrs, msg))
		return (FALSE);
	(void) XDR_GETPOS(xdrs);  /* called just for overhead */
	return (TRUE);
}

static bool_t
svc_raw_getargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{
	register struct svc_raw_private *srp = svc_raw_private;

	if (srp == NULL)
		return (FALSE);
	return ((*xdr_args)(&srp->xdr_stream, args_ptr));
}

static bool_t
svc_raw_freeargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{
	register struct svc_raw_private *srp = svc_raw_private;
	register XDR *xdrs;

	if (srp == NULL)
		return (FALSE);
	xdrs = &srp->xdr_stream;
	xdrs->x_op = XDR_FREE;
	return ((*xdr_args)(xdrs, args_ptr));
}

static void
svc_raw_destroy()
{
}

static struct xp_ops *
svc_raw_ops()
{
	static struct xp_ops ops;

	if (ops.xp_recv == NULL) {
		ops.xp_recv = svc_raw_recv;
		ops.xp_stat = svc_raw_stat;
		ops.xp_getargs = svc_raw_getargs;
		ops.xp_reply = svc_raw_reply;
		ops.xp_freeargs = svc_raw_freeargs;
		ops.xp_destroy = svc_raw_destroy;
	}
	return (&ops);
}

