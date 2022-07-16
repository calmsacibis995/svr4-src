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


#ident	"@(#)librpc:svc_vc.c	1.8.2.1"

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
/*
 * svc_vc.c, Server side for Connection Oriented RPC.
 *
 * Actually implements two flavors of transporter -
 * a rendezvouser (a listener and connection establisher)
 * and a record stream.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <errno.h>
#include <sys/stat.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define	LOG_ERR 3
#endif /* SYSLOG */
#include <rpc/nettype.h>

#ifndef MIN
#define	MIN(a, b)	(((a) < (b)) ? (a) : (b))
#endif

extern bool_t	abort();
extern int	errno;

static struct xp_ops 	*svc_vc_ops();
static struct xp_ops 	*svc_vc_rendezvous_ops();
static void		svc_vc_destroy();
static int 		read_vc();
static int 		write_vc();
static SVCXPRT 		*makefd_xprt();
extern char 		*strdup(), *malloc();

struct cf_rendezvous { /* kept in xprt->xp_p1 for rendezvouser */
	u_int sendsize;
	u_int recvsize;
};

struct cf_conn {	/* kept in xprt->xp_p1 for actual connection*/
	enum xprt_stat strm_stat;
	u_long x_id;
	XDR xdrs;
	char verf_body[MAX_AUTH_BYTES];
};

/*
 * Usage:
 *	xprt = svc_vc_create(fd, sendsize, recvsize);
 * Since connection streams do buffered io similar to stdio, the caller
 * can specify how big the send and receive buffers are. If recvsize
 * or sendsize are 0, defaults will be chosen.
 * fd should be open and bound.
 */
SVCXPRT *
svc_vc_create(fd, sendsize, recvsize)
	register int fd;
	u_int sendsize;
	u_int recvsize;
{
	register struct cf_rendezvous *r;
	SVCXPRT *xprt;
	struct t_info tinfo;

	xprt = (SVCXPRT *)mem_alloc(sizeof (SVCXPRT));
	if (xprt == (SVCXPRT *)NULL) {
		(void) syslog(LOG_ERR, "svc_vc_create: out of memory");
		return ((SVCXPRT *)NULL);
	}
	memset((char *)xprt, 0, sizeof (SVCXPRT));

	r = (struct cf_rendezvous *)mem_alloc(sizeof (*r));
	if (r == (struct cf_rendezvous *)NULL) {
		(void) syslog(LOG_ERR, "svc_vc_create: out of memory");
		(void) mem_free(xprt, sizeof (SVCXPRT));
		return ((SVCXPRT *)NULL);
	}
	if (t_getinfo(fd, &tinfo) == -1) {
		if ((sendsize == 0) || (recvsize == 0)) {
			(void) syslog(LOG_ERR,
		"svc_vc_create: could not get transport information");
			(void) mem_free(xprt, sizeof (SVCXPRT));
			return ((SVCXPRT *)NULL);
		}
	} else {
		/*
		 * Find the receive and the send size
		 */
		sendsize = _rpc_get_t_size((int)sendsize, tinfo.tsdu);
		recvsize = _rpc_get_t_size((int)recvsize, tinfo.tsdu);
	}
	r->sendsize = sendsize;
	r->recvsize = recvsize;
	xprt->xp_fd = fd;
	xprt->xp_port = -1;	/* It is the rendezvouser */
	xprt->xp_p1 = (caddr_t)r;
	xprt->xp_p2 = NULL;
	xprt->xp_p3 = NULL;
	xprt->xp_verf = _null_auth;
	xprt->xp_ops = svc_vc_rendezvous_ops();
	xprt_register(xprt);
	return (xprt);
}

/*
 * used for the actual connection.
 */
SVCXPRT *
svc_fd_create(fd, sendsize, recvsize)
	int fd;
	u_int sendsize;
	u_int recvsize;
{
	struct t_info tinfo;

	if (t_getinfo(fd, &tinfo) == -1) {
		if ((sendsize == 0) || (recvsize == 0)) {
			(void) syslog(LOG_ERR,
		"svc_fd_create: could not get transport information");
			return ((SVCXPRT *)NULL);
		}
	} else {
		/*
		 * Find the receive and the send size
		 */
		sendsize = _rpc_get_t_size((int)sendsize, tinfo.tsdu);
		recvsize = _rpc_get_t_size((int)recvsize, tinfo.tsdu);
	}

	return (makefd_xprt(fd, sendsize, recvsize));
}

static SVCXPRT *
makefd_xprt(fd, sendsize, recvsize)
	int fd;
	u_int sendsize;
	u_int recvsize;
{
	register SVCXPRT *xprt;
	register struct cf_conn *cd;

	xprt = (SVCXPRT *)mem_alloc(sizeof (SVCXPRT));
	if (xprt == (SVCXPRT *)NULL) {
		(void) syslog(LOG_ERR, "svc_vc: makefd_xprt: out of memory");
		return ((SVCXPRT *)NULL);
	}
	(void) memset((char *)xprt, 0, sizeof (SVCXPRT));
	cd = (struct cf_conn *)mem_alloc(sizeof (struct cf_conn));
	if (cd == (struct cf_conn *)NULL) {
		(void) syslog(LOG_ERR, "svc_vc: makefd_xprt: out of memory");
		(void) mem_free((char *) xprt, sizeof (SVCXPRT));
		return ((SVCXPRT *)NULL);
	}
	cd->strm_stat = XPRT_IDLE;
	xdrrec_create(&(cd->xdrs), sendsize, recvsize, (caddr_t)xprt,
			read_vc, write_vc);
	xprt->xp_p1 = (caddr_t)cd;
	xprt->xp_p2 = NULL;
	xprt->xp_verf.oa_base = cd->verf_body;
	xprt->xp_ops = svc_vc_ops();	/* truely deals with calls */
	xprt->xp_port = 0;	/* this is a connection, not a rendezvouser */
	xprt->xp_fd = fd;
	xprt_register(xprt);
	return (xprt);
}

/*
 * This routine is called by svc_getreqset(), when a packet is recd.
 * The listener process creates another end point on which the actual
 * connection is carried. It returns FALSE to indicate that it was
 * not a rpc packet (falsely though), but as a side effect creates
 * another endpoint which is also registered, which then always
 * has a request ready to be served.
 */
static bool_t
rendezvous_request(xprt)
	register SVCXPRT *xprt;
{
	register SVCXPRT *xprtnew = NULL;
	register int fd = RPC_ANYFD;
	struct cf_rendezvous *r;
	struct t_call *t_call, t_call2;
	struct t_info tinfo;
	struct t_bind *res = NULL;
	char *tpname = NULL;
	char devbuf[256];
	extern int errno, t_errno;

	r = (struct cf_rendezvous *)xprt->xp_p1;

again:
	switch (t_look(xprt->xp_fd)) {
	case T_DISCONNECT:
		(void) t_rcvdis(xprt->xp_fd, NULL);
		return (FALSE);

	case T_LISTEN:

		t_call = (struct t_call *) t_alloc(xprt->xp_fd,
				T_CALL, T_ADDR | T_OPT);
		if (t_call == NULL) {
			(void) syslog(LOG_ERR, "rendezvous_request: no memory");
			return (FALSE);
		}
		if (t_listen(xprt->xp_fd, t_call) == -1) {
			if (errno == EINTR)
				goto again;
			(void) t_free((char *)t_call, T_CALL);
			return (FALSE);
		}
		break;
	default:
		return (FALSE);
	}
	/*
	 * Now create another endpoint, and accept the connection
	 * on it.
	 */
	if (xprt->xp_tp) {
		tpname = xprt->xp_tp;
	} else {
		/*
		 * If xprt->xp_tp is NULL, then try all
		 * possible connection oriented transports until
		 * one succeeds in finding an appropriate one.
		 */
		struct stat statbuf;
		struct stat fdstatbuf;
		void *hndl;
		struct netconfig *nconf;

		if (fstat(xprt->xp_fd, &statbuf) == -1) {
			(void) syslog(LOG_ERR,
			"rendezvous_request: cant find dev number");
			goto err;
		}

		hndl = setnetconfig();
		if (hndl == NULL) {
			(void) syslog(LOG_ERR,
		"rendezvous_request: cannot read netconfig database");
			goto err;
		}
		tpname = devbuf;
		while (nconf = getnetconfig(hndl)) {
			if ((nconf->nc_semantics != NC_TPI_COTS) &&
				(nconf->nc_semantics != NC_TPI_COTS_ORD))
				continue;
			if (!stat(nconf->nc_device, &fdstatbuf) &&
				(statbuf.st_dev == fdstatbuf.st_dev)) {
				strcpy(tpname, nconf->nc_device);
				break;
			}
		}
		endnetconfig(hndl);
		if (!nconf) {
			(void) syslog(LOG_ERR,
			"rendezvous_request: no suitable transport");
			goto err;
		}
	}
	fd = t_open(tpname, O_RDWR, &tinfo);
	if (fd == -1) {
		(void) syslog(LOG_ERR,
			"rendezvous_request: cant open connection");
		goto err;
	}
	if ((tinfo.servtype != T_COTS) &&
			(tinfo.servtype != T_COTS_ORD)) {
		/* Not a connection oriented mode */
		(void) syslog(LOG_ERR,
			"rendezvous_request: illegal transport");
		goto err;
	}
	res = (struct t_bind *) t_alloc(fd, T_BIND, T_ADDR);
	if (res == NULL) {
		(void) syslog(LOG_ERR, "rendezvous_request: no memory");
		goto err;
	}
	if (t_bind(fd, (struct t_bind *)NULL, res) == -1) {
		(void) syslog(LOG_ERR, "cannot bind svc connection");
		goto err;
	}
	/*
	 * This connection is not listening, hence no need to set
	 * the qlen.
	 */

	/*
	 * XXX: The local transport chokes on its own listen
	 * options so we zero them for now
	 */
	t_call2 = *t_call;
	t_call2.opt.len = 0;
	t_call2.opt.maxlen = 0;
	t_call2.opt.buf = NULL;

	if (t_accept(xprt->xp_fd, fd, &t_call2) == -1) {
		if (t_errno == TLOOK)
			(void) t_rcvdis(xprt->xp_fd, NULL);
		(void) syslog(LOG_ERR, 	"cannot accept connection");
		goto err;
	}
	/*
	 * make a new transporter
	 */
	xprtnew = makefd_xprt(fd, r->sendsize, r->recvsize);
	if (xprtnew == (SVCXPRT *)NULL)
		goto err;
	/*
	 * Copy the new local and remote bind information
	 */
	xprtnew->xp_ltaddr = res->addr;
	res->addr.buf = (char *)NULL;
	(void) t_free((char *)res, T_BIND);
	xprtnew->xp_rtaddr = t_call->addr;

	xprtnew->xp_tp = strdup(tpname);
	xprtnew->xp_netid = strdup(xprt->xp_netid);
	if ((xprtnew->xp_tp == NULL) || (xprtnew->xp_netid == NULL)) {
		(void) syslog(LOG_ERR,
			"rendezvous_request: no memory");
		goto err;
	}
	if (t_call->opt.len > 0) {
		xprtnew->xp_p2 = malloc(sizeof (struct netbuf));
		if (xprtnew->xp_p2 != NULL) {
			*((struct netbuf *) xprtnew->xp_p2)= t_call->opt;
			t_call->opt.buf = NULL;
		}
	}
	t_call->addr.buf = (char *)NULL;
	(void) t_free((char *)t_call, T_CALL);
	return (FALSE); /* there is never an rpc msg to be processed */
err:
	(void) t_free((char *)t_call, T_CALL);
	if (res)
		(void) t_free((char *)res, T_BIND);
	if (xprtnew)
		svc_vc_destroy(xprtnew);
	else if (fd != RPC_ANYFD)
		t_close(fd);
	return (FALSE);
}

static enum xprt_stat
rendezvous_stat()
{
	return (XPRT_IDLE);
}

static void
svc_vc_destroy(xprt)
	register SVCXPRT *xprt;
{
	register struct cf_conn *cd = (struct cf_conn *)xprt->xp_p1;

	xprt_unregister(xprt);
	(void) t_close(xprt->xp_fd);
	if (xprt->xp_port != 0) {
		/* a rendezvouser end point */
		xprt->xp_port = 0;
	} else {
		/* an actual connection end point */
		XDR_DESTROY(&(cd->xdrs));
	}
	(void) mem_free((caddr_t)cd, sizeof (*cd));
	if (xprt->xp_rtaddr.buf)
		(void) mem_free(xprt->xp_rtaddr.buf, xprt->xp_rtaddr.maxlen);
	if (xprt->xp_ltaddr.buf)
		(void) mem_free(xprt->xp_ltaddr.buf, xprt->xp_ltaddr.maxlen);
	if (xprt->xp_tp)
		(void) free(xprt->xp_tp);
	if (xprt->xp_netid)
		(void) free(xprt->xp_netid);
	(void) mem_free((caddr_t)xprt, sizeof (SVCXPRT));

	if (xprt->xp_p2) {
		(void) mem_free((caddr_t)((struct netbuf *) xprt->xp_p2)->buf,
			((struct netbuf *) xprt->xp_p2)->len);
		(void) mem_free((struct netbuf *) xprt->xp_p2,
			sizeof (struct netbuf));
	}
}

/*
 * All read operations timeout after 35 seconds.
 * A timeout is fatal for the connection.
 */
static struct timeval wait_per_try = { 35, 0 };

/*
 * reads data from the vc conection.
 * any error is fatal and the connection is closed.
 * (And a read of zero bytes is a half closed stream => error.)
 */
static int
read_vc(xprt, buf, len)
	register SVCXPRT *xprt;
	caddr_t buf;
	register int len;
{
	register int fd = xprt->xp_fd;
	fd_set mask;
	fd_set readfds;

	FD_ZERO(&mask);
	FD_SET(fd, &mask);
	do {
		readfds = mask;
		if (select(_rpc_dtbsize(), &readfds, (fd_set *)NULL,
			(fd_set *)NULL, &wait_per_try) <= 0) {
			if (errno == EINTR)
				continue;
			goto fatal_err;
		}
	} while (!FD_ISSET(fd, &readfds));
	if ((len = t_rcvall(fd, buf, len)) > 0)
		return (len);
fatal_err:
	((struct cf_conn *)(xprt->xp_p1))->strm_stat = XPRT_DIED;
	return (-1);
}

/*
 * Receive the required bytes of data, even if it is fragmented.
 */
static int
t_rcvall(fd, buf, len)
	int fd;
	char *buf;
	int len;
{
	int flag;
	int final = 0;
	int res;

	do {
		res = t_rcv(fd, buf, (unsigned)len, &flag);
		if (res == -1)
			break;
		final += res;
		buf += res;
		len -= res;
	} while (len && (flag & T_MORE));
	return (res == -1 ? -1 : final);
}

/*
 * writes data to the vc connection.
 * Any error is fatal and the connection is closed.
 */
static int
write_vc(xprt, buf, len)
	register SVCXPRT *xprt;
	caddr_t buf;
	int len;
{
	register int i, cnt;
	int flag;
	struct t_info tinfo;
	long maxsz;

	if (t_getinfo(xprt->xp_fd, &tinfo) == -1) {
		((struct cf_conn *)(xprt->xp_p1))->strm_stat = XPRT_DIED;
		return (-1);
	}

	maxsz = tinfo.tsdu;
	if (maxsz == -2) {	/* Transfer of data unsupported */
		((struct cf_conn *)(xprt->xp_p1))->strm_stat = XPRT_DIED;
		return (-1);
	}
	if ((maxsz == 0) || (maxsz == -1)) {
		if ((len = t_snd(xprt->xp_fd, buf, (unsigned)len,
				(int)0)) == -1) {
			((struct cf_conn *)(xprt->xp_p1))->strm_stat = XPRT_DIED;
		}
		return (len);
	}

	/*
	 * This for those transports which have a max size for data.
	 */
	for (cnt = len; cnt > 0; cnt -= i, buf += i) {
		flag = cnt > maxsz ? T_MORE : 0;
		if ((i = t_snd(xprt->xp_fd, buf,
			(unsigned)MIN(cnt, maxsz), flag)) == -1) {
			((struct cf_conn *)(xprt->xp_p1))->strm_stat = XPRT_DIED;
			return (-1);
		}
	}
	return (len);
}

static enum xprt_stat
svc_vc_stat(xprt)
	SVCXPRT *xprt;
{
	register struct cf_conn *cd = (struct cf_conn *)(xprt->xp_p1);

	if (cd->strm_stat == XPRT_DIED)
		return (XPRT_DIED);
	if (! xdrrec_eof(&(cd->xdrs)))
		return (XPRT_MOREREQS);
	return (XPRT_IDLE);
}

static bool_t
svc_vc_recv(xprt, msg)
	SVCXPRT *xprt;
	register struct rpc_msg *msg;
{
	register struct cf_conn *cd = (struct cf_conn *)(xprt->xp_p1);
	register XDR *xdrs = &(cd->xdrs);

	xdrs->x_op = XDR_DECODE;
	(void) xdrrec_skiprecord(xdrs);
	if (xdr_callmsg(xdrs, msg)) {
		cd->x_id = msg->rm_xid;
		return (TRUE);
	}
	return (FALSE);
}

static bool_t
svc_vc_getargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{

	return ((*xdr_args)(&(((struct cf_conn *)(xprt->xp_p1))->xdrs), args_ptr));
}

static bool_t
svc_vc_freeargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{
	register XDR *xdrs = &(((struct cf_conn *)(xprt->xp_p1))->xdrs);

	xdrs->x_op = XDR_FREE;
	return ((*xdr_args)(xdrs, args_ptr));
}

static bool_t
svc_vc_reply(xprt, msg)
	SVCXPRT *xprt;
	register struct rpc_msg *msg;
{
	register struct cf_conn *cd = (struct cf_conn *)(xprt->xp_p1);
	register XDR *xdrs = &(cd->xdrs);
	register bool_t stat;

	xdrs->x_op = XDR_ENCODE;
	msg->rm_xid = cd->x_id;
	stat = xdr_replymsg(xdrs, msg);
	(void) xdrrec_endofrecord(xdrs, TRUE);
	return (stat);
}

static struct xp_ops *
svc_vc_ops()
{
	static struct xp_ops ops;

	if (ops.xp_recv == NULL) {
		ops.xp_recv = svc_vc_recv;
		ops.xp_stat = svc_vc_stat;
		ops.xp_getargs = svc_vc_getargs;
		ops.xp_reply = svc_vc_reply;
		ops.xp_freeargs = svc_vc_freeargs;
		ops.xp_destroy = svc_vc_destroy;
	}
	return (&ops);
}

static struct xp_ops *
svc_vc_rendezvous_ops()
{
	static struct xp_ops ops;

	if (ops.xp_recv == NULL) {
		ops.xp_recv = rendezvous_request;
		ops.xp_stat = rendezvous_stat;
		ops.xp_getargs = abort;
		ops.xp_reply = abort;
		ops.xp_freeargs = abort,
		ops.xp_destroy = svc_vc_destroy;
	}
	return (&ops);
}
