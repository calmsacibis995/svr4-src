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

#ident	"@(#)librpc:rpc_soc.c	1.4.2.1"

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
static char sccsid[] = "@(#)rpc_soc.c 1.41 89/05/02 Copyr 1988 Sun Micro";
#endif

#ifdef PORTMAP
/*
 * rpc_soc.c
 *
 * The backward compatibility routines for the earlier implementation
 * of RPC, where the only transports supported were tcp/ip and udp/ip.
 * Based on berkeley socket abstraction, now implemented on the top
 * of TLI/Streams
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netdir.h>
#include <errno.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_ERR 3
#endif /* SYSLOG */
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_prot.h>
#include <rpc/nettype.h>
#include <sys/byteorder.h>

extern int errno;
extern int t_errno;

/*
 * A common clnt create routine
 */
static CLIENT *
clnt_com_create(raddr, prog, vers, sockp, sendsz, recvsz, tp)
	register struct sockaddr_in *raddr;
	u_long prog;
	u_long vers;
	int *sockp;
	u_int sendsz;
	u_int recvsz;
	char *tp;
{
	CLIENT *cl;
	int madefd = FALSE;
	int fd = *sockp;
	struct t_bind *tbind;
	struct netconfig *nconf;
	int port;

	if ((nconf = _rpc_getconfip(tp)) == NULL) {
		rpc_createerr.cf_stat = RPC_UNKNOWNPROTO;
		return ((CLIENT *)NULL);
	}
	if (fd == RPC_ANYSOCK) {
		fd = t_open(nconf->nc_device, O_RDWR, (struct t_info *)NULL);
		if (fd == -1) {
			(void) freenetconfigent(nconf);
			rpc_createerr.cf_stat = RPC_SYSTEMERROR;
			rpc_createerr.cf_error.re_errno = errno;
			rpc_createerr.cf_error.re_terrno = t_errno;
			return ((CLIENT *)NULL);
		}
		madefd = TRUE;
	}
	if (raddr->sin_port == 0) {
		u_int proto;
		u_short sport;

		proto = strcmp(tp, "udp") == 0 ? IPPROTO_UDP : IPPROTO_TCP;
		sport = pmap_getport(raddr, prog, vers, proto);
		if (sport == 0) {
			rpc_createerr.cf_stat = RPC_PROGUNAVAIL;
			goto err;
		}
		raddr->sin_port = htons(sport);
	}

	/* Transform sockaddr_in to netbuf */
	tbind = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	if (tbind == NULL) {
		(void) syslog(LOG_ERR, "clnt_create: out of memory");
		rpc_createerr.cf_stat = RPC_SYSTEMERROR;
		rpc_createerr.cf_error.re_errno = errno;
		rpc_createerr.cf_error.re_terrno = t_errno;
		goto err;
	}
	(void) memcpy(tbind->addr.buf, (char *)raddr, (int)tbind->addr.maxlen);
	tbind->addr.len = tbind->addr.maxlen;

	(void) bindresvport(fd, (struct sockaddr_in *)NULL, &port, 0);
	cl = clnt_tli_create(fd, nconf, &(tbind->addr), prog, vers,
				sendsz, recvsz);
	(void) freenetconfigent(nconf);
	(void) t_free((char *)tbind, T_BIND);
	if (cl) {
		*sockp = fd;
		if (madefd == TRUE)
			/*
			 * The fd should be closed while destroying the handle.
			 */
			(void) CLNT_CONTROL(cl, CLSET_FD_CLOSE, (char *)NULL);
		return (cl);
	}

err:	if (madefd == TRUE)
		(void) t_close(fd);
	(void) freenetconfigent(nconf);
	return ((CLIENT *)NULL);
}

CLIENT *
clntudp_bufcreate(raddr, prog, vers, wait, sockp, sendsz, recvsz)
	register struct sockaddr_in *raddr;
	u_long prog;
	u_long vers;
	struct timeval wait;
	int *sockp;
	u_int sendsz;
	u_int recvsz;
{
	CLIENT *cl;

	cl = clnt_com_create(raddr, prog, vers, sockp, sendsz, recvsz, "udp");
	if (cl == (CLIENT *)NULL)
		return ((CLIENT *)NULL);
	(void) CLNT_CONTROL(cl, CLSET_RETRY_TIMEOUT, &wait);
	return (cl);
}

CLIENT *
clntudp_create(raddr, program, version, wait, sockp)
	struct sockaddr_in *raddr;
	u_long program;
	u_long version;
	struct timeval wait;
	int *sockp;
{
	return(clntudp_bufcreate(raddr, program, version, wait, sockp,
					UDPMSGSIZE, UDPMSGSIZE));
}

CLIENT *
clnttcp_create(raddr, prog, vers, sockp, sendsz, recvsz)
	struct sockaddr_in *raddr;
	u_long prog;
	u_long vers;
	register int *sockp;
	u_int sendsz;
	u_int recvsz;
{
	return (clnt_com_create(raddr, prog, vers, sockp, sendsz, recvsz, "tcp"));
}

CLIENT *
clntraw_create(prog, vers)
	u_long prog;
	u_long vers;
{
	return (clnt_raw_create(prog, vers));
}

/*
 * A common server create routine
 */
static SVCXPRT *
svc_com_create(fd, sendsize, recvsize, netid)
	register int fd;
	u_int sendsize;
	u_int recvsize;
	char *netid;
{
	struct netconfig *nconf;
	SVCXPRT *svc;
	int madefd = FALSE;
	int port;
	int res;

	if ((nconf = _rpc_getconfip(netid)) == NULL) {
		(void) syslog(LOG_ERR, "Could not get %s transport", netid);
		return ((SVCXPRT *)NULL);
	}
	if (fd == RPC_ANYSOCK) {
		fd = t_open(nconf->nc_device, O_RDWR, (struct t_info *)NULL);
		if (fd == -1) {
			(void) freenetconfigent(nconf);
			(void) syslog(LOG_ERR,
			"svc%s_create: could not open connection", netid);
			return ((SVCXPRT *)NULL);
		}
		madefd = TRUE;
	}

	res = bindresvport(fd, (struct sockaddr_in *)NULL, &port, 8);
	svc = svc_tli_create(fd, nconf, (struct t_bind *)NULL,
				sendsize, recvsize);
	(void) freenetconfigent(nconf);
	if (svc == (SVCXPRT *)NULL) {
		if (madefd)
			(void) t_close(fd);
		return ((SVCXPRT *)NULL);
	}
	if (res == -1){
		/* Specifically set xp_port now */
		svc->xp_port = ((struct sockaddr_in *)svc->xp_ltaddr.buf)->sin_port;
		svc->xp_port = ntohs(svc->xp_port);
	}else
		svc->xp_port = ntohs(port);
	return (svc);
}

SVCXPRT *
svctcp_create(fd, sendsize, recvsize)
	register int fd;
	u_int sendsize;
	u_int recvsize;
{
	return (svc_com_create(fd, sendsize, recvsize, "tcp"));
}

SVCXPRT *
svcudp_bufcreate(fd, sendsz, recvsz)
	register int fd;
	u_int sendsz, recvsz;
{
	return (svc_com_create(fd, sendsz, recvsz, "udp"));
}

SVCXPRT *
svcfd_create(fd, sendsize, recvsize)
      int fd;
      u_int sendsize;
      u_int recvsize;
{
      return (svc_fd_create(fd, sendsize, recvsize));
}


SVCXPRT *
svcudp_create(fd)
	register int fd;
{
	return (svc_com_create(fd, UDPMSGSIZE, UDPMSGSIZE, "udp"));
}

SVCXPRT *
svcraw_create()
{
	return (svc_raw_create());
}

/*
 * Bind a fd to a privileged IP port
 */
static int
bindresvport(fd, sin, portp, qlen)
	int fd;
	struct sockaddr_in *sin;
	int *portp;
	int qlen;
{
	int res;
	static short port;
	struct sockaddr_in myaddr;
	extern int errno;
	extern int t_errno;
	int i;
	struct t_bind *tbind, *tres;

#define STARTPORT 600
#define ENDPORT (IPPORT_RESERVED - 1)
#define NPORTS	(ENDPORT - STARTPORT + 1)

	if (geteuid()) {
		errno = EACCES;
		return (-1);
	}
	if ((i = t_getstate(fd)) != T_UNBND) {
		if (t_errno == TBADF)
			errno = EBADF;
		if (i != -1)
			errno = EISCONN;
		return (-1);
	}
	if (sin == (struct sockaddr_in *)NULL) {
		sin = &myaddr;
		(void)memset((char *)sin, 0, sizeof (*sin));
		sin->sin_family = AF_INET;
	} else if (sin->sin_family != AF_INET) {
		errno = EPFNOSUPPORT;
		return (-1);
	}
	if (port == 0)
		port = (getpid() % NPORTS) + STARTPORT;
	res = -1;
	errno = EADDRINUSE;
	/* Transform sockaddr_in to netbuf */
	tbind = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	if (tbind == NULL) {
		if (t_errno == TBADF)
			errno = EBADF;
		return (-1);
	}
	tres = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	if (tres == NULL) {
		(void) t_free((char *)tbind, T_BIND);
		return (-1);
	}

	tbind->qlen = qlen;
	(void) memcpy(tbind->addr.buf, (char *)sin, (int)tbind->addr.maxlen);
	tbind->addr.len = tbind->addr.maxlen;
	sin = (struct sockaddr_in *)tbind->addr.buf;

	for (i = 0; i < NPORTS && errno == EADDRINUSE; i++) {
		sin->sin_port = htons(port++);
		if (port > ENDPORT)
			port = STARTPORT;
		res = t_bind(fd, tbind, tres);
		if ((res == 0) && (memcmp(tbind->addr.buf, tres->addr.buf,
					(int)tres->addr.len) == 0))
			break;
	}

	if (res == 0)
		*portp = sin->sin_port;
	(void) t_free((char *)tbind, T_BIND);
	(void) t_free((char *)tres, T_BIND);
	return (res);
}

/* 
 * Get clients IP address.
 * don't use gethostbyname, which would invoke yellow pages
 * Remains only for backward compatibility reasons.
 * Used mainly by the portmapper so that it can register
 * with itself. Also used by pmap*() routines
 */
int
get_myaddress(addr)
	struct sockaddr_in *addr;
{
	memset((char *)addr, 0, sizeof (struct sockaddr_in));
	addr->sin_port = htons(PMAPPORT);
	addr->sin_family = AF_INET;
	return (0);
}

/*
 * For connectionless "udp" transport. Obsoleted by rpc_call().
 */
callrpc(host, prognum, versnum, procnum, inproc, in, outproc, out)
	char *host;
	u_long prognum, versnum, procnum;
	xdrproc_t inproc, outproc;
	char *in, *out;
{
	return ((int)rpc_call(host, prognum, versnum, procnum, inproc,
				in, outproc, out, "udp"));
}

/*
 * For connectionless kind of transport. Obsoleted by rpc_reg()
 */
registerrpc(prognum, versnum, procnum, progname, inproc, outproc)
	u_long prognum, versnum, procnum;
	char *(*progname)();
	xdrproc_t inproc, outproc;
{
	return (rpc_reg(prognum, versnum, procnum, progname, inproc,
				outproc, "udp"));
}

/*
 * All the following clnt_broadcast stuff is convulated; it supports
 * the earlier calling style of the callback function
 */
typedef bool_t (*resultproc_t)();

static resultproc_t clnt_broadcast_result;
/*
 * Need to translate the netbuf address into sockaddr_in address.
 * Dont care about netid here.
 */
static bool_t
rpc_wrap_bcast(resultp, addr, nconf)
	char *resultp;		/* results of the call */
	struct netbuf *addr;	/* address of the guy who responded */
	struct netconfig *nconf;/* Netconf of the transport */
{
	struct sockaddr_in sockaddr;

	(void) memset((char *)&sockaddr, 0, sizeof(sockaddr));
	(void) memcpy((char *)&sockaddr, addr->buf, addr->len);
	return((*clnt_broadcast_result)(resultp, &sockaddr));
}

/*
 * Broadcasts on UDP transport. Obsoleted by rpc_broadcast().
 */
enum clnt_stat 
clnt_broadcast(prog, vers, proc, xargs, argsp, xresults, resultsp, eachresult)
	u_long		prog;		/* program number */
	u_long		vers;		/* version number */
	u_long		proc;		/* procedure number */
	xdrproc_t	xargs;		/* xdr routine for args */
	caddr_t		argsp;		/* pointer to args */
	xdrproc_t	xresults;	/* xdr routine for results */
	caddr_t		resultsp;	/* pointer to results */
	resultproc_t	eachresult;	/* call with each result obtained */
{
	clnt_broadcast_result = eachresult;
	return (rpc_broadcast(prog, vers, proc, xargs, argsp, xresults,
				resultsp, rpc_wrap_bcast, "udp"));
}

/*
 * Create the client des authentication object. Obsoleted by
 * authdes_seccreate().
 */
AUTH *
authdes_create(servername, window, syncaddr, ckey)
	char *servername;		/* network name of server */
	u_int window;			/* time to live */
	struct sockaddr_in *syncaddr;	/* optional hostaddr to sync with */
	des_block *ckey;		/* optional conversation key to use*/
{
	char *hostname = NULL;
	struct netconfig *nconf;
	struct netbuf nb_syncaddr;
	struct nd_hostservlist *hlist;
	AUTH *nauth;

	if (syncaddr) {
		/*
	 	 * Change addr to hostname.
		 */
		if ((nconf = getnetconfigent("tcp")) == NULL &&
		    (nconf = getnetconfigent("udp")) == NULL)
			return (authdes_seccreate(servername, window, hostname, ckey));

		nb_syncaddr.maxlen = nb_syncaddr.len = sizeof(struct sockaddr_in);
		nb_syncaddr.buf = (char *)syncaddr;

		if (netdir_getbyaddr(nconf, &hlist, &nb_syncaddr) < 0) {
			(void)freenetconfigent(nconf);
			return (authdes_seccreate(servername, window, hostname, ckey));
		}

		if (hlist && hlist->h_cnt > 0 && hlist->h_hostservs)
			hostname = hlist->h_hostservs->h_host;
		nauth = authdes_seccreate(servername, window, hostname, ckey);
		(void)netdir_free(hlist, ND_HOSTSERVLIST);
		(void)freenetconfigent(nconf);
		return(nauth);
	}
	return (authdes_seccreate(servername, window, hostname, ckey));
}

#endif /* PORTMAP */
