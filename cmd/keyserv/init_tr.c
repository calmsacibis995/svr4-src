/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)keyserv:init_tr.c	1.2.2.1"

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

#ifndef lint
static	char sccsid[] = "@(#)rpcbind.c 1.35 89/04/21 Copyr 1984 Sun Micro";
#endif

/*
 * init_tr.c
 * registers the keyserv with all transports at a known address
 * 
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/rpcb_prot.h>
#include <netconfig.h>
#include <netdir.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/termios.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_DAEMON (3<<3)
#define LOG_CONS 0x02
#define LOG_ERR 3
#endif /* SYSLOG */

extern char *malloc();
extern char *strdup();

int
rpc_init_transport(nconf, service, prog, vers, callback)
	struct netconfig *nconf;	/* Transport provider info */
	char *service;
	unsigned prog, vers;
	void *callback;
{
	int fd;
	int bounds_checking;		/*true is bounds checking
					  is on for a given transport*/
	struct t_bind *taddr, *baddr;
	RPCBLIST *rbl;
	SVCXPRT	*my_xprt;
	struct nd_addrlist *nas;
	struct nd_hostserv hs;
	struct t_info tinfo;

	if (!(nconf->nc_flag ))
		return (1);

#ifdef ND_DEBUG
	{
	int i;
	char **s;

	(void) fprintf(stderr, "%s: %d lookup routines :\n",
		nconf->nc_netid, nconf->nc_nlookups);
	for (i = 0, s = nconf->nc_lookups; i < nconf->nc_nlookups; i++, s++)
		fprintf(stderr, "[%d] - %s\n", i, *s);
	}
#endif

	if ((fd = t_open(nconf->nc_device, O_RDWR, &tinfo)) < 0) {
		fprintf(stderr, "%s: cannot open connection",
				nconf->nc_netid);
		return (1);
	}

	taddr = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	baddr = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	if ((baddr == NULL) || (taddr == NULL)) {
		fprintf(stderr, "%s: cannot allocate netbuf",
				nconf->nc_netid);
		exit(1);
	}

	/*
	 * for binding address use null string
	 * for ip returns 0.0.0.0 for other transports
	 * netdir may use _rpc_gethostname() if it sees this
	 */

	hs.h_host = HOST_SELF;
	hs.h_serv =  service;
	
	if (netdir_getbyname(nconf, &hs, &nas))
		goto error;

	/* Copy the address */
	taddr->addr.len = nas->n_addrs->len;
	memcpy(taddr->addr.buf, nas->n_addrs->buf, (int)nas->n_addrs->len);
#ifdef ND_DEBUG
	{
	/* for debugging print out our universal address */
	char *uaddr;

	uaddr = taddr2uaddr(nconf, nas->n_addrs); 
	(void) fprintf(stderr, "keyserv : my address is %s\n", uaddr);
	(void) free(uaddr);
	}
#endif
	netdir_free((char *)nas, ND_ADDRLIST);
	/* Set the qlen only for cots transports */
	switch (tinfo.servtype) {
	case T_COTS:
	case T_COTS_ORD:
		taddr->qlen = 8;	/* should be enough */
		break;
	case T_CLTS:
		break;
	default:
		goto error;
	}

	if (t_bind(fd, taddr, baddr) != 0) {
		fprintf(stderr, "%s: cannot bind",
			nconf->nc_netid);
		goto error;
	}

	if (memcmp(taddr->addr.buf, baddr->addr.buf, (int)baddr->addr.len)) {
		fprintf(stderr, "%s: address in use", nconf->nc_netid);
		goto error;
	}

	my_xprt = (SVCXPRT *)svc_tli_create(fd, nconf, baddr, 0, 0);
	if (my_xprt == (SVCXPRT *)NULL) {
		fprintf(stderr, "%s: could not create service",
				nconf->nc_netid);
		goto error;
	}


	/* version 3 registration */
	if (!svc_reg(my_xprt, prog, vers, callback, nconf)) {
		fprintf(stderr, "could not register %s version %d",
				nconf->nc_netid,vers);
		goto error;
	}

	(void) t_free((char *)baddr, T_BIND);
	(void) t_free((char *)taddr, T_BIND);
	return (0);
error:
	(void) t_free((char *)taddr, T_BIND);
	(void) t_free((char *)baddr, T_BIND);
	(void) t_close(fd);
	return (1);
}
