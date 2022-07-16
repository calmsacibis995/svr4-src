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

#ident	"@(#)librpc:clnt_dg.c	1.3.2.1"

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
static char sccsid[] = "@(#)clnt_dg.c 1.19 89/03/16 Copyr 1988 Sun Micro";
#endif

/*
 * Implements a connectionless client side RPC.
 */

#include <rpc/rpc.h>
#include <errno.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define	LOG_ERR 3
#endif /* SYSLOG */

#define	RPC_MAX_BACKOFF		30 /* seconds */
extern int errno;
extern int t_errno;

extern char *malloc();
static struct clnt_ops *clnt_dg_ops();

/*
 * Private data kept per client handle
 */
struct cu_data {
	int			cu_fd;		/* connections fd */
	bool_t			cu_closeit;	/* opened by library */
	struct netbuf		cu_raddr;	/* remote address */
	struct timeval		cu_wait;	/* retransmit interval */
	struct timeval		cu_total;	/* total time for the call */
	struct rpc_err		cu_error;
	XDR			cu_outxdrs;
	u_int			cu_xdrpos;
	u_int			cu_sendsz;	/* send size */
	char			*cu_outbuf;
	u_int			cu_recvsz;	/* recv size */
	char			cu_inbuf[1];
};

/*
 * Connection less client creation returns with client handle parameters.
 * Default options are set, which the user can change using clnt_control().
 * fd should be open and bound.
 * NB: The rpch->cl_auth is initialized to null authentication.
 * 	Caller may wish to set this something more useful.
 *
 * sendsz and recvsz are the maximum allowable packet sizes that can be
 * sent and received. Normally they are the same, but they can be
 * changed to improve the program efficiency and buffer allocation.
 * If they are 0, use the transport default.
 *
 * If svcaddr is NULL, returns NULL.
 */
CLIENT *
clnt_dg_create(fd, svcaddr, program, version, sendsz, recvsz)
	int fd;				/* open file descriptor */
	struct netbuf *svcaddr;		/* servers address */
	u_long program;			/* program number */
	u_long version;			/* version number */
	u_int sendsz;			/* buffer recv size */
	u_int recvsz;			/* buffer send size */
{
	CLIENT *cl = NULL;			/* client handle */
	register struct cu_data *cu = NULL;	/* private data */
	struct t_info tinfo;
	struct timeval now;
	struct rpc_msg call_msg;

	if (svcaddr == (struct netbuf *)NULL) {
		rpc_createerr.cf_stat = RPC_UNKNOWNADDR;
		return ((CLIENT *)NULL);
	}
	if (t_getinfo(fd, &tinfo) == -1) {
		if ((sendsz == 0) || (recvsz == 0)) {
			rpc_createerr.cf_stat = RPC_TLIERROR;
			rpc_createerr.cf_error.re_errno = 0;
			rpc_createerr.cf_error.re_terrno = t_errno;
			return ((CLIENT *)NULL);
		}
	} else {
		/*
		 * Find the receive and the send size
		 */
		sendsz = _rpc_get_t_size((int)sendsz, tinfo.tsdu);
		recvsz = _rpc_get_t_size((int)recvsz, tinfo.tsdu);
	}

	if ((cl = (CLIENT *)mem_alloc(sizeof (CLIENT))) == (CLIENT *)NULL)
		goto err;
	/*
	 * Should be multiple of 4 for XDR.
	 */
	sendsz = ((sendsz + 3) / 4) * 4;
	recvsz = ((recvsz + 3) / 4) * 4;
	cu = (struct cu_data *)mem_alloc(sizeof (*cu) + sendsz + recvsz);
	if (cu == (struct cu_data *)NULL)
		goto err;
	cu->cu_raddr = *svcaddr;
	if ((cu->cu_raddr.buf = mem_alloc(svcaddr->len)) == NULL)
		goto err;
	(void) memcpy(cu->cu_raddr.buf, svcaddr->buf, (int)svcaddr->len);
	cu->cu_outbuf = &cu->cu_inbuf[recvsz];
	/* Other values can also be set through clnt_control() */
	cu->cu_wait.tv_sec = 15;	/* heuristically chosen */
	cu->cu_wait.tv_usec = 0;
	cu->cu_total.tv_sec = -1;
	cu->cu_total.tv_usec = -1;
	cu->cu_sendsz = sendsz;
	cu->cu_recvsz = recvsz;
	(void) gettimeofday(&now, (struct timezone *)NULL);
	call_msg.rm_xid = getpid() ^ now.tv_sec ^ now.tv_usec;
	call_msg.rm_call.cb_prog = program;
	call_msg.rm_call.cb_vers = version;
	xdrmem_create(&(cu->cu_outxdrs), cu->cu_outbuf, sendsz, XDR_ENCODE);
	if (! xdr_callhdr(&(cu->cu_outxdrs), &call_msg)) {
		mem_free((caddr_t)cl, sizeof (CLIENT));
		mem_free((caddr_t)cu, sizeof (*cu) + sendsz + recvsz);
		return ((CLIENT *)NULL);
	}
	cu->cu_xdrpos = XDR_GETPOS(&(cu->cu_outxdrs));
	/*
	 * By default, closeit is always FALSE. It is users responsibility
	 * to do a t_close on it, else the user may use clnt_control
	 * to let clnt_destroy do it for him/her.
	 */
	cu->cu_closeit = FALSE;
	cu->cu_fd = fd;
	cl->cl_ops = clnt_dg_ops();
	cl->cl_private = (caddr_t)cu;
	cl->cl_auth = authnone_create();
	cl->cl_tp = (char *) NULL;
	cl->cl_netid = (char *) NULL;
	return (cl);
err:
	(void) syslog(LOG_ERR, "clnt_dg_create: out of memory");
	rpc_createerr.cf_stat = RPC_SYSTEMERROR;
	rpc_createerr.cf_error.re_errno = errno;
	rpc_createerr.cf_error.re_terrno = 0;
	if (cl) {
		mem_free((caddr_t)cl, sizeof (CLIENT));
		if (cu)
			mem_free((caddr_t)cu, sizeof (*cu) + sendsz + recvsz);
	}
	return ((CLIENT *)NULL);
}

static enum clnt_stat
clnt_dg_call(cl, proc, xargs, argsp, xresults, resultsp, utimeout)
	register CLIENT	*cl;		/* client handle */
	u_long		proc;		/* procedure number */
	xdrproc_t	xargs;		/* xdr routine for args */
	caddr_t		argsp;		/* pointer to args */
	xdrproc_t	xresults;	/* xdr routine for results */
	caddr_t		resultsp;	/* pointer to results */
	struct timeval	utimeout;	/* seconds to wait before giving up */
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;
	register XDR *xdrs;
	register int outlen;
	fd_set readfds;
	fd_set mask;
	struct rpc_msg reply_msg;
	XDR reply_xdrs;
	struct timeval time_waited;
	bool_t ok;
	int nrefreshes = 2;	/* number of times to refresh cred */
	struct timeval timeout;
	struct timeval retransmit_time;
	struct t_unitdata tu_data, *tr_data = NULL;
	int res;		/* result of operations */

	if (cu->cu_total.tv_usec == -1) {
		timeout = utimeout;	/* use supplied timeout */
	} else {
		timeout = cu->cu_total; /* use default timeout */
	}

	time_waited.tv_sec = 0;
	time_waited.tv_usec = 0;
	retransmit_time = cu->cu_wait;

	tu_data.addr = cu->cu_raddr;

call_again:
	xdrs = &(cu->cu_outxdrs);
	xdrs->x_op = XDR_ENCODE;
	XDR_SETPOS(xdrs, cu->cu_xdrpos);
	/*
	 * the transaction is the first thing in the out buffer
	 */
	(*(u_short *)(cu->cu_outbuf))++;
	if ((! XDR_PUTLONG(xdrs, (long *)&proc)) ||
	    (! AUTH_MARSHALL(cl->cl_auth, xdrs)) ||
	    (! (*xargs)(xdrs, argsp)))
		return (cu->cu_error.re_status = RPC_CANTENCODEARGS);
	outlen = (int)XDR_GETPOS(xdrs);

send_again:
	tu_data.udata.buf = cu->cu_outbuf;
	tu_data.udata.len = outlen;
	tu_data.opt.len = 0;
	if (t_sndudata(cu->cu_fd, &tu_data) == -1) {
		cu->cu_error.re_terrno = t_errno;
		cu->cu_error.re_errno = errno;
		return (cu->cu_error.re_status = RPC_CANTSEND);
	}

	/*
	 * Hack to provide rpc-based message passing
	 */
	if (timeout.tv_sec == 0 && timeout.tv_usec == 0) {
		return (cu->cu_error.re_status = RPC_TIMEDOUT);
	}
	/*
	 * sub-optimal code appears here because we have
	 * some clock time to spare while the packets are in flight.
	 * (We assume that this is actually only executed once.)
	 */
	reply_msg.acpted_rply.ar_verf = _null_auth;
	reply_msg.acpted_rply.ar_results.where = resultsp;
	reply_msg.acpted_rply.ar_results.proc = xresults;
	FD_ZERO(&mask);
	FD_SET(cu->cu_fd, &mask);
	for (;;) {
		extern void (*_svc_getreqset_proc)();
		extern fd_set svc_fdset;
		int fds;

		if (tr_data != NULL) {
			tr_data->udata.buf = NULL;
			t_free(tr_data, T_UNITDATA);
			tr_data = NULL;
		}
		readfds = mask;
		if (_svc_getreqset_proc) {
			for (fds = 0; fds < howmany(FD_SETSIZE, NFDBITS); fds++)
				readfds.fds_bits[fds] |=
					svc_fdset.fds_bits[fds];
		}
		switch (select(_rpc_dtbsize(), &readfds, (fd_set *)NULL,
				(fd_set *)NULL, &(retransmit_time))) {

		case 0:
			time_waited.tv_sec += retransmit_time.tv_sec;
			time_waited.tv_usec += retransmit_time.tv_usec;
			while (time_waited.tv_usec >= 1000000) {
				time_waited.tv_sec++;
				time_waited.tv_usec -= 1000000;
			}
			/* update retransmit_time */
			if (retransmit_time.tv_sec < RPC_MAX_BACKOFF) {
				retransmit_time.tv_usec *= 2;
				retransmit_time.tv_sec *= 2;
				while (retransmit_time.tv_usec >= 1000000) {
					retransmit_time.tv_sec++;
					retransmit_time.tv_usec -= 1000000;
				}
			}

			if ((time_waited.tv_sec < timeout.tv_sec) ||
				((time_waited.tv_sec == timeout.tv_sec) &&
				(time_waited.tv_usec < timeout.tv_usec)))
				goto send_again;
			return (cu->cu_error.re_status = RPC_TIMEDOUT);

		/*
		 * buggy in other cases because time_waited is not being
		 * updated.
		 */
		case -1:
			if (errno == EINTR) {
				errno = 0;	/* reset it */
				continue;
			}
			cu->cu_error.re_errno = errno;
			cu->cu_error.re_terrno = 0;
			return (cu->cu_error.re_status = RPC_CANTRECV);
		}

		if (!FD_ISSET(cu->cu_fd, &readfds)) {
			/* must be for server side of the house; for callback */
			(*_svc_getreqset_proc)(&readfds);
			continue;	/* do select again */
		}

		/* We have some data now */
		tr_data = (struct t_unitdata *)t_alloc(cu->cu_fd,
				T_UNITDATA, T_ADDR);
		if (tr_data == (struct t_unitdata *)NULL) {
			cu->cu_error.re_errno = errno;
			cu->cu_error.re_terrno = t_errno;
			return (cu->cu_error.re_status = RPC_SYSTEMERROR);
		}
		tr_data->udata.maxlen = cu->cu_recvsz;
		tr_data->udata.buf = cu->cu_inbuf;
		tr_data->opt.maxlen = 0;

		do {
			int moreflag;	/* flag indicating more data */

			moreflag = 0;
 			if (errno == EINTR) {
				/*
				 * Must make sure errno was not already
				 * EINTR in case t_rcvudata() returns -1.
				 * This way will only stay in the loop
				 * if getmsg() sets errno to EINTR.
				 */
				errno = 0;
 			}
			res = t_rcvudata(cu->cu_fd, tr_data, &moreflag);
			if (moreflag & T_MORE) {
				/*
				 * Drop this packet. I aint got any
				 * more space.
				 */
				res = -1;
				/* I should not really be doing this */
				errno = 0;
				/*
				 * XXX: Not really Buffer overflow in the
				 * sense of TLI.
				 */
				t_errno = TBUFOVFLW;
			}
		} while (res < 0 && errno == EINTR);
		if (res < 0) {
#ifdef sun
			if (errno == EWOULDBLOCK)
#else
			if (errno == EAGAIN)
#endif
				continue;
			if (t_errno == TLOOK) {
				int old;

				old = t_errno;
				if (t_rcvuderr(cu->cu_fd, NULL) == 0)
					continue;
				else
					cu->cu_error.re_terrno = old;
			} else {
				cu->cu_error.re_terrno = t_errno;
			}
			tr_data->udata.buf = NULL;
			t_free(tr_data, T_UNITDATA);
			cu->cu_error.re_errno = errno;
			return (cu->cu_error.re_status = RPC_CANTRECV);
		}
		if (tr_data->udata.len < sizeof (u_long))
			continue;
		/* see if reply transaction id matches sent id */
		if (*((u_long *)(cu->cu_inbuf)) != *((u_long *)(cu->cu_outbuf)))
			continue;
		/* we now assume we have the proper reply */
		break;
	}

	/*
	 * now decode and validate the response
	 */
	xdrmem_create(&reply_xdrs, cu->cu_inbuf,
			(u_int)tr_data->udata.len, XDR_DECODE);
	tr_data->udata.buf = NULL;
	t_free((char *)tr_data, T_UNITDATA);
	tr_data = NULL;
	ok = xdr_replymsg(&reply_xdrs, &reply_msg);
	/* XDR_DESTROY(&reply_xdrs);	save a few cycles on noop destroy */
	if (ok) {
		_seterr_reply(&reply_msg, &(cu->cu_error));
		if (cu->cu_error.re_status == RPC_SUCCESS) {
			if (! AUTH_VALIDATE(cl->cl_auth,
				&reply_msg.acpted_rply.ar_verf)) {
				cu->cu_error.re_status = RPC_AUTHERROR;
				cu->cu_error.re_why = AUTH_INVALIDRESP;
			}
			if (reply_msg.acpted_rply.ar_verf.oa_base != NULL) {
				xdrs->x_op = XDR_FREE;
				(void)xdr_opaque_auth(xdrs,
					&(reply_msg.acpted_rply.ar_verf));
			}
		} /* end successful completion */
		else {
			/* maybe our credentials need to be refreshed ... */
			if (nrefreshes > 0 && AUTH_REFRESH(cl->cl_auth)) {
				nrefreshes--;
				goto call_again;
			}
		} /* end of unsuccessful completion */
	} /* end of valid reply message */
	else {
		cu->cu_error.re_status = RPC_CANTDECODERES;
	}
	return (cu->cu_error.re_status);
}

static void
clnt_dg_geterr(cl, errp)
	CLIENT *cl;
	struct rpc_err *errp;
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;

	*errp = cu->cu_error;
}

static bool_t
clnt_dg_freeres(cl, xdr_res, res_ptr)
	CLIENT *cl;
	xdrproc_t xdr_res;
	caddr_t res_ptr;
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;
	register XDR *xdrs = &(cu->cu_outxdrs);

	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr));
}

static void
clnt_dg_abort(/* h */)
	/* CLIENT *h; */
{
}

static bool_t
clnt_dg_control(cl, request, info)
	CLIENT *cl;
	int request;
	char *info;
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;

	switch (request) {
	case CLSET_TIMEOUT:
		cu->cu_total = *(struct timeval *)info;
		break;
	case CLGET_TIMEOUT:
		*(struct timeval *)info = cu->cu_total;
		break;
	case CLGET_SERVER_ADDR:		/* Give him the fd address */
		/* Now obsolete. Only for backword compatibility */
		(void) memcpy(info, cu->cu_raddr.buf, (int)cu->cu_raddr.len);
		break;
	case CLSET_RETRY_TIMEOUT:
		cu->cu_wait = *(struct timeval *)info;
		break;
	case CLGET_RETRY_TIMEOUT:
		*(struct timeval *)info = cu->cu_wait;
		break;
	case CLGET_FD:
		*(int *)info = cu->cu_fd;
		break;
	case CLGET_SVC_ADDR:
		*(struct netbuf *)info = cu->cu_raddr;
		break;
	case CLSET_FD_CLOSE:
		cu->cu_closeit = TRUE;
		break;
	case CLSET_FD_NCLOSE:
		cu->cu_closeit = FALSE;
		break;
	default:
		return (FALSE);
	}
	return (TRUE);
}

static void
clnt_dg_destroy(cl)
	CLIENT *cl;
{
	register struct cu_data *cu = (struct cu_data *)cl->cl_private;

	if (cu->cu_closeit)
		(void) t_close(cu->cu_fd);
	XDR_DESTROY(&(cu->cu_outxdrs));
	(void) mem_free((caddr_t)cu,
		(sizeof (*cu) + cu->cu_sendsz + cu->cu_recvsz));
	if (cl->cl_netid && cl->cl_netid[0])
		(void) mem_free(cl->cl_netid, strlen(cl->cl_netid) +1);
	if (cl->cl_tp && cl->cl_tp[0])
		(void) mem_free(cl->cl_tp, strlen(cl->cl_tp) +1);
	(void) mem_free((caddr_t)cl, sizeof (CLIENT));
}

static struct clnt_ops *
clnt_dg_ops()
{
	static struct clnt_ops ops;

	if (ops.cl_call == NULL) {
		ops.cl_call = clnt_dg_call;
		ops.cl_abort = clnt_dg_abort;
		ops.cl_geterr = clnt_dg_geterr;
		ops.cl_freeres = clnt_dg_freeres;
		ops.cl_destroy = clnt_dg_destroy;
		ops.cl_control = clnt_dg_control;
	}
	return (&ops);
}

