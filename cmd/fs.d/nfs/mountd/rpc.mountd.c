/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/mountd/rpc.mountd.c	1.11.2.1"

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
#include <sys/types.h>
#include <string.h>
#include <syslog.h>
#include <sys/param.h>
#include <rpc/rpc.h>
#include <sys/stat.h>
#include <netconfig.h>
#include <netdir.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/resource.h>
#include <nfs/nfs.h>
#include <rpcsvc/mount.h>
#include "sharetab.h"

#define MAXHOSTNAMELEN		64
#define MAXRMTABLINELEN		(MAXPATHLEN + MAXHOSTNAMELEN + 2)
#define MIN(a,b)	((a) < (b) ? (a) : (b))

extern int errno;

char RMTAB[] = "/etc/rmtab";

int mnt();
char *exmalloc();
struct groups **newgroup();
struct exports **newexport();
void log_cant_reply();
void mount();
void check_sharetab();
void sh_free();

/*
 * mountd's version of a "struct mountlist". It is the same except
 * for the added ml_pos field.
 */
struct mountdlist {
/* same as XDR mountlist */
	char *ml_name;
	char *ml_path;
	struct mountdlist *ml_nxt;
/* private to mountd */
	long ml_pos;		/* position of mount entry in RMTAB */
};

struct mountdlist *mountlist;

struct sh_list {		/* cached share list */
	struct sh_list *shl_next;
	struct share    shl_sh;
} *sh_list;

int nfs_portmon = 1;
char *domain;

void rmtab_load();
void rmtab_delete();
long rmtab_insert();

main(argc, argv)
	int argc;
	char **argv;
{
	register int i;
	struct rlimit rl;

	if (argc == 2) {
		if (strcmp(argv[1], "-n") == 0) {
			nfs_portmon = 0;
		} else {
			usage();
		}
	} else if (argc > 2) {
		usage();
	}

	switch(fork()) {
	case 0:		/* child */
		break;
	case -1:
		perror("mountd: can't fork");
		exit(1);
	default:	/* parent */
		exit(0);
	}

	/*
	 * Close existing file descriptors, open "/dev/null" as
	 * standard input, output, and error, and detach from
	 * controlling terminal.
	 */
	getrlimit(RLIMIT_NOFILE, &rl);
	for (i = 0 ; i < rl.rlim_max ; i++)
		(void) close(i);
	(void) open("/dev/null", O_RDONLY);
	(void) open("/dev/null", O_WRONLY);
	(void) dup(1);
	(void) setsid();

	openlog("mountd", LOG_PID, LOG_DAEMON);

	/*
	 * Create datagram service
	 */
	if (svc_create(mnt, MOUNTPROG, MOUNTVERS, "datagram_v") == 0) {
		syslog(LOG_ERR, "couldn't register datagram_v");
		exit(1);
	}

	/*
	 * Create connection oriented service
	 */
	if (svc_create(mnt, MOUNTPROG, MOUNTVERS, "circuit_v") == 0) {
		syslog(LOG_ERR, "couldn't register circuit_v");
		exit(1);
	}

#ifdef YPNFS
	/*
	 * Initialize the world 
	 */
	(void) yp_get_default_domain(&domain);
#endif /* YPNFS */

	/*
	 * Start serving 
	 */
	rmtab_load();
	svc_run();
	syslog(LOG_ERR, "Error: svc_run shouldn't have returned");
	abort();
	/* NOTREACHED */
}

/*
 * Server procedure switch routine 
 */
mnt(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	switch (rqstp->rq_proc) {
	case NULLPROC:
		errno = 0;
		if (!svc_sendreply(transp, xdr_void, (char *)0))
			log_cant_reply(transp);
		return;
	case MOUNTPROC_MNT:
		mount(rqstp);
		return;
	case MOUNTPROC_DUMP:
		errno = 0;
		if (!svc_sendreply(transp, xdr_mountlist, (char *)&mountlist))
			log_cant_reply(transp);
		return;
	case MOUNTPROC_UMNT:
		umount(rqstp);
		return;
	case MOUNTPROC_UMNTALL:
		umountall(rqstp);
		return;
	case MOUNTPROC_EXPORT:
	case MOUNTPROC_EXPORTALL:
		export(rqstp);
		return;
	default:
		svcerr_noproc(transp);
		return;
	}
}

/*
 * Get the client's hostname from the transport handle
 * If the name is not available then return "(anon)".
 */
struct nd_hostservlist *
getclientsnames(transp)
	SVCXPRT *transp;
{
	struct netbuf *nbuf;
	struct netconfig *nconf;
	static struct nd_hostservlist *serv;
	static struct nd_hostservlist anon_hsl;
	static struct nd_hostserv     anon_hs;
	static char anon_hname[] = "(anon)";
	static char anon_sname[] = "";

	/* Set up anonymous client */
	anon_hs.h_host = anon_hname;
	anon_hs.h_serv = anon_sname;
	anon_hsl.h_cnt = 1;
	anon_hsl.h_hostservs = &anon_hs;

	if (serv) {
		netdir_free((char *) serv, ND_HOSTSERVLIST);
		serv = NULL;
	}
	nconf = getnetconfigent(transp->xp_netid);
	if (nconf == NULL) {
		syslog(LOG_ERR, "%s: getnetconfigent failed",
			transp->xp_netid);
		return (&anon_hsl);
	}

	nbuf = svc_getrpccaller(transp);
	if (nbuf == NULL) {
		freenetconfigent(nconf);
		return (&anon_hsl);
	}
	if (netdir_getbyaddr(nconf, &serv, nbuf)) {
		freenetconfigent(nconf);
		return (&anon_hsl);
	}
	freenetconfigent(nconf);
	return (serv);
}

void
log_cant_reply(transp)
	SVCXPRT *transp;
{
	int saverrno;
	struct nd_hostservlist *clnames;
	register char *name;

	saverrno = errno;	/* save error code */
	clnames = getclientsnames(transp);
	if (clnames == NULL)
		return;
	name = clnames->h_hostservs->h_host;

	errno = saverrno;
	if (errno == 0)
		syslog(LOG_ERR, "couldn't send reply to %s", name);
	else
		syslog(LOG_ERR, "couldn't send reply to %s: %m", name);
}

/*
 * Check mount requests, add to mounted list if ok 
 */
void
mount(rqstp)
	struct svc_req *rqstp;
{
	SVCXPRT *transp;
	fhandle_t fh;
	struct fhstatus fhs;
	char *path, rpath[MAXPATHLEN];
	struct mountdlist *ml;
	char *gr, *grl;
	struct share *sh;
	struct share *findentry();
	struct stat st;
	char **aliases;
	struct nd_hostservlist *clnames;
	int i, restricted;
	char *name;

	transp = rqstp->rq_xprt;
	path = NULL;
	fhs.fhs_status = 0;
	clnames = getclientsnames(transp);
	if (clnames == NULL) {
		fhs.fhs_status = EACCES;
		goto done;	
	}
	name = clnames->h_hostservs[0].h_host;
	if (!svc_getargs(transp, xdr_path, &path)) {
		svcerr_decode(transp);
		return;
	}
	if (lstat(path, &st) < 0) {
		fhs.fhs_status = EACCES;
		goto done;	
	}

	/*
	 * If the path is a symbolic link then
	 * get what it points to.
	 */
	if ((st.st_mode & S_IFMT) == S_IFLNK) {
#ifdef later
		if (realpath(path, rpath) == NULL) {
			syslog(LOG_DEBUG,
				"mount request: realpath failed on %s: %m",
				path);
			fhs.fhs_status = EACCES;
			goto done;
		}
#else
		(void) strcpy(rpath, path);
#endif later
	} else {
		(void) strcpy(rpath, path);
	}

	if (nfs_getfh(rpath, &fh) < 0) {
		fhs.fhs_status = errno == EINVAL ? EACCES : errno;
		syslog(LOG_DEBUG, "mount request: getfh failed on %s: %m",
		    path);
		goto done;
	}

	sh = findentry(rpath);
	if (sh == NULL) {
		fhs.fhs_status = EACCES;
		goto done;
	}

	/*
	 * Check "ro" list (used to be "access" list)
	 * Try hostnames first - then netgroups.
	 */

	restricted = 0;

	grl = getshareopt(sh->sh_opts, SHOPT_RO);
	if (grl != NULL) {
		if (*grl == '\0')
			goto done;	/* no list */
		restricted++;
		while ((gr = strtok(grl, ":")) != NULL) {
			grl = NULL;
			for (i = 0 ; i < clnames->h_cnt ; i++) {
				name = clnames->h_hostservs[i].h_host;
				if (strcmp(gr, name) == 0) {
					goto done;
				}
			}
		}
	}
#ifdef YPNFS
	grl = getshareopt(sh->sh_opts, SHOPT_RO);
	if (grl != NULL) {
		while ((gr = strtok(grl, ":")) != NULL) {
			grl = NULL;
			for (i = 0 ; i < clnames->h_cnt ; i++) {
				name = clnames->h_hostservs[i].h_host;
				if (in_netgroup(gr, name, domain)) {
					goto done;
				}
			}
		}
	}
#endif /* YPNFS */
	
	/* Check "rw" list (hostname only) */

	grl = getshareopt(sh->sh_opts, SHOPT_RW);
	if (grl != NULL) {
		if (*grl == '\0')
			goto done;	/* no list */
		restricted++;
		while ((gr = strtok(grl, ":")) != NULL) {
			grl = NULL;
			for (i = 0 ; i < clnames->h_cnt ; i++) {
				name = clnames->h_hostservs[i].h_host;
				if (strcmp(gr, name) == 0)
					goto done;
			}
		}
	}

	/* Check "root" list (hostname only) */

	grl = getshareopt(sh->sh_opts, SHOPT_ROOT);
	if (grl != NULL) {
		while ((gr = strtok(grl, ":")) != NULL) {
			grl = NULL;
			for (i = 0 ; i < clnames->h_cnt ; i++) {
				name = clnames->h_hostservs[i].h_host;
				if (strcmp(gr, name) == 0)
					goto done;
			}
		}
	}

	if (restricted)
		fhs.fhs_status = EACCES;

done:
	if (fhs.fhs_status == 0)
		fhs.fhs_fh = fh;
	errno = 0;
	if (!svc_sendreply(transp, xdr_fhstatus, (char *)&fhs))
		log_cant_reply(transp);
	if (path != NULL)
		svc_freeargs(transp, xdr_path, &path);
	if (fhs.fhs_status)
		return;

	/*
	 *  Add an entry for this mount to the mount list.
	 *  First check whether it's there already - the client
	 *  may have crashed and be rebooting.
	 */
	for (ml = mountlist; ml != NULL ; ml = ml->ml_nxt) {
		if (strcmp(ml->ml_path, rpath) == 0) {
			if (strcmp(ml->ml_name, name) == 0) {
				return;
			}
		}
	}

	/*
	 * Add this mount to the mount list.
	 */
	ml = (struct mountdlist *) exmalloc(sizeof(struct mountdlist));
	ml->ml_path = (char *) exmalloc(strlen(rpath) + 1);
	(void) strcpy(ml->ml_path, rpath);
	ml->ml_name = (char *) exmalloc(strlen(name) + 1);
	(void) strcpy(ml->ml_name, name);
	ml->ml_nxt = mountlist;
	ml->ml_pos = rmtab_insert(name, rpath);
	mountlist = ml;
	return;
}

struct share *
findentry(path)
	char *path;
{
	struct share *sh;
	struct sh_list *shp;
	register char *p1, *p2;

	check_sharetab();

	for (shp = sh_list ; shp ; shp = shp->shl_next) {
		sh = &shp->shl_sh;
		for (p1 = sh->sh_path, p2 = path ; *p1 == *p2 ; p1++, p2++)
			if (*p1 == '\0')
				return (sh);	/* exact match */

		if ((*p1 == '\0' && *p2 == '/' ) ||
		    (*p1 == '\0' && *(p1-1) == '/') ||
		    (*p2 == '\0' && *p1 == '/' && *(p1+1) == '\0')) {
			if (issubdir(path, sh->sh_path))
				return (sh);
		}
	}
	return ((struct share *) 0);
}

#ifdef YPNFS
#define MAXGRPLIST 256
/*
 * Use cached netgroup info if the previous call was
 * from the same client.  Two lists are maintained
 * here: groups the client is a member of, and groups
 * the client is not a member of.
 */
int
in_netgroup(group, hostname, domain)
	char *group, *hostname, *domain;
{
	static char prev_hostname[MAXHOSTNAMELEN+1];
	static char grplist[MAXGRPLIST+1], nogrplist[MAXGRPLIST+1];
	char key[256];
	char *ypline = NULL;
	int yplen;
	register char *gr, *p;
	static time_t last;
	time_t time();
	time_t time_now;
	static int cache_time = 30; /* sec */

	time_now = time((long *)0);
	if (time_now > last + cache_time ||
	    strcmp(hostname, prev_hostname) != 0) {
		last = time_now;
		(void) strcpy(key, hostname);
		(void) strcat(key, ".");
		(void) strcat(key, domain);
		if (yp_match(domain, "netgroup.byhost", key,
		    strlen(key), &ypline, &yplen) == 0) {
			(void) strncpy(grplist, ypline, MIN(yplen, MAXGRPLIST));
			free(ypline);
		} else {
			grplist[0] = '\0';
		}
		nogrplist[0] = '\0';
		(void) strcpy(prev_hostname, hostname);
	}

	for (gr = grplist ; *gr ; gr = p ) {
		for (p = gr ; *p && *p != ',' ; p++)
			;
		if (strncmp(group, gr, p - gr) == 0)
			return (1);
		if (*p == ',')
			p++;
	}
	for (gr = nogrplist ; *gr ; gr = p ) {
		for (p = gr ; *p && *p != ',' ; p++)
			;
		if (strncmp(group, gr, p - gr) == 0)
			return (0);
		if (*p == ',')
			p++;
	}

	if (innetgr(group, hostname, (char *)NULL, domain)) {
		if (strlen(grplist)+1+strlen(group) > MAXGRPLIST)
			return (1);
		if (*grplist)
			(void) strcat(grplist, ",");
		(void) strcat(grplist, group);
		return (1);
	} else {
		if (strlen(nogrplist)+1+strlen(group) > MAXGRPLIST)
			return (1);
		if (*nogrplist)
			(void) strcat(nogrplist, ",");
		(void) strcat(nogrplist, group);
		return (0);
	}
}
#endif /* YPNFS */

void
check_sharetab()
{
	FILE *f;
	struct stat st;
	static long last_sharetab_time;
	struct share *sh;
	struct sh_list *shp, *shp_prev;
	int res, c = 0;

	/*
	 *  read in /etc/dfs/sharetab if it has changed 
	 */

	if (stat(SHARETAB, &st) != 0) {
		syslog(LOG_ERR, "Cannot stat %s: %m", SHARETAB);
		return;
	}
	if (st.st_mtime == last_sharetab_time)
		return;				/* no change */

	sh_free(sh_list);			/* free old list */
	sh_list = NULL;
	
	f = fopen(SHARETAB, "r");
	if (f == NULL)
		return;

	while ((res = getshare(f, &sh)) > 0) {
		c++;
		if (strcmp(sh->sh_fstype, "nfs") != 0)
			continue;

		shp = (struct sh_list *)malloc(sizeof(struct sh_list));
		if (shp == NULL)
			goto alloc_failed;
		if (sh_list == NULL)
			sh_list = shp;
		else
			shp_prev->shl_next = shp;
		shp_prev = shp;
		memset((char *)shp, 0, sizeof(struct sh_list));
		shp->shl_sh.sh_path = strdup(sh->sh_path);
		if (shp->shl_sh.sh_path == NULL)
			goto alloc_failed;
		if (sh->sh_opts) {
			shp->shl_sh.sh_opts = strdup(sh->sh_opts);
			if (shp->shl_sh.sh_opts == NULL)
				goto alloc_failed;
		}
	}
	if (res < 0)
		syslog(LOG_ERR, "%s: invalid at line %d\n",
			SHARETAB, c + 1);

	(void) fclose(f);
	last_sharetab_time = st.st_mtime;
	return;

alloc_failed:
	syslog(LOG_ERR, "check_sharetab: no memory");
	sh_free(sh_list);
	sh_list = NULL;
	(void) fclose(f);
	return;
}

void
sh_free(shp)
	struct sh_list *shp;
{
	register struct sh_list *next;

	while (shp) {
		if (shp->shl_sh.sh_path)
			free(shp->shl_sh.sh_path);
		if (shp->shl_sh.sh_opts)
			free(shp->shl_sh.sh_opts);
		next = shp->shl_next;
		free((char *)shp);
		shp = next;
	}
}


/*
 * Remove an entry from mounted list 
 */
umount(rqstp)
	struct svc_req *rqstp;
{
	char *path;
	struct mountdlist *ml, *oldml;
	struct nd_hostservlist *clnames;
	SVCXPRT *transp;

	transp = rqstp->rq_xprt;
	path = NULL;
	if (!svc_getargs(transp, xdr_path, &path)) {
		svcerr_decode(transp);
		return;
	}
	errno = 0;
	if (!svc_sendreply(transp, xdr_void, (char *)NULL))
		log_cant_reply(transp);

	clnames = getclientsnames(transp);
	if (clnames != NULL) {
		oldml = mountlist;
		for (ml = mountlist; ml != NULL;
		     oldml = ml, ml = ml->ml_nxt) {
			if (strcmp(ml->ml_path, path) == 0 &&
			    strcmp(ml->ml_name, clnames->h_hostservs->h_host) == 0) {
				if (ml == mountlist) {
					mountlist = ml->ml_nxt;
				} else {
					oldml->ml_nxt = ml->ml_nxt;
				}
				rmtab_delete(ml->ml_pos);
				free(ml->ml_path);
				free(ml->ml_name);
				free((char *)ml);
				break;
			}
		}
	}
	svc_freeargs(transp, xdr_path, &path);
}

/*
 * Remove all entries for one machine from mounted list 
 */
umountall(rqstp)
	struct svc_req *rqstp;
{
	struct mountdlist *ml, *oldml;
	struct nd_hostservlist *clnames;
	SVCXPRT *transp;

	transp = rqstp->rq_xprt;
	if (!svc_getargs(transp, xdr_void, NULL)) {
		svcerr_decode(transp);
		return;
	}
	/*
	 * We assume that this call is asynchronous and made via rpcbind
	 * callit routine.  Therefore return control immediately. The error
	 * causes rpcbind to remain silent, as apposed to every machine
	 * on the net blasting the requester with a response. 
	 */
	svcerr_systemerr(transp);
	clnames = getclientsnames(transp);
	if (clnames == NULL) {
		return;
	}
	oldml = mountlist;
	for (ml = mountlist; ml != NULL; ml = ml->ml_nxt) {
		if (strcmp(ml->ml_name, clnames->h_hostservs->h_host) == 0) {
			if (ml == mountlist) {
				mountlist = ml->ml_nxt;
				oldml = mountlist;
			} else {
				oldml->ml_nxt = ml->ml_nxt;
			}
			rmtab_delete(ml->ml_pos);
			free(ml->ml_path);
			free(ml->ml_name);
			free((char *)ml);
		} else {
			oldml = ml;
		}
	}
}

/*
 * send current export list 
 */
export(rqstp)
	struct svc_req *rqstp;
{
	struct exports *ex;
	struct exports **tail;
	char *grl;
	char *gr;
	struct groups *groups;
	struct groups **grtail;
	SVCXPRT *transp;
	struct share *sh;
	struct sh_list *shp;

	transp = rqstp->rq_xprt;
	if (!svc_getargs(transp, xdr_void, NULL)) {
		svcerr_decode(transp);
		return;
	}

	check_sharetab();

	ex = NULL;
	tail = &ex;
	for (shp = sh_list ; shp ; shp = shp->shl_next) {
		sh = &shp->shl_sh;

		grl = getshareopt(sh->sh_opts, SHOPT_RO);
		groups = NULL;
		if (grl != NULL) {
			grtail = &groups;
			while ((gr = strtok(grl, ":")) != NULL) {
				grl = NULL;
				grtail = newgroup(gr, grtail);
			}
		}
		tail = newexport(sh->sh_path, groups, tail);
	}

	errno = 0;
	if (!svc_sendreply(transp, xdr_exports, (char *)&ex))
		log_cant_reply(transp);
	freeexports(ex);
}


freeexports(ex)
	struct exports *ex;
{
	struct groups *groups, *tmpgroups;
	struct exports *tmpex;

	while (ex) {
		groups = ex->ex_groups;
		while (groups) {
			tmpgroups = groups->g_next;
			free(groups->g_name);
			free((char *)groups);
			groups = tmpgroups;
		}
		tmpex = ex->ex_next;
		free(ex->ex_name);
		free((char *)ex);
		ex = tmpex;
	}
}


struct groups **
newgroup(name, tail)
	char *name;
	struct groups **tail;
{
	struct groups *new;
	char *newname;

	new = (struct groups *) exmalloc(sizeof(*new));
	newname = (char *) exmalloc(strlen(name) + 1);
	(void) strcpy(newname, name);

	new->g_name = newname;
	new->g_next = NULL;
	*tail = new;
	return (&new->g_next);
}


struct exports **
newexport(name, groups, tail)
	char *name;
	struct groups *groups;
	struct exports **tail;
{
	struct exports *new;
	char *newname;

	new = (struct exports *) exmalloc(sizeof(*new));
	newname = (char *) exmalloc(strlen(name) + 1);
	(void) strcpy(newname, name);

	new->ex_name = newname;
	new->ex_groups = groups;
	new->ex_next = NULL;
	*tail = new;
	return (&new->ex_next);
}

char *
exmalloc(size)
	int size;
{
	char *ret;

	if ((ret = (char *) malloc((u_int)size)) == 0) {
		syslog(LOG_ERR, "Out of memory");
		exit(1);
	}
	return (ret);
}

usage()
{
	(void) fprintf(stderr, "Usage: mountd [-n]\n");
	exit(1);
}

FILE *f;

void
rmtab_load()
{
	char *path;
	char *name;
	char *end;
	struct mountdlist *ml;
	char line[MAXRMTABLINELEN];

	mountlist = NULL;
	f = fopen(RMTAB, "r");
	if (f != NULL) {
		while (fgets(line, sizeof(line), f) != NULL) {
			name = line;
			path = strchr(name, ':');
			if (*name != '#' && path != NULL) {
				*path = 0;
				path++;
				end = strchr(path, '\n');
				if (end != NULL) {
					*end = 0;
				}
				ml = (struct mountdlist *) 
					exmalloc(sizeof(struct mountdlist));
				ml->ml_path = (char *)
					exmalloc(strlen(path) + 1);
				(void) strcpy(ml->ml_path, path);
				ml->ml_name = (char *)
					exmalloc(strlen(name) + 1);
				(void) strcpy(ml->ml_name, name);
				ml->ml_nxt = mountlist;
				mountlist = ml;
			}
		}
		(void) fclose(f);
		(void) truncate(RMTAB, (off_t)0);
	} 
	f = fopen(RMTAB, "w+");
	if (f != NULL) {
		for (ml = mountlist; ml != NULL; ml = ml->ml_nxt) {
			ml->ml_pos = rmtab_insert(ml->ml_name, ml->ml_path);
		}
	}
}


long
rmtab_insert(name, path)
	char *name;
	char *path;
{
	long pos;

	if (f == NULL || fseek(f, 0L, 2) == -1) {
		return (-1);
	}
	pos = ftell(f);
	if (fprintf(f, "%s:%s\n", name, path) == EOF) {
		return (-1);
	}
	(void) fflush(f);
	return (pos);
}

void
rmtab_delete(pos)
	long pos;
{
	if (f != NULL && pos != -1 && fseek(f, pos, 0) == 0) {
		(void) fprintf(f, "#");
		(void) fflush(f);
	}
}
