/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_SNODE_H
#define _FS_SNODE_H

#ident	"@(#)head.sys:sys/fs/snode.h	1.21.3.1"
/*
 * The snode represents a special file in any filesystem.  There is
 * one snode for each active special file.  Filesystems that support
 * special files use specvp(vp, dev, type, cr) to convert a normal
 * vnode to a special vnode in the ops lookup() and create().
 *
 * To handle having multiple snodes that represent the same
 * underlying device vnode without cache aliasing problems,
 * the s_commonvp is used to point to the "common" vnode used for
 * caching data.  If an snode is created internally by the kernel,
 * then the s_realvp field is NULL and s_commonvp points to s_vnode.
 * The other snodes which are created as a result of a lookup of a
 * device in a file system have s_realvp pointing to the vp which
 * represents the device in the file system while the s_commonvp points
 * into the "common" vnode for the device in another snode.
 */

struct snode {
	struct	snode *s_next;		/* must be first */
	struct	vnode s_vnode;		/* vnode associated with this snode */
	struct	vnode *s_realvp;	/* vnode for the fs entry (if any) */
	struct	vnode *s_commonvp;	/* common device vnode */
	ushort	s_flag;			/* flags, see below */
	dev_t	s_dev;			/* device the snode represents */
	dev_t	s_fsid;			/* file system identifier */
	daddr_t	s_nextr;		/* next byte read offset (read-ahead) */
	long	s_size;			/* block device size in bytes */
	time_t  s_atime;		/* time of last access */
	time_t  s_mtime;		/* time of last modification */
	time_t  s_ctime;		/* time of last attributes change */
	int	s_count;		/* count of opened references */
        long    s_mapcnt;               /* count of mappings of pages */
	long	s_pad1;			/* reserved for security */
	long	s_pad2;			/* reserved for security */
	long	s_pad3;			/* reserved for security */
	struct proc *s_powns;		/* XXX vm debugging */
};

/* flags */
#define SLOCKED		0x01		/* snode is locked */
#define SUPD		0x02		/* update device access time */
#define SACC		0x04		/* update device modification time */
#define SWANT		0x10		/* some process waiting on lock */
#define SCHG		0x40		/* update device change time */

/*
 * Convert between vnode and snode
 */
#define	VTOS(vp)	((struct snode *)((vp)->v_data))
#define	STOV(sp)	(&(sp)->s_vnode)

extern struct proc *curproc;		/* XXX vm debugging */
/*
 * Lock and unlock snodes.
 */
#define SNLOCK(sp) { \
	while ((sp)->s_flag & SLOCKED) { \
		(sp)->s_flag |= SWANT; \
		(void) sleep((caddr_t)(sp), PINOD); \
	} \
	(sp)->s_flag |= SLOCKED; \
	if (((sp)->s_vnode.v_flag & VISSWAP) != 0) { \
		curproc->p_swlocks++; \
		curproc->p_flag |= SSWLOCKS; \
	} \
	(sp)->s_powns = curproc; \
}

#define SNUNLOCK(sp) { \
	(sp)->s_flag &= ~SLOCKED; \
	if (((sp)->s_vnode.v_flag & VISSWAP) != 0) \
		if (--curproc->p_swlocks == 0) \
			curproc->p_flag &= ~SSWLOCKS; \
	if ((sp)->s_flag & SWANT) { \
		(sp)->s_flag &= ~SWANT; \
		wakeprocs((caddr_t)(sp), PRMPT); \
	} \
	(sp)->s_powns = NULL; \
}

/*
 * Construct a spec vnode for a given device that shadows a particular
 * "real" vnode.
 */
extern struct vnode *specvp();

/*
 * Construct a spec vnode for a given device that shadows nothing.
 */
extern struct vnode *makespecvp();

/*
 * Find any other spec vnode that refers to the same device as another vnode.
 */
extern struct vnode *other_specvp();

/*
 * Convert a device vnode pointer into a common device vnode pointer.
 */
extern struct vnode *common_specvp();

/*
 * Wait for access to a file, or signal other processes waiting for access.
 */
extern int openwait();
extern int openprivwait();
extern int setprivwait();
extern void privsig();

/* XENIX Support */
extern int spec_rdchk();
/* End XENIX Support */

/*
 * If driver does not have a size routine (e.g. old drivers), the size of the
 * device is assumed to be infinite.
 */
#define UNKNOWN_SIZE 	0x7fffffff

/*
 * Snode lookup stuff.
 * These routines maintain a table of snodes hashed by dev so
 * that the snode for an dev can be found if it already exists.
 * NOTE: STABLESIZE must be a power of 2 for STABLEHASH to work!
 */

#define	STABLESIZE	16
#define	STABLEHASH(dev)	((getmajor(dev) + getminor(dev)) & (STABLESIZE - 1))
extern struct snode *stable[];

extern struct vnodeops spec_vnodeops;

#endif	/* _FS_SNODE_H */
