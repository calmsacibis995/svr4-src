/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/mount/mount.c	1.18.8.1"

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
 * nfs mount
 */

#define	NFSCLIENT
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <varargs.h>
#include <unistd.h>
#include <sys/param.h>
#include <rpc/rpc.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/mount.h>
#include <sys/mntent.h>
#include <sys/mnttab.h>
#include <nfs/nfs.h>
#include <nfs/mount.h>
#include <rpcsvc/mount.h>
#include <netdir.h>
#include <netconfig.h>

#define MNTTYPE_NFS "nfs"

#define RET_OK		0
#define RET_RETRY	32
#define RET_ERR		33

#define BIGRETRY	10000

/* maximum length of RPC header for NFS messages */
#define NFS_RPC_HDR	432

extern int errno;
extern int _nderror;

int mount_nfs();
int get_knconf();
void pr_err();
void addtomnttab();
void usage();
struct netbuf *get_addr();

char *myname;
char typename[64];
struct t_info	tinfo;

int bg;
int retries = BIGRETRY;
u_short nfs_port = 0;

main(argc, argv)
	int argc;
	char **argv;
{
	struct mnttab mnt, *mntp;
	extern char *optarg;
	extern int optind;
	char optbuf[256];
	int ro = 0;
	int r, c;

	myname = strrchr(argv[0], '/');
	myname = myname ? myname+1 : argv[0];
	(void) sprintf(typename, "%s %s", MNTTYPE_NFS, myname);
	argv[0] = typename;

	mnt.mnt_mntopts = optbuf;
	(void) strcpy(optbuf, "rw");

	/*
	 * Set options
	 */
	while ((c = getopt(argc, argv, "ro:")) != EOF) {
		switch (c) {
		case 'r':
			ro++;
			break;
		case 'o':
			strcpy(mnt.mnt_mntopts, optarg);
			break;
		default:
			usage();
			exit (RET_ERR);
		}
	}
	if (argc - optind != 2) {
		usage();
		exit (RET_ERR);
	}

	mnt.mnt_special = argv[optind];
	mnt.mnt_mountp  = argv[optind+1];
	mntp = &mnt;

	if (geteuid() != 0) {
		pr_err("not super user\n");
		exit (RET_ERR);
	}

	r = mount_nfs(mntp, ro);
	if (r == RET_RETRY && retries)
		r = retry(mntp, ro);

	exit (r);
}

void
pr_err(fmt, va_alist)
	char *fmt;
	va_dcl
{
	va_list ap;

	va_start(ap);
	(void) fprintf(stderr, "%s: ", typename);
	(void) vfprintf(stderr, fmt, ap);
	(void) fflush(stderr);
	va_end(ap);
}

void
usage()
{
	(void) fprintf(stderr,
	    "Usage: nfs mount [-r] [-o opts] server:path dir\n");
	exit (RET_ERR);
}

int
mount_nfs(mntp, ro)
	struct mnttab *mntp;
	int ro;
{
	struct nfs_args nfs_args;
	char *p, *fsname, *fshost, *fspath;
	struct netconfig *nconf = NULL;
	int mntflags = 0;
	int r;

	memset(&nfs_args, 0, sizeof (struct nfs_args));

	mntp->mnt_fstype = MNTTYPE_NFS;

	/*
	 * split server:/dir into two strings: fshost & fspath
	 */
	fsname = (char *) strdup(mntp->mnt_special);
	if (fsname == NULL) {
		pr_err("no memory");
		return (RET_ERR);
	}
	p = (char *) strchr(fsname, ':');
	if (p == NULL) {
		pr_err("nfs file system; use host:path\n");
		return (RET_ERR);
	}
	*p++ = '\0';
	fshost = fsname;
	fspath = p;

	if (r = set_args(&mntflags, &nfs_args, fshost, mntp))
		return (r);

	if (ro) {
		mntflags |= MS_RDONLY;
		if (p = strstr(mntp->mnt_mntopts, "rw"))	/* "rw"->"ro" */
			if (*(p+2) == ',' || *(p+2) == '\0')
				*(p+1) = 'o';
	}

	if (r = get_fh(&nfs_args, fshost, fspath))
		return (r);

	if (getaddr_nfs(&nfs_args, fshost, &nconf) < 0)
		return (RET_ERR);

	if (nfs_args.flags & NFSMNT_SECURE) {
		if (make_secure(&nfs_args, fshost, &nconf) < 0)
			return (RET_ERR);
	}

	if (mount("", mntp->mnt_mountp, mntflags | MS_DATA, MNTTYPE_NFS,
		&nfs_args, sizeof (nfs_args)) < 0) {
		pr_err("mount: ");
		perror(mntp->mnt_mountp);
		return (RET_ERR);
	}
	addtomnttab(mntp);
	free(fsname);

	return (RET_OK);
}

#define MNTOPT_INTR	"intr"
#define MNTOPT_PORT	"port"
#define MNTOPT_SECURE	"secure"
#define MNTOPT_RSIZE	"rsize"
#define MNTOPT_WSIZE	"wsize"
#define MNTOPT_TIMEO	"timeo"
#define MNTOPT_RETRANS	"retrans"
#define MNTOPT_ACTIMEO	"actimeo"
#define MNTOPT_ACREGMIN	"acregmin"
#define MNTOPT_ACREGMAX	"acregmax"
#define MNTOPT_ACDIRMIN	"acdirmin"
#define MNTOPT_ACDIRMAX	"acdirmax"
#define MNTOPT_NOAC	"noac"
#define MNTOPT_BG	"bg"
#define MNTOPT_FG	"fg"
#define MNTOPT_RETRY	"retry"
#define MNTOPT_SUID	"suid"

char *optlist[] = {
#define OPT_RO		0
	MNTOPT_RO,
#define OPT_RW		1
	MNTOPT_RW,
#define OPT_QUOTA	2
	MNTOPT_QUOTA,
#define OPT_NOQUOTA	3
	MNTOPT_NOQUOTA,
#define OPT_SOFT	4
	MNTOPT_SOFT,
#define OPT_HARD	5
	MNTOPT_HARD,
#define OPT_NOSUID	6
	MNTOPT_NOSUID,
#define OPT_NOAUTO	7
	MNTOPT_NOAUTO,
#define OPT_GRPID	8
	MNTOPT_GRPID,
#define OPT_REMOUNT	9
	MNTOPT_REMOUNT,
#define OPT_NOSUB	10
	MNTOPT_NOSUB,
#define OPT_INTR	11
	MNTOPT_INTR,
#define OPT_PORT	12
	MNTOPT_PORT,
#define OPT_SECURE	13
	MNTOPT_SECURE,
#define OPT_RSIZE	14
	MNTOPT_RSIZE,
#define OPT_WSIZE	15
	MNTOPT_WSIZE,
#define OPT_TIMEO	16
	MNTOPT_TIMEO,
#define OPT_RETRANS	17
	MNTOPT_RETRANS,
#define OPT_ACTIMEO	18
	MNTOPT_ACTIMEO,
#define OPT_ACREGMIN	19
	MNTOPT_ACREGMIN,
#define OPT_ACREGMAX	20
	MNTOPT_ACREGMAX,
#define OPT_ACDIRMIN	21
	MNTOPT_ACDIRMIN,
#define OPT_ACDIRMAX	22
	MNTOPT_ACDIRMAX,
#define OPT_BG		23
	MNTOPT_BG,
#define OPT_FG		24
	MNTOPT_FG,
#define OPT_RETRY	25
	MNTOPT_RETRY,
#define OPT_NOAC	26
	MNTOPT_NOAC,
#define	OPT_SUID	27
	MNTOPT_SUID,
	NULL
};

#define bad(val) (val == NULL || !isdigit(*val))

int
set_args (mntflags, args, fshost, mnt)
	int *mntflags;
	struct nfs_args *args;
	char *fshost;
	struct mnttab *mnt;
{
	char *saveopt, *optstr, *opts, *val;

	args->flags = 0;
	args->flags |= NFSMNT_HOSTNAME;
	args->hostname = fshost;
	optstr = opts = strdup(mnt->mnt_mntopts);
	if (opts == NULL) {
		pr_err("no memory");
		return (RET_ERR);
	}
	while (*opts) {
		saveopt = opts;
		switch (getsubopt(&opts, optlist, &val)) {
		case OPT_RO:
			*mntflags |= MS_RDONLY;
			break;
		case OPT_RW:
			*mntflags &= ~(MS_RDONLY);
			break;
		case OPT_QUOTA:
		case OPT_NOQUOTA:
			break;
		case OPT_SOFT:
			args->flags |= NFSMNT_SOFT;
			break;
		case OPT_HARD:
			args->flags &= ~(NFSMNT_SOFT);
			break;
		case OPT_SUID:
			break;
		case OPT_NOSUID:
			*mntflags |= MS_NOSUID;
			break;
		case OPT_NOAUTO:
			break;
		case OPT_GRPID:
			args->flags |= NFSMNT_GRPID;
			break;
		case OPT_REMOUNT:
			break;
		case OPT_INTR:
			args->flags |= NFSMNT_INT;
			break;
		case OPT_NOAC:
			args->flags |= NFSMNT_NOAC;
			break;
		case OPT_PORT:
			if (bad(val))
				goto badopt;
			nfs_port = htons(atoi(val));
			break;

		case OPT_SECURE:
			args->flags |= NFSMNT_SECURE;
			break;

		case OPT_RSIZE:
			args->flags |= NFSMNT_RSIZE;
			if (bad(val))
				goto badopt;
			args->rsize = atoi(val);
			break;
		case OPT_WSIZE:
			args->flags |= NFSMNT_WSIZE;
			if (bad(val))
				goto badopt;
			args->wsize = atoi(val);
			break;
		case OPT_TIMEO:
			args->flags |= NFSMNT_TIMEO;
			if (bad(val))
				goto badopt;
			args->timeo = atoi(val);
			break;
		case OPT_RETRANS:
			args->flags |= NFSMNT_RETRANS;
			if (bad(val))
				goto badopt;
			args->retrans = atoi(val);
			break;
		case OPT_ACTIMEO:
			args->flags |= NFSMNT_ACDIRMAX;
			args->flags |= NFSMNT_ACREGMAX;
			if (bad(val))
				goto badopt;
			args->acdirmax = args->acregmax = atoi(val);
			break;
		case OPT_ACREGMIN:
			args->flags |= NFSMNT_ACREGMIN;
			if (bad(val))
				goto badopt;
			args->acregmin = atoi(val);
			break;
		case OPT_ACREGMAX:
			args->flags |= NFSMNT_ACREGMAX;
			if (bad(val))
				goto badopt;
			args->acregmax = atoi(val);
			break;
		case OPT_ACDIRMIN:
			args->flags |= NFSMNT_ACDIRMIN;
			if (bad(val))
				goto badopt;
			args->acdirmin = atoi(val);
			break;
		case OPT_ACDIRMAX:
			args->flags |= NFSMNT_ACDIRMAX;
			if (bad(val))
				goto badopt;
			args->acdirmax = atoi(val);
			break;
		case OPT_BG:
			bg++;
			break;
		case OPT_FG:
			bg = 0;
			break;
		case OPT_RETRY:
			if (bad(val))
				goto badopt;
			retries = atoi(val);
			break;
		default:
			goto badopt;
		}
	}
	free(optstr);
	return (RET_OK);

badopt:
	free(optstr);
	pr_err("invalid option: \"%s\"\n", saveopt);
	return (RET_ERR);
}

#include <netinet/in.h>

int
make_secure(args, hostname, nconfp)
	struct nfs_args *args;
	char *hostname;
	struct netconfig **nconfp;
{
	static char netname[MAXNETNAMELEN+1];

	args->flags |= NFSMNT_SECURE;

	/*
	 * XXX: need to support other netnames
	 * outside domain and not always just use
	 * the default conversion.
	 */
	if (!host2netname(netname, hostname, NULL)) {
		pr_err("host2netname: %s: unknown",
			hostname);
		return(-1);
	}
	args->netname = netname;

	/*
	 * Get the network address for the time service on
	 * the server.  If an RPC based time service is
	 * not available then try the old UDP time service
	 * if it's a UDP transport.
	 */
	args->syncaddr = get_addr(hostname, RPCBPROG, RPCBVERS, 0, nconfp);
	if (args->syncaddr != NULL) {
		args->flags |= NFSMNT_RPCTIMESYNC;
	} else {
		struct nd_hostserv hs;
		struct nd_addrlist *retaddrs;

		hs.h_host = hostname;
		hs.h_serv = "rpcbind";
		if (netdir_getbyname(&nconfp, &hs, &retaddrs) != ND_OK) {
			goto err;
		}
		args->syncaddr = retaddrs->n_addrs;
		((struct sockaddr_in *) args->syncaddr->buf)->sin_port
			= IPPORT_TIMESERVER;
	}
	return (0);

err:
	pr_err("%s: secure: no time service\n", hostname);
	return (-1);
}

/*
 * Get the network address on "hostname" for program "prog"
 * with version "vers".  If the port number is specified (non zero)
 * then try for a UDP transport and set the port number of the
 * resulting IP address.
 * Finally, ping the null procedure of that service.
 *
 * If the address of a netconfig pointer was passed then
 * if it's not null use it as the netconfig otherwise
 * assign the address of the netconfig that was used to
 * establish contact with the service.
 */
struct netbuf *
get_addr(hostname, prog, vers, port, nconfp)
	char *hostname;
	int prog, vers, port;
	struct netconfig **nconfp;
{
	struct netbuf *nb = NULL;
	struct t_bind *tbind = NULL;
	struct netconfig *nconf;
	struct netconfig *getnetconfig();
	struct netconfig *getnetconfigent();
	static NCONF_HANDLE *nc = NULL;
	enum clnt_stat cs;
	CLIENT *cl = NULL;
	struct timeval tv;
	int fd = -1;

	if (nconfp && *nconfp) {
		nconf = *nconfp;
	} else {
		nc = setnetconfig();
		if (nc == NULL)
			goto done;

retry:
		/*
		 * If the port number is specified then UDP is needed.
		 * Otherwise any connectionless transport will do.
		 */
		while (nconf = getnetconfig(nc)) {
			if ((nconf->nc_flag & NC_VISIBLE) &&
			     nconf->nc_semantics == NC_TPI_CLTS) {
				if (port == 0)
					break;
				if ((strcmp(nconf->nc_protofmly, NC_INET) == 0) &&
				    (strcmp(nconf->nc_proto, NC_UDP) == 0))
					break;
			} 
		}
		if (nconf == NULL) {
			/* If we got this far, the host or svc doesn't exist */
			pr_err("get_addr: (host %s, prog %d, vers %d) not found on any transport\n",
				hostname, prog, vers);
			exit (RET_ERR);
		}
	}

	if ((fd = t_open(nconf->nc_device, O_RDWR, &tinfo)) == -1) {
		if (nc)
			goto retry;
		else
			goto done;
	}

	tbind = (struct t_bind *) t_alloc(fd, T_BIND, T_ADDR);
	if (tbind == NULL) 
		if (nc) {
			(void) t_close(fd);
			goto retry;
		} else
			goto done;

	if (rpcb_getaddr(prog, vers, nconf, &tbind->addr, hostname) == 0) {
		if (nc) {
			(void) t_free(tbind, T_BIND);
			tbind = NULL;
			(void) t_close(fd);
			goto retry;
		} else
			goto done;
	}

	if (port)
		((struct sockaddr_in *) tbind->addr.buf)->sin_port = port;

	cl = clnt_tli_create(fd, nconf, &tbind->addr, prog, vers, 0, 0);
	if (cl == NULL) {
		if (nc) {
			(void) t_free(tbind, T_BIND);
			tbind = NULL;
			(void) t_close(fd);
			goto retry;
		} else
			goto done;
	}
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	cs = clnt_call(cl, NULLPROC, xdr_void, 0, xdr_void, 0, tv);
	if (cs != RPC_SUCCESS) {
		if (nc) {
			clnt_destroy(cl);
			cl = NULL;
			(void) t_free(tbind, T_BIND);
			tbind = NULL;
			(void) t_close(fd);
			goto retry;
		} else
			goto done;
	}

	/*
	 * Make a copy of the netbuf to return
	 */
	nb = (struct netbuf *)malloc(sizeof(struct netbuf));
	if (nb == NULL) {
		pr_err("no memory\n");
		goto done;
	}
	*nb = tbind->addr;
	nb->buf = (char *)malloc(nb->maxlen);
	if (nb->buf == NULL) {
		pr_err("no memory\n");
		free(nb);
		nb = NULL;
		goto done;
	}
	(void) memcpy(nb->buf, tbind->addr.buf, nb->len);
	if (nconfp && *nconfp == NULL) {
		*nconfp = getnetconfigent(nconf->nc_netid);
		if (*nconfp == NULL) {
			pr_err("no memory\n");
			free(nb);
			nb = NULL;
			goto done;
		}
	}

done:
	if (cl)
		clnt_destroy(cl);
	if (tbind)
		t_free((char *) tbind, T_BIND);
	if (fd >= 0)
		(void) t_close(fd);
	if (nc)
		endnetconfig(nc);
	return (nb);
}

/*
 * get fhandle of remote path from server's mountd
 */
get_fh(args, fshost, fspath)
	struct nfs_args *args;
	char *fshost, *fspath;
{
	static struct fhstatus fhs;
	struct timeval timeout = { 25, 0};
	CLIENT *cl;
	enum clnt_stat rpc_stat;


	cl = clnt_create(fshost, MOUNTPROG, MOUNTVERS, "datagram_v");
	if (cl == NULL) {
		pr_err("%s:%s: server not responding%s\n",
			fshost, fspath, clnt_spcreateerror(""));
		/* certain errors are fatal. Give up on them */
		if (_nderror == ND_NOHOST || _nderror == ND_NOSERV)
			exit (RET_ERR);
		return (RET_RETRY);
	}
	if (bindresvport(cl) < 0) {
		pr_err("Couldn't bind to reserved port\n");
		clnt_destroy(cl);
		return (RET_RETRY);
	}

	cl->cl_auth = authsys_create_default();
	rpc_stat = clnt_call(cl, MOUNTPROC_MNT, xdr_path, &fspath,
	    xdr_fhstatus, &fhs, timeout);
	if (rpc_stat != RPC_SUCCESS) {
		pr_err("%s:%s server not responding%s\n",
			fshost, fspath, clnt_sperror(cl, ""));
		clnt_destroy(cl);
		return (RET_RETRY);
	}
	clnt_destroy(cl);

	if (errno = fhs.fhs_status) {
		if (errno == EACCES) {
			pr_err("access denied for %s:%s\n", fshost, fspath);
		} else {
			pr_err("%s:%s", fshost, fspath);
			perror("");
		}
		return (RET_ERR);
	}
	args->fh = (caddr_t) &fhs.fhs_fh;
	return (RET_OK);
}

/*
 * Fill in the address for the server's NFS service and
 * fill in a knetconfig structure for the transport that
 * the service is available on.
 */
int
getaddr_nfs(args, fshost, nconfp)
	struct nfs_args *args;
	struct netconfig **nconfp;
{
	struct stat sb;
	struct netconfig *nconf;
	static struct knetconfig knconf;

	args->addr = get_addr(fshost, NFS_PROGRAM, NFS_VERSION,
					nfs_port, nconfp);
	if (args->addr == NULL) {
		pr_err("%s: NFS service not responding\n", fshost);
		return (-1);
	}
	nconf = *nconfp;

	if (stat(nconf->nc_device, &sb) < 0) {
		pr_err("get_knconf: couldn't stat:");
		perror(nconf->nc_device);
		return (-1);
	}

	knconf.knc_semantics = nconf->nc_semantics;
	knconf.knc_protofmly = nconf->nc_protofmly;
	knconf.knc_proto = nconf->nc_proto;
	knconf.knc_rdev = sb.st_rdev;

	/* make sure we don't overload the transport */
	if (tinfo.tsdu > 0 && tinfo.tsdu < NFS_MAXDATA + NFS_RPC_HDR) {
		args->flags |= (NFSMNT_RSIZE | NFSMNT_WSIZE);
		if (args->rsize == 0 || args->rsize > tinfo.tsdu - NFS_RPC_HDR)
			args->rsize = tinfo.tsdu - NFS_RPC_HDR;
		if (args->wsize == 0 || args->wsize > tinfo.tsdu - NFS_RPC_HDR)
			args->wsize = tinfo.tsdu - NFS_RPC_HDR;
	}

	args->flags |= NFSMNT_KNCONF;
	args->knconf = &knconf;
	return (0);
}

#define TIME_MAX 16
#define MNTINFO_DEV "dev"

/*
 * add a new entry to the /etc/mnttab file
 */
void
addtomnttab(mntp)
	struct mnttab *mntp;
{
	FILE *fd;
	char tbuf[TIME_MAX];

	fd = fopen(MNTTAB, "a");
	if (fd == NULL) {
		pr_err("%s: cannot open mnttab\n", myname);
		exit (RET_ERR);
	}

	if (lockf(fileno(fd), F_LOCK, 0L) < 0) {
		pr_err("%s: cannot lock mnttab", myname);
		perror(myname);
		(void) fclose(fd);
		exit (RET_ERR);
	}
	(void) fseek(fd, 0L, 2); /* guarantee at EOF */

	(void) sprintf(tbuf, "%ld", time(0L));
	mntp->mnt_time = tbuf;

	putmntent(fd, mntp);
	(void) fclose(fd);
}

int
retry(mntp, ro)
	struct mnttab *mntp;
	int ro;
{
	int delay = 5;
	int count = retries;
	int r;

	if (bg) {
		if (fork() > 0)
			return (RET_OK);
		pr_err("backgrounding: %s\n", mntp->mnt_mountp);
	}
	pr_err("retrying: %s\n", mntp->mnt_mountp);

	while (count--) {
		if ((r = mount_nfs(mntp, ro)) == RET_OK) {
			pr_err("%s: mounted OK\n", mntp->mnt_mountp);
			return (RET_OK);
		}
		if (r != RET_RETRY)
			break;

		sleep(delay);
		delay *= 2;
		if (delay > 120)
			delay = 120;
	}
	pr_err("giving up on: %s\n", mntp->mnt_mountp);
	return (RET_ERR);
}
