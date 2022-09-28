/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/nfsd/nfsd.c	1.10.3.1"


/*
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
 
*/
/* NFS server */

#include	<sys/param.h>
#include	<sys/types.h>
#include	<syslog.h>
#include	<tiuser.h>
#include	<rpc/rpc.h>
#include	<sys/errno.h>
#include	<sys/resource.h>
#include	<sys/time.h>
#include	<sys/file.h>
#include	<nfs/nfs.h>
#include	<stdio.h>
#include	<signal.h>
#include	<netconfig.h>
#include	<netdir.h>

extern int	errno, t_errno;
void	usage();

char	*MyName;

#define	NETSELDECL(x)	char	*x
#define	NETSELEQ(x, y)	(!strcmp((x), (y)))

NETSELDECL(defaultproto) = NC_UDP;

void 
catch()
{
}

main(ac, av)
char	**av;
{
	char *dir = "/";
	int allflag = 0, nservers = 1;
	int pid, t;
	int i;
	struct rlimit rl;
	extern int optind;
	extern char *optarg;
	extern void do_one();
	char *provider = (char *) NULL;
	NETSELDECL(proto) = defaultproto;

	MyName = *av;

	while ((i = getopt(ac, av, "ap:t:")) != EOF)
		switch (i) {
			case 'a':
				allflag = 1;
				break;
			case 'p':
				proto = optarg;
				break;

			case 't':
				provider = optarg;
				break;

			case '?':
				usage();
				/* NOTREACHED */
		}
	if (optind == ac - 1)
		nservers = atoi(av[optind]);
	else
		if (optind > ac - 1)
			usage();

	/*
	 * Set current and root dir to server root
	 */
	if (chroot(dir) < 0) {
		fprintf(stderr, "%s:  ", MyName);
		perror(dir);
		exit(1);
	}
	if (chdir(dir) < 0) {
		fprintf(stderr, "%s:  ", MyName);
		perror(dir);
		exit(1);
	}

#ifndef DEBUG
	/*
	 * Background 
	 */
        pid = fork();
	if (pid < 0) {
		perror("nfsd: fork");
		exit(1);
	}
	if (pid != 0)
		exit(0);

	{
		int i;
		struct rlimit rl;
		/*
		 * Close existing file descriptors, open "/dev/null" as
		 * standard input, output, and error, and detach from
		 * controlling terminal.
		 */
		getrlimit(RLIMIT_NOFILE, &rl);
		for (i = 0; i < rl.rlim_max; i++)
			(void) close(i);
		(void) open("/dev/null", O_RDONLY);
		(void) open("/dev/null", O_WRONLY);
		(void) dup(1);
		(void) setsid();
	}
#endif
	openlog(MyName, LOG_PID, LOG_DAEMON);

	if (allflag)
		do_all(nservers);
	else
		do_one(nservers, provider, proto);
	/* NOTREACHED */
}

static void
do_one(nservers, provider, proto)
	int nservers;
	char *provider;
	NETSELDECL(proto);
{
	register int sock;
	struct netbuf *retaddr;
	struct netconfig *retnconf;

	if (provider)
		sock = bind_to_provider(provider, &retaddr, &retnconf);
	else
		sock = bind_to_proto(proto, &retaddr, &retnconf);
	if (sock == -1) {
		exit(1);
	}

	rpcb_unset(NFS_PROGRAM, NFS_VERSION, retnconf);
	rpcb_set(NFS_PROGRAM, NFS_VERSION, retnconf, retaddr);

	while (--nservers) {
		if (!fork()) {
			break;
		}
	}
	signal(SIGTERM, catch);
	nfssvc(sock);
	/* Should never come here */
	exit(errno);
	/* NOTREACHED */
}

static int
do_all(nservers)
{
	struct netconfig *nconf;
	NCONF_HANDLE *nc;

	if ((nc = setnetconfig()) == (NCONF_HANDLE *) NULL) {
		syslog(LOG_ERR, "setnetconfig failed: %m");
		return (-1);
	}
	while (nconf = getnetconfig(nc)) {
		if ((nconf->nc_flag & NC_VISIBLE) &&
		    nconf->nc_semantics == NC_TPI_CLTS) {
			switch (fork()) {
				case -1:
					syslog(LOG_ERR, "can't fork: %m");
					break;
				/*
				 * 	The child goes on to register itself
				 *	on this transport and begin NFS service
				 */
				case 0:
					do_one(nservers, nconf->nc_device,
					    nconf->nc_proto);
					break;
				/*
				 * 	The parent returns to scan the next
				 * 	netconfig entry.
				 */
				default:
					continue;
			}
		}
	}
	endnetconfig(nc);
	return (0);
}

static int
bind_to_provider(provider, addr, retnconf)
char	*provider;
struct netbuf	**addr;
struct netconfig	**retnconf;
{
	struct netconfig	*nconf;
	NCONF_HANDLE 		*nc;


	if ((nc = setnetconfig()) == (NCONF_HANDLE *) NULL) {
		syslog(LOG_ERR, "setnetconfig failed: %m");
		return (-1);
	}
	while (nconf = getnetconfig(nc)) {
		if ((nconf->nc_semantics == NC_TPI_CLTS) && 
		     !strcmp(nconf->nc_device, provider)) {
			*retnconf = nconf;
			return (bindit(nconf, addr));
		}
	}
	endnetconfig(nc);

	syslog(LOG_ERR, "couldn't find NC_TPI_CLTS netconfig entry for provider %s",
	    provider);
	return (-1);
}

static int
bind_to_proto(proto, addr, retnconf)
NETSELDECL(proto);
struct netbuf	**addr;
struct netconfig	**retnconf;
{
	struct netconfig	*nconf;
	NCONF_HANDLE 		*nc = NULL;

	if ((nc = setnetconfig()) == (NCONF_HANDLE *) NULL) {
		syslog(LOG_ERR, "setnetconfig failed: %m");
		return (-1);
	}
	while (nconf = getnetconfig(nc)) {
		if ((nconf->nc_semantics == NC_TPI_CLTS) && 
		    NETSELEQ(nconf->nc_proto, proto)) {
			*retnconf = nconf;
			return (bindit(nconf, addr));
		}
	}
	endnetconfig(nc);

	syslog(LOG_ERR, "couldn't find NC_TPI_CLTS netconfig entry for protocol %s",
	    proto);
	return (-1);
}

static int
bindit(nconf, addr)
register struct netconfig *nconf;
struct netbuf	**addr;
{
	int fd;
	struct t_bind *tb = (struct t_bind *) NULL, *ntb;
	struct nd_hostserv hs;
	struct nd_addrlist *addrlist;

	if ((nconf == (struct netconfig *) NULL) || 
	   (nconf->nc_device == (char *) NULL)) {
		syslog(LOG_ERR, "bindit:  no netconfig device");
		return (-1);
	}

	if ((fd = t_open(nconf->nc_device, O_RDWR, (struct t_info *) NULL))
	   == -1) {
		syslog(LOG_ERR, "t_open %s failed:  t_errno %d, %m",
		    nconf->nc_device, t_errno);
		return (-1);
	}

	hs.h_host = HOST_SELF;
	hs.h_serv = "nfsd";
	if (netdir_getbyname(nconf, &hs, &addrlist) < 0) {
		syslog(LOG_ERR, "netdir_getbyname (transport %s, host/serv %s/%s), %m",
			nconf->nc_netid, hs.h_host, hs.h_serv);
		(void) t_close(fd);
		return (-1);
	}

	if ((tb = (struct t_bind *) t_alloc(fd, T_BIND, T_ALL)) ==
	   (struct t_bind *) NULL) {
		syslog(LOG_ERR, "t_alloc failed:  t_errno %d, %m", t_errno);
		(void) t_close(fd);
		return (-1);
	}

	if ((ntb = (struct t_bind *) t_alloc(fd, T_BIND, T_ALL)) ==
	   (struct t_bind *) NULL) {
		syslog(LOG_ERR, "t_alloc failed:  t_errno %d, %m", t_errno);
		if (tb != (struct t_bind *) NULL)
			(void) t_free((char *) tb, T_BIND);
		(void) t_close(fd);
		return (-1);
	}

	tb->addr = *(addrlist->n_addrs);	/* structure copy */

	if (t_bind(fd, tb, ntb) == -1) {
		syslog(LOG_ERR, "t_bind failed:  t_errno %d, %m", t_errno);
		(void) t_free((char *) ntb, T_BIND);
		if (tb != (struct t_bind *) NULL)
			(void) t_free(( char *) tb, T_BIND);
		(void) t_close(fd);
		return (-1);
	}

	/* make sure we bound to the right address */
	if (tb->addr.len != ntb->addr.len ||
	    memcmp(tb->addr.buf, ntb->addr.buf, tb->addr.len)) {
		syslog(LOG_ERR, "t_bind to wrong address, %m");
		(void) t_free((char *) ntb, T_BIND);
		(void) t_free((char *) tb, T_BIND);
		(void) t_close(fd);
		return (-1);
	}
	*addr = &ntb->addr;
	return (fd);
}

void
usage()
{
	fprintf(stderr, "usage:  %s [ -a ] [ -p protocol ] [ -t transport ] [ nservers ]\n", MyName);
	fprintf(stderr, "\twhere -a causes <nservers> to be started on each appropriate transport, \n");
	fprintf(stderr, "\tprotocol is a protocol identifier\n");
	fprintf(stderr, "\tand transport is a transport provider name (i.e. device)\n");
	exit(1);
}
