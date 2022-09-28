/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rpcinfo:rpcinfo.c	1.7.5.1"

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
static char sccsid[] = "@(#)rpcinfo.c 1.16 89/04/05 Copyr 1986 Sun Micro";
#endif

/*
 * rpcinfo: ping a particular rpc program
 * 	or dump the the registered programs on the remote machine.
 */

/*
 * If PORTMAP is defined, rpcinfo will talk to both portmapper and
 * rpcbind programs; else it talks only to rpcbind. In the later case
 * all the portmapper specific options such as -u, -t, -p become void.
 */
#include <rpc/rpc.h>
#include <stdio.h>
#include <rpc/rpcb_prot.h>
#include <rpc/nettype.h>
#include <netdir.h>
#include <rpc/rpcent.h>

#ifdef PORTMAP		/* Support for version 2 portmapper */
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#endif

#define	MAXHOSTLEN 256
#define	MIN_VERS	((u_long) 0)
#define	MAX_VERS	((u_long) 4294967295L)
#define	UNKNOWN		"unknown"

extern int	t_errno;
extern long	strtol();

#ifdef PORTMAP
static void	ip_ping(/*u_short portflag, char *trans,
				int argc, char **argv*/);
static CLIENT	*clnt_com_create(/* struct sockaddr_in *addr, long prog,
			long vers, int *fd, char *trans*/);
static void	pmapdump(/*int argc, char **argv*/);
static void	get_inet_address(/*struct sockaddr_in *addr, char *host*/);
#endif

static bool_t	reply_proc(/*void *res, struct netbuf *who*,
			struct netconfig *nconf*/);
static void	brdcst(/*int argc, char **argv*/);
static void	addrping(/*char *address, char *netid,
				int argc, char **argv*/);
static void	progping(/* char *netid, int argc, char **argv*/);
static CLIENT	*clnt_addr_create(/* char *addr, struct netconfig *nconf,
				long prog, long vers*/);
static int	pstatus(/*CLIENT *client, u_long prognum, u_long vers*/);
static void	rpcbdump(/*char *netid, int argc, char **argv*/);
static void	deletereg(/*char *netid, int argc, char **argv */);
static void	usage(/*void*/);
static u_long	getprognum(/*char *arg*/);
static u_long	getvers(/*char *arg*/);

/*
 * Functions to be performed.
 */
#define	NONE		0	/* no function */
#define	PMAPDUMP	1	/* dump portmapper registrations */
#define	TCPPING		2	/* ping TCP service */
#define	UDPPING		3	/* ping UDP service */
#define	BROADCAST	4	/* ping broadcast service */
#define	DELETES		5	/* delete registration for the service */
#define	ADDRPING	6	/* pings at the given address */
#define	PROGPING	7	/* pings a program on a given host */
#define	RPCBDUMP	8	/* dump rpcbind registrations */

int
main(argc, argv)
	int argc;
	char **argv;
{
	register int c;
	extern char *optarg;
	extern int optind;
	int errflg;
	int function;
	char *netid = NULL;
	char *address = NULL;
#ifdef PORTMAP
	char *strptr;
	u_short portnum = 0;
#endif

	function = NONE;
	errflg = 0;
#ifdef PORTMAP
	while ((c = getopt(argc, argv, "a:bdn:ptT:u")) != EOF) {
#else
	while ((c = getopt(argc, argv, "a:bdT:")) != EOF) {
#endif
		switch (c) {
#ifdef PORTMAP
		case 'p':
			if (function != NONE)
				errflg = 1;
			else
				function = PMAPDUMP;
			break;

		case 't':
			if (function != NONE)
				errflg = 1;
			else
				function = TCPPING;
			break;

		case 'u':
			if (function != NONE)
				errflg = 1;
			else
				function = UDPPING;
			break;

		case 'n':
			portnum = (u_short) strtol(optarg, &strptr, 10);
			if (strptr == optarg || *strptr != '\0') {
				fprintf(stderr,
			"rpcinfo: %s is illegal port number\n",
					optarg);
				exit(1);
			}
			break;
#endif
		case 'a':
			address = optarg;
			if (function != NONE)
				errflg = 1;
			else
				function = ADDRPING;
			break;
		case 'b':
			if (function != NONE)
				errflg = 1;
			else
				function = BROADCAST;
			break;

		case 'd':
			if (function != NONE)
				errflg = 1;
			else
				function = DELETES;
			break;

		case 'T':
			netid = optarg;
			break;
		case '?':
			errflg = 1;
			break;
		}
	}

	if (errflg || ((function == ADDRPING) && !netid)) {
		usage();
		return (1);
	}
	if (function == NONE) {
		if (argc - optind > 1)
			function = PROGPING;
		else
			function = RPCBDUMP;
	}

	switch (function) {
#ifdef PORTMAP
	case PMAPDUMP:
		if (portnum != 0) {
			usage();
			return (1);
		}
		pmapdump(argc - optind, argv + optind);
		break;

	case UDPPING:
		ip_ping(portnum, "udp", argc - optind, argv + optind);
		break;

	case TCPPING:
		ip_ping(portnum, "tcp", argc - optind, argv + optind);
		break;
#endif
	case BROADCAST:
		brdcst(argc - optind, argv + optind);
		break;
	case DELETES:
		deletereg(netid, argc - optind, argv + optind);
		break;
	case ADDRPING:
		addrping(address, netid, argc - optind, argv + optind);
		break;
	case PROGPING:
		progping(netid, argc - optind, argv + optind);
		break;
	case RPCBDUMP:
		rpcbdump(netid, argc - optind, argv + optind);
		break;
	}
	return (0);
}

#ifdef PORTMAP
static CLIENT *
clnt_com_create(addr, prog, vers, fdp, trans)
	struct sockaddr_in *addr;
	u_long prog;
	u_long vers;
	int *fdp;
	char *trans;
{
	CLIENT *clnt;

	if (strcmp(trans, "tcp") == 0) {
		clnt = clnttcp_create(addr, prog, vers, fdp, 0, 0);
	} else {
		struct timeval to;

		to.tv_sec = 5;
		to.tv_usec = 0;
		clnt = clntudp_create(addr, prog, vers, to, fdp);
	}
	if (clnt == (CLIENT *)NULL) {
		clnt_pcreateerror("rpcinfo");
		if (vers == MIN_VERS)
			printf("program %lu is not available\n", prog);
		else
			printf("program %lu version %lu is not available\n",
							prog, vers);
		exit(1);
	}
	return (clnt);
}

/*
 * If portnum is 0, then go and get the address from portmapper, which happens
 * transparently through clnt*_create(); If version number is not given, it
 * tries to find out the version number by making a call to version 0 and if
 * that fails, it obtains the high order and the low order version number. If
 * version 0 calls succeeds, it tries for MAXVERS call and repeats the same.
 */
static void
ip_ping(portnum, trans, argc, argv)
	u_short portnum;
	char *trans;
	int argc;
	char **argv;
{
	CLIENT *client;
	int fd = RPC_ANYFD;
	struct timeval to;
	struct sockaddr_in addr;
	enum clnt_stat rpc_stat;
	u_long prognum, vers, minvers, maxvers;
	struct rpc_err rpcerr;
	int failure = 0;

	if (argc < 2 || argc > 3) {
		usage();
		exit(1);
	}
	to.tv_sec = 10;
	to.tv_usec = 0;
	prognum = getprognum(argv[1]);
	get_inet_address(&addr, argv[0]);
	if (argc == 2) {	/* Version number not known */
		/*
		 * A call to version 0 should fail with a program/version
		 * mismatch, and give us the range of versions supported.
		 */
		vers = MIN_VERS;
	} else {
		vers = getvers(argv[2]);
	}
	addr.sin_port = htons(portnum);
	client = clnt_com_create(&addr, prognum, vers, &fd, trans);
	rpc_stat = CLNT_CALL(client, NULLPROC, xdr_void, (char *)NULL,
				xdr_void, (char *)NULL, to);
	if (argc != 2) {
		/* Version number was known */
		if (pstatus(client, prognum, vers) < 0)
			exit(1);
		(void) CLNT_DESTROY(client);
		return;
	}
	/* Version number not known */
	(void) CLNT_CONTROL(client, CLSET_FD_NCLOSE, (char *)NULL);
	if (rpc_stat == RPC_PROGVERSMISMATCH) {
		clnt_geterr(client, &rpcerr);
		minvers = rpcerr.re_vers.low;
		maxvers = rpcerr.re_vers.high;
	} else if (rpc_stat == RPC_SUCCESS) {
		/*
		 * Oh dear, it DOES support version 0.
		 * Let's try version MAX_VERS.
		 */
		(void) CLNT_DESTROY(client);
		addr.sin_port = htons(portnum);
		client = clnt_com_create(&addr, prognum, MAX_VERS, &fd, trans);
		rpc_stat = CLNT_CALL(client, NULLPROC, xdr_void,
				(char *)NULL, xdr_void, (char *)NULL, to);
		if (rpc_stat == RPC_PROGVERSMISMATCH) {
			clnt_geterr(client, &rpcerr);
			minvers = rpcerr.re_vers.low;
			maxvers = rpcerr.re_vers.high;
		} else if (rpc_stat == RPC_SUCCESS) {
			/*
			 * It also supports version MAX_VERS.
			 * Looks like we have a wise guy.
			 * OK, we give them information on all
			 * 4 billion versions they support...
			 */
			minvers = 0;
			maxvers = MAX_VERS;
		} else {
			(void) pstatus(client, prognum, MAX_VERS);
			exit(1);
		}
	} else {
		(void) pstatus(client, prognum, (u_long)0);
		exit(1);
	}
	(void) CLNT_DESTROY(client);
	for (vers = minvers; vers <= maxvers; vers++) {
		addr.sin_port = htons(portnum);
		client = clnt_com_create(&addr, prognum, vers, &fd, trans);
		rpc_stat = CLNT_CALL(client, NULLPROC, xdr_void,
				(char *)NULL, xdr_void, (char *)NULL, to);
		if (pstatus(client, prognum, vers) < 0)
				failure = 1;
		(void) CLNT_DESTROY(client);
	}
	if (failure)
		exit(1);
	(void) t_close(fd);
	return;
}

/*
 * Dump all the portmapper registerations
 */
static void
pmapdump(argc, argv)
	int argc;
	char **argv;
{
	struct sockaddr_in server_addr;
	register struct hostent *hp;
	struct pmaplist *head = NULL;
	int socket = RPC_ANYSOCK;
	struct timeval minutetimeout;
	register CLIENT *client;
	struct rpcent *rpc;

	struct netconfig *nconf;
	struct nd_hostserv service;
	struct nd_addrlist *addrs;

	if (argc > 1) {
		usage();
		exit(1);
	}
	if (argc == 1)
		get_inet_address(&server_addr, argv[0]);
	else {
		(void) memset((char *)&server_addr, 0, sizeof server_addr);
		server_addr.sin_family = AF_INET;
		if ((nconf = getnetconfigent("tcp")) == NULL &&
		    (nconf = getnetconfigent("udp")) == NULL) {
			server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
		} else {
			service.h_host = HOST_SELF;
			service.h_serv = "rpcbind";
			if (netdir_getbyname(nconf, &service, &addrs) != 0 ) {
				(void) freenetconfigent(nconf);
				server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
			} else {
				(void) memcpy((caddr_t)&server_addr,
				    addrs->n_addrs->buf, addrs->n_addrs->len);
				(void) freenetconfigent(nconf);
				(void) netdir_free(addrs, ND_ADDRLIST);
			}
		}
	}
	minutetimeout.tv_sec = 60;
	minutetimeout.tv_usec = 0;
	server_addr.sin_port = htons(PMAPPORT);
	if ((client = clnttcp_create(&server_addr, PMAPPROG,
		PMAPVERS, &socket, 50, 500)) == NULL) {
		clnt_pcreateerror("rpcinfo: can't contact portmapper");
		exit(1);
	}
	if (CLNT_CALL(client, PMAPPROC_DUMP, xdr_void, NULL,
		xdr_pmaplist, &head, minutetimeout) != RPC_SUCCESS) {
		clnt_perror(client, "rpcinfo: can't contact portmapper");
		exit(1);
	}
	if (head == NULL) {
		printf("No remote programs registered.\n");
	} else {
		printf("   program vers proto   port\n");
		for (; head != NULL; head = head->pml_next) {
			printf("%10ld%5ld",
				head->pml_map.pm_prog,
				head->pml_map.pm_vers);
			if (head->pml_map.pm_prot == IPPROTO_UDP)
				printf("%6s",  "udp");
			else if (head->pml_map.pm_prot == IPPROTO_TCP)
				printf("%6s", "tcp");
			else
				printf("%6ld",  head->pml_map.pm_prot);
			printf("%7ld", head->pml_map.pm_port);
			rpc = getrpcbynumber(head->pml_map.pm_prog);
			if (rpc)
				printf("  %s\n", rpc->r_name);
			else
				printf("\n");
		}
	}
}

static void
get_inet_address(addr, host)
	struct sockaddr_in *addr;
	char *host;
{
	register struct hostent *hp;

	struct netconfig *nconf;
	struct nd_hostserv service;
	struct nd_addrlist *naddrs;

	(void) memset((char *)addr, 0, sizeof *addr);
	addr->sin_addr.s_addr = inet_addr(host);
	if (addr->sin_addr.s_addr == -1 || addr->sin_addr.s_addr == 0) {
		if ((nconf = getnetconfigent("tcp")) == NULL &&
		    (nconf = getnetconfigent("udp")) == NULL) {
			fprintf(stderr, "rpcinfo: %s is unknown host\n", host);
			exit(1);
		} else {
			service.h_host = host;
			service.h_serv = "rpcbind";
			if (netdir_getbyname(nconf, &service, &naddrs) != 0 ) {
				(void) freenetconfigent(nconf);
				fprintf(stderr, "rpcinfo: %s is unknown host\n", host);
				exit(1);
			} else {
				(void) memcpy((caddr_t)addr,
				    naddrs->n_addrs->buf, naddrs->n_addrs->len);
				(void) freenetconfigent(nconf);
				(void) netdir_free(naddrs, ND_ADDRLIST);
			}
		}
	}
	addr->sin_family = AF_INET;
}
#endif /* PORTMAP */

/*
 * reply_proc collects replies from the broadcast.
 * to get a unique list of responses the output of rpcinfo should
 * be piped through sort(1) and then uniq(1).
 */

/*ARGSUSED*/
static bool_t
reply_proc(res, who, nconf)
	void *res;		/* Nothing comes back */
	struct netbuf *who;	/* Who sent us the reply */
	struct netconfig *nconf;/* On which transport the reply came */
{
	struct nd_hostservlist *serv;
	char *uaddr;
	char *hostname;

	if (netdir_getbyaddr(nconf, &serv, who)) {
		fprintf(stderr, "rpcinfo: %s", nconf->nc_netid);
		netdir_perror("");
		hostname = UNKNOWN;
	} else {
		hostname = serv->h_hostservs->h_host;
	}
	if (!(uaddr = taddr2uaddr(nconf, who))) {
		fprintf(stderr, "rpcinfo: %s", nconf->nc_netid);
		netdir_perror("");
		uaddr = UNKNOWN;
	}
	printf("%s\t%s\n", uaddr, hostname);
	if (strcmp(hostname, UNKNOWN))
		netdir_free((char *)serv, ND_HOSTSERVLIST);
	if (strcmp(uaddr, UNKNOWN))
		free((char *)uaddr);
	return (FALSE);
}

static void
brdcst(argc, argv)
	int argc;
	char **argv;
{
	enum clnt_stat rpc_stat;
	u_long prognum, vers;

	if (argc != 2) {
		usage();
		exit(1);
	}
	prognum = getprognum(argv[0]);
	vers = getvers(argv[1]);
	rpc_stat = rpc_broadcast(prognum, vers, NULLPROC, xdr_void,
		(char *)NULL, xdr_void, (char *)NULL, reply_proc, NULL);
	if ((rpc_stat != RPC_SUCCESS) && (rpc_stat != RPC_TIMEDOUT)) {
		fprintf(stderr, "rpcinfo: broadcast failed: %s\n",
			clnt_sperrno(rpc_stat));
		exit(1);
	}
	exit(0);
}

static void
rpcbdump(netid, argc, argv)
	char *netid;
	int argc;
	char **argv;
{
	RPCBLIST *head = NULL;
	struct timeval minutetimeout;
	register CLIENT *client;
	struct rpcent *rpc;
	char *host;
	static char *tlist[3] = {
		"circuit_n", "circuit_v", "datagram_v"
	};

	if (argc > 1) {
		usage();
		exit(1);
	}
	if (argc == 1)
		host = argv[0];
	else
		host = _rpc_gethostname();
	if (netid == NULL) {
		int i;

		for (i = 0; i < 3; i++) {
			client = clnt_create(host, RPCBPROG,
					RPCBVERS, tlist[i]);
			if (client)
				break;
		}
	} else {
		struct netconfig *nconf;

		nconf = getnetconfigent(netid);
		client = clnt_tp_create(host, RPCBPROG, RPCBVERS, nconf);
		if (nconf)
			(void) freenetconfigent(nconf);
	}
	if (client == (CLIENT *)NULL) {
		clnt_pcreateerror("rpcinfo: can't contact rpcbind");
		exit(1);
	}
	minutetimeout.tv_sec = 60;
	minutetimeout.tv_usec = 0;
	if (CLNT_CALL(client, RPCBPROC_DUMP, xdr_void, NULL,
		xdr_rpcblist, &head, minutetimeout) != RPC_SUCCESS) {
		clnt_perror(client, "rpcinfo: can't contact rpcbind: ");
		exit(1);
	}
	if (head == NULL) {
		printf("No remote programs registered.\n");
	} else {
		printf(
	"   program version netid address	service owner\n");
		for (; head != NULL; head = head->rpcb_next) {
			printf("%10ld%5ld  ",
				head->rpcb_map.r_prog, head->rpcb_map.r_vers);
			printf("%6s  ", head->rpcb_map.r_netid);
			printf("%s", head->rpcb_map.r_addr);
			rpc = getrpcbynumber(head->rpcb_map.r_prog);
			if (rpc)
				printf("  %s", rpc->r_name);
			else
				printf(" - ");
			printf("  %s\n", head->rpcb_map.r_owner);
		}
	}
	return;
}

/*
 * Delete registeration for this (prog, vers, netid)
 */
static void
deletereg(netid, argc, argv)
	char *netid;
	int argc;
	char **argv;
{
	struct netconfig *nconf = NULL;

	if (argc != 2) {
		usage();
		exit(1);
	}
	if (netid) {
		nconf = getnetconfigent(netid);
		if (nconf == NULL) {
			fprintf(stderr, "rpcinfo: Illegal netid\n");
			exit(1);
		}
	}
	if ((rpcb_unset(getprognum(argv[0]), getvers(argv[1]), nconf)) == 0) {
		fprintf(stderr,
	"rpcinfo: Could not delete registration for prog %s version %s\n",
			argv[0], argv[1]);
		exit(1);
	}
}

/*
 * Create and return a handle for the given nconf.
 * Exit if cannot create handle.
 */
static CLIENT *
clnt_addr_create(address, nconf, prog, vers)
	char *address;
	struct netconfig *nconf;
	u_long prog;
	u_long vers;
{
	CLIENT *client;
	static struct netbuf *nbuf;
	static int fd = RPC_ANYFD;
	struct t_info tinfo;

	if (fd == RPC_ANYFD) {
		if ((fd = t_open(nconf->nc_device, O_RDWR, &tinfo)) == -1) {
			rpc_createerr.cf_stat = RPC_TLIERROR;
			rpc_createerr.cf_error.re_terrno = t_errno;
			clnt_pcreateerror("rpcinfo");
			exit(1);
		}
		/* Convert the uaddr to taddr */
		nbuf = uaddr2taddr(nconf, address);
		if (nbuf == NULL) {
			netdir_perror("rpcinfo");
			exit(1);
		}
	}
	client = clnt_tli_create(fd, nconf, nbuf, prog, vers, 0, 0);
	if (client == (CLIENT *)NULL) {
		clnt_pcreateerror("rpcinfo");
		exit(1);
	}
	return (client);
}

/*
 * If the version number is given, ping that (prog, vers); else try to find
 * the version numbers supported for that prog and ping all the versions.
 * Remote rpcbind is not contacted for this service. The requests are
 * sent directly to the services themselves.
 */
static void
addrping(address, netid, argc, argv)
	char *address;
	char *netid;
	int argc;
	char **argv;
{
	CLIENT *client;
	struct timeval to;
	enum clnt_stat rpc_stat;
	u_long prognum, versnum, minvers, maxvers;
	struct rpc_err rpcerr;
	int failure = 0;
	struct netconfig *nconf;
	int fd;

	if (argc < 1 || argc > 2 || (netid == NULL)) {
		usage();
		exit(1);
	}
	nconf = getnetconfigent(netid);
	if (nconf == (struct netconfig *)NULL) {
		fprintf(stderr, "rpcinfo: Could not find %s\n", netid);
		exit(1);
	}
	to.tv_sec = 10;
	to.tv_usec = 0;
	prognum = getprognum(argv[0]);
	if (argc == 1) {	/* Version number not known */
		/*
		 * A call to version 0 should fail with a program/version
		 * mismatch, and give us the range of versions supported.
		 */
		versnum = MIN_VERS;
	} else {
		versnum = getvers(argv[1]);
	}
	client = clnt_addr_create(address, nconf, prognum, versnum);
	rpc_stat = CLNT_CALL(client, NULLPROC, xdr_void, (char *)NULL,
				xdr_void, (char *)NULL, to);
	if (argc == 2) {
		/* Version number was known */
		if (pstatus(client, prognum, versnum) < 0)
			failure = 1;
		(void) CLNT_DESTROY(client);
		if (failure)
			exit(1);
		return;
	}
	/* Version number not known */
	(void) CLNT_CONTROL(client, CLSET_FD_NCLOSE, (char *)NULL);
	(void) CLNT_CONTROL(client, CLGET_FD, (char *)&fd);
	if (rpc_stat == RPC_PROGVERSMISMATCH) {
		clnt_geterr(client, &rpcerr);
		minvers = rpcerr.re_vers.low;
		maxvers = rpcerr.re_vers.high;
	} else if (rpc_stat == RPC_SUCCESS) {
		/*
		 * Oh dear, it DOES support version 0.
		 * Let's try version MAX_VERS.
		 */
		(void) CLNT_DESTROY(client);
		client = clnt_addr_create(address, nconf, prognum, MAX_VERS);
		rpc_stat = CLNT_CALL(client, NULLPROC, xdr_void,
				(char *)NULL, xdr_void, (char *)NULL, to);
		if (rpc_stat == RPC_PROGVERSMISMATCH) {
			clnt_geterr(client, &rpcerr);
			minvers = rpcerr.re_vers.low;
			maxvers = rpcerr.re_vers.high;
		} else if (rpc_stat == RPC_SUCCESS) {
			/*
			 * It also supports version MAX_VERS.
			 * Looks like we have a wise guy.
			 * OK, we give them information on all
			 * 4 billion versions they support...
			 */
			minvers = 0;
			maxvers = MAX_VERS;
		} else {
			(void) pstatus(client, prognum, MAX_VERS);
			exit(1);
		}
	} else {
		(void) pstatus(client, prognum, (u_long)0);
		exit(1);
	}
	(void) CLNT_DESTROY(client);
	for (versnum = minvers; versnum <= maxvers; versnum++) {
		client = clnt_addr_create(address, nconf, prognum, versnum);
		rpc_stat = CLNT_CALL(client, NULLPROC, xdr_void,
				(char *)NULL, xdr_void, (char *)NULL, to);
		if (pstatus(client, prognum, versnum) < 0)
				failure = 1;
		(void) CLNT_DESTROY(client);
	}
	(void) t_close(fd);
	if (failure)
		exit(1);
	return;
}

/*
 * If the version number is given, ping that (prog, vers); else try to find
 * the version numbers supported for that prog and ping all the versions.
 * Remote rpcbind is *contacted* for this service. The requests are
 * then sent directly to the services themselves.
 */
static void
progping(netid, argc, argv)
	char *netid;
	int argc;
	char **argv;
{
	CLIENT *client;
	struct timeval to;
	enum clnt_stat rpc_stat;
	u_long prognum, versnum, minvers, maxvers;
	struct rpc_err rpcerr;
	int failure = 0;
	struct netconfig *nconf;
	int fd;

	if (argc < 2 || argc > 3 || (netid == NULL)) {
		usage();
		exit(1);
	}
	prognum = getprognum(argv[1]);
	if (argc == 2) { /* Version number not known */
		/*
		 * A call to version 0 should fail with a program/version
		 * mismatch, and give us the range of versions supported.
		 */
		versnum = MIN_VERS;
	} else {
		versnum = getvers(argv[2]);
	}
	if (netid) {
		nconf = getnetconfigent(netid);
		if (nconf == (struct netconfig *)NULL) {
			fprintf(stderr, "rpcinfo: Could not find %s\n", netid);
			exit(1);
		}
		client = clnt_tp_create(argv[0], prognum, versnum, nconf);
		if (client == (CLIENT *)NULL) {
			clnt_pcreateerror("rpcinfo");
			exit(1);
		}
	}
	to.tv_sec = 10;
	to.tv_usec = 0;
	rpc_stat = CLNT_CALL(client, NULLPROC, xdr_void, (char *)NULL,
				xdr_void, (char *)NULL, to);
	if (argc == 3) {
		/* Version number was known */
		if (pstatus(client, prognum, versnum) < 0)
			failure = 1;
		(void) CLNT_DESTROY(client);
		if (failure)
			exit(1);
		return;
	}
	/* Version number not known */
	(void) CLNT_CONTROL(client, CLSET_FD_NCLOSE, (char *)NULL);
	(void) CLNT_CONTROL(client, CLGET_FD, (char *)&fd);
	if (rpc_stat == RPC_PROGVERSMISMATCH) {
		clnt_geterr(client, &rpcerr);
		minvers = rpcerr.re_vers.low;
		maxvers = rpcerr.re_vers.high;
	} else if (rpc_stat == RPC_SUCCESS) {
		/*
		 * Oh dear, it DOES support version 0.
		 * Let's try version MAX_VERS.
		 */
		(void) CLNT_DESTROY(client);
		client = clnt_tp_create(argv[0], prognum, MAX_VERS, nconf);
		rpc_stat = CLNT_CALL(client, NULLPROC, xdr_void,
				(char *)NULL, xdr_void, (char *)NULL, to);
		if (rpc_stat == RPC_PROGVERSMISMATCH) {
			clnt_geterr(client, &rpcerr);
			minvers = rpcerr.re_vers.low;
			maxvers = rpcerr.re_vers.high;
		} else if (rpc_stat == RPC_SUCCESS) {
			/*
			 * It also supports version MAX_VERS.
			 * Looks like we have a wise guy.
			 * OK, we give them information on all
			 * 4 billion versions they support...
			 */
			minvers = 0;
			maxvers = MAX_VERS;
		} else {
			(void) pstatus(client, prognum, MAX_VERS);
			exit(1);
		}
	} else {
		(void) pstatus(client, prognum, (u_long)0);
		exit(1);
	}
	(void) CLNT_DESTROY(client);
	for (versnum = minvers; versnum <= maxvers; versnum++) {
		client = clnt_addr_create(argv[0], nconf, prognum, versnum);
		rpc_stat = CLNT_CALL(client, NULLPROC, xdr_void,
				(char *)NULL, xdr_void, (char *)NULL, to);
		if (pstatus(client, prognum, versnum) < 0)
				failure = 1;
		(void) CLNT_DESTROY(client);
	}
	(void) t_close(fd);
	if (failure)
		exit(1);
	return;
}

static void
usage()
{
	fprintf(stderr, "Usage: rpcinfo [ host ]\n");
#ifdef PORTMAP
	fprintf(stderr, "       rpcinfo -p [ host ]\n");
#endif
	fprintf(stderr, "       rpcinfo -T netid host prognum [versnum]\n");
#ifdef PORTMAP
	fprintf(stderr,
"       rpcinfo [ -n portnum ] -u host prognum [ versnum ]\n");
	fprintf(stderr,
"       rpcinfo [ -n portnum ] -t host prognum [ versnum ]\n");
#endif
	fprintf(stderr,
"       rpcinfo -a serv_address -T netid prognum [ version ]\n");
	fprintf(stderr, "       rpcinfo -b prognum versnum\n");
	fprintf(stderr, "       rpcinfo -d [-T netid] prognum versnum\n");
}

static u_long
getprognum(arg)
	char *arg;
{
	char *strptr;
	register struct rpcent *rpc;
	register u_long prognum;

	if (isalpha(*arg)) {
		rpc = getrpcbyname(arg);
		if (rpc == NULL) {
			fprintf(stderr, "rpcinfo: %s is unknown service\n",
				arg);
			exit(1);
		}
		prognum = rpc->r_number;
	} else {
		prognum = strtol(arg, &strptr, 10);
		if (strptr == arg || *strptr != '\0') {
			fprintf(stderr,
		"rpcinfo: %s is illegal program number\n", arg);
			exit(1);
		}
	}
	return (prognum);
}

static u_long
getvers(arg)
	char *arg;
{
	char *strptr;
	register u_long vers;

	vers = (int) strtol(arg, &strptr, 10);
	if (strptr == arg || *strptr != '\0') {
		fprintf(stderr, "rpcinfo: %s is illegal version number\n",
			arg);
		exit(1);
	}
	return (vers);
}

/*
 * This routine should take a pointer to an "rpc_err" structure, rather than
 * a pointer to a CLIENT structure, but "clnt_perror" takes a pointer to
 * a CLIENT structure rather than a pointer to an "rpc_err" structure.
 * As such, we have to keep the CLIENT structure around in order to print
 * a good error message.
 */
static int
pstatus(client, prog, vers)
	register CLIENT *client;
	u_long prog;
	u_long vers;
{
	struct rpc_err rpcerr;

	clnt_geterr(client, &rpcerr);
	if (rpcerr.re_status != RPC_SUCCESS) {
		clnt_perror(client, "rpcinfo");
		printf("program %lu version %lu is not available\n",
			prog, vers);
		return (-1);
	} else {
		printf("program %lu version %lu ready and waiting\n",
			prog, vers);
		return (0);
	}
}
