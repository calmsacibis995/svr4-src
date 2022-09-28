/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nfs.cmds:nfs/automount/nfs_cast.c	1.6.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
 *     Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 *
 *  (c) 1986,1987,1988.1989  Sun Microsystems, Inc
 *  (c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *            All rights reserved.
 *
 */

/*
 * nfs_cast: broadcast to a specific group of NFS servers
 */

#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef notdef
#include <netdb.h>
#include <net/if.h>
#endif
#include <rpc/rpc.h>
#include <rpc/clnt_soc.h>
#include <rpc/nettype.h>
#include <netconfig.h>
#include <netdir.h>
#include "nfs_prot.h"
#include <nfs/nfs_clnt.h>
#define NFSCLIENT
typedef nfs_fh fhandle_t;
#include <nfs/mount.h>
#include "automount.h"

extern int errno;
extern int verbose;
void free_transports();

typedef bool_t (*resultproc_t)();

/* A list of addresses - all belonging to the same transport */

static struct addrs {
	struct addrs		*addr_next;
	int			addr_inx;
	struct nd_addrlist	*addr_addrs;
};

/* A list of connectionless transports */

static struct transp {
	struct transp		*tr_next;
	int			tr_fd;
	char			*tr_device;
	struct t_bind		*tr_taddr;
	struct addrs		*tr_addrs;
};

/*
 * This routine is designed to be able to "ping"
 * a list of hosts to find the host that is
 * up and available and responds fastest.
 * This must be done without any prior
 * contact with the host - therefore the "ping"
 * must be to a "well-known" address.  The outstanding
 * candidate here is the address of "rpcbind".
 *
 * A response to a ping is no guarantee that the host
 * is running NFS, has a mount daemon, or exports
 * the required filesystem.  If the subsequent
 * mount attempt fails then the host will be marked
 * "ignore" and the host list will be re-pinged
 * (sans the bad host). This process continues
 * until a successful mount is achieved or until
 * there are no hosts left to try.
 */
enum clnt_stat 
nfs_cast(mfs, eachresult, timeout)
	struct mapfs *mfs;		/* list of locations */
	resultproc_t	eachresult;	/* call with each result obtained */
	int		timeout;	/* timeout (sec) */
{
	enum clnt_stat stat;
	AUTH *sys_auth = authsys_create_default();
	XDR xdr_stream;
	register XDR *xdrs = &xdr_stream;
	int outlen, inlen;
	int flag;
	int sent, addr_cnt;
	fd_set readfds, mask;
	bool_t done = FALSE;
	register u_long xid;		/* xid - unique per addr */
	register int i;
	struct rpc_msg msg;
	struct timeval t; 
	char outbuf[UDPMSGSIZE], inbuf[UDPMSGSIZE];
	struct t_unitdata t_udata, t_rdata;
	struct nd_hostserv hs;
	struct nd_addrlist *retaddrs;
	struct mapfs *m;
	struct transp *tr_head;
	struct transp *trans, *prev_trans;
	struct addrs *a, *prev_addr;
	NCONF_HANDLE *nc = NULL;
	struct netconfig *nconf;
	struct netconfig *getnetconfig();

	/*
	 * For each connectionless transport get a list of
	 * host addresses.  Any single host may have
	 * addresses on several transports.
	 */
	nc = setnetconfig();
	if (nc == NULL) {
		stat = RPC_CANTSEND;
		goto done_broad;
	}
	addr_cnt = 0;
	tr_head = NULL;
	FD_ZERO(&mask);


	while (nconf = getnetconfig(nc)) {
		if (!(nconf->nc_flag & NC_VISIBLE) ||
		    nconf->nc_semantics != NC_TPI_CLTS ||
		    (strcmp(nconf->nc_protofmly, NC_LOOPBACK) == 0))
			continue;

		trans = (struct transp *) malloc(sizeof(*trans));
		if (trans == NULL) {
			syslog(LOG_ERR, "no memory");
			stat = RPC_CANTSEND;
			goto done_broad;
		}
		memset(trans, 0, sizeof(*trans));
		if (tr_head == NULL)
			tr_head = trans;
		else
			prev_trans->tr_next = trans;
		prev_trans = trans;

		trans->tr_fd = t_open(nconf->nc_device, O_RDWR, NULL);
		if (trans->tr_fd < 0) {
			syslog(LOG_ERR, "nfscast: t_open: %s:%m", 
			   nconf->nc_device);
			stat = RPC_CANTSEND;
			goto done_broad;
		}
		if (t_bind(trans->tr_fd, (struct t_bind *) NULL, 
		  (struct t_bind *) NULL) < 0) {
			syslog(LOG_ERR, "nfscast: t_bind: %m");
			stat = RPC_CANTSEND;
			goto done_broad;
		}
		trans->tr_taddr = 
		   (struct t_bind *) t_alloc(trans->tr_fd, T_BIND, T_ADDR);
		if (trans->tr_taddr == (struct t_bind *) NULL) {
			syslog(LOG_ERR, "nfscast: t_alloc: %m");
			stat = RPC_SYSTEMERROR;
			goto done_broad;
		}

		trans->tr_device = nconf->nc_device;
		FD_SET(trans->tr_fd, &mask);

		for (i = 0, m = mfs ; m; i++, m = m->mfs_next) {
			if (mfs->mfs_ignore)
				continue;
			hs.h_host = m->mfs_host;
			hs.h_serv = "rpcbind";
			if (netdir_getbyname(nconf, &hs, &retaddrs) == ND_OK) {
				a = (struct addrs *) malloc(sizeof(*a));
				if (a == NULL) {
					syslog(LOG_ERR, "no memory");
					stat = RPC_CANTSEND;
					goto done_broad;
				}
				memset(a, 0, sizeof(*a));
				if (trans->tr_addrs == NULL)
					trans->tr_addrs = a;
				else
					prev_addr->addr_next = a;
				prev_addr = a;
				a->addr_inx = i;
				a->addr_addrs = retaddrs;
				addr_cnt++;
			} else {
				if (verbose)
					syslog(LOG_ERR, "%s: address not known",
						m->mfs_host);
			}
		}
	}
	if (addr_cnt == 0) {
		syslog(LOG_ERR, "nfscast: couldn't find addresses");
		stat = RPC_CANTSEND;
		goto done_broad;
	}

	(void) gettimeofday(&t, (struct timezone *) 0);
	xid = (getpid() ^ t.tv_sec ^ t.tv_usec) & ~0xFF;
	t.tv_usec = 0;

	/* serialize the RPC header */

	msg.rm_direction = CALL;
	msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	msg.rm_call.cb_prog = RPCBPROG;
	msg.rm_call.cb_vers = RPCBVERS;	/* any version will do */
	msg.rm_call.cb_proc = NULLPROC;
	if (sys_auth == (AUTH *) NULL) {
		stat = RPC_SYSTEMERROR;
		goto done_broad;
	}
	msg.rm_call.cb_cred = sys_auth->ah_cred;
	msg.rm_call.cb_verf = sys_auth->ah_verf;
	xdrmem_create(xdrs, outbuf, sizeof outbuf, XDR_ENCODE);
	if (! xdr_callmsg(xdrs, &msg)) {
		stat = RPC_CANTENCODEARGS;
		goto done_broad;
	}
	outlen = (int)xdr_getpos(xdrs);
	xdr_destroy(xdrs);

	t_udata.opt.len = 0;
	t_udata.udata.buf = outbuf;
	t_udata.udata.len = outlen;

	/*
	 * Basic loop: send packet to all hosts and wait for response(s).
	 * The response timeout grows larger per iteration.
	 * A unique xid is assigned to each address in order to
	 * correctly match the replies.
	 */
	for (t.tv_sec = 4; timeout > 0; t.tv_sec += 2) {
		timeout -= t.tv_sec;
		if (timeout < 0)
			t.tv_sec = timeout - t.tv_sec;
		sent = 0;
		for (trans = tr_head; trans; trans = trans->tr_next) {
			for (a = trans->tr_addrs ; a ; a = a->addr_next) {
				t_udata.addr = *a->addr_addrs->n_addrs;
				/* xid is the first thing in
				 * preserialized buffer
				 */
				*((u_long *)outbuf) = htonl(xid + a->addr_inx);
				if (t_sndudata(trans->tr_fd, &t_udata) != 0) {
					syslog(LOG_ERR,
					  "nfscast: Cannot send packet: %s:%m",
					   trans->tr_device);
					continue;
				}
				sent++;
			}
		}
		if (sent == 0) {		/* no packets sent ? */
			stat = RPC_CANTSEND;
			goto done_broad;
		}

		/*
		 * Have sent all the packets.  Now collect the responses...
		 */
	recv_again:
		msg.acpted_rply.ar_verf = _null_auth;
		msg.acpted_rply.ar_results.proc = xdr_void;
		readfds = mask;
		switch (select(_rpc_dtbsize(), &readfds, 
		        (fd_set *) NULL, (fd_set *) NULL, &t)) {

		case 0:  /* timed out */
			stat = RPC_TIMEDOUT;
			continue;

		case -1:  /* some kind of error */
			if (errno == EINTR)
				goto recv_again;
			syslog(LOG_ERR, "nfscast: select: %m");
			stat = RPC_CANTRECV;
			goto done_broad;

		}  /* end of select results switch */

		for (trans = tr_head; trans ; trans = trans->tr_next) {
			if (FD_ISSET(trans->tr_fd, &readfds))
				break;
		}
		if (trans == NULL)
			goto recv_again;

	try_again:
		t_rdata.addr = trans->tr_taddr->addr;
		t_rdata.udata.buf = inbuf;
		t_rdata.udata.maxlen = sizeof(inbuf);
		t_rdata.udata.len = 0;
		t_rdata.opt.len = 0;
		if (t_rcvudata(trans->tr_fd, &t_rdata, &flag) < 0) {
			if (errno == EINTR)
				goto try_again;
			syslog(LOG_ERR, "nfscast: t_rcvudata: %s:%m",
			   trans->tr_device);
			stat = RPC_CANTRECV;
			continue;
		}
		if (t_rdata.udata.len < sizeof(u_long))
			goto recv_again;
		if (flag & T_MORE) {
			syslog(LOG_ERR, 
			   "nfscast: t_rcvudata: %s: buffer overflow",
			   trans->tr_device);
			goto recv_again;
		}
		/*
		 * see if reply transaction id matches sent id.
		 * If so, decode the results.
		 * Note: received addr is ignored, it could be different
		 * from the send addr if the host has more than one addr.
		 */
		xdrmem_create(xdrs, inbuf, (u_int) t_rdata.udata.len, XDR_DECODE);
		if (xdr_replymsg(xdrs, &msg)) {
			if (msg.rm_reply.rp_stat == MSG_ACCEPTED &&
			   (msg.rm_xid & ~0xFF) == xid) {
				i = msg.rm_xid & 0xFF;
				for (m = mfs ; m ; m = m->mfs_next) {
					if (m->mfs_ignore)
						continue;
					if (i-- <= 0)
						break;
				}
				done = (*eachresult)(m);
			}
			/* otherwise, we just ignore the errors ... */
		}
		xdrs->x_op = XDR_FREE;
		msg.acpted_rply.ar_results.proc = xdr_void;
		(void)xdr_replymsg(xdrs, &msg);
		XDR_DESTROY(xdrs);
		if (done) {
			stat = RPC_SUCCESS;
			goto done_broad;
		} else {
			goto recv_again;
		}
	}
	stat = RPC_TIMEDOUT;

done_broad:
	if (nc)
		endnetconfig(nc);
	free_transports(tr_head);
	AUTH_DESTROY(sys_auth);
	return (stat);
}

void
free_transports(trans)
	struct transp *trans;
{
	struct transp *t, *tmpt;
	struct addrs *a, *tmpa;

	for (t = trans ; t ; t = tmpt) {
		if (t->tr_taddr)
			(void) t_free(t->tr_taddr, T_BIND);
		if (t->tr_fd > 0)
			(void) t_close(t->tr_fd);
		for (a = t->tr_addrs ; a ; a = tmpa) {
			(void) netdir_free((char *)a->addr_addrs, ND_ADDRLIST);
			tmpa = a->addr_next;
			free(a);
		}
		tmpt = t->tr_next;
		free(t);
	}
}
