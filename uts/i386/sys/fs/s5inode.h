/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_S5INODE_H
#define _FS_S5INODE_H

#ident	"@(#)head.sys:sys/fs/s5inode.h	11.30.4.3"

#include <sys/proc.h>	/* XXX -- needed for user-context kludge in ILOCK */
#include <sys/disp.h>	/* XXX */

#define	NADDR	13
#define	NSADDR	(NADDR*sizeof(daddr_t)/sizeof(short))

struct inode {
	struct	inode *i_forw;	/* inode hash chain */
	struct	inode *i_back;	/* " */
	struct	inode *av_forw;	/* freelist chain */
	struct	inode *av_back;	/* " */
	u_short	i_flag;		/* flags */
	o_ino_t	i_number;	/* inode number */
	dev_t	i_dev;		/* device where inode resides */
	o_mode_t i_mode;	/* file mode and type */
	o_uid_t	i_uid;		/* owner */
	o_gid_t	i_gid;		/* group */
	o_nlink_t i_nlink;	/* number of links */
	off_t	i_size;		/* size in bytes */
	time_t	i_atime;	/* last access time */
	time_t	i_mtime;	/* last modification time */
	time_t	i_ctime;	/* last "inode change" time */
	daddr_t	i_addr[NADDR];	/* block address list */
	short	i_nilocks;	/* XXX -- count of recursive ilocks */
	short	i_owner;	/* XXX -- proc slot of ilock owner */
	daddr_t	i_nextr;	/* next byte read offset (read-ahead) */
	u_char 	i_gen;		/* generation number */
	long    i_mapcnt;       /* number of mappings of pages */
	u_long	i_vcode;	/* version code attribute */
	struct vnode i_vnode;	/* Contains an instance of a vnode */
	int	*i_map;		/* block list for the corresponding file */
	dev_t	i_rdev;		/* rdev field for block/char specials */

	int	i_mapsz;	/* kmem_alloc'ed size */
	int	i_oldsz;	/* i_size when you did the allocation */
};

#define	i_oldrdev	i_addr[0]
#define i_bcflag	i_addr[1]	/* block/char special flag occupies
					** bytes 3-5 in di_addr
					*/
#define NDEVFORMAT	0x1	/* device number stored in new area */
#define i_major		i_addr[2] /* major component occupies bytes 6-8 in di_addr */
#define i_minor		i_addr[3] /* minor component occupies bytes 9-11 in di_addr */

typedef struct inode inode_t;

extern struct inode *inode;
extern long int ninode;

struct	ifreelist {	/* must match struct inode */
	struct inode	*pad[2];
	struct inode	*av_forw;
	struct inode	*av_back;
};

extern struct ifreelist ifreelist;

/* Flags */

#define	ILOCKED	0x0001		/* inode is locked */
#define	IUPD	0x0002		/* file has been modified */
#define	IACC	0x0004		/* inode access time to be updated */
#define	IWANT	0x0010		/* some process waiting on lock */
#define	ICHG	0x0040		/* inode has been changed */
#define	ISYN	0x0080		/* do synchronous write for iupdat */
#define	IMOD	0x0100		/* inode times have been modified */
#define	INOACC	0x0200		/* no access time update in getpage */
#define	ISYNC	0x0400		/* do all block allocation synchronously */
#define	IMODTIME 0x0800		/* mod time already set */
#define	IRWLOCKED	0x1000		/* inode is rwlocked */
#define	IINACTIVE	0x2000		/* iinactive in progress */

/*
 * File types.
 */

#define	IFMT	0xF000		/* type of file */
#define		IFIFO	0x1000	/* fifo special */
#define		IFCHR	0x2000	/* character special */
#define		IFDIR	0x4000	/* directory */
#define		IFNAM	0x5000	/* XENIX special named file */
#define		IFBLK	0x6000	/* block special */
#define		IFREG	0x8000	/* regular */
#define		IFLNK	0xA000	/* symbolic link */

/*
 * File modes.
 */
#define	ISUID	VSUID		/* set user id on execution */
#define	ISGID	VSGID		/* set group id on execution */
#define ISVTX	VSVTX		/* save swapped text even after use */

/*
 * Permissions.
 */
#define	IREAD		VREAD	/* read permission */
#define	IWRITE		VWRITE	/* write permission */
#define	IEXEC		VEXEC	/* execute permission */

#ifdef _KERNEL

extern int iget(), ialloc();
extern void iinactive();

extern int bmap(), bmapalloc();

/*
 * inode-to-vnode conversion.
 */
#define	ITOV(ip)	((struct vnode *)&(ip)->i_vnode)
#define VTOI(vp)	((struct inode *)(vp)->v_data)

#define ESAME	(-1)		/* Special KLUDGE error for rename */

#define	S5_HOLE	(-1)		/* Value used when no block allocated */

enum de_op	{ DE_CREATE, DE_MKDIR, DE_LINK, DE_RENAME }; /* direnter ops */
enum dr_op	{ DR_REMOVE, DR_RMDIR, DR_RENAME }; /* dirremove ops */

/*
 * This overlays the fid structure (see vfs.h).
 */
struct ufid {
	u_short	ufid_len;
	o_ino_t	ufid_ino;
	long	ufid_gen;
};

/*
 * S5 VFS private data.
 */
struct s5vfs {
	struct vnode	*vfs_root;	/* root vnode */
	struct buf	*vfs_bufp;	/* buffer containing superblock */
	struct vnode	*vfs_devvp;	/* block device vnode */
	long		vfs_nindir;	/* bsize/sizeof(daddr_t) */
	long		vfs_inopb;	/* bsize/sizeof(dinode) */
	long		vfs_bmask;	/* bsize-1 */
	long		vfs_nmask;	/* nindir-1 */
	long		vfs_ltop;	/* ltop or ptol shift constant */
	long		vfs_bshift;	/* log2(bsize) */
	long		vfs_nshift;	/* log2(nindir) */
	long		vfs_inoshift;	/* log2(inopb) */
};

#define S5VFS(vfsp) ((struct s5vfs *)((vfsp)->vfs_data))

/*
 * Lock and unlock inodes.
 *
 * XXX -- Uses process context.  Rewrite to remove this.
 */
#define	IRWLOCK(ip) { \
	while ((ip)->i_flag & IRWLOCKED) { \
		(ip)->i_flag |= IWANT; \
		(void) sleep((caddr_t)(ip), PINOD); \
	} \
	(ip)->i_flag |= IRWLOCKED; \
	if ((ip)->i_vnode.v_flag & VISSWAP) { \
		curproc->p_swlocks++; \
		curproc->p_flag |= SSWLOCKS; \
	} \
}

#define	IRWUNLOCK(ip) { \
	ASSERT((ip)->i_flag & IRWLOCKED); \
	if ((ip)->i_vnode.v_flag & VISSWAP) { \
		if (--curproc->p_swlocks == 0) \
			curproc->p_flag &= ~SSWLOCKS; \
	} \
	(ip)->i_flag &= ~IRWLOCKED; \
	if ((ip)->i_flag & IWANT) { \
		(ip)->i_flag &= ~IWANT; \
		wakeprocs((caddr_t)(ip), PRMPT); \
	} \
}

#define	ILOCK(ip) { \
	while (((ip)->i_flag & ILOCKED) && (ip)->i_owner != curproc->p_slot) { \
		(ip)->i_flag |= IWANT; \
		(void) sleep((caddr_t)(ip), PINOD); \
	} \
	(ip)->i_owner = curproc->p_slot; \
	(ip)->i_nilocks++; \
	(ip)->i_flag |= ILOCKED; \
	if ((ip)->i_vnode.v_flag & VISSWAP) { \
		curproc->p_swlocks++; \
		curproc->p_flag |= SSWLOCKS; \
	} \
}

#define	IUNLOCK(ip) { \
	ASSERT((ip)->i_flag & ILOCKED); \
	--(ip)->i_nilocks; \
	ASSERT((ip)->i_nilocks >= 0); \
	if ((ip)->i_vnode.v_flag & VISSWAP) { \
		if (--curproc->p_swlocks == 0) \
			curproc->p_flag &= ~SSWLOCKS; \
	} \
	if ((ip)->i_nilocks == 0) { \
		(ip)->i_flag &= ~ILOCKED; \
		if ((ip)->i_flag & IWANT) { \
			(ip)->i_flag &= ~IWANT; \
			wakeprocs((caddr_t)(ip), PRMPT); \
		} \
	} \
}

#define ITIMES(ip) { \
	if ((ip)->i_flag & (IUPD|IACC|ICHG)) { \
		(ip)->i_flag |= IMOD; \
		if ((ip)->i_flag & IACC) \
			(ip)->i_atime = hrestime.tv_sec; \
		if ((ip)->i_flag & IUPD) {\
			(ip)->i_mtime = hrestime.tv_sec; \
			(ip)->i_flag |= IMODTIME; \
		} \
		if ((ip)->i_flag & ICHG) \
			(ip)->i_ctime = hrestime.tv_sec; \
		(ip)->i_flag &= ~(IACC|IUPD|ICHG); \
	} \
}

#endif

#ifndef NADDR
#define NADDR 13
#endif

#endif	/* _FS_S5INODE_H */
