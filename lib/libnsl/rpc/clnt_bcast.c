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


#ident	"@(#)librpc:clnt_bcast.c	1.6.1.1"

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
static char sccsid[] = "@(#)clnt_bcast.c 1.15 89/04/21 Copyr 1988 Sun Micro";
#endif

/*
 * clnt_bcast.c
 * Client interface to broadcast service.
 *
 * Copyright (C) 1988, Sun Microsystems, Inc.
 *
 * The following is kludged-up support for simple rpc broadcasts.
 * Someday a large, complicated system will replace these trivial
 * routines.
 */

#include <rpc/rpc.h>
#include <rpc/nettype.h>
#include <netdir.h>
#include <sys/byteorder.h>
#ifdef PORTMAP
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_rmt.h>
#endif
#ifdef DEBUG
#include <stdio.h>
#endif
#include <errno.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define	LOG_ERR 3
#endif /* SYSLOG */

extern int errno;

#define	MAXBCAST 20	/* Max no of broadcasting transports */

extern char *malloc();
typedef bool_t (*resultproc_t)();

/*
 * If nettype is NULL, it broadcasts on all the locally available
 * datagram_n transports. May potentially lead to broadacst storms
 * and hence should be used with caution, care and courage.
 *
 * The current parameter xdr packet size is limited by the max tsdu
 * size of the transport. If the max tsdu size of any transport is
 * smaller than the parameter xdr packet, then broadcast is not
 * sent on that transport.
 * Also, the packet size should be less the packet size of
 * the data link layer (for ethernet it is 1400 bytes). The result
 * size  has to be smaller than the transport tsdu size.
 *
 * If PORTMAP has been defined, we send two packets for UDP, one for
 * rpcbind and one for portmap. For those machines which support
 * both rpcbind and portmap, it will cause them to reply twice, and
 * also here it will get two responses ... kinda' inefficient and clumsy.
 */
enum clnt_stat
rpc_broadcast(prog, vers, proc, xargs, argsp, xresults, resultsp,
			eachresult, nettype)
	u_long		prog;		/* program number */
	u_long		vers;		/* version number */
	u_long		proc;		/* procedure number */
	xdrproc_t	xargs;		/* xdr routine for args */
	caddr_t		argsp;		/* pointer to args */
	xdrproc_t	xresults;	/* xdr routine for results */
	caddr_t		resultsp;	/* pointer to results */
	resultproc_t	eachresult;	/* call with each result obtained */
	char		*nettype;	/* transport type */
{
	enum clnt_stat stat = RPC_SUCCESS;	/* Return status */
	XDR 		xdr_stream;		/* XDR stream */
	register XDR 	*xdrs = &xdr_stream;
	fd_set		mask;		/* File descriptor mask */
	fd_set		readfds;
	struct rpc_msg	msg;		/* RPC message */
	struct timeval	t;
	char 		*outbuf = NULL;	/* Broadcast msg buffer */
	char		*inbuf = NULL;		/* Reply buf */
	register int 	fdlistno = 0;
	long 		maxbufsize = 0;
	AUTH 		*sys_auth = authsys_create_default();
	register int	i, j;
	bool_t		done = FALSE;
	int		net;
	char		uaddress[1024];		/* A self imposed limit */
	char		*uaddrp = uaddress;
	int 		pmap_reply_flag = 0;	/* reply recvd from PORTMAP */
	struct {
		int fd;			/* File descriptor */
		struct netconfig *nconf; /* Netconfig structure */
		u_int asize;		/* Size of the addr buf */
		u_int dsize;		/* Size of the data buf */
		struct netbuf raddr;	/* Remote address */
		struct nd_addrlist *nal; /* Broadcast addrs */
		int udpip;		/* This is UDP/IP */
	} fdlist[MAXBCAST];
	struct rpcb_rmtcallargs barg;	/* Remote arguments */
	struct rpcb_rmtcallres bres;	/* Remote results */
	struct t_unitdata t_udata, t_rdata;
	struct netconfig *nconf;
	struct nd_hostserv hs;

#ifdef PORTMAP
	u_long port;			/* Remote port number */
	int pmap_flag = 0;		/* UDP exists ? */
	char *outbuf_pmap = NULL;
	struct rmtcallargs barg_pmap;	/* Remote arguments */
	struct rmtcallres bres_pmap;	/* Remote results */
	struct t_unitdata t_udata_pmap;
	int udpbufsz = 0;
#endif /* PORTMAP */

	if (sys_auth == (AUTH *)NULL)
		return (RPC_SYSTEMERROR);
	/*
	 * initialization: create a fd, a broadcast address, and send the
	 * request on the broadcast transport.
	 * Listen on all of them and on replies, call the user supplied
	 * function.
	 */
	FD_ZERO(&mask);

	if (nettype == NULL)
		nettype = "datagram_n";
	if ((net = _rpc_setconf(nettype)) == 0)
		return (RPC_UNKNOWNPROTO);
	while (nconf = _rpc_getconf(net)) {
		struct t_bind *taddr;
		struct t_info tinfo;
		int fd;

		if (!(nconf->nc_flag & NC_BROADCAST))
			continue;
		if (fdlistno >= MAXBCAST)
			break;	/* No more slots available */
		if ((fd = t_open(nconf->nc_device, O_RDWR, &tinfo)) == -1) {
			stat = RPC_CANTSEND;
			continue;
		}
		if (t_bind(fd, (struct t_bind *)NULL,
			(struct t_bind *)NULL) == -1) {
			(void) t_close(fd);
			stat = RPC_CANTSEND;
			continue;
		}
		/* Do protocol specific negotiating for broadcast */
		if (netdir_options(nconf, ND_SET_BROADCAST, fd, NULL)) {
			(void) t_close(fd);
			stat = RPC_NOBROADCAST;
			continue;
		}
		taddr = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
		if (taddr == (struct t_bind *)NULL) {
			(void) t_close(fd);
			stat = RPC_SYSTEMERROR;
			goto done_broad;
		}
		FD_SET(fd, &mask);
		fdlist[fdlistno].fd = fd;
		fdlist[fdlistno].nconf = nconf;
		fdlist[fdlistno].asize = _rpc_get_a_size(tinfo.addr);
		fdlist[fdlistno].dsize = _rpc_get_t_size(0, tinfo.tsdu);
		fdlist[fdlistno].raddr = taddr->addr;
		if (maxbufsize <= fdlist[fdlistno].dsize)
			maxbufsize = fdlist[fdlistno].dsize;
		taddr->addr.buf = NULL;
		(void) t_free((char *)taddr, T_BIND);
#ifdef PORTMAP
		if ((nconf->nc_semantics == NC_TPI_CLTS) &&
			!strcmp(nconf->nc_protofmly, NC_INET) &&
			!strcmp(nconf->nc_proto, NC_UDP)) {
			fdlist[fdlistno].udpip = 1;
			udpbufsz = fdlist[fdlistno].dsize;
			if ((outbuf_pmap = malloc(udpbufsz)) == NULL){
				t_close(fd);
				continue;
			}
			pmap_flag = 1;
		} else {
			fdlist[fdlistno].udpip = 0;
		}
#endif
		fdlistno++;
	}

	if (fdlist == 0) {
		if (stat == RPC_SUCCESS)
			stat = RPC_UNKNOWNPROTO;
		goto done_broad;
	}
	if (maxbufsize == 0) {
		if (stat == RPC_SUCCESS)
			stat = RPC_CANTSEND;
		goto done_broad;
	}
	inbuf = malloc(maxbufsize);
	outbuf = malloc(maxbufsize);
	if ((inbuf == NULL) || (outbuf == NULL)) {
		stat = RPC_SYSTEMERROR;
		goto done_broad;
	}

	/* Serialize all the arguments which have to be sent */
	(void) gettimeofday(&t, (struct timezone *)0);
	msg.rm_xid = getpid() ^ t.tv_sec ^ t.tv_usec;
	msg.rm_direction = CALL;
	msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	msg.rm_call.cb_prog = RPCBPROG;
	msg.rm_call.cb_vers = RPCBVERS;
	msg.rm_call.cb_proc = RPCBPROC_CALLIT;
	barg.prog = prog;
	barg.vers = vers;
	barg.proc = proc;
	barg.xdr_args = xargs;
	barg.args_ptr = argsp;
	bres.addr_ptr = uaddrp;
	bres.xdr_results = xresults;
	bres.results_ptr = resultsp;
	msg.rm_call.cb_cred = sys_auth->ah_cred;
	msg.rm_call.cb_verf = sys_auth->ah_verf;
	xdrmem_create(xdrs, outbuf, maxbufsize, XDR_ENCODE);
	if ((! xdr_callmsg(xdrs, &msg)) ||
			(! xdr_rpcb_rmtcallargs(xdrs, &barg))) {
		stat = RPC_CANTENCODEARGS;
		goto done_broad;
	}
	t_udata.opt.len = 0;
	t_udata.udata.buf = outbuf;
	t_udata.udata.len = xdr_getpos(xdrs);
	t_rdata.opt.len = 0;
	xdr_destroy(xdrs);

#ifdef PORTMAP
	/* Prepare the packet for version 2 PORTMAP */
	if (pmap_flag) {
		msg.rm_xid++;	/* One way to distinguish */
		msg.rm_call.cb_prog = PMAPPROG;
		msg.rm_call.cb_vers = PMAPVERS;
		msg.rm_call.cb_proc = PMAPPROC_CALLIT;
		barg_pmap.prog = prog;
		barg_pmap.vers = vers;
		barg_pmap.proc = proc;
		barg_pmap.xdr_args = xargs;
		barg_pmap.args_ptr = argsp;
		bres_pmap.port_ptr = &port;
		bres_pmap.xdr_results = xresults;
		bres_pmap.results_ptr = resultsp;
		xdrmem_create(xdrs, outbuf_pmap, udpbufsz, XDR_ENCODE);
		if ((! xdr_callmsg(xdrs, &msg)) ||
			(! xdr_rmtcall_args(xdrs, &barg_pmap))) {
			stat = RPC_CANTENCODEARGS;
			goto done_broad;
		}
		t_udata_pmap.opt.len = 0;
		t_udata_pmap.udata.buf = outbuf_pmap;
		t_udata_pmap.udata.len = xdr_getpos(xdrs);
		xdr_destroy(xdrs);
	}
#endif /* PORTMAP */

	/*
	 * Basic loop: broadcast the packets to transports which
	 * support data packets of size such that one can encode
	 * all the arguments.
	 * Wait a while for response(s).
	 * The response timeout grows larger per iteration.
	 */
	hs.h_host = HOST_BROADCAST;
	hs.h_serv = "rpcbind";

	t.tv_usec = 0;
	for (t.tv_sec = 4; t.tv_sec <= 14; t.tv_sec += 2) {
	    /* Broadcast all the packets now */
	    for (i = 0; i < fdlistno; i++) {
		struct nd_addrlist *addrlist;

		if (fdlist[i].dsize < t_udata.udata.len) {
			stat = RPC_CANTSEND;
			continue;
		}
		if (netdir_getbyname(fdlist[i].nconf, &hs, &addrlist) ||
			(addrlist->n_cnt == 0)) {
			stat = RPC_N2AXLATEFAILURE;
			continue;
		}

		for (j = 0; j < addrlist->n_cnt; j++) {
			t_udata.addr = addrlist->n_addrs[j];
			if (t_sndudata(fdlist[i].fd, &t_udata)) {
				(void) syslog(LOG_ERR,
				"Cannot send broadcast packet: %m");
#ifdef DEBUG
				t_error("rpc_broadcast: t_sndudata");
#endif
				stat = RPC_CANTSEND;
				continue;
			}
#ifdef DEBUG
			fprintf(stderr, "Broadcast packet sent for %s\n",
					fdlist[i].nconf->nc_netid);
#endif
#ifdef PORTMAP
			/* Send the version 2 packet also */
			if (fdlist[i].udpip) {
				if (t_sndudata(fdlist[i].fd, &t_udata_pmap)) {
					(void) syslog(LOG_ERR,
				"Cannot send broadcast packet: %m");
#ifdef DEBUG
					t_error("rpc_broadcast: t_sndudata");
#endif
					stat = RPC_CANTSEND;
					continue;
				}
			}
#ifdef DEBUG
			fprintf(stderr, "PMAP Broadcast packet sent for %s\n",
					fdlist[i].nconf->nc_netid);
#endif
#endif /* PORTMAP */
		} /* End for sending all packets on this transport */
		(void) netdir_free((char *)addrlist, ND_ADDRLIST);
	    } /* End for sending on all transports */

	    if (eachresult == NULL) {
		stat = RPC_SUCCESS;
		goto done_broad;
	    }

	    /*
	     * Get all the replies from these broadcast requests
	     */
	recv_again:
	    readfds = mask;

	    switch (select(_rpc_dtbsize(), &readfds, (fd_set *)NULL,
			(fd_set *)NULL, &t)) {

	    case 0:  /* timed out */
		stat = RPC_TIMEDOUT;
		continue;
	    case -1:  /* some kind of error */
		if (errno == EINTR)
			goto recv_again;
		(void) syslog(LOG_ERR, "Broadcast select problem: %m");
		stat = RPC_CANTRECV;
		goto done_broad;
	    }  /* end of select results switch */

	    t_rdata.udata.buf = inbuf;

	    for (i = 0; i < fdlistno; i++) {
		int flag;

		if (!FD_ISSET(fdlist[i].fd, &readfds))
			continue;
#ifdef DEBUG
		fprintf(stderr, "response for %s\n",
				fdlist[i].nconf->nc_netid);
#endif
	try_again:
		t_rdata.udata.maxlen = fdlist[i].dsize;
		t_rdata.udata.len = 0;
		t_rdata.addr = fdlist[i].raddr;
		if (t_rcvudata(fdlist[i].fd, &t_rdata, &flag) == -1) {
			if (errno == EINTR)
				goto try_again;
			(void) syslog(LOG_ERR,
			"Cannot receive reply to broadcast: %m");
			stat = RPC_CANTRECV;
			continue;
		}
		/*
		 * Not taking care of flag for T_MORE. We are assuming that
		 * such calls should not take more than one transport packet.
		 */
		if (flag & T_MORE)
			continue;	/* Drop that and go ahead */
		if (t_rdata.udata.len < sizeof (u_long))
			continue;	/* Drop that and go ahead */
		/*
		 * see if reply transaction id matches sent id.
		 * If so, decode the results. If return id is xid + 1, it was
		 * a PORTMAP reply
		 */
		pmap_reply_flag = 0;
		if (*((u_long *)(inbuf)) == *((u_long *)(outbuf))) {
			msg.acpted_rply.ar_verf = _null_auth;
			msg.acpted_rply.ar_results.where = (caddr_t)&bres;
			msg.acpted_rply.ar_results.proc = xdr_rpcb_rmtcallres;
#ifdef PORTMAP
		} else if (pmap_flag &&
			*((u_long *)(inbuf)) == *((u_long *)(outbuf_pmap))) {
			pmap_reply_flag = 1;
			msg.acpted_rply.ar_verf = _null_auth;
			msg.acpted_rply.ar_results.where = (caddr_t)&bres_pmap;
			msg.acpted_rply.ar_results.proc = xdr_rmtcallres;
#endif /* PORTMAP */
		} else
			continue;
		xdrmem_create(xdrs, inbuf,
			(u_int)t_rdata.udata.len, XDR_DECODE);
		if (xdr_replymsg(xdrs, &msg)) {
			if ((msg.rm_reply.rp_stat == MSG_ACCEPTED) &&
				(msg.acpted_rply.ar_stat == SUCCESS)) {
				struct netbuf *taddr;
#ifdef PORTMAP
				if (pmap_flag && pmap_reply_flag) {
					/* convert port to taddr */
					((struct sockaddr_in *)t_rdata.addr.buf)->sin_port =
						htons((u_short)port);
					taddr = &t_rdata.addr;
				} else /* Convert the uaddr to taddr */
#endif
					taddr = uaddr2taddr(fdlist[i].nconf, uaddrp);
				done = (*eachresult)(resultsp, taddr,
						fdlist[i].nconf);
#ifdef DEBUG
				{
				int k;

				printf("rmt addr = ");
				for (k = 0; k < taddr->len; k++)
					printf("%d ", taddr->buf[k]);
				printf("\n");
				}
#endif
				if (taddr && !pmap_reply_flag)
					netdir_free((char *)taddr, ND_ADDR);
			}
			/* otherwise, we just ignore the errors ... */
		} /* else some kind of deserialization problem ... */
		xdrs->x_op = XDR_FREE;
		msg.acpted_rply.ar_results.proc = xdr_void;
		(void) xdr_replymsg(xdrs, &msg);
		(void) (*xresults)(xdrs, resultsp);
		XDR_DESTROY(xdrs);
		if (done) {
			stat = RPC_SUCCESS;
			goto done_broad;
		} else {
			goto recv_again;
		}
	    } /* The recv for loop */
	} /* The giant for loop */

done_broad:
	if (inbuf)
		(void) free(inbuf);
	if (outbuf)
		(void) free(outbuf);
#ifdef PORTMAP
	if (outbuf_pmap)
		(void) free(outbuf_pmap);
#endif
	for (i = 0; i < fdlistno; i++) {
		(void) t_close(fdlist[i].fd);
		(void) free(fdlist[i].raddr.buf);
	}
	AUTH_DESTROY(sys_auth);
	(void) _rpc_endconf();
	return (stat);
}
