/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rpcbind:rpcbind.c	1.14.5.1"

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
 * rpcbind.c
 * Implements the program, version to address mapping for rpc.
 *
 * Copyright (c) 1984 by Sun Microsystems, Inc.
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
#ifdef PORTMAP
#include <netinet/in.h>
#include <rpc/pmap_prot.h>
#endif
#include <sys/termios.h>
#include "rpcbind.h"
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define	LOG_DAEMON (3<<3)
#define	LOG_CONS 0x02
#define	LOG_ERR 3
#endif /* SYSLOG */

extern void *malloc();
extern char *strdup();

#ifdef PORTMAP
extern int pmap_service();
#endif
extern int rpcb_service();
#ifdef WAIT3
void reap();
#endif
void terminate();

/* Global variables */
int debugging = 0;
RPCBLIST *list_rbl;	/* A list of version 3 rpcbind services */
char *loopback_dg;	/* Datagram loopback transport, for set and unset */
char *loopback_vc;	/* COTS loopback transport, for set and unset */
char *loopback_vc_ord;	/* COTS_ORD loopback transport, for set and unset */

#ifdef PORTMAP
PMAPLIST *list_pml;	/* A list of version 2 rpcbind services */
char *udptrans;		/* Name of UDP transport */
char *tcptrans;		/* Name of TCP transport */
char *udp_uaddr;	/* Universal UDP address */
char *tcp_uaddr;	/* Universal TCP address */
#endif
static char servname[] = "rpcbind";
static char superuser[] = "superuser";

extern int t_errno;
extern char *t_errlist[];

main()
{
	struct netconfig *nconf;
	NCONF_HANDLE *nc_handle;	/* Net config handle */

	openlog("rpcbind", LOG_CONS, LOG_DAEMON);
	if (geteuid()) { /* This command allowed only to root */
		syslog(LOG_ERR, "Sorry. You are not superuser\n");
		exit(1);
	}
	nc_handle = setnetconfig();
	if (nc_handle == 0) {	/* open netconfig file */
		syslog(LOG_ERR, "could not read /etc/netconfig");
		exit(1);
	}
#ifdef DEBUG
	debugging = 1;
#endif
	loopback_dg = "";
	loopback_vc = "";
	loopback_vc_ord = "";
#ifdef PORTMAP
	udptrans = "";
	tcptrans = "";
#endif

	while (nconf = getnetconfig(nc_handle)) {
		init_transport(nconf);
	}
	endnetconfig(nc_handle);

	if ((loopback_dg[0] == NULL) && (loopback_vc[0] == NULL) &&
		(loopback_vc_ord[0] == NULL)) {
		syslog(LOG_ERR, "could not find loopback transports\n");
		exit(1);
	}

	(void) signal(SIGCHLD, SIG_IGN); /* XXX see reap below */
	(void) signal(SIGINT, terminate);
	if (!debugging)
		(void) detachfromtty();
	svc_run();
	syslog(LOG_ERR, "svc_run returned unexpectedly");
	abort();
	/* NOTREACHED */
}

/*
 * Adds the entry into the rpcbind database.
 * If PORTMAP, then for UDP and TCP, it adds the entries for version 2 also
 * Returns 0 if succeeds, else fails
 */
static int
init_transport(nconf)
	struct netconfig *nconf;	/* Transport provider info */
{
	int fd;
	struct t_bind *taddr, *baddr;
	RPCBLIST *rbl;
	SVCXPRT	*my_xprt;
	struct nd_addrlist *nas;
	struct nd_hostserv hs;
	int status;	/* bound checking ? */

	if ((nconf->nc_semantics != NC_TPI_CLTS) &&
		(nconf->nc_semantics != NC_TPI_COTS) &&
		(nconf->nc_semantics != NC_TPI_COTS_ORD))
		return (1);	/* not my type */
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

	if ((fd = t_open(nconf->nc_device, O_RDWR, NULL)) < 0) {
		syslog(LOG_ERR, "%s: cannot open connection: %s",
				nconf->nc_netid, t_errlist[t_errno]);
		return (1);
	}

	taddr = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	baddr = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	if ((baddr == NULL) || (taddr == NULL)) {
		syslog(LOG_ERR, "%s: cannot allocate netbuf: %s",
				nconf->nc_netid, t_errlist[t_errno]);
		exit(1);
	}

	/* Get rpcbind's address on this transport */
	hs.h_host = HOST_SELF;
	hs.h_serv = servname;
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
	(void) fprintf(stderr, "rpcbind : my address is %s\n", uaddr);
	(void) free(uaddr);
	}
#endif
	netdir_free((char *)nas, ND_ADDRLIST);

	if (nconf->nc_semantics == NC_TPI_CLTS)
		taddr->qlen = 0;
	else
		taddr->qlen = 8;	/* should be enough */


	if (t_bind(fd, taddr, baddr) != 0) {
		syslog(LOG_ERR, "%s: cannot bind: %s",
			nconf->nc_netid, t_errlist[t_errno]);
		goto error;
	}

	if (memcmp(taddr->addr.buf, baddr->addr.buf, (int)baddr->addr.len)) {
		syslog(LOG_ERR, "%s: address in use", nconf->nc_netid);
		goto error;
	}

	my_xprt = (SVCXPRT *)svc_tli_create(fd, nconf, baddr, 0, 0);
	if (my_xprt == (SVCXPRT *)NULL) {
		syslog(LOG_ERR, "%s: could not create service",
				nconf->nc_netid);
		goto error;
	}

#ifdef PORTMAP
	/*
	 * Register both the versions for tcp/ip and udp/ip
	 */
	if ((strcmp(nconf->nc_protofmly, NC_INET) == 0) &&
		((strcmp(nconf->nc_proto, NC_TCP) == 0) ||
		(strcmp(nconf->nc_proto, NC_UDP) == 0))) {
		PMAPLIST *pml;

		if (!svc_register(my_xprt, PMAPPROG, PMAPVERS,
			pmap_service, NULL)) {
			syslog(LOG_ERR, "could not register on %s",
					nconf->nc_netid);
			goto error;
		}
		pml = (PMAPLIST *)malloc((u_int)sizeof (PMAPLIST));
		if (pml == (PMAPLIST *)NULL) {
			syslog(LOG_ERR, "no memory!");
			exit(1);
		}
		pml->pml_map.pm_prog = PMAPPROG;
		pml->pml_map.pm_vers = PMAPVERS;
		pml->pml_map.pm_port = PMAPPORT;
		if (strcmp(nconf->nc_proto, NC_TCP) == 0) {
			if (tcptrans[0]) {
				syslog(LOG_ERR,
				"cannot have more than one TCP transport");
				goto error;
			}
			tcptrans = strdup(nconf->nc_netid);
			pml->pml_map.pm_prot = IPPROTO_TCP;

			/* Let's snarf the universal address */
			/* "h1.h2.h3.h4.p1.p2" */
			tcp_uaddr = taddr2uaddr(nconf, &baddr->addr);
		} else {
			if (udptrans[0]) {
				syslog(LOG_ERR,
				"cannot have more than one UDP transport");
				goto error;
			}
			udptrans = strdup(nconf->nc_netid);
			pml->pml_map.pm_prot = IPPROTO_UDP;

			/* Let's snarf the universal address */
			/* "h1.h2.h3.h4.p1.p2" */
			udp_uaddr = taddr2uaddr(nconf, &baddr->addr);
		}
		pml->pml_next = list_pml;
		list_pml = pml;

		/* Add version 3 information */
		pml = (PMAPLIST *)malloc((u_int)sizeof (PMAPLIST));
		if (pml == (PMAPLIST *)NULL) {
			syslog(LOG_ERR, "no memory!");
			exit(1);
		}
		pml->pml_map = list_pml->pml_map;
		pml->pml_map.pm_vers = RPCBVERS;
		pml->pml_next = list_pml;
		list_pml = pml;

		/* Also add version 2 stuff to rpcbind list */
		rbl = (RPCBLIST *)malloc((u_int)sizeof (RPCBLIST));
		if (rbl == (RPCBLIST *)NULL) {
			syslog(LOG_ERR, "no memory!");
			exit(1);
		}

		rbl->rpcb_map.r_prog = PMAPPROG;
		rbl->rpcb_map.r_vers = PMAPVERS;	/* Version 2 */
		rbl->rpcb_map.r_netid = strdup(nconf->nc_netid);
		rbl->rpcb_map.r_addr = taddr2uaddr(nconf, &baddr->addr);
		rbl->rpcb_map.r_owner = superuser;
		rbl->rpcb_next = list_rbl;	/* Attach to global list */
		list_rbl = rbl;
	}
#endif

	/* version 3 registration */
	if (!svc_reg(my_xprt, RPCBPROG, RPCBVERS, rpcb_service, NULL)) {
		syslog(LOG_ERR, "could not register %s version 3",
				nconf->nc_netid);
		goto error;
	}
	rbl = (RPCBLIST *)malloc((u_int)sizeof (RPCBLIST));
	if (rbl == (RPCBLIST *)NULL) {
		syslog(LOG_ERR, "no memory!");
		exit(1);
	}

	rbl->rpcb_map.r_prog = RPCBPROG;
	rbl->rpcb_map.r_vers = RPCBVERS; /* The new version number */
	rbl->rpcb_map.r_netid = strdup(nconf->nc_netid);
	rbl->rpcb_map.r_addr = taddr2uaddr(nconf, &baddr->addr);
	rbl->rpcb_map.r_owner = superuser;
	rbl->rpcb_next = list_rbl;	/* Attach to global list */
	list_rbl = rbl;

	/*
	 * Tell RPC library to shut up about version mismatches so that new
	 * revs of broadcast protocols don't cause all the old servers to
	 * say: "wrong version".
	 */
	svc_versquiet(my_xprt);

	/*
	 * In case of loopback transports, negotiate for
	 * returning of the uid of the caller.
	 */
	if (strcmp(nconf->nc_protofmly, NC_LOOPBACK) == 0) {
		if (nconf->nc_semantics == NC_TPI_CLTS)
			loopback_dg = strdup(nconf->nc_netid);
		else if (nconf->nc_semantics == NC_TPI_COTS)
			loopback_vc = strdup(nconf->nc_netid);
		else if (nconf->nc_semantics == NC_TPI_COTS_ORD)
			loopback_vc_ord = strdup(nconf->nc_netid);
		if (_rpc_negotiate_uid(fd)) {
			syslog(LOG_ERR,
			"could not negotiate with loopback tranport %s",
				nconf->nc_netid);
		}
	}

	/* decide if bound checking works for this transport */
	status = add_bndlist(nconf, taddr, baddr);
#ifdef BIND_DEBUG
	if (status < 0) {
		fprintf(stderr, "Error in finding bind status for %s\n",
			nconf->nc_netid);
	} else if (status == 0) {
		fprintf(stderr, "check binding for %s\n",
			nconf->nc_netid);
	} else if (status > 0) {
		fprintf(stderr, "No check binding for %s\n",
			nconf->nc_netid);
	}
#endif

	(void) t_free((char *)taddr, T_BIND);
	(void) t_free((char *)baddr, T_BIND);
	return (0);
error:
	(void) t_free((char *)taddr, T_BIND);
	(void) t_free((char *)baddr, T_BIND);
	(void) t_close(fd);
	return (1);
}

/*
 * XXX this should be fixed to reap our children rather than ignoring the
 * signal like we do for now ...
 */
#ifdef WAIT3
static void
reap()
{
	while (wait3(NULL, WNOHANG, NULL) > 0);
}
#endif

/*
 * Catch the signal and die
 */
static void
terminate()
{
	syslog(LOG_ERR, "terminating on signal");
	exit(2);
}

/*
 * detach from tty
 */
static
detachfromtty()
{
	close(0);
	close(1);
	close(2);
	switch (fork()) {
	case (pid_t)-1:
		perror("fork");
		break;
	case 0:
		break;
	default:
		exit(0);
	}
	setsid();
	(void)open("/dev/null", O_RDWR, 0);
	dup(0);
	dup(0);
}
