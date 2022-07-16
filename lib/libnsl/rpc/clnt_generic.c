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

#ident	"@(#)librpc:clnt_generic.c	1.3.1.1"

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
static char sccsid[] = "@(#)clnt_generic.c 1.32 89/03/16 Copyr 1988 Sun Micro";
#endif

#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/errno.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_ERR 3
#endif
#include <rpc/nettype.h>

extern int errno;
extern int t_errno;
extern char *strdup();
extern char *malloc();

/*
 * Top level client creation routine.
 * Generic client creation: takes (servers name, program-number, nettype) and
 * returns client handle. Default options are set, which the user can 
 * change using the rpc equivalent of ioctl()'s.
 * 
 * It tries for all the netids in that particular class of netid until
 * it succeeds.
 * XXX The error message in the case of failure will be the one
 * pertaining to the last create error.
 *
 * It calls clnt_tp_create();
 */
CLIENT *
clnt_create(hostname, prog, vers, nettype)
	char *hostname;				/* server name */
	u_long prog;				/* program number */
	u_long vers;				/* version number */
	char *nettype;				/* net type */
{
	struct netconfig *nconf;
	CLIENT *clnt = NULL;
	int net;

	if ((net = _rpc_setconf(nettype)) == 0) {
		rpc_createerr.cf_stat = RPC_UNKNOWNPROTO;
		return ((CLIENT *)NULL);
	}
	rpc_createerr.cf_stat = RPC_SUCCESS;
	while (clnt == (CLIENT *)NULL) {
		if ((nconf = _rpc_getconf(net)) == (struct netconfig *)NULL) {
			if (rpc_createerr.cf_stat == RPC_SUCCESS)
				rpc_createerr.cf_stat = RPC_UNKNOWNPROTO;
			break;
		}
		clnt = clnt_tp_create(hostname, prog, vers, nconf);
		if (clnt)
			break;
	}	
	_rpc_endconf();
	return (clnt);
}

/*
 * Generic client creation: takes (servers name, program-number, netconf) and
 * returns client handle. Default options are set, which the user can
 * change using the rpc equivalent of ioctl()'s : clnt_control()
 * It finds out the server address from rpcbind and calls clnt_tli_create()
 */
CLIENT *
clnt_tp_create(hostname, prog, vers, nconf)
	char *hostname;				/* server name */
	u_long prog;				/* program number */
	u_long vers;				/* version number */
	register struct netconfig *nconf;	/* net config struct */
{
	struct netbuf *svcaddr;			/* servers address */
	CLIENT *cl;				/* client handle */
	struct t_bind *tbind;			/* bind info */
	int fd;					/* end point descriptor */

	if (nconf == (struct netconfig *)NULL) {
		rpc_createerr.cf_stat = RPC_UNKNOWNPROTO;
		return ((CLIENT *)NULL);
	}
	if ((fd = t_open(nconf->nc_device, O_RDWR, NULL)) == -1) {
		rpc_createerr.cf_stat = RPC_TLIERROR;
		rpc_createerr.cf_error.re_terrno = t_errno;
		return ((CLIENT *)NULL);
	}

	/*
	 * Get the address of the server
	 */
	tbind = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	if (tbind == (struct t_bind *)NULL) {
		rpc_createerr.cf_stat = RPC_SYSTEMERROR;
		rpc_createerr.cf_error.re_errno = errno;
		rpc_createerr.cf_error.re_terrno = t_errno;
		(void) t_close(fd);
		return ((CLIENT *)NULL);
	}
	svcaddr = &tbind->addr;
	if (rpcb_getaddr(prog, vers, nconf, svcaddr, hostname) == FALSE) {
		(void) t_free((char *)tbind, T_BIND);
		/* appropriate error number is set by rpcbind libraries */
		(void) t_close(fd);
		return ((CLIENT *)NULL);
	}
	cl = clnt_tli_create(fd, nconf, svcaddr, prog, vers, 0, 0);
	(void) t_free((char *)tbind, T_BIND);
	if (cl == (CLIENT *)NULL) {
		(void) t_close(fd);
		return ((CLIENT *)NULL);
	}
	/*
	 * The fd should be closed while destroying the handle.
	 */
	(void) CLNT_CONTROL(cl, CLSET_FD_CLOSE, (char *)NULL);
	return (cl);
}

/*
 * Generic client creation:  returns client handle.
 * Default options are set, which the user can 
 * change using the rpc equivalent of ioctl()'s : clnt_control().
 * If fd is RPC_ANYFD, it will be opened using nconf.
 * It will be bound if not so.
 * If sizes are 0; appropriate defaults will be chosen.
 */
CLIENT *
clnt_tli_create(fd, nconf, svcaddr, prog, vers, sendsz, recvsz)
	register int fd;		/* fd */
	struct netconfig *nconf;	/* netconfig structure */
	struct netbuf *svcaddr;		/* servers address */
	u_long prog;			/* program number */
	u_long vers;			/* version number */
	u_int sendsz;			/* send size */
	u_int recvsz;			/* recv size */
{
	CLIENT *cl = NULL;		/* client handle */
	struct t_info tinfo;		/* transport info */
	bool_t madefd = FALSE;		/* whether fd opened here */

	if (fd == RPC_ANYFD) {
		if (nconf == (struct netconfig *)NULL) {
			rpc_createerr.cf_stat = RPC_UNKNOWNPROTO;
			return ((CLIENT *)NULL);
		}
		fd = t_open(nconf->nc_device, O_RDWR, &tinfo);
		if (fd == -1)
			goto err;
		madefd = TRUE;
		if (t_bind(fd, (struct t_bind *)NULL, (struct t_bind *)NULL) == -1)
				goto err;
	} else {
		int state;		/* Current state of provider */

		/*
		 * Sync the opened fd.
		 * Check whether bound or not, else bind it
		 */
		if (((state = t_sync(fd)) == -1) ||
		    ((state == T_UNBND) && (t_bind(fd, (struct t_bind *)NULL,
				(struct t_bind *)NULL) == -1)) ||
		    (t_getinfo(fd, &tinfo) == -1))
			goto err;
	}

	switch (tinfo.servtype) {
	case T_COTS:
	case T_COTS_ORD:
		cl = clnt_vc_create(fd, svcaddr, prog, vers, sendsz, recvsz);
		break;
	case T_CLTS:
		cl = clnt_dg_create(fd, svcaddr, prog, vers, sendsz, recvsz);
		break;
	default:
		goto err;
	}

	if (cl == (CLIENT *)NULL)
		goto err;
	if (nconf) {
		cl->cl_netid = strdup(nconf->nc_netid);
		cl->cl_tp = strdup(nconf->nc_device);
	} else {
		cl->cl_netid = "";
		cl->cl_tp = "";
	}
	if (madefd)
		(void) CLNT_CONTROL(cl, CLSET_FD_CLOSE, (char *)NULL);
	return (cl);
err:
	if (madefd)
		(void) t_close(fd);
	rpc_createerr.cf_stat = RPC_TLIERROR;
	rpc_createerr.cf_error.re_errno = 0;
	rpc_createerr.cf_error.re_terrno = t_errno;
	return ((CLIENT *)NULL);
}
