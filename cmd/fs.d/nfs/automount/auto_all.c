/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/automount/auto_all.c	1.5.3.1"

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
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <sys/mntent.h>
#include <netdb.h>
#include <errno.h>
#include "nfs_prot.h"
#define NFSCLIENT
typedef nfs_fh fhandle_t;
#include <rpcsvc/mount.h>
#include <nfs/mount.h>
#include "automount.h"

extern int trace;

do_unmount(fsys)
	struct filsys *fsys;
{
	struct q tmpq;
	struct filsys *fs, *nextfs, *rootfs;
	nfsstat remount();
	dev_t olddev;
	int newdevs;

	tmpq.q_head = tmpq.q_tail = NULL;
	for (fs = HEAD(struct filsys, fs_q); fs; fs = nextfs) {
		nextfs = NEXT(struct filsys, fs);
		if (fs->fs_rootfs == fsys) {
			REMQUE(fs_q, fs);
			INSQUE(tmpq, fs);
		}
	}
	/* walk backwards trying to unmount */
	for (fs = TAIL(struct filsys, tmpq); fs; fs = nextfs) {
		nextfs = PREV(struct filsys, fs);
		if (fs->fs_unmounted)
			continue;
		if (trace > 1) {
			fprintf(stderr, "unmount %s ", fs->fs_mntpnt);
			fflush(stderr);
		}
		if (!pathok(tmpq, fs) || umount(fs->fs_mntpnt) < 0) {
			if (trace > 1)
				fprintf(stderr, "unmount %s: BUSY\n",
				    fs->fs_mntpnt);
			goto inuse;
		}
		fs->fs_unmounted = 1;
		if (trace > 1)
			fprintf(stderr, "unmount %s: OK\n", fs->fs_mntpnt);
	}
	/* all ok - walk backwards removing directories */
	clean_mnttab(0, HEAD(struct filsys, tmpq));
	for (fs = TAIL(struct filsys, tmpq); fs; fs = nextfs) {
		nextfs = PREV(struct filsys, fs);
		nfsunmount(fs);
		safe_rmdir(fs->fs_mntpnt);
		REMQUE(tmpq, fs);
		INSQUE(fs_q, fs);
		free_filsys(fs);
	}
	/* success */
	return (1);

inuse:
	/* remount previous unmounted ones */
	newdevs = 0;
	for (fs = HEAD(struct filsys, tmpq); fs; fs = nextfs) {
		nextfs = NEXT(struct filsys, fs);
		if (fs->fs_unmounted) {
			olddev = fs->fs_mountdev;
			if (remount(fs) == NFS_OK) {
				fs->fs_unmounted = 0;
				if (fs->fs_mountdev != olddev)
					newdevs++;
			}
		}
	}

	/* remove entries with old dev ids */
	if (newdevs) {
		clean_mnttab(0, HEAD(struct filsys, tmpq));
		rootfs = HEAD(struct filsys, tmpq)->fs_rootfs;
	}

	/* put things back on the correct list */
	for (fs = HEAD(struct filsys, tmpq); fs; fs = nextfs) {
		nextfs = NEXT(struct filsys, fs);
		REMQUE(tmpq, fs);
		INSQUE(fs_q, fs);
	}

	/* put updated entries back */
	if (newdevs)
		addtomnttab(rootfs);

	return (0);
}

/*
 * Check a path prior to using it.  This avoids hangups in
 * the unmount system call from dead mount points in the 
 * path to the mount point we're trying to unmount.
 * If all the mount points ping OK then return 1.
 */
 int
 pathok(tmpq, tfs)
	struct q tmpq;
	struct filsys *tfs;
{
	struct filsys *fs, *nextfs;
	extern dev_t tmpdev;
	enum clnt_stat pingmount();

	while (tfs->fs_mntpntdev != tmpdev && tfs->fs_rootfs != tfs) {
		for (fs = HEAD(struct filsys, tmpq); fs; fs = nextfs) {
			nextfs = NEXT(struct filsys, fs);
			if (tfs->fs_mntpntdev == fs->fs_mountdev)
				break;
		}
		if (fs == NULL) {
			syslog(LOG_ERR,
			       "pathok: couldn't find devid %04x(%04x) for %s",
			       tfs->fs_mntpntdev & 0xFFF,
			       tmpdev & 0xFFF,
			       tfs->fs_mntpnt);
			return (1);
		}
		if (trace > 1)
			fprintf(stderr, "pathok: %s\n", fs->fs_mntpnt);
		if (pingmount(fs->fs_host) != RPC_SUCCESS) {
			if (trace > 1)
				fprintf(stderr, "pathok: %s is dead\n",
					fs->fs_mntpnt);
			return (0);
		}
		if (trace > 1)
			fprintf(stderr, "pathok: %s is OK\n",
				fs->fs_mntpnt);
		tfs = fs;
	}
	return (1);
}

void
freeex(ex)
	struct exports *ex;
{
	struct groups *groups, *tmpgroups;
	struct exports *tmpex;

	while (ex) {
		free(ex->ex_name);
		groups = ex->ex_groups;
		while (groups) {
			tmpgroups = groups->g_next;
			free((char *)groups);
			groups = tmpgroups;
		}
		tmpex = ex->ex_next;
		free((char *)ex);
		ex = tmpex;
	}
}

mkdir_r(dir)
	char *dir;
{
	int err;
	char *slash;

	if (mkdir(dir, 0555) == 0 || errno == EEXIST)
		return (0);
	if (errno != ENOENT)
		return (-1);
	slash = strrchr(dir, '/');
	if (slash == NULL)
		return (-1);
	*slash = '\0';
	err = mkdir_r(dir);
	*slash++ = '/';
	if (err || !*slash)
		return (err);
	return (mkdir(dir, 0555));
}

safe_rmdir(dir)
	char *dir;
{
	extern dev_t tmpdev;
	struct stat stbuf;

	if (stat(dir, &stbuf)) {
		return;
	}
	if (stbuf.st_dev == tmpdev) {
		(void) rmdir(dir);
	}
}
