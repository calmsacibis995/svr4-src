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


#ident	"@(#)librpc:clnt_vc.c	1.5.2.1"

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
static char sccsid[] = "@(#)clnt_vc.c 1.19 89/03/16 Copyr 1988 Sun Micro";
#endif

/*
 * clnt_vc.c
 *
 * Implements a connectionful client side RPC.
 *
 * Connectionful RPC supports 'batched calls'.
 * A sequence of calls may be batched-up in a send buffer. The rpc call
 * return immediately to the client even though the call was not necessarily
 * sent. The batching occurs if the results' xdr routine is NULL (0) AND
 * the rpc timeout value is zero (see clnt.h, rpc).
 *
 * Clients should NOT casually batch calls that in fact return results; that
 * is the server side should be aware that a call is batched and not produce
 * any return message. Batched calls that produce many result messages can
 * deadlock (netlock) the client and the server....
 */

#include <rpc/rpc.h>
#include <errno.h>
#include <sys/byteorder.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define	LOG_ERR 3
#endif /* SYSLOG */

#define	MCALL_MSG_SIZE 24
#ifndef MIN
#define	MIN(a, b)	(((a) < (b)) ? (a) : (b))
#endif

extern int errno;
extern int t_errno;
extern char *malloc();

static struct clnt_ops	*clnt_vc_ops();
static int		read_vc();
static int		write_vc();

/*
 * Private data structure
 */
struct ct_data {
	int		ct_fd;		/* connection's fd */
	bool_t		ct_closeit;	/* close it on destroy */
	struct timeval	ct_wait;
	bool_t		ct_waitset;	/* wait set by clnt_control? */
	struct netbuf	ct_addr;	/* remote addr */
	struct rpc_err	ct_error;
	char		ct_mcall[MCALL_MSG_SIZE]; /* marshalled callmsg */
	u_int		ct_mpos;	/* pos after marshal */
	XDR		ct_xdrs;	/* XDR stream */
};

/*
 * Create a client handle for a connection.
 * Default options are set, which the user can change using clnt_control()'s.
 * The rpc/vc package does buffering similar to stdio, so the client
 * must pick send and receive buffer sizes, 0 => use the default.
 * NB: fd is copied into a private area.
 * NB: The rpch->cl_auth is set null authentication. Caller may wish to
 * set this something more useful.
 *
 * fd should be open and bound.
 */
CLIENT *
clnt_vc_create(fd, svcaddr, prog, vers, sendsz, recvsz)
	register int fd;		/* open file descriptor */
	struct netbuf *svcaddr;		/* servers address */
	u_long prog;			/* program number */
	u_long vers;			/* version number */
	u_int sendsz;			/* buffer recv size */
	u_int recvsz;			/* buffer send size */
{
	CLIENT *cl;			/* client handle */
	register struct ct_data *ct;	/* private data */
	struct timeval now;
	struct rpc_msg call_msg;
	struct t_call *sndcall, *rcvcall;
	struct t_info tinfo;
	int state;

	cl = (CLIENT *)mem_alloc(sizeof (*cl));
	ct = (struct ct_data *)mem_alloc(sizeof (*ct));
	if ((cl == (CLIENT *)NULL) || (ct == (struct ct_data *)NULL)) {
		(void) syslog(LOG_ERR, "clnt_vc_create: out of memory");
		rpc_createerr.cf_stat = RPC_SYSTEMERROR;
		rpc_createerr.cf_error.re_errno = errno;
		rpc_createerr.cf_error.re_terrno = 0;
		goto err;
	}

	state = t_getstate(fd);
	if (state == -1) {
		rpc_createerr.cf_stat = RPC_TLIERROR;
		rpc_createerr.cf_error.re_errno = 0;
		rpc_createerr.cf_error.re_terrno = t_errno;
		goto err;
	}

	switch (state) {
	case T_IDLE:
		if (svcaddr == (struct netbuf *)NULL) {
			rpc_createerr.cf_stat = RPC_UNKNOWNADDR;
			goto err;
		}
		/*
		 * Connect only if state is IDLE and svcaddr known
		 */
		rcvcall = (struct t_call *)t_alloc(fd, T_CALL, T_OPT|T_ADDR);
		sndcall = (struct t_call *)t_alloc(fd, T_CALL, T_OPT);
		if ((rcvcall == NULL) || (sndcall == NULL)) {
			rpc_createerr.cf_stat = RPC_TLIERROR;
			rpc_createerr.cf_error.re_terrno = t_errno;
			rpc_createerr.cf_error.re_errno = errno;
			goto err;
		}
		/*
		 * The underlying TLI TCP implementation sends back
		 * the options, even if the opt.maxlen = 0. A BUG.
		 */
		rcvcall->udata.maxlen = 0;
		sndcall->addr = *svcaddr;
		/*
		 * Even NULL could have sufficed for rcvcall, because
		 * the address returned is same for all cases except
		 * for the broadcast case, and hence required.
		 */
		if (t_connect(fd, sndcall, rcvcall) == -1) {
			(void) t_free((char *)rcvcall, T_CALL);
			(void) t_free((char *)sndcall, T_CALL);
			rpc_createerr.cf_stat = RPC_TLIERROR;
			if (t_errno == TLOOK) {
				int old, res;

				old = t_errno;
				if (res = t_look(fd))
					rpc_createerr.cf_error.re_terrno = res;
				else
					rpc_createerr.cf_error.re_terrno = old;
			} else {
				rpc_createerr.cf_error.re_terrno = t_errno;
			}
			rpc_createerr.cf_error.re_errno = 0;
			goto err;
		}
		ct->ct_addr = rcvcall->addr;	/* To get the new address */
		/* So that address buf does not get freed */
		rcvcall->addr.buf = NULL;
		sndcall->addr.buf = NULL;
		(void) t_free((char *)rcvcall, T_CALL);
		(void) t_free((char *)sndcall, T_CALL);
		break;
	case T_DATAXFER:
	case T_OUTCON:
		if (svcaddr == (struct netbuf *)NULL) {
			/*
			 * svcaddr could also be NULL in cases where the
			 * client is already bound and connected.
			 */
			memset((char *)&ct->ct_addr, 0, sizeof (struct netbuf));
		} else {
			ct->ct_addr = *svcaddr;
			ct->ct_addr.buf = malloc(svcaddr->len);
			if (ct->ct_addr.buf == (char *)NULL) {
				(void) syslog(LOG_ERR,
				"clnt_vc_create: out of memory");
				rpc_createerr.cf_stat = RPC_SYSTEMERROR;
				rpc_createerr.cf_error.re_errno = errno;
				rpc_createerr.cf_error.re_terrno = 0;
				goto err;
			}
			memcpy(ct->ct_addr.buf, svcaddr->buf,
					(int)svcaddr->len);
		}
		break;
	default:
		rpc_createerr.cf_stat = RPC_UNKNOWNADDR;
		goto err;
	}

	/*
	 * Set up other members of private data struct
	 */
	ct->ct_fd = fd;
	ct->ct_wait.tv_usec = 0;
	ct->ct_waitset = FALSE;
	/*
	 * By default, closeit is always FALSE. It is users responsibility
	 * to do a t_close on it, else the user may use clnt_control
	 * to let clnt_destroy do it for him/her.
	 */
	ct->ct_closeit = FALSE;

	/*
	 * Initialize call message
	 */
	(void) gettimeofday(&now, (struct timezone *)0);
	call_msg.rm_xid = getpid() ^ now.tv_sec ^ now.tv_usec;
	call_msg.rm_call.cb_prog = prog;
	call_msg.rm_call.cb_vers = vers;

	/*
	 * pre-serialize the static part of the call msg and stash it away
	 */
	xdrmem_create(&(ct->ct_xdrs), ct->ct_mcall, MCALL_MSG_SIZE, XDR_ENCODE);
	if (! xdr_callhdr(&(ct->ct_xdrs), &call_msg)) {
		goto err;
	}
	ct->ct_mpos = XDR_GETPOS(&(ct->ct_xdrs));
	XDR_DESTROY(&(ct->ct_xdrs));

	if (t_getinfo(fd, &tinfo) == -1) {
		if ((sendsz == 0) || (recvsz == 0)) {
			rpc_createerr.cf_stat = RPC_TLIERROR;
			rpc_createerr.cf_error.re_terrno = t_errno;
			rpc_createerr.cf_error.re_errno = 0;
			goto err;
		}
	} else {
		/*
		 * Find the receive and the send size
		 */
		sendsz = _rpc_get_t_size((int)sendsz, tinfo.tsdu);
		recvsz = _rpc_get_t_size((int)recvsz, tinfo.tsdu);
	}
	/*
	 * Create a client handle which uses xdrrec for serialization
	 * and authnone for authentication.
	 */
	xdrrec_create(&(ct->ct_xdrs), sendsz, recvsz, (caddr_t)ct,
				read_vc, write_vc);
	cl->cl_ops = clnt_vc_ops();
	cl->cl_private = (caddr_t) ct;
	cl->cl_auth = authnone_create();
	cl->cl_tp = (char *) NULL;
	cl->cl_netid = (char *) NULL;
	return (cl);

err:
	if (cl) {
		if (ct)
			(void) mem_free((caddr_t)ct, sizeof (struct ct_data));
		(void) mem_free((caddr_t)cl, sizeof (CLIENT));
	}
	return ((CLIENT *)NULL);
}

static enum clnt_stat
clnt_vc_call(cl, proc, xdr_args, args_ptr, xdr_results, results_ptr, timeout)
	register CLIENT *cl;
	u_long proc;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
	xdrproc_t xdr_results;
	caddr_t results_ptr;
	struct timeval timeout;
{
	register struct ct_data *ct = (struct ct_data *) cl->cl_private;
	register XDR *xdrs = &(ct->ct_xdrs);
	struct rpc_msg reply_msg;
	u_long x_id;
	u_long *msg_x_id = (u_long *)(ct->ct_mcall);	/* yuk */
	register bool_t shipnow;
	int refreshes = 2;

	if (!ct->ct_waitset) {
		ct->ct_wait = timeout;
	}

	shipnow = ((xdr_results == (xdrproc_t)0) && (timeout.tv_sec == 0) &&
			(timeout.tv_usec == 0)) ? FALSE : TRUE;

call_again:
	xdrs->x_op = XDR_ENCODE;
	ct->ct_error.re_status = RPC_SUCCESS;
	x_id = ntohl(--(*msg_x_id));
	if ((! XDR_PUTBYTES(xdrs, ct->ct_mcall, ct->ct_mpos)) ||
	    (! XDR_PUTLONG(xdrs, (long *)&proc)) ||
	    (! AUTH_MARSHALL(cl->cl_auth, xdrs)) ||
	    (! (*xdr_args)(xdrs, args_ptr))) {
		if (ct->ct_error.re_status == RPC_SUCCESS)
			ct->ct_error.re_status = RPC_CANTENCODEARGS;
		(void) xdrrec_endofrecord(xdrs, TRUE);
		return (ct->ct_error.re_status);
	}
	if (! xdrrec_endofrecord(xdrs, shipnow))
		return (ct->ct_error.re_status = RPC_CANTSEND);
	if (! shipnow)
		return (RPC_SUCCESS);
	/*
	 * Hack to provide rpc-based message passing
	 */
	if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {
		return (ct->ct_error.re_status = RPC_TIMEDOUT);
	}

	/*
	 * Keep receiving until we get a valid transaction id
	 */
	xdrs->x_op = XDR_DECODE;
	while (TRUE) {
		reply_msg.acpted_rply.ar_verf = _null_auth;
		reply_msg.acpted_rply.ar_results.where = NULL;
		reply_msg.acpted_rply.ar_results.proc = xdr_void;
		if (! xdrrec_skiprecord(xdrs))
			return (ct->ct_error.re_status);
		/* now decode and validate the response header */
		if (! xdr_replymsg(xdrs, &reply_msg)) {
			if (ct->ct_error.re_status == RPC_SUCCESS)
				continue;
			return (ct->ct_error.re_status);
		}
		if (reply_msg.rm_xid == x_id)
			break;
	}

	/*
	 * process header
	 */
	_seterr_reply(&reply_msg, &(ct->ct_error));
	if (ct->ct_error.re_status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(cl->cl_auth,
				&reply_msg.acpted_rply.ar_verf)) {
			ct->ct_error.re_status = RPC_AUTHERROR;
			ct->ct_error.re_why = AUTH_INVALIDRESP;
		} else if (! (*xdr_results)(xdrs, results_ptr)) {
			if (ct->ct_error.re_status == RPC_SUCCESS)
				ct->ct_error.re_status = RPC_CANTDECODERES;
		}
		/* free verifier ... */
		if (reply_msg.acpted_rply.ar_verf.oa_base != NULL) {
			xdrs->x_op = XDR_FREE;
			(void) xdr_opaque_auth(xdrs,
				&(reply_msg.acpted_rply.ar_verf));
		}
	} /* end successful completion */
	else {
		/* maybe our credentials need to be refreshed ... */
		if (refreshes-- && AUTH_REFRESH(cl->cl_auth))
			goto call_again;
	} /* end of unsuccessful completion */
	return (ct->ct_error.re_status);
}

static void
clnt_vc_geterr(cl, errp)
	CLIENT *cl;
	struct rpc_err *errp;
{
	register struct ct_data *ct = (struct ct_data *) cl->cl_private;

	*errp = ct->ct_error;
}

static bool_t
clnt_vc_freeres(cl, xdr_res, res_ptr)
	CLIENT *cl;
	xdrproc_t xdr_res;
	caddr_t res_ptr;
{
	register struct ct_data *ct = (struct ct_data *)cl->cl_private;
	register XDR *xdrs = &(ct->ct_xdrs);

	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr));
}

static void
clnt_vc_abort()
{
}

static bool_t
clnt_vc_control(cl, request, info)
	CLIENT *cl;
	int request;
	char *info;
{
	register struct ct_data *ct = (struct ct_data *)cl->cl_private;

	switch (request) {
	case CLSET_TIMEOUT:
		ct->ct_wait = *(struct timeval *)info;
		ct->ct_waitset = TRUE;
		break;
	case CLGET_TIMEOUT:
		*(struct timeval *)info = ct->ct_wait;
		break;
	case CLGET_SERVER_ADDR:	/* For compatibility only */
		(void) memcpy(info, ct->ct_addr.buf, (int)ct->ct_addr.len);
		break;
	case CLGET_FD:
		*(int *)info = ct->ct_fd;
		break;
	case CLGET_SVC_ADDR:
		/* The caller should not this memory area */
		*(struct netbuf *)info = ct->ct_addr;
		break;
	case CLSET_FD_CLOSE:
		ct->ct_closeit = TRUE;
		break;
	case CLSET_FD_NCLOSE:
		ct->ct_closeit = FALSE;
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}

static void
clnt_vc_destroy(cl)
	CLIENT *cl;
{
	register struct ct_data *ct = (struct ct_data *) cl->cl_private;

	if (ct->ct_closeit)
		(void) t_close(ct->ct_fd);
	XDR_DESTROY(&(ct->ct_xdrs));
	if (ct->ct_addr.buf)
		(void) free(ct->ct_addr.buf);
	(void) mem_free((caddr_t)ct, sizeof (struct ct_data));
	if (cl->cl_netid && cl->cl_netid[0])
		(void) mem_free(cl->cl_netid, strlen(cl->cl_netid) +1);
	if (cl->cl_tp && cl->cl_tp[0])
		(void) mem_free(cl->cl_tp, strlen(cl->cl_tp) +1);
	(void) mem_free((caddr_t)cl, sizeof (CLIENT));
}

/*
 * Interface between xdr serializer and vc connection.
 * Behaves like the system calls, read & write, but keeps some error state
 * around for the rpc level.
 */
static int
read_vc(ct, buf, len)
	register struct ct_data *ct;
	caddr_t buf;
	register int len;
{
	fd_set mask;
	fd_set readfds;

	if (len == 0)
		return (0);
	FD_ZERO(&mask);
	FD_SET(ct->ct_fd, &mask);
	while (TRUE) {
		extern void (*_svc_getreqset_proc)();
		extern fd_set svc_fdset;
		int fds;

		readfds = mask;
		if (_svc_getreqset_proc) {
			for (fds = 0; fds < howmany(FD_SETSIZE, NFDBITS); fds++)
				readfds.fds_bits[fds] |=
					svc_fdset.fds_bits[fds];
		}
		switch (select(_rpc_dtbsize(), &readfds,
			(fd_set *)NULL, (fd_set *)NULL, &(ct->ct_wait))) {
		case 0:
			ct->ct_error.re_status = RPC_TIMEDOUT;
			return (-1);

		case -1:
			if (errno == EINTR)
				continue;
			ct->ct_error.re_status = RPC_CANTRECV;
			ct->ct_error.re_errno = errno;
			return (-1);
		}
		if (!FD_ISSET(ct->ct_fd, &readfds)) {
			/* must be for server side of the house */
			(*_svc_getreqset_proc)(&readfds);
			continue;	/* do select again */
		}
		break;
	}

	switch (len = t_rcvall(ct->ct_fd, buf, len)) {
	case 0:
		/* premature eof */
#ifdef sun
		ct->ct_error.re_errno = ECONNRESET;
#else
		ct->ct_error.re_errno = ENOLINK;
#endif
		ct->ct_error.re_terrno = 0;
		ct->ct_error.re_status = RPC_CANTRECV;
		len = -1;	/* it's really an error */
		break;

	case -1:
		ct->ct_error.re_terrno = t_errno;
		ct->ct_error.re_errno = 0;
		ct->ct_error.re_status = RPC_CANTRECV;
		break;
	}
	return (len);
}

static int
write_vc(ct, buf, len)
	struct ct_data *ct;
	caddr_t buf;
	int len;
{
	register int i, cnt;
	int flag;
	struct t_info tinfo;
	long maxsz;

	if (t_getinfo(ct->ct_fd, &tinfo) < 0) {
		ct->ct_error.re_terrno = t_errno;
		ct->ct_error.re_errno = 0;
		ct->ct_error.re_status = RPC_CANTSEND;
		return (-1);
	}

	maxsz = tinfo.tsdu;
	if (maxsz == -2)	/* Transfer of data unsupported */
		return (-1);
	if ((maxsz == 0) || (maxsz == -1)) {
		if ((len = t_snd(ct->ct_fd, buf, (unsigned)len, 0)) == -1) {
			ct->ct_error.re_terrno = t_errno;
			ct->ct_error.re_errno = 0;
			ct->ct_error.re_status = RPC_CANTSEND;
		}
		return (len);
	}

	/*
	 * This for those transports which have a max size for data.
	 */
	for (cnt = len; cnt > 0; cnt -= i, buf += i) {
		flag = cnt > maxsz ? T_MORE : 0;
		if ((i = t_snd(ct->ct_fd, buf, (unsigned)MIN(cnt, maxsz),
				flag)) == -1) {
			ct->ct_error.re_terrno = t_errno;
			ct->ct_error.re_errno = 0;
			ct->ct_error.re_status = RPC_CANTSEND;
			return (-1);
		}
	}
	return (len);
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
	int moreflag;
	int final = 0;
	int res;

	do {
		moreflag = 0;
		res = t_rcv(fd, buf, (unsigned)len, &moreflag);
		if (res == -1)
			return (-1);
		else if (res == 0)
			return (0);
		final += res;
		buf += res;
		len -= res;
	} while ((len > 0) && (moreflag & T_MORE));
	return (final);
}

static struct clnt_ops *
clnt_vc_ops()
{
	static struct clnt_ops ops;

	if (ops.cl_call == NULL) {
		ops.cl_call = clnt_vc_call;
		ops.cl_abort = clnt_vc_abort;
		ops.cl_geterr = clnt_vc_geterr;
		ops.cl_freeres = clnt_vc_freeres;
		ops.cl_destroy = clnt_vc_destroy;
		ops.cl_control = clnt_vc_control;
	}
	return (&ops);
}
