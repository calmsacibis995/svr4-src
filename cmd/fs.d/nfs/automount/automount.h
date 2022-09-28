/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nfs.cmds:nfs/automount/automount.h	1.3.2.1"

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
#define MAXHOSTNAMELEN  64
#define MAXNETNAMELEN   255

#define	FH_HASH_SIZE	8
#define MNTTYPE_NFS	"nfs"

/*
 * General queue structure 
 */
struct q {
	struct q	*q_next;
#define	q_head	q_next
	struct q	*q_prev;
#define	q_tail	q_prev
};

#define	INSQUE(head, ptr) my_insque(&(head), &(ptr)->q)
#define	REMQUE(head, ptr) my_remque(&(head), &(ptr)->q)
#define HEAD(type, head) ((type *)(head.q_head))
#define NEXT(type, ptr)	((type *)(ptr->q.q_next))
#define	TAIL(type, head) ((type *)(head.q_tail))
#define PREV(type, ptr)	((type *)(ptr->q.q_prev))
	

/*
 * Types of filesystem entities (vnodes)
 * We support only one level of DIR; everything else is a symbolic LINK
 */
enum vn_type { VN_DIR, VN_LINK};
struct avnode {
	struct q q;
	nfs_fh	vn_fh;		/* fhandle */
	struct fattr vn_fattr;	/* file attributes */
	enum vn_type vn_type;	/* type of avnode */
	caddr_t	vn_data;	/* avnode private data */
};

struct avnode *fhtovn();		/* lookup avnode given fhandle */

/*
 * Structure describing a host/filesystem/dir tuple in a YP map entry
 */
struct mapfs {
	struct mapfs *mfs_next;		/* next in entry */
	int 	mfs_ignore;		/* ignore this entry */
	char	*mfs_host;		/* host name */
	char	*mfs_dir;		/* dir to mount */
	char	*mfs_subdir;		/* subdir of dir */
};

/*
 * YP entry - lookup of name in DIR gets us this
 */
struct mapent {
	char	*map_root;
	char	*map_mntpnt;
	char	*map_mntopts;
	struct mapfs *map_fs;
	struct mapent *map_next;
};
struct mapent *getmapent();

/*
 * Everthing we know about a mounted filesystem
 * Can include things not mounted by us (fs_mine == 0)
 */
struct filsys {
	struct q q;			/* next in q */
	int	fs_death;		/* time when no longer valid */
	int	fs_mine;		/* 1 if we mounted this fs */
	int	fs_present;		/* for checking unmounts */
	int 	fs_unmounted;		/* 1 if unmounted OK */
	char	*fs_type;		/* type of filesystem */
	char	*fs_host;		/* host name */
	char	*fs_dir;		/* dir of host mounted */
	char	*fs_mntpnt;		/* local mount point */
	char	*fs_opts;		/* mount options */
	dev_t	fs_mntpntdev;		/* device of mntpnt */
	dev_t	fs_mountdev;		/* device of mount */
	struct nfs_args fs_nfsargs;	/* nfs mount args */
	struct filsys *fs_rootfs;	/* root for this hierarchy */
	nfs_fh	fs_rootfh;		/* file handle for nfs mount */
	int	fs_mflags;		/* mount flags */
};
struct q fs_q;
struct filsys *already_mounted(), *alloc_fs();

/*
 * Structure for recently referenced links
 */
struct link {
	struct q q;		/* next in q */
	struct avnode link_vnode;	/* space for avnode */
	struct autodir *link_dir;	/* dir which we are part of */
	char	*link_name;	/* this name in dir */
	struct filsys *link_fs;	/* mounted file system */
	char	*link_path;	/* dir within file system */
	long	link_death;	/* time when no longer valid */
};
struct link *makelink();
struct link *findlink();
	
/*
 * Descriptor for each directory served by the automounter 
 */
struct autodir {
	struct q q;
	struct	avnode dir_vnode;	/* avnode */
	char	*dir_name;	/* mount point */
	char	*dir_map;	/* name of map for dir */
	char	*dir_opts;	/* default mount options */
	int	dir_remove;	/* remove mount point */
	struct q dir_head;
};
struct q dir_q;

char self[64];		/* my hostname */
char mydomain[64];	/* my domain name */
char mysite[64];	/* my site */
char tmpdir[200];	/* real name of /tmp */

time_t time_now;		/* set at start of processing of each RPC call */
int mount_timeout;	/* max seconds to wait for mount */
int max_link_time;	/* seconds to keep link around */
int nomounts;		/* don't do any mounts - for cautious servers */
nfsstat lookup(), nfsmount();
