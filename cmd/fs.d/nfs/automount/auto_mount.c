/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nfs.cmds:nfs/automount/auto_mount.c	1.9.2.1"

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
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <sys/mount.h>
#include <sys/mntent.h>
#include <sys/mnttab.h>
#include <netdb.h>
#include <errno.h>
#include "nfs_prot.h"
typedef nfs_fh fhandle_t;
#include <rpcsvc/mount.h>
#define NFSCLIENT
#include <nfs/mount.h>
#include <netconfig.h>
#include <netdir.h>
#include "automount.h"

#define MAXHOSTS  20

struct mapfs *find_server();
struct filsys *already_mounted();
void addtomnttab();
char *hasmntopt();
extern int trace;
extern int verbose;
extern dev_t tmpdev;
void free_knconf();

static long last_mnttab_time = 0;

nfsstat
do_mount(dir, me, rootfs, linkpath)
	struct autodir *dir;
	struct mapent *me;
	struct filsys **rootfs;
	char **linkpath;
{
	char mntpnt[MAXPATHLEN];
	static char linkbuf[MAXPATHLEN];
	enum clnt_stat pingmount();
	struct filsys *fs, *tfs;
	struct mapfs *mfs;
	struct mapent *m, *mapnext;
	nfsstat status = NFSERR_NOENT;
	char *prevhost;
	int imadeit;
	struct stat stbuf;

	*rootfs = NULL;
	*linkpath = "";
	prevhost = "";

	for (m = me ; m ; m = mapnext) {
		mapnext = m->map_next;

		(void) sprintf(mntpnt, "%s%s%s%s", tmpdir,
			dir->dir_name, me->map_root, m->map_mntpnt);

		/* check whether the mntpnt is already mounted on */

		if (m == me) {
			for (fs = HEAD(struct filsys, fs_q); fs;
			     fs = NEXT(struct filsys, fs)) {
				if (strcmp(mntpnt, fs->fs_mntpnt) == 0) {
					(void) sprintf(linkbuf, "%s%s%s%s",
						tmpdir,
						dir->dir_name,
						me->map_root,
						me->map_fs->mfs_subdir);
					if (trace > 1) 
						(void) fprintf(stderr,
						  "renew link for %s\n",
						  linkbuf);
					*linkpath = linkbuf;
					return (NFS_OK);
				}
			}
		}

		tfs = NULL;
		mfs = find_server(m, &tfs, m == me, prevhost);
		if (mfs == NULL)
			continue;

		/*
		 * It may be possible to return a symlink
		 * to an existing mount point without
		 * actually having to do a mount.
		 */
	
		if (me->map_next == NULL && *me->map_mntpnt == '\0') {

			/* Is it my host ? */
			if (strcmp(mfs->mfs_host, self) == 0 ||
			    strcmp(mfs->mfs_host, "localhost") == 0) {
				(void) strcpy(linkbuf, mfs->mfs_dir);
				(void) strcat(linkbuf, mfs->mfs_subdir);
				*linkpath = linkbuf;
				if (trace > 1)
					(void) fprintf(stderr,
						"It's on my host\n");
				return (NFS_OK);
			}

			/*
			 * An existing mount point?
			 * XXX Note: this is a bit risky - the
			 * mount may belong to another automount
			 * daemon - it could unmount it anytime and
			 * this symlink would then point to an empty
			 * or non-existent mount point.
			 */
			if (tfs != NULL) {
				if (trace > 1)
					(void) fprintf(stderr,
					"already mounted %s:%s on %s (%s)\n",
					tfs->fs_host, tfs->fs_dir,
					tfs->fs_mntpnt, tfs->fs_opts);

				(void) strcpy(linkbuf, tfs->fs_mntpnt);
				(void) strcat(linkbuf, mfs->mfs_subdir);
				*linkpath = linkbuf;
				*rootfs = tfs;
				return (NFS_OK);
			}
		}

		if (nomounts)
			return (NFSERR_PERM);

		 /* Create the mount point if necessary */

		imadeit = 0;
		if (stat(mntpnt, &stbuf) != 0) {
			if (mkdir_r(mntpnt) == 0) {
				imadeit = 1;
				if (stat(mntpnt, &stbuf) < 0) {
					syslog(LOG_ERR,
					"Couldn't stat created mountpoint %s: %m",
					mntpnt);
					continue;
				}
			} else {
				if (verbose)
					syslog(LOG_ERR,
					"Couldn't create mountpoint %s: %m",
					mntpnt);
				if (trace > 1)
					(void) fprintf(stderr,
					"couldn't create mntpnt %s\n",
					mntpnt);
				continue;
			}
		}
		if (verbose && *rootfs == NULL && tmpdev != stbuf.st_dev)
			syslog(LOG_ERR, "WARNING: %s already mounted on",
				mntpnt);

		/*  Now do the mount */

		tfs = NULL;
		status = nfsmount(mfs->mfs_host, mfs->mfs_dir,
				mntpnt, m->map_mntopts, &tfs);
		if (status == NFS_OK) {
			if (*rootfs == NULL) {
				*rootfs = tfs;
				(void) sprintf(linkbuf, "%s%s%s%s",
					tmpdir, dir->dir_name,
					me->map_root,
					mfs->mfs_subdir);
				*linkpath = linkbuf;
			}
			tfs->fs_rootfs = *rootfs;
			tfs->fs_mntpntdev = stbuf.st_dev;
			if (stat(mntpnt, &stbuf) < 0) {
				syslog(LOG_ERR, "Couldn't stat: %s: %m",
					mntpnt);
			} else {
				tfs->fs_mountdev = stbuf.st_dev;
			}
			prevhost = mfs->mfs_host;
		} else {
			if (imadeit)
				safe_rmdir(mntpnt);
			mfs->mfs_ignore = 1;
			prevhost = "";
			mapnext = m;
			continue;	/* try another server */
		}
	}
	if (*rootfs != NULL) {
		addtomnttab(*rootfs);
		return (NFS_OK);
	}
	return (status);
}

struct mapfs *
find_server(me, fsp, rootmount, preferred)
	struct mapent *me;
	struct filsys **fsp;
	int rootmount;
	char *preferred;
{
	int entrycount, localcount;
	struct mapfs *mfs, *mfs_one;
	struct mapfs *trymany();
	struct mnttab mnt;

	/*
	 * get addresses & see if any are myself
	 * or were mounted from previously in a
	 * hierarchical mount.
	 */
	entrycount = localcount = 0;
	mfs_one = NULL;
	for (mfs = me->map_fs; mfs; mfs = mfs->mfs_next) {
		if (mfs->mfs_ignore)
			continue;
		mfs_one = mfs;
		if (strcmp(mfs->mfs_host, "localhost") == 0 ||
		    strcmp(mfs->mfs_host, self) == 0 ||
		    strcmp(mfs->mfs_host, preferred) == 0)
			return (mfs);
		entrycount++;
	}
	if (entrycount == 0)
		return (NULL);

	/* see if any already mounted */
	if (rootmount) {
		for (mfs = me->map_fs; mfs; mfs = mfs->mfs_next) {
			if (mfs->mfs_ignore)
				continue;
		   	 if (*fsp = already_mounted(mfs->mfs_host, 
		  	      mfs->mfs_dir, me->map_mntopts )) 
			   	 return (mfs);
	    	}
	}

	if (entrycount > 1)
		return (trymany(me->map_fs, mount_timeout / 2));
	else
		return (mfs_one);

}

/*
 * Read the mnttab and correlate with internal fs info
 */
void
read_mnttab()
{
	struct stat stbuf;
	struct filsys *fs, *fsnext;
	FILE *mnttab;
	struct mnttab mt;
	int found, c;
	char *tmphost, *tmppath, *p, tmpc;
	int r;

	/* reset the present flags */
	for (fs = HEAD(struct filsys, fs_q); fs;
	     fs = NEXT(struct filsys, fs))
		fs->fs_present = 0;

	/* now see what's been mounted */

	mnttab = fopen(MNTTAB, "r");
	if (mnttab == NULL) {
		syslog(LOG_ERR, "%s: %m", MNTTAB);
		return;
	}

	for (c = 1 ;; c++) {
		if (r = getmntent(mnttab, &mt)) {
			if (r < 0)
				break;	/* EOF */
			syslog(LOG_ERR, "WARNING: %s: line %d: bad entry",
				MNTTAB, c);
			continue;
		}

		if (strcmp(mt.mnt_fstype, MNTTYPE_NFS) != 0)
			continue;
		p = strchr(mt.mnt_special, ':');
		if (p == NULL)
			continue;
		tmpc = *p;
		*p = '\0';
		tmphost = mt.mnt_special;
		tmppath = p+1;
		if (tmppath[0] != '/')
			continue;
		found = 0;
		for (fs = HEAD(struct filsys, fs_q); fs;
		     fs = NEXT(struct filsys, fs)) {
			if (strcmp(mt.mnt_mountp, fs->fs_mntpnt) == 0 &&
			    strcmp(tmphost, fs->fs_host) == 0 &&
			    strcmp(tmppath, fs->fs_dir) == 0 &&
			    (fs->fs_mine ||
			      strcmp(mt.mnt_mntopts, fs->fs_opts) == 0)) {
				fs->fs_present = 1;
				found++;
				break;
			}
		}
		if (!found) {
			fs = alloc_fs(tmphost, tmppath,
				mt.mnt_mountp, mt.mnt_mntopts);
			if (fs == NULL)
				break;
			fs->fs_present = 1;
		}
		*p = tmpc;
	}
	(void) fclose(mnttab);

	/* free fs's that are no longer present */
	for (fs = HEAD(struct filsys, fs_q); fs; fs = fsnext) {
		fsnext = NEXT(struct filsys, fs);
		if (!fs->fs_present) {
			if (fs->fs_mine)
				syslog(LOG_ERR,
					"%s:%s no longer mounted\n",
					fs->fs_host, fs->fs_dir);
			if (trace > 1)
				(void) fprintf(stderr,
					"%s:%s no longer mounted\n",
					fs->fs_host, fs->fs_dir);
			flush_links(fs);
			free_filsys(fs);
		}
	}

	if (stat(MNTTAB, &stbuf) != 0) {
		syslog(LOG_ERR, "Cannot stat %s: %m", MNTTAB);
		return;
	}
	last_mnttab_time = stbuf.st_mtime;
}

/*
 *  If mnttab has changed update internal fs info
 */
void
check_mnttab()
{
	struct stat stbuf;

	if (stat(MNTTAB, &stbuf) < 0) {
		syslog(LOG_ERR, "Cannot stat %s: %m", MNTTAB);
		return;
	}
	if (stbuf.st_mtime != last_mnttab_time)
		read_mnttab();
}

#define MNTOPT_INTR "intr"
#define MNTOPT_SECURE "secure"

/*
 * Search the mount table to see if the given file system is already
 * mounted. 
 */
struct filsys *
already_mounted(host, fsname, opts)
	char *host;
	char *fsname;
	char *opts;
{
	struct filsys *fs;
	struct stat stbuf;
	struct mnttab m1, m2;
	int has1, has2;
	int fd;
	struct autodir *dir;
	int mydir;
	extern int verbose;

	check_mnttab();
	m1.mnt_mntopts = opts;
	for (fs = HEAD(struct filsys, fs_q); fs; fs = NEXT(struct filsys, fs)){
		if (strcmp(fsname, fs->fs_dir) != 0)
			continue;
		if (strcmp(host, fs->fs_host) != 0)
			continue;

		/*
		 * Check it's not on one of my mount points.
		 * I might be mounted on top of a previous 
		 * mount of the same file system.
		 */

		for (mydir = 0, dir = HEAD(struct autodir, dir_q); dir;
			dir = NEXT(struct autodir, dir)) {
			if (strcmp(dir->dir_name, fs->fs_mntpnt) == 0) {
				mydir = 1;
				if (verbose)
					syslog(LOG_ERR,
					"%s:%s already mounted on %s",
					host, fsname, fs->fs_mntpnt);
				break;
			}
		}
		if (mydir)
			continue;

		m2.mnt_mntopts = fs->fs_opts;
		has1 = hasmntopt(&m1, MNTOPT_RO) != NULL;
		has2 = hasmntopt(&m2, MNTOPT_RO) != NULL;
		if (has1 != has2)
			continue;
		has1 = hasmntopt(&m1, MNTOPT_NOSUID) != NULL;
		has2 = hasmntopt(&m2, MNTOPT_NOSUID) != NULL;
		if (has1 != has2)
			continue;
		has1 = hasmntopt(&m1, MNTOPT_SOFT) != NULL;
		has2 = hasmntopt(&m2, MNTOPT_SOFT) != NULL;
		if (has1 != has2)
			continue;
		has1 = hasmntopt(&m1, MNTOPT_INTR) != NULL;
		has2 = hasmntopt(&m2, MNTOPT_INTR) != NULL;
		if (has1 != has2)
			continue;
		has1 = hasmntopt(&m1, MNTOPT_SECURE) != NULL;
		has2 = hasmntopt(&m2, MNTOPT_SECURE) != NULL;
		if (has1 != has2)
			continue;
		/*
		 * dir under mountpoint may have been removed by other means
		 * If so, we get a stale file handle error here if we fsync
		 * the dir to remove the attr cache info
		 */
		fd = open(fs->fs_mntpnt, 0);
		if (fd < 0)
			continue;
		if (fsync(fd) != 0 || fstat(fd, &stbuf) != 0) {
			(void) close(fd);
			continue;
		}
		(void) close(fd);
		return (fs);
	}
	return (0);
}

nfsunmount(fs)
	struct filsys *fs;
{
	struct timeval timeout;
	CLIENT *cl;
	enum clnt_stat rpc_stat;

	cl = clnt_create(fs->fs_host, MOUNTPROG, MOUNTVERS, "datagram_v");
	if (cl == NULL) {
		syslog(LOG_ERR, "%s:%s %s",
		    fs->fs_host, fs->fs_dir,
		    clnt_spcreateerror("server not responding"));
		return;
	}
	if (bindresvport(cl) < 0) {
		syslog(LOG_ERR, "umount %s:%s: %s",
		    fs->fs_host, fs->fs_dir,
		    "Couldn't bind to reserved port");
		clnt_destroy(cl);
		return;
	}
	cl->cl_auth = authsys_create_default();
	timeout.tv_usec = 0;
	timeout.tv_sec = 25;
	rpc_stat = clnt_call(cl, MOUNTPROC_UMNT, xdr_path, &fs->fs_dir,
	    xdr_void, (char *)NULL, timeout);
	AUTH_DESTROY(cl->cl_auth);
	clnt_destroy(cl);
	if (rpc_stat != RPC_SUCCESS)
		syslog(LOG_ERR, "%s", clnt_sperror(cl, "unmount"));
}

enum clnt_stat 
pingmount(hostname)
	char *hostname;
{
	CLIENT *cl;
	struct timeval tottime;
	enum clnt_stat clnt_stat;
	static char goodhost[MAXHOSTNAMELEN+1];
	static char deadhost[MAXHOSTNAMELEN+1];
	static time_t goodtime, deadtime;
	int cache_time = 60;  /* sec */

	if (goodtime > time_now && strcmp(hostname, goodhost) == 0)
			return (RPC_SUCCESS);
	if (deadtime > time_now && strcmp(hostname, deadhost) == 0)
			return (RPC_TIMEDOUT);

	if (trace > 1)
		(void) fprintf(stderr, "ping %s ", hostname);

	/* ping the NFS nullproc on the server */

	cl = clnt_create(hostname, NFS_PROGRAM, NFS_VERSION, "datagram_v");
	if (cl == NULL) {
		syslog(LOG_ERR, "pingmount: %s%s", 
			hostname, clnt_spcreateerror(""));
		clnt_stat = RPC_TIMEDOUT;
	} else {
		tottime.tv_sec = 10;
		tottime.tv_usec = 0;
		clnt_stat = clnt_call(cl, NULLPROC,
			xdr_void, 0, xdr_void, 0, tottime);
		clnt_destroy(cl);
	}

	if (clnt_stat == RPC_SUCCESS) {
		(void) strcpy(goodhost, hostname);
		goodtime = time_now + cache_time;
	} else {
		(void) strcpy(deadhost, hostname);
		deadtime = time_now + cache_time;
	}

	if (trace > 1)
		(void) fprintf(stderr, "%s\n", clnt_stat == RPC_SUCCESS ?
			"OK" : "NO RESPONSE");

	return (clnt_stat);
}

struct mapfs *got_one;

/* ARGSUSED */
catchfirst(mfs)
	struct mapfs *mfs;
{
	got_one = mfs;
	return (1);	/* first one ends search */
}

/*
 * ping a bunch of hosts at once and find out who
 * responds first
 */
struct mapfs *
trymany(mfs, timeout)
	struct mapfs *mfs;
	int timeout;
{
	enum clnt_stat nfs_cast();
	enum clnt_stat clnt_stat;

	if (trace > 1) {
		struct mapfs *m;

		(void) fprintf(stderr, "nfs_cast: ");
		for (m = mfs; m; m = m->mfs_next)
			(void) fprintf(stderr, "%s ", m->mfs_host);
		(void) fprintf(stderr, "\n");
	}
		
	got_one = NULL;
	clnt_stat = nfs_cast(mfs, catchfirst, timeout);
	if (trace > 1) {
		(void) fprintf(stderr, "nfs_cast: got %s\n",
			(int) clnt_stat ? "no response" : got_one->mfs_host);
	}
	if (clnt_stat)
		syslog(LOG_ERR, "trymany: servers not responding: %s",
			clnt_sperrno(clnt_stat));
	return (got_one);
}


nfsstat
nfsmount(host, dir, mntpnt, opts, fsp)
	char *host, *dir, *mntpnt, *opts;
	struct filsys **fsp;
{
	struct filsys *fs;
	char netname[MAXNETNAMELEN+1];
	char remname[MAXPATHLEN];
	struct mnttab m;
	struct nfs_args args;
	int flags;
	static struct fhstatus fhs;
	struct timeval timeout;
	static CLIENT *cl = NULL;
	enum clnt_stat rpc_stat;
	enum clnt_stat pingmount();
	nfsstat status;
	struct stat stbuf;
	struct netbuf *get_addr();
	struct netconfig *nconf;
	struct knetconfig *get_knconf();
	void netbuf_free();
	static time_t time_valid = 0;
	int cache_time = 60;	/* sec */
	static char prevhost[MAXHOSTNAMELEN+1];

	if (lstat(mntpnt, &stbuf) < 0) {
		syslog(LOG_ERR, "Couldn't stat %s: %m", mntpnt);
		return (NFSERR_NOENT);
	}
	if ((stbuf.st_mode & S_IFMT) == S_IFLNK) {
		if (readlink(mntpnt, remname, sizeof remname) < 0)
			return (NFSERR_NOENT);
		if (remname[0] == '/') {
			syslog(LOG_ERR,
				"%s -> %s from %s: absolute symbolic link",
				mntpnt, remname, host);
			return (NFSERR_NOENT);
		}
	}
	(void) sprintf(remname, "%s:%s", host, dir);
	 
	/*
	 * Get a client handle if it's a new host or if
	 * the handle is too old.
	 */
	 if (time_now > time_valid || strcmp(host, prevhost) != 0) {
		if (cl) {
			if (cl->cl_auth)
				AUTH_DESTROY(cl->cl_auth);
			clnt_destroy(cl);
		}
		cl = clnt_create(host, MOUNTPROG, MOUNTVERS, "udp");
		if (cl == NULL) {
			syslog(LOG_ERR, "%s %s", remname,
			    clnt_spcreateerror("server not responding"));
			return (NFSERR_NOENT);
		}
		if (bindresvport(cl) < 0) {
			syslog(LOG_ERR, "mount %s:%s: %s", host, dir,
			    "Couldn't bind to reserved port");
			clnt_destroy(cl);
			return (NFSERR_NOENT);
		}
		cl->cl_auth = authsys_create_default();

		(void) strcpy(prevhost, host);
		time_valid = time_now + cache_time;
	}

	/*
	 * Get fhandle of remote path from server's mountd
	 */
	timeout.tv_usec = 0;
	timeout.tv_sec = mount_timeout;
	rpc_stat = clnt_call(cl, MOUNTPROC_MNT, xdr_path, &dir,
	    xdr_fhstatus, &fhs, timeout);
	if (rpc_stat != RPC_SUCCESS) {
		/*
		 * Given the way "clnt_sperror" works, the "%s" immediately
		 * following the "not responding" is correct.
		 */
		syslog(LOG_ERR, "%s server not responding%s", remname,
		    clnt_sperror(cl, ""));
		clnt_destroy(cl);
		return (NFSERR_NOENT);
	}

	if (errno = fhs.fhs_status)  {
                if (errno == EACCES) {
			status = NFSERR_ACCES;
                } else {
			syslog(LOG_ERR, "%s: %m", remname);
			status = NFSERR_IO;
                }
                return (status);
        }        

	/*
	 * set mount args
	 */
	memset(&args, 0, sizeof(args));
	args.fh = (caddr_t)&fhs.fhs_fh;
	args.flags = 0;
	args.hostname = host;
	args.flags |= NFSMNT_HOSTNAME;

	args.addr = get_addr(host, NFS_PROGRAM, NFS_VERSION, &nconf);
	if (args.addr == NULL) {
		syslog(LOG_ERR, "%s: no NFS service", host);
		return (NFSERR_NOENT);
	}

	args.flags |= NFSMNT_KNCONF;
	args.knconf = get_knconf(nconf);
	if (args.knconf == NULL) {
		netbuf_free(args.addr);
		return (NFSERR_NOSPC);
	}

	m.mnt_mntopts = opts;

	if (hasmntopt(&m, MNTOPT_SOFT) != NULL) {
		args.flags |= NFSMNT_SOFT;
	}
	if (hasmntopt(&m, MNTOPT_INTR) != NULL) {
		args.flags |= NFSMNT_INT;
	}
	if (hasmntopt(&m, MNTOPT_SECURE) != NULL) {
		args.flags |= NFSMNT_SECURE;
		/*
                 * XXX: need to support other netnames outside domain
                 * and not always just use the default conversion
                 */
                if (!host2netname(netname, host, NULL)) {
			netbuf_free(args.addr);
			free_knconf(args.knconf);
                        return (NFSERR_NOENT); /* really unknown host */
                }
                args.netname = netname;
		args.syncaddr = get_addr(host, RPCBPROG, RPCBVERS, &nconf);
		if (args.syncaddr) {
			args.flags |= NFSMNT_RPCTIMESYNC;
		} else {
			/* If it's a UDP transport
			 * then use the time service
			 */
			if (strcmp(nconf->nc_protofmly, NC_INET) == 0 &&
			    strcmp(nconf->nc_proto, NC_UDP) == 0) {
				struct nd_hostserv hs;
				static struct nd_addrlist *retaddrs;

				hs.h_host = host;
				hs.h_serv = "rpcbind";
				if (netdir_getbyname(nconf, &hs, &retaddrs)
				   != ND_OK) {
					netbuf_free(args.addr);
					free_knconf(args.knconf);
					syslog(LOG_ERR,
					   "%s: no time service", host);
					return (NFSERR_IO);
				}
				args.syncaddr = retaddrs->n_addrs;
				((struct sockaddr_in *) args.syncaddr->buf)->sin_port
				   = IPPORT_TIMESERVER;
			} else {
				netbuf_free(args.addr);
				free_knconf(args.knconf);
				syslog(LOG_ERR,
				   "%s: no time service", host);
				return (NFSERR_IO);
			}
		}
	} /* end of secure stuff */

	if (hasmntopt(&m, MNTOPT_GRPID) != NULL) {
		args.flags |= NFSMNT_GRPID;
	}
	if (args.rsize = nopt(&m, "rsize")) {
		args.flags |= NFSMNT_RSIZE;
	}
	if (args.wsize = nopt(&m, "wsize")) {
		args.flags |= NFSMNT_WSIZE;
	}
	if (args.timeo = nopt(&m, "timeo")) {
		args.flags |= NFSMNT_TIMEO;
	}
	if (args.retrans = nopt(&m, "retrans")) {
		args.flags |= NFSMNT_RETRANS;
	}

	flags = 0;
	flags |= (hasmntopt(&m, MNTOPT_RO) == NULL) ? 0 : MS_RDONLY;
	flags |= (hasmntopt(&m, MNTOPT_NOSUID) == NULL) ? 0 : MS_NOSUID;

	if (trace > 1) {
		(void) fprintf(stderr, "mount %s %s (%s)\n",
			remname, mntpnt, opts);
	}
	if (mount("", mntpnt, flags | MS_DATA, MNTTYPE_NFS,
		&args, sizeof (args)) < 0) {
		netbuf_free(args.addr);
		netbuf_free(args.syncaddr);
		free_knconf(args.knconf);
		syslog(LOG_ERR, "Mount of %s on %s: %m", remname, mntpnt);
		return (NFSERR_IO);
	}
	if (trace > 1) {
		(void) fprintf(stderr, "mount %s OK\n", remname);
	}
	if (*fsp)
		fs = *fsp;
	else {
		fs = alloc_fs(host, dir, mntpnt, opts);
		if (fs == NULL) {
			netbuf_free(args.addr);
			netbuf_free(args.syncaddr);
			free_knconf(args.knconf);
			return (NFSERR_NOSPC);
		}
	}
	fs->fs_type = MNTTYPE_NFS;
	fs->fs_mine = 1;
	fs->fs_nfsargs = args;
	fs->fs_mflags = flags;
	fs->fs_nfsargs.hostname = fs->fs_host;
	fs->fs_nfsargs.fh = (caddr_t)&fs->fs_rootfh;
	memcpy(&fs->fs_rootfh, &fhs.fhs_fh, sizeof fs->fs_rootfh);
	*fsp = fs;

	return (NFS_OK);
}

struct knetconfig *
get_knconf(nconf)
	struct netconfig *nconf;
{
	struct stat stbuf;
	struct knetconfig *k;

	if (stat(nconf->nc_device, &stbuf) < 0) {
		syslog(LOG_ERR, "get_knconf: stat %s: %m", nconf->nc_device);
		return (NULL);
	}
	k = (struct knetconfig *) malloc(sizeof(*k));
	if (k == NULL)
		goto nomem;
	k->knc_semantics = nconf->nc_semantics;
	k->knc_protofmly = strdup(nconf->nc_protofmly);
	if (k->knc_protofmly == NULL)
		goto nomem;
	k->knc_proto = strdup(nconf->nc_proto);
	if (k->knc_proto == NULL)
		goto nomem;
	k->knc_rdev = stbuf.st_rdev;

	return (k);

nomem:
	syslog(LOG_ERR, "get_knconf: no memory");
	free_knconf(k);
	return (NULL);
}

void
free_knconf(k)
	struct knetconfig *k;
{
	if (k == NULL)
		return;
	if (k->knc_protofmly)
		free(k->knc_protofmly);
	if (k->knc_proto)
		free(k->knc_proto);
	free(k);
}

void
netbuf_free(nb)
	struct netbuf *nb;
{
	if (nb == NULL)
		return;
	if (nb->buf)
		free(nb->buf);
	free(nb);
}

/*
 * Get the network address for the service identified by "prog"
 * and "vers" on "hostname".  The netconfig address is returned
 * in the value of "nconfp".
 * If the hostname is the same as the last call, then the same
 * transport is used as last time (same netconfig entry).
 */

struct netbuf *
get_addr(hostname, prog, vers, nconfp)
	char *hostname;
	int prog, vers;
	struct netconfig **nconfp;
{
	static char prevhost[MAXHOSTNAMELEN+1];
	static struct netconfig *nconf;
	static NCONF_HANDLE *nc = NULL;
	struct netbuf *nb = NULL;
	struct t_bind *tbind = NULL;
	struct netconfig *getnetconfig();
	struct netconfig *getnetconfigent();
	enum clnt_stat cs;
	struct timeval tv;
	int fd = -1;

	if (strcmp(hostname, prevhost) != 0) {
		if (nc)
			endnetconfig(nc);
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
				break;
			}
		}
		if (nconf == NULL)
			goto done;
		(void) strcpy(prevhost, hostname);
	}

	fd = t_open(nconf->nc_device, O_RDWR, NULL);
	if (fd < 0)
		goto done;

	tbind = (struct t_bind *) t_alloc(fd, T_BIND, T_ADDR);
	if (tbind == NULL)
		goto done;
	
	if (rpcb_getaddr(prog, vers, nconf, &tbind->addr, hostname) == 0) {
		goto retry;
	}
	*nconfp = nconf;

	/*
	 * Make a copy of the netbuf to return
	 */
	nb = (struct netbuf *) malloc(sizeof(struct netbuf));
	if (nb == NULL) {
		syslog(LOG_ERR, "no memory");
		goto done;
	}
	*nb = tbind->addr;
	nb->buf = (char *)malloc(nb->len);
	if (nb->buf == NULL) {
		syslog(LOG_ERR, "no memory");
		free(nb);
		nb = NULL;
		goto done;
	}
	(void) memcpy(nb->buf, tbind->addr.buf, tbind->addr.len);

done:
	if (tbind)
		t_free((char *) tbind, T_BIND);
	if (fd >= 0)
		(void) t_close(fd);
	return (nb);
}

nfsstat
remount(fs)
	struct filsys *fs;
{
	char remname[1024];

	if (fs->fs_nfsargs.fh == 0) 
		return (nfsmount(fs->fs_host, fs->fs_dir,
				fs->fs_mntpnt, fs->fs_opts, &fs));
	(void) sprintf(remname, "%s:%s", fs->fs_host, fs->fs_dir);
	if (trace > 1) {
		(void) fprintf(stderr, "remount %s %s (%s)\n",
			remname, fs->fs_mntpnt, fs->fs_opts);
	}
	if (mount("", fs->fs_mntpnt, fs->fs_mflags | MS_DATA, MNTTYPE_NFS, 
	    &fs->fs_nfsargs, sizeof(fs->fs_nfsargs)) < 0) {
		syslog(LOG_ERR, "Remount of %s on %s: %m", remname,
		    fs->fs_mntpnt);
		return (NFSERR_IO);
	}
	if (trace > 1) {
		(void) fprintf(stderr, "remount %s OK\n", remname);
	}
	return (NFS_OK);
}

#define TIME_MAX 16

/*
 * Add one or more entries to /etc/mnttab
 */
void
addtomnttab(rootfs)
	struct filsys *rootfs;
{
	FILE *fp;
	struct filsys *fs;
	struct stat stbuf;
	struct mnttab mnt;
	char remname[MAXPATHLEN];
	char tbuf[TIME_MAX];
	
	fp = fopen(MNTTAB, "a");
	if (fp == NULL) {
		syslog(LOG_ERR, "%s: %m", MNTTAB);
		return;
	}

	if (lockf(fileno(fp), F_LOCK, 0L) < 0) {
		syslog(LOG_ERR, "cannot lock %s: %m", MNTTAB);
		(void) fclose(fp);
		return;
	}
	(void) fseek(fp, 0L, 2); /* guarantee at EOF */

	(void) sprintf(tbuf, "%ld", time(0L));
	mnt.mnt_time = tbuf;

	for (fs = TAIL(struct filsys, fs_q); fs; fs = PREV(struct filsys, fs)) {
		if (fs->fs_rootfs != rootfs)
			continue;

		(void) sprintf(remname, "%s:%s", fs->fs_host, fs->fs_dir);
		mnt.mnt_special = remname;
		mnt.mnt_mountp = fs->fs_mntpnt;
		mnt.mnt_fstype = MNTTYPE_NFS;
		mnt.mnt_mntopts = fs->fs_opts;

		if (putmntent(fp, &mnt) <= 0) {
			(void) fclose(fp);
			syslog(LOG_ERR, "%s: %m", MNTTAB);
			return;
		}
	}
	(void) fclose(fp);
	if (stat(MNTTAB, &stbuf) < 0)
		syslog(LOG_ERR, "%s: %m", MNTTAB);
	else
		last_mnttab_time = stbuf.st_mtime;
}

/*
 * This structure is used to build a list of
 * mnttab structures from /etc/mtab.
 */
struct mntlist {
	struct mnttab  *mntl_mnt;
	struct mntlist *mntl_next;
};

struct mnttab *
dupmnttab(mnt)
	struct mnttab *mnt;
{
	struct mnttab *new;
	void freemnttab();

	new = (struct mnttab *)malloc(sizeof(*new));
	if (new == NULL)
		goto alloc_failed;
	memset((char *)new, 0, sizeof(*new));
	new->mnt_special = strdup(mnt->mnt_special);
	if (new->mnt_special == NULL)
		goto alloc_failed;
	new->mnt_mountp = strdup(mnt->mnt_mountp);
	if (new->mnt_mountp == NULL)
		goto alloc_failed;
	new->mnt_fstype = strdup(mnt->mnt_fstype);
	if (new->mnt_fstype == NULL)
		goto alloc_failed;
	new->mnt_mntopts = strdup(mnt->mnt_mntopts);
	if (new->mnt_mntopts == NULL)
		goto alloc_failed;
	new->mnt_time = strdup(mnt->mnt_time);
	if (new->mnt_time == NULL)
		goto alloc_failed;

	return (new);

alloc_failed:
	syslog(LOG_ERR, "dupmnttab: memory allocation failed");
	freemnttab(new);
	return (NULL);
}

/*
 * Free a single mnttab structure
 */
void
freemnttab(mntl)
	struct mntlist *mntl;
{
	register struct mntlist *mntl_tmp;
	register struct mnttab *mnt;

	if (mnt) {
		if (mnt->mnt_special)
			free(mnt->mnt_special);
		if (mnt->mnt_mountp)
			free(mnt->mnt_mountp);
		if (mnt->mnt_fstype)
			free(mnt->mnt_fstype);
		if (mnt->mnt_mntopts)
			free(mnt->mnt_mntopts);
		if (mnt->mnt_time)
			free(mnt->mnt_time);
		free(mnt);
	}
}

/*
 * Free a list of mnttab structures
 */
void
freemntlist(mntl)
	struct mntlist *mntl;
{
	register struct mntlist *mntl_tmp;

	while (mntl) {
		freemnttab(mntl->mntl_mnt);
		mntl_tmp = mntl;
		mntl = mntl->mntl_next;
		free(mntl_tmp);
	}
}

/*
 * Remove one or more entries from the mount table.
 * If mntpnt is non-null then remove the entry
 * for that mount point.
 * Otherwise use rootfs - it is the root fs of
 * a mounted hierarchy.  Remove all entries for
 * the hierarchy.
 */
void
clean_mnttab(mntpnt, rootfs)
	char *mntpnt;
	struct filsys *rootfs;
{
	FILE *mnttab;
	struct mnttab mnt;
	struct stat stbuf;
	struct filsys *fs;
	struct mntlist *mntl_head = NULL;
	struct mntlist *mntl_prev, *mntl;
	int delete;

	mnttab = fopen(MNTTAB, "r+");
	if (mnttab == NULL) {
		syslog(LOG_ERR, "%s: %m", MNTTAB);
		return;
	}

	if (lockf(fileno(mnttab), F_LOCK, 0L) < 0) {
		syslog(LOG_ERR, "%s: cannot lock %s: %m", MNTTAB);
		(void) fclose(mnttab);
		return;
	}

	/*
	 * Read the entire mnttab into memory except for the
	 * entries we're trying to delete.
	 */
	while (getmntent(mnttab, &mnt) == 0) {

		if (mntpnt) {
			if (strcmp(mnt.mnt_mountp, mntpnt) == 0)
				continue;
		} else {
			delete = 0;
			for (fs = rootfs ; fs ; fs = NEXT(struct filsys, fs)) {
				if (strcmp(mnt.mnt_mountp, fs->fs_mntpnt) == 0) {
					delete = 1;
					break;
				}
			}
			if (delete)
				continue;
		}
		mntl = (struct mntlist *) malloc(sizeof(*mntl));
		if (mntl == NULL)
			goto alloc_failed;
		if (mntl_head == NULL)
			mntl_head = mntl;
		else
			mntl_prev->mntl_next = mntl;
		mntl_prev = mntl;
		mntl->mntl_next = NULL;
		mntl->mntl_mnt = dupmnttab(&mnt);
		if (mntl->mntl_mnt == NULL)
			goto alloc_failed;
	}

	/* now truncate the mnttab and write almost all of it back */

	rewind(mnttab);
	if (ftruncate(fileno(mnttab), 0) < 0) {
		syslog(LOG_ERR, "truncate %s: %m", MNTTAB);
		(void) fclose(mnttab);
		return;
	}

	for (mntl = mntl_head ; mntl ; mntl = mntl->mntl_next) {
		if (putmntent(mnttab, mntl->mntl_mnt) <= 0) {
			syslog(LOG_ERR, "putmntent: %m");
			(void) fclose(mnttab);
			return;
		}
	}
	(void) fclose(mnttab);
	freemnttab(mntl_head);

	if (stat(MNTTAB, &stbuf) < 0)
		syslog(LOG_ERR, "%s: %m", MNTTAB);
	else
		last_mnttab_time = stbuf.st_mtime;

alloc_failed:
	freemnttab(mntl_head);
	(void) fclose(mnttab);
	return;
}
/*
 * Return the value of a numeric option of the form foo=x, if
 * option is not found or is malformed, return 0.
 */
nopt(mnt, opt)
	struct mnttab *mnt;
	char *opt;
{
	int val = 0;
	char *equal;
	char *str;

	if (str = hasmntopt(mnt, opt)) {
		if (equal = strchr(str, '=')) {
			val = atoi(&equal[1]);
		} else {
			syslog(LOG_ERR, "Bad numeric option '%s'", str);
		}
	}
	return (val);
}

char *
mntopt(p)
	char **p;
{
	char *cp = *p;
	char *retstr;

	while (*cp && isspace(*cp))
		cp++;
	retstr = cp;
	while (*cp && *cp != ',')
		cp++;
	if (*cp) {
		*cp = '\0';
		cp++;
	}
	*p = cp;
	return (retstr);
}

char *
hasmntopt(mnt, opt)
	register struct mnttab *mnt;
	register char *opt;
{
	char *f, *opts;
	static char *tmpopts;

	if (tmpopts == 0) {
		tmpopts = (char *)calloc(256, sizeof (char));
		if (tmpopts == 0)
			return (0);
	}
	(void) strcpy(tmpopts, mnt->mnt_mntopts);
	opts = tmpopts;
	f = mntopt(&opts);
	for (; *f; f = mntopt(&opts)) {
		if (strncmp(opt, f, strlen(opt)) == 0)
			return (f - tmpopts + mnt->mnt_mntopts);
	} 
	return (NULL);
}
