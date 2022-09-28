/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nfs.cmds:nfs/automount/auto_main.c	1.6.2.1"

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
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <errno.h>
#include <rpc/rpc.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "nfs_prot.h"
#include <netinet/in.h>
#include <sys/mnttab.h>
#include <sys/mntent.h>
#include <sys/mount.h>
#include <netdb.h>
#include <sys/signal.h>
#include <sys/file.h>
#ifdef YP
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
#endif
#include <nfs/nfs_clnt.h>
#include <netconfig.h>

#define NFSCLIENT
typedef nfs_fh fhandle_t;
#include <nfs/mount.h>

#include "automount.h"

extern errno;

void catch();
struct netconfig *loopback_trans();

#define	MAXDIRS	10

#define	MASTER_MAPNAME	"auto.master"

int maxwait = 60;
int mount_timeout = 30;
int max_link_time = 5*60;
dev_t tmpdev;
int verbose;

main(argc, argv)
	int argc;
	char *argv[];
{
	SVCXPRT *xprt;
	extern void nfs_program_2();
	extern void read_mnttab();
	struct netconfig *nconf;
	struct nfs_args args;
	struct autodir *dir, *dir_next;
	int pid;
	int bad;
	int master_yp = 1;
	char *master_file;
	struct stat sb;
	struct knetconfig knconf;
	extern int trace;
	char pidbuf[64];
	struct stat stbuf;
	int timeo;


	if (geteuid() != 0) {
		fprintf(stderr, "Must be root to use automount\n");
		exit(1);
	}

	argc--;
	argv++;

	openlog("automount", LOG_PID | LOG_NOWAIT | LOG_CONS, LOG_DAEMON);
	(void) umask(0);
	(void) setbuf(stdout, (char *)NULL);
	(void) gethostname(self, sizeof self);
#ifdef YP
	(void) getdomainname(mydomain, sizeof mydomain);
	if (bad = yp_bind(mydomain))
		syslog(LOG_ERR, "YP bind failed: %s", yperr_string(bad));
#endif YP

	(void) strcpy(tmpdir, "/tmp_mnt");
	master_file = NULL;

	while (argc && argv[0][0] == '-') switch (argv[0][1]) {
	case 'n':
		nomounts++;
		argc--;
		argv++;
		break;
	case 'm':
		master_yp = 0;
		argc--;
		argv++;
		break;
	case 'f':
		master_file = argv[1];
		argc -= 2;
		argv += 2;
		break;
	case 'M':
		(void) strcpy(tmpdir, argv[1]);
		argc -= 2;
		argv += 2;
		break;
	case 't':	/* timeout */
		if (argc < 2 || (timeo = atoi(argv[1])) <= 0) {
			(void) fprintf(stderr, "Bad timeout value\n");
			usage();
		}
		switch(argv[0][2]) {
		case 'm':
			mount_timeout = timeo;
			break;
		case 'l':
			max_link_time = timeo;
			break;
		case 'w':
			maxwait = timeo;
			break;
		default:
			(void) fprintf(stderr, "automount: bad timeout switch\n");
			usage();
		}
		argc -= 2;
		argv += 2;
		break;

	case 'T':
		trace++;
		argc--;
		argv++;
		break;

	case 'D':
		if (argv[0][2])
			(void) putenv(&argv[0][2]);
		else {
			(void) putenv(argv[1]);
			argc--;
			argv++;
		}
		argc--;
		argv++;
		break;

	case 'v':
		verbose++;
		argc--;
		argv++;
		break;

	default:
		usage();
	}

	if (argc == 0 && master_yp == 0 && master_file == NULL) {
		syslog(LOG_ERR, "no mount maps specified");
		usage();
	}

	read_mnttab();
	/*
	 * Get mountpoints and maps off the command line
	 */
	while (argc >= 2) {
		if (argc >= 3 && argv[2][0] == '-') {
			dirinit(argv[0], argv[1], argv[2]+1, 0);
			argc -= 3;
			argv += 3;
		} else {
			dirinit(argv[0], argv[1], "rw", 0);
			argc -= 2;
			argv += 2;
		}
	}
	if (argc)
		usage();

#ifdef notdef
	if (getenv("ARCH") == NULL) {
		char buf[16], str[32];
		int len;
		FILE *f;

		f = popen("arch", "r");
		(void) fgets(buf, 16, f);
		(void) pclose(f);
		if (len = strlen(buf))
			buf[len - 1] = '\0';
		(void) sprintf(str, "ARCH=%s", buf);
		(void) putenv(str);
	}
#endif
	
	if (master_file) {
		(void) loadmaster_file(master_file);
	}
#ifdef YP
	if (master_yp) {
		(void) loadmaster_yp(MASTER_MAPNAME);
	}
#endif YP

	/*
	 * Remove -null map entries
	 */
	for (dir = HEAD(struct autodir, dir_q); dir; dir = dir_next) {
	    	dir_next = NEXT(struct autodir, dir);
		if (strcmp(dir->dir_map, "-null") == 0) {
			REMQUE(dir_q, dir);
		}
	}
	if (HEAD(struct autodir, dir_q) == NULL)   /* any maps ? */
		exit(1);

	nconf = loopback_trans();
	if (nconf == (struct netconfig *) NULL) {
		syslog(LOG_ERR, "no tpi_clts loopback transport available");
		exit(1);
	}
	xprt = svc_tli_create(RPC_ANYFD, nconf, (struct t_bind *) NULL, 0, 0);
	if (xprt == (SVCXPRT *) NULL) {
		syslog(LOG_ERR, "Cannot create server handle");
		exit(1);
	}
	if (!svc_reg(xprt, NFS_PROGRAM, NFS_VERSION, nfs_program_2,
		(struct netconfig *)0)) {
		syslog(LOG_ERR, "Could not register service");
		exit(1);
	}
	if (mkdir_r(tmpdir) < 0) {
		syslog(LOG_ERR, "couldn't create %s: %m", tmpdir);
		exit(1);
	}
	if (stat(tmpdir, &stbuf) < 0) {
		syslog(LOG_ERR, "couldn't stat %s: %m", tmpdir);
		exit(1);
	}
	tmpdev = stbuf.st_dev;

	/*
	 *  Fork the automount daemon here
	 */
	switch (pid = fork()) {
	case -1:
		syslog(LOG_ERR, "Cannot fork: %m");
		exit(1);
	case 0:
		/* child */
		(void) setsid();
		signal(SIGTERM, catch);
		signal(SIGHUP, read_mnttab);
		signal(SIGCHLD, SIG_IGN); /* no zombies */
		auto_run();
		syslog(LOG_ERR, "svc_run returned");
		exit(1);
	}

	/* parent */
	args.flags = NFSMNT_INT + NFSMNT_TIMEO +
		     NFSMNT_HOSTNAME + NFSMNT_RETRANS;
	args.addr = &xprt->xp_ltaddr;

	if (stat(nconf->nc_device, &sb) < 0) {
		syslog(LOG_ERR, "Couldn't stat %s", nconf->nc_device);
		exit(1);
	}
	knconf.knc_semantics = nconf->nc_semantics;
	knconf.knc_protofmly = nconf->nc_protofmly;
	knconf.knc_proto = nconf->nc_proto;
	knconf.knc_rdev = sb.st_rdev;
	args.flags |= NFSMNT_KNCONF;
	args.knconf = &knconf;

	args.timeo = (mount_timeout + 5) * 10;
	args.retrans = 5;
	bad = 1;
	/*
	 * Mount the daemon at its mount points.
	 * Start at the end of the list because that's
	 * the first on the command line.
	 */
	for (dir = TAIL(struct autodir, dir_q); dir;
	    dir = PREV(struct autodir, dir)) {
		(void) sprintf(pidbuf, "(pid%d@%s)", pid, dir->dir_name);
		if (strlen(pidbuf) >= HOSTNAMESZ-1)
			(void) strcpy(&pidbuf[HOSTNAMESZ-3], "..");
		args.hostname = pidbuf;
		args.fh = (caddr_t)&dir->dir_vnode.vn_fh; 

		if (mount("", dir->dir_name, MS_RDONLY | MS_DATA, MNTTYPE_NFS, 
				&args, sizeof(args)) < 0) {
			syslog(LOG_ERR, "Can't mount %s: %m", dir->dir_name);
			bad++;
		} else {
#ifdef AUTO_MNTTAB
			domnttab(pid, dir->dir_name);
#endif AUTO_MNTTAB
			bad = 0;
		}
	}
	if (bad)
		(void) kill(pid, SIGKILL);
	exit(bad);
	/*NOTREACHED*/
}

/*
 * Get a netconfig entry for loopback transport
 */
struct netconfig *
loopback_trans()
{
	struct netconfig *nconf;
	struct netconfig *getnetconfig();
	struct netconfig *getnetconfigent();
	NCONF_HANDLE *nc;

#ifdef notdef
	nc = setnetconfig();
	if (nc == NULL)
		return (NULL);

	while (nconf = getnetconfig(nc)) {
		if (nconf->nc_flag & NC_VISIBLE &&
		    nconf->nc_semantics == NC_TPI_CLTS &&
		    strcmp(nconf->nc_protofmly, NC_LOOPBACK) == 0) {
			nconf = getnetconfigent(nconf->nc_netid);
			break;
		}
	}

	endnetconfig(nc);
	return(nconf);
#else
	/* XXX
	 * For some reason a loopback mount will not work
	 * with "ticlts". It does work with "udp" so
	 * until the "ticlts" problem is sorted out
	 * stick with udp loopback.
	 */
	return (getnetconfigent("udp"));
#endif
}

#ifdef AUTO_MNTTAB
#define TIME_MAX 16

domnttab(pid, mntpoint)
	int pid;
	char *mntpoint;
{
	FILE *f;
	struct mnttab mnt;
	char fsname[64];
	char mntopts[100];
	char tbuf[TIME_MAX];

	f = fopen(MNTTAB, "a");
	if (f == NULL) {
		syslog(LOG_ERR, "Can't update %s", MNTTAB);
		return;
	}

	if (lockf(fileno(f), F_LOCK, 0L) < 0) {
		syslog(LOG_ERR, "cannot lock %s: %m", MNTTAB);
		(void) fclose(f);
		return;
	}

	(void) sprintf(fsname, "%s:(pid%d)", self, pid);
	mnt.mnt_special = fsname;
	mnt.mnt_mountp  = mntpoint;
	mnt.mnt_fstype  = MNTTYPE_IGNORE;
	(void) strcpy(mntopts, "ro,intr");
	mnt.mnt_mntopts = mntopts;
	(void) sprintf(tbuf, "%ld", time(0L));
	mnt.mnt_time = tbuf;

	(void) fseek(f, 0L, 2); /* guarantee at EOF */
	putmntent(f, &mnt);

	(void) fclose(f);
}
#endif AUTO_MNTTAB

void
catch()
{
	struct autodir *dir;
	int child;
	struct filsys *fs, *fs_next;
	struct stat stbuf;
	struct fattr *fa;

	/*
	 *  The automounter has received a SIGTERM.
	 *  Here it forks a child to carry on servicing
	 *  its mount points in order that those
	 *  mount points can be unmounted.  The child
	 *  checks for symlink mount points and changes them
	 *  into directories to prevent the unmount system
	 *  call from following them.
	 */
	if ((child = fork()) == 0) {
		for (dir = HEAD(struct autodir, dir_q); dir;
	   	 dir = NEXT(struct autodir, dir)) {
			if (dir->dir_vnode.vn_type != VN_LINK)
				continue;
	
			dir->dir_vnode.vn_type = VN_DIR;
			fa = &dir->dir_vnode.vn_fattr;
			fa->type = NFDIR;
			fa->mode = NFSMODE_DIR + 0555;
		}
		return;
	}
	for (dir = HEAD(struct autodir, dir_q); dir;
	    dir = NEXT(struct autodir, dir)) {

		/*  This lstat is a kludge to force the kernel
		 *  to flush the attr cache.  If it was a direct
		 *  mount point (symlink) the kernel needs to
		 *  do a getattr to find out that it has changed
		 *  back into a directory.
		 */
		if (lstat(dir->dir_name, &stbuf) < 0) {
			syslog(LOG_ERR, "lstat %s: %m", dir->dir_name);
		}

		if (umount(dir->dir_name) < 0) {
			if (errno != EBUSY)
				syslog(LOG_ERR, "umount %s: %m", dir->dir_name);
		} else {
			if (dir->dir_remove)
				safe_rmdir(dir->dir_name);
#ifdef AUTO_MNTTAB
			clean_mnttab(dir->dir_name, 0);
#endif AUTO_MNTTAB
		}
	}
	(void) kill (child, SIGKILL);
	for (fs = HEAD(struct filsys, fs_q); fs; fs = fs_next) {
		fs_next = NEXT(struct filsys, fs);
		if (fs->fs_mine && fs == fs->fs_rootfs) {
			if (do_unmount(fs))
				fs_next = HEAD(struct filsys, fs_q);
		}
	}
	syslog(LOG_ERR, "exiting");
	exit(0);
}

auto_run()
{
	int read_fds, n;
	time_t time();
	long last;
	struct timeval tv;

	last = time((long *)0);
	tv.tv_sec = maxwait;
	tv.tv_usec = 0;
	for (;;) {
		read_fds = svc_fds;
		n = select(32, &read_fds, (int *)0, (int *)0, &tv);
		time_now = time((long *)0);
		if (n)
			svc_getreq(read_fds);
		if (time_now >= last + maxwait) {
			last = time_now;
			do_timeouts();
		}
	}
}

usage()
{
	fprintf(stderr, "Usage: automount\n%s%s%s%s%s%s%s%s%s%s%s",
		"\t[-n]\t\t(no mounts)\n",
		"\t[-m]\t\t(no auto.master)\n",
		"\t[-T]\t\t(trace nfs requests)\n",
		"\t[-v]\t\t(verbose error msgs)\n",
		"\t[-tl n]\t\t(mount duration)\n",
		"\t[-tm n]\t\t(attempt interval)\n",
		"\t[-tw n]\t\t(unmount interval)\n",
		"\t[-M dir]\t(temporary mount dir)\n",
		"\t[-D n=s]\t(define env variable)\n",
		"\t[-f file]\t(get mntpnts from file)\n",
		"\t[ dir map [-mntopts] ] ...\n");
	exit(1);
}

#ifdef YP

loadmaster_yp(mapname)
	char *mapname;
{
	int first, err;
	char *key, *nkey, *val;
	int kl, nkl, vl;
	char dir[100], map[100];
	char *p, *opts;


	first = 1;
	key  = NULL; kl  = 0;
	nkey = NULL; nkl = 0;
	val  = NULL; vl  = 0;

	for (;;) {
		if (first) {
			first = 0;
			err = yp_first(mydomain, mapname, &nkey, &nkl, &val, &vl);
		} else {
			err = yp_next(mydomain, mapname, key, kl, &nkey, &nkl,
				&val, &vl);
		}
		if (err) {
			if (err != YPERR_NOMORE && err != YPERR_MAP)
				syslog(LOG_ERR, "%s: %s",
					mapname, yperr_string(err));
			return;
		}
		if (key)
			free(key);
		key = nkey;
		kl = nkl;

		if (kl >= 100 || vl >= 100)
			return (FALSE);
		if (kl < 2 || vl < 1)
			return (FALSE);
		if (isspace(*key) || *key == '#')
			return (FALSE);
		(void) strncpy(dir, key, kl);
		dir[kl] = '\0';
		(void) strncpy(map, val, vl);
		map[vl] = '\0';
		p = map;
		while (*p && !isspace(*p))
			p++;
		opts = "rw";
		if (*p) {
			*p++ = '\0';
			while (*p && isspace(*p))
				p++;
			if (*p == '-')
				opts = p+1;
		}

		dirinit(dir, map, opts, 0);

		free(val);
	}
}
#endif YP

loadmaster_file(masterfile)
	char *masterfile;
{
	FILE *fp;
	int done = 0;
	char *line, *dir, *map, *opts;
	extern char *get_line();
	char linebuf[1024];

	if ((fp = fopen(masterfile, "r")) == NULL) {
		return (-1);
	}

	while ((line = get_line(fp, linebuf, sizeof linebuf)) != NULL) {
		dir = line;
		while (*dir && isspace(*dir)) dir++;
		if (*dir == '\0')
			continue;
		map = dir;
		while (*map && !isspace(*map)) map++;
		if (*map)
			*map++ = '\0';
		if (*dir == '+') {
#ifdef YP
			(void) loadmaster_yp(dir+1);
#endif YP
		} else {
			while (*map && isspace(*map)) map++;
			if (*map == '\0')
				continue;
			opts = map;
			while (*opts && !isspace(*opts)) opts++;
			if (*opts) {
				*opts++ = '\0';
				while (*opts && isspace(*opts)) opts++;
			}
			if (*opts != '-')
				opts = "-rw";
			
			dirinit(dir, map, opts+1, 0);
		}
		done++;
	}
	(void) fclose(fp);
	return (done);
}
