/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nfs.cmds:nfs/automount/auto_proc.c	1.3.2.1"

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
#include <syslog.h>
#include <sys/types.h>
#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/auth_unix.h>
#include <rpc/xdr.h>
#include <netinet/in.h>
#include "nfs_prot.h"
#define NFSCLIENT
typedef nfs_fh fhandle_t;
#include <nfs/mount.h>
#include "automount.h"

/*
 * add up sizeof (valid + fileid + name + cookie) - strlen(name)
 */
#define ENTRYSIZE (3 * BYTES_PER_XDR_UNIT + NFS_COOKIESIZE)
/*
 * sizeof(status + eof)
 */
#define JUNKSIZE (2 * BYTES_PER_XDR_UNIT)

attrstat *
nfsproc_getattr_2(fh)
	nfs_fh *fh;
{
	struct avnode *avnode;
	static attrstat astat;

	avnode = fhtovn(fh);
	if (avnode == NULL) {
		astat.status = NFSERR_STALE;
		return (&astat);
	}
	astat.status = NFS_OK;
	astat.attrstat_u.attributes = avnode->vn_fattr;
	return (&astat);
}

attrstat *
nfsproc_setattr_2(args)
	sattrargs *args;
{
	static attrstat astat;
	struct avnode *avnode;

	avnode = fhtovn(&args->file);
	if (avnode == NULL)
		astat.status = NFSERR_STALE;
	else
		astat.status = NFSERR_ROFS;
	return (&astat);
}

void *
nfsproc_root_2()
{
	return (NULL);
}


diropres *
nfsproc_lookup_2(args, cred)
	diropargs *args;
	struct authunix_parms *cred;
{
	struct avnode *avnode;
	static diropres res;
	nfsstat status;

	avnode = fhtovn(&args->dir);
	if (avnode == NULL) {
		res.status = NFSERR_STALE;
		return (&res);
	}
	if (avnode->vn_type != VN_DIR) {
		res.status = NFSERR_NOTDIR;
		return (&res);
	}
	status = lookup((struct autodir *)(avnode->vn_data),
		args->name, &avnode, cred);
	if (status != NFS_OK) {
		res.status = status;
		return (&res);
	}
	res.diropres_u.diropres.file = avnode->vn_fh;
	res.diropres_u.diropres.attributes = avnode->vn_fattr;
	res.status = NFS_OK;
	return (&res);
}
			
	
readlinkres *
nfsproc_readlink_2(fh, cred)
	nfs_fh *fh;
	struct authunix_parms *cred;
{
	struct avnode *avnode;
	struct link *link;
	static readlinkres res;
	nfsstat status;

	avnode = fhtovn(fh);
	if (avnode == NULL) {
		res.status = NFSERR_STALE;
		return (&res);
	}
	if (avnode->vn_type != VN_LINK) {
		res.status = NFSERR_STALE;	/* XXX: no NFSERR_INVAL */
		return (&res);
	}
	link = (struct link *)(avnode->vn_data);
	if (time_now >= link->link_death) {
		status = lookup(link->link_dir, link->link_name, &avnode, cred);
		if (status != NFS_OK) {
			free_link(link);
			res.status = status;
			return (&res);
		}
		link = (struct link *)(avnode->vn_data);
	}

	link->link_death = time_now + max_link_time;
	if (link->link_fs)
		link->link_fs->fs_rootfs->fs_death = time_now + max_link_time;
	res.readlinkres_u.data = link->link_path;
	res.status = NFS_OK;
	return (&res);
}

/*ARGSUSED*/
readres *
nfsproc_read_2(args)
	readargs *args;
{
	static readres res;

	res.status = NFSERR_ISDIR;	/* XXX: should return better error */
	return (&res);
}

void *
nfsproc_writecache_2()
{
	return (NULL);
}	

/*ARGSUSED*/
attrstat *
nfsproc_write_2(args)
	writeargs *args;
{
	static attrstat res;

	res.status = NFSERR_ROFS;	/* XXX: should return better error */
	return (&res);
}

/*ARGSUSED*/
diropres *
nfsproc_create_2(args, cred)
	createargs *args;
	struct authunix_parms *cred;
{
	static diropres res;

	res.status = NFSERR_ROFS;
	return (&res);
}

/*ARGSUSED*/
nfsstat *
nfsproc_remove_2(args)
	diropargs *args;
{
	static nfsstat status;

	status = NFSERR_ROFS;
	return (&status);
}

/*ARGSUSED*/
nfsstat *
nfsproc_rename_2(args)
	renameargs *args;
{
	static nfsstat status;

	status = NFSERR_ROFS;
	return (&status);
}

/*ARGSUSED*/
nfsstat *
nfsproc_link_2(args)
	linkargs *args;
{
	static nfsstat status;

	status = NFSERR_ROFS;
	return (&status);
}

/*ARGSUSED*/
nfsstat *
nfsproc_symlink_2(args, cred)
	symlinkargs *args;
	struct authunix_parms *cred;
{
	static nfsstat status;

	status = NFSERR_ROFS;
	return (&status);
}

/*ARGSUSED*/
diropres *
nfsproc_mkdir_2(args, cred)
	createargs *args;
	struct authunix_parms *cred;
{
	static diropres res;

	res.status = NFSERR_ROFS;
	return (&res);
}

/*ARGSUSED*/
nfsstat *
nfsproc_rmdir_2(args)
	diropargs *args;	
{
	static nfsstat status;

	status = NFSERR_ROFS;
	return (&status);
}

readdirres *
nfsproc_readdir_2(args)
	readdirargs *args;
{
	static readdirres res;

	struct avnode *avnode;
	struct entry *e, *nexte;
	struct entry **entp;
	struct autodir *dir;
	struct link *link;
	int cookie;
	int count;
	int entrycount;

	/*
	 * Free up old stuff
	 */
	e = res.readdirres_u.reply.entries;
	while (e != NULL) {
		nexte = e->nextentry;
		free((char *)e);
		e = nexte;
	}
	res.readdirres_u.reply.entries = NULL;

	avnode = fhtovn(&args->dir);
	if (avnode == NULL) {
		res.status = NFSERR_STALE;
		return (&res);
	}
	if (avnode->vn_type != VN_DIR) {
		res.status = NFSERR_NOTDIR;
		return (&res);
	}
	dir = (struct autodir *)avnode->vn_data;
	cookie = *(unsigned *)args->cookie;
	count = args->count - JUNKSIZE;

	entrycount = 0;
	entp = &res.readdirres_u.reply.entries;
	for (link = HEAD(struct link, dir->dir_head); link;
	    link = NEXT(struct link, link)) {
		if (count <= ENTRYSIZE) 
			goto full;
		if (entrycount++ < cookie)
			continue;
		if (link->link_death && time_now >= link->link_death)
			continue;
		*entp = (struct entry *) malloc(sizeof(entry));
		if (*entp == NULL) {
			syslog(LOG_ERR, "Memory allocation failed: %m");
			break;
		}
		(*entp)->fileid = link->link_vnode.vn_fattr.fileid;
		if (link->link_death && time_now >= link->link_death)
			(*entp)->fileid = 0;
		else
			(*entp)->fileid = link->link_vnode.vn_fattr.fileid;
		(*entp)->name = link->link_name;
		*(unsigned *)((*entp)->cookie) = ++cookie;
		(*entp)->nextentry = NULL;
		entp = &(*entp)->nextentry;
		count -= (ENTRYSIZE + strlen(link->link_name));
	}
full:
	if (count > ENTRYSIZE)
		res.readdirres_u.reply.eof = TRUE;
	else
		res.readdirres_u.reply.eof = FALSE;
	res.status = NFS_OK;
	return (&res);
}
		
statfsres *
nfsproc_statfs_2()
{
	static statfsres res;

	res.status = NFS_OK;
	res.statfsres_u.reply.tsize = 512;
	res.statfsres_u.reply.bsize = 512;
	res.statfsres_u.reply.blocks = 0;
	res.statfsres_u.reply.bfree = 0;
	res.statfsres_u.reply.bavail = 0;
	return (&res);
}
