/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)head.sys:sys/fs/ufs_inode.h	1.17.4.3"

#ifndef _SYS_FS_UFS_INODE_H
#define _SYS_FS_UFS_INODE_H



/*
 * The I node is the focus of all local file activity in UNIX.
 * There is a unique inode allocated for each active file,
 * each current directory, each mounted-on file, each mapping,
 * and the root.  An inode is `named' by its dev/inumber pair.
 * Data in icommon is read in from permanent inode on volume.
 */

#define EFT_MAGIC 0x90909090	/* magic cookie for EFT */
#define	NDADDR	12		/* direct addresses in inode */
#define	NIADDR	3		/* indirect addresses in inode */

struct inode {
	struct	inode *i_chain[2];	/* must be first */
	struct	vnode i_vnode;	/* vnode associated with this inode */
	struct	vnode *i_devvp;	/* vnode for block I/O */
	u_short	i_flag;
	dev_t	i_dev;		/* device where inode resides */
	ino_t	i_number;	/* i number, 1-to-1 with device address */
	off_t	i_diroff;	/* offset in dir, where we found last entry */
	struct	fs *i_fs;	/* file sys associated with this inode */
	struct	dquot *i_dquot;	/* quota structure controlling this file */
	short	i_owner;	/* proc index of process locking inode */
	short	i_count;	/* number of inode locks for i_owner */
	daddr_t	i_nextr;	/* next byte read offset (read-ahead) */
	struct inode  *i_freef;	/* free list forward */
	struct inode **i_freeb;	/* free list back */
	ulong	i_vcode;	/* version code attribute */
	long	i_mapcnt;	/* mappings to file pages */
	int	*i_map;		/* block list for the corresponding file */

	int	i_mapsz;	/* kmem_alloc'ed size */
	int	i_oldsz;	/* i_size when you did the allocation */

	struct 	icommon {
		o_mode_t ic_smode;	/*  0: mode and type of file */
		short	ic_nlink;	/*  2: number of links to file */
		o_uid_t	ic_suid;		/*  4: owner's user id */
		o_gid_t	ic_sgid;		/*  6: owner's group id */
		quad	ic_size;	/*  8: number of bytes in file */
#ifdef _KERNEL
		struct timeval ic_atime;/* 16: time last accessed */
		struct timeval ic_mtime;/* 24: time last modified */
		struct timeval ic_ctime;/* 32: last time inode changed */
#else
		time_t	ic_atime;	/* 16: time last accessed */
		long	ic_atspare;
		time_t	ic_mtime;	/* 24: time last modified */
		long	ic_mtspare;
		time_t	ic_ctime;	/* 32: last time inode changed */
		long	ic_ctspare;
#endif
		daddr_t	ic_db[NDADDR];	/* 40: disk block addresses */
		daddr_t	ic_ib[NIADDR];	/* 88: indirect blocks */
		long	ic_flags;	/* 100: status, currently unused */
		long	ic_blocks;	/* 104: blocks actually held */
		long	ic_gen;		/* 108: generation number */
		mode_t	ic_mode;	/* 112: EFT version of mode*/
		uid_t	ic_uid;		/* 116: EFT version of uid */
		gid_t	ic_gid;		/* 120: EFT version of gid */
		ulong	ic_eftflag;	/* 124: indicate EFT version*/

	} i_ic;
};

struct dinode {
	union {
		struct	icommon di_icom;
		char	di_size[128];
	} di_un;
};
#define i_mode		i_ic.ic_mode
#define	i_nlink		i_ic.ic_nlink
#define i_uid		i_ic.ic_uid
#define i_gid		i_ic.ic_gid
#define i_smode		i_ic.ic_smode
#define i_suid		i_ic.ic_suid
#define i_sgid		i_ic.ic_sgid
#define i_eftflag	i_ic.ic_eftflag

/* ugh! -- must be fixed */
#if defined(vax) || defined(i386)
#define	i_size		i_ic.ic_size.val[0]
#endif
#if defined(mc68000) || defined(sparc) || defined(u3b2) || defined(u3b15)
#define	i_size		i_ic.ic_size.val[1]
#endif
#define	i_db		i_ic.ic_db
#define	i_ib		i_ic.ic_ib

#define	i_atime		i_ic.ic_atime
#define	i_mtime		i_ic.ic_mtime
#define	i_ctime		i_ic.ic_ctime

#define i_blocks	i_ic.ic_blocks
#define	i_oldrdev	i_ic.ic_db[0]
#define	i_rdev		i_ic.ic_db[1]
#define	i_gen		i_ic.ic_gen
#define	i_forw		i_chain[0]
#define	i_back		i_chain[1]

#define di_ic		di_un.di_icom
#define	di_mode		di_ic.ic_mode
#define	di_nlink	di_ic.ic_nlink
#define	di_uid		di_ic.ic_uid
#define	di_gid		di_ic.ic_gid
#define di_smode	di_ic.ic_smode
#define di_suid		di_ic.ic_suid
#define di_sgid		di_ic.ic_sgid
#define di_eftflag	di_ic.ic_eftflag

#if defined(vax) || defined(i386)
#define	di_size		di_ic.ic_size.val[0]
#endif
#if defined(mc68000) || defined(sparc) || defined(u3b2) || defined(u3b15)
#define	di_size		di_ic.ic_size.val[1]
#endif
#define	di_db		di_ic.ic_db
#define	di_ib		di_ic.ic_ib

#define	di_atime	di_ic.ic_atime
#define	di_mtime	di_ic.ic_mtime
#define	di_ctime	di_ic.ic_ctime

#define	di_oldrdev	di_ic.ic_db[0]
#define	di_rdev		di_ic.ic_db[1]
#define	di_blocks	di_ic.ic_blocks
#define	di_gen		di_ic.ic_gen

/* flags */
#define	ILOCKED		0x001		/* inode is locked */
#define	IUPD		0x002		/* file has been modified */
#define	IACC		0x004		/* inode access time to be updated */
#define	IMOD		0x008		/* inode has been modified */
#define	IWANT		0x010		/* some process waiting on lock */
#define	ISYNC		0x020		/* do all allocation synchronously */
#define	ICHG		0x040		/* inode has been changed */
#define	ILWAIT		0x080		/* someone waiting on file lock */
#define	IREF		0x100		/* inode is being referenced */
#define	INOACC		0x200		/* no access time update in getpage */
#define	IMODTIME	0x400		/* mod time already set */
#define	IINACTIVE	0x800		/* iinactive in progress */
#define	IRWLOCKED	0x1000		/* inode is rwlocked */

/* modes */
#define	IFMT		0170000		/* type of file */
#define	IFIFO		0010000		/* named pipe (fifo) */
#define	IFCHR		0020000		/* character special */
#define	IFDIR		0040000		/* directory */
#define	IFBLK		0060000		/* block special */
#define	IFREG		0100000		/* regular */
#define	IFLNK		0120000		/* symbolic link */
#define	IFSOCK		0140000		/* socket */

#define	ISUID		04000		/* set user id on execution */
#define	ISGID		02000		/* set group id on execution */
#define	ISVTX		01000		/* save swapped text even after use */
#define	IREAD		0400		/* read, write, execute permissions */
#define	IWRITE		0200
#define	IEXEC		0100


#ifdef _KERNEL
struct inode *ufs_inode;		/* the inode table itself */
struct inode *inodeNINODE;		/* the end of the inode table */
extern int ufs_ninode;			/* number of slots in the table */

extern struct vnodeops ufs_vnodeops;	/* vnode operations for ufs */

extern ino_t dirpref();
extern daddr_t blkpref();

/*
 * A struct fbuf is used to get a mapping to part of a file using the
 * segkmap facilities.  After you get a mapping, you can fbrelse() it
 * (giving a seg code to pass back to segmap_release), you can fbwrite()
 * it (causes a synchronous write back using inode mapping information),
 * or you can fbiwrite it (causing indirect synchronous write back to
 * the block number given without using inode mapping information).
 */

struct fbuf {
	addr_t	fb_addr;
	u_int	fb_count;
};

extern int fbread(/* vp, off, len, rw, fbpp */);
extern void fbzero(/* vp, off, len, fbpp */);
extern int fbwrite(/* fb */);
extern int fbiwrite(/* fb, ip, bn */);
extern void fbrelse(/* fb, rw */);

/*
 * Convert between inode pointers and vnode pointers
 */
#define VTOI(VP)	((struct inode *)(VP)->v_data)
#define ITOV(IP)	((struct vnode *)&(IP)->i_vnode)

#ifdef notneeded
Look at sys/mode.h and os/vnode.c
/*
 * Convert between vnode types and inode formats
 */
extern enum vtype	iftovt_tab[];
extern int		vttoif_tab[];
#define IFTOVT(M)	(iftovt_tab[((M) & IFMT) >> 13])
#define VTTOIF(T)	(vttoif_tab[(int)(T)])

#define MAKEIMODE(T, M)	(VTTOIF(T) | (M))
#endif

/*
 * Lock and unlock inodes.
 */
#define	IRWLOCK(ip) { \
	while ((ip)->i_flag & IRWLOCKED) { \
		(ip)->i_flag |= IWANT; \
		(void) sleep((caddr_t)(ip), PINOD); \
	} \
	(ip)->i_flag |= IRWLOCKED; \
	if (((ip)->i_vnode.v_flag & VISSWAP) != 0){ \
		curproc->p_swlocks++;	\
		curproc->p_flag |= SSWLOCKS;\
	} \
}

#define	IRWUNLOCK(ip) { \
	ASSERT((ip)->i_flag & IRWLOCKED); \
	if (((ip)->i_vnode.v_flag & VISSWAP) != 0) \
		if (--curproc->p_swlocks == 0)  \
			curproc->p_flag &= ~SSWLOCKS; \
	(ip)->i_flag &= ~IRWLOCKED; \
	if ((ip)->i_flag & IWANT) { \
		(ip)->i_flag &= ~IWANT; \
		wakeprocs((caddr_t)(ip), PRMPT); \
	} \
}

#define	ILOCK(ip) { \
	while (((ip)->i_flag & ILOCKED) && \
	    (ip)->i_owner != curproc->p_slot) { \
		(ip)->i_flag |= IWANT; \
		(void) sleep((caddr_t)(ip), PINOD); \
	} \
	(ip)->i_owner = curproc->p_slot; \
	(ip)->i_count++; \
	(ip)->i_flag |= ILOCKED; \
	if (((ip)->i_vnode.v_flag & VISSWAP) != 0){ \
		curproc->p_swlocks++;	\
		curproc->p_flag |= SSWLOCKS;\
	} \
}

#define	IUNLOCK(ip) { \
	if (--(ip)->i_count < 0) \
		panic("IUNLOCK"); \
	if (((ip)->i_vnode.v_flag & VISSWAP) != 0) \
		if (--curproc->p_swlocks == 0)  \
			curproc->p_flag &= ~SSWLOCKS; \
	if ((ip)->i_count == 0) { \
		(ip)->i_flag &= ~ILOCKED; \
		if ((ip)->i_flag & IWANT) { \
			(ip)->i_flag &= ~IWANT; \
			wakeprocs((caddr_t)(ip), PRMPT); \
		} \
	} \
}

#define IUPDAT(ip, waitfor) { \
	if (ip->i_flag & (IUPD|IACC|ICHG|IMOD)) \
		ufs_iupdat(ip, waitfor); \
}

/*
 * Mark an inode with the current (unique) timestamp.
 */
struct timeval iuniqtime;

#define IMARK(ip) { \
	if (hrestime.tv_sec > iuniqtime.tv_sec || \
		hrestime.tv_nsec/1000 > iuniqtime.tv_usec) { \
		iuniqtime.tv_sec = hrestime.tv_sec; \
		iuniqtime.tv_usec = hrestime.tv_nsec/1000; \
	} else { \
		iuniqtime.tv_usec++; \
	} \
	if ((ip)->i_flag & IACC) \
		(ip)->i_atime = iuniqtime; \
	if ((ip)->i_flag & IUPD) { \
		(ip)->i_mtime = iuniqtime; \
		(ip)->i_flag |= IMODTIME; \
	} \
	if ((ip)->i_flag & ICHG) { \
		ip->i_diroff = 0; \
		(ip)->i_ctime = iuniqtime; \
	} \
}

#define ITIMES(ip) { \
	if ((ip)->i_flag & (IUPD|IACC|ICHG)) { \
		(ip)->i_flag |= IMOD; \
		IMARK(ip); \
		(ip)->i_flag &= ~(IACC|IUPD|ICHG); \
	} \
}

/*
 * Allocate the specified block in the inode
 * and make sure any in-core pages are initialized.
 */
#define	BMAPALLOC(ip, lbn, size) \
	ufs_bmap((ip), (lbn), (daddr_t *)NULL, (daddr_t *)NULL, (size), S_WRITE, 0)

#define ESAME	(-1)		/* trying to rename linked files (special) */

/*
 * Check that file is owned by current user or user is su.
 */
#define OWNER(CR, IP)	(((CR)->cr_uid == (IP)->i_uid)? 0: (suser(CR)? 0: EPERM))

#define	UFS_HOLE	(daddr_t)-1	/* value used when no block allocated */

/*
 * enums
 */
enum de_op	{ DE_CREATE, DE_MKDIR, DE_LINK, DE_RENAME };	/* direnter ops */
enum dr_op	{ DR_REMOVE, DR_RMDIR, DR_RENAME };	/* dirremove ops */

/*
 * This overlays the fid structure (see vfs.h)
 */
struct ufid {
	u_short	ufid_len;
	ino_t	ufid_ino;
	long	ufid_gen;
};

/*
 * UFS VFS private data.
 */
struct ufsvfs {
	struct vnode	*vfs_root;	/* root vnode */
	struct buf	*vfs_bufp;	/* buffer containing superblock */
	struct vnode	*vfs_devvp;	/* block device vnode */
	struct inode	*vfs_qinod;	/* QUOTA: pointer to quota file */
	u_short		vfs_qflags;	/* QUOTA: filesystem flags */
	u_long		vfs_btimelimit;	/* QUOTA: block time limit */
	u_long		vfs_ftimelimit;	/* QUOTA: file time limit */
};

#endif

#endif /* _SYS_FS_UFS_INODE_H */
