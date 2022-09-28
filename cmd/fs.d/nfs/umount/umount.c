/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/umount/umount.c	1.7.4.1"

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
 * nfs umount
 */

#include <stdio.h>
#include <string.h>
#include <varargs.h>
#include <signal.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <sys/mnttab.h>
#include <nfs/nfs.h>
#include <rpcsvc/mount.h>
#include <errno.h>

#define RET_OK	0
#define RET_ERR	32

void	pr_err();
void	usage();
int	nfs_unmount();
void	inform_server();
void	delete_mnttab();
void	freemnttab();
struct mnttab *dupmnttab();
struct mnttab *mnttab_find();

char *myname;
char typename[64];

extern int errno;

main(argc, argv)
	int argc;
	char **argv;
{
	extern int optind;
	int c;

	myname = strrchr(argv[0], '/');
	myname = myname ? myname+1 : argv[0];
	(void) sprintf(typename, "nfs %s", myname);
	argv[0] = typename;

	/*
	 * Set options
	 */
	while ((c = getopt(argc, argv, "")) != EOF) {
		switch (c) {
		default:
			usage();
			exit(RET_ERR);
		}
	}
	if (argc - optind != 1) {
		usage();
		exit(RET_ERR);
	}

	if (geteuid() != 0) {
		pr_err("not super user\n");
		exit(RET_ERR);
	}

	exit (nfs_unmount(argv[optind]));
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
	va_end(ap);
}

void
usage()
{
	(void) fprintf(stderr,
	    "Usage: nfs umount [-o opts] {server:path | dir}\n");
	exit(RET_ERR);
}

int
nfs_unmount(name)
	char *name;
{
	struct mnttab *mntp;

	mntp = mnttab_find(name);
	if (mntp) {
		name = mntp->mnt_mountp;
	}
	
	if (umount(name) < 0) {
		if (errno == EBUSY) {
			pr_err("%s: is busy\n", name);
		} else {
			pr_err("%s: not mounted\n", name);
		}
		return (RET_ERR);
	}

	if (mntp) {
		inform_server(mntp->mnt_special);
		delete_mnttab(mntp->mnt_mountp);
	}

	return (RET_OK);
}

/*  Find the mnttab entry that corresponds to "name".
 *  We're not sure what the name represents: either
 *  a mountpoint name, or a special name (server:/path).
 *  Return the last entry in the file that matches.
 */
struct mnttab *
mnttab_find(name)
	char *name;
{
	FILE *fp;
	struct mnttab mnt;
	struct mnttab *res = NULL;

	fp = fopen(MNTTAB, "r");
	if (fp == NULL) {
		pr_err("%s", MNTTAB);
		perror("");
		return NULL;
	}
	while (getmntent(fp, &mnt) == 0) {
		if (strcmp(mnt.mnt_mountp , name) == 0 ||
		    strcmp(mnt.mnt_special, name) == 0) {
			if (res)
				freemnttab(res);
			res = dupmnttab(&mnt);
		}
	}

	fclose(fp);
	return (res);
}

/*
 * This structure is used to build a list of
 * mnttab structures from /etc/mnttab.
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
	pr_err("dupmnttab: no mem\n");
	return (NULL);
}

void
freemnttab(mnt)
	struct mnttab *mnt;
{
	free(mnt->mnt_special);
	free(mnt->mnt_mountp);
	free(mnt->mnt_fstype);
	free(mnt->mnt_mntopts);
	free(mnt->mnt_time);
	free(mnt);
}

/*
 * Delete an entry from the mount table.
 */
void
delete_mnttab(mntpnt)
	char *mntpnt;
{
	FILE *fp;
	struct mnttab mnt;
	struct mntlist *mntl_head = NULL;
	struct mntlist *mntl_prev, *mntl;
	struct mntlist *delete = NULL;

	fp = fopen(MNTTAB, "r+");
	if (fp == NULL) {
		pr_err("%s", MNTTAB);
		perror("");
		return;
	}

	if (lockf(fileno(fp), F_LOCK, 0L) < 0) {
		pr_err("cannot lock %s", MNTTAB);
		perror("");
		(void) fclose(fp);
		return;
	}

	/*
	 * Read the entire mnttab into memory.
	 * Remember the *last* instance of the unmounted
	 * mount point (have to take stacked mounts into
	 * account) and make sure that it's not written
	 * back out.
	 */
	while (getmntent(fp, &mnt) == 0) {
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
		if (strcmp(mnt.mnt_mountp, mntpnt) == 0)
			delete = mntl;
	}

	/* now truncate the mnttab and write almost all of it back */

	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT,  SIG_IGN);

	rewind(fp);
	if (ftruncate(fileno(fp), 0) < 0) {
		pr_err("truncate %s", MNTTAB);
		perror("");
		(void) fclose(fp);
		return;
	}

	for (mntl = mntl_head ; mntl ; mntl = mntl->mntl_next) {
		if (mntl == delete)
			continue;
		if (putmntent(fp, mntl->mntl_mnt) <= 0) {
			pr_err("putmntent");
			perror("");
			(void) fclose(fp);
			return;
		}
	}

alloc_failed:
	(void) fclose(fp);
	return;
}

void
inform_server(fsname)
	char *fsname;
{
	char *host, *path;
	struct timeval timeout;
	CLIENT *cl;
	enum clnt_stat rpc_stat;

	host = strdup(fsname);
	if (host == NULL) {
		pr_err("no mem\n");
		return;
	}
	path = strchr(host, ':');
	if (path == NULL) {
		pr_err("%s is not hostname:path format\n");
		return;
	}
	*path++ = '\0';

	cl = clnt_create(host, MOUNTPROG, MOUNTVERS, "datagram_n");
	if (cl == NULL) {
		pr_err("%s:%s %s\n", host, path,
		    clnt_spcreateerror("server not responding"));
		return;
	}
	if (bindresvport(cl) < 0) {
		pr_err("couldn't bind to reserved port\n");
		clnt_destroy(cl);
		return;
	}
	timeout.tv_usec = 0;
	timeout.tv_sec = 5;
	clnt_control(cl, CLSET_RETRY_TIMEOUT, &timeout);
	cl->cl_auth = authsys_create_default();
	timeout.tv_sec = 25;
	rpc_stat = clnt_call(cl, MOUNTPROC_UMNT, xdr_path, &path,
	    xdr_void, (char *)NULL, timeout);
	AUTH_DESTROY(cl->cl_auth);
	clnt_destroy(cl);
	if (rpc_stat != RPC_SUCCESS)
		pr_err("%s\n", clnt_sperror(cl, "unmount"));
	return;
}
