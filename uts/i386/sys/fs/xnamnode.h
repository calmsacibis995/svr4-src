/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_XNAMNODE_H
#define _FS_XNAMNODE_H

#ident	"@(#)head.sys:sys/fs/xnamnode.h	1.5.2.1"
/*
 * The xnamnode represents a special XENIX file in any filesystem.  There is
 * one xnamnode for each active special XENIX file.  Filesystems that support
 * special files use xnamvp(vp, dev, type, cr) to convert a normal
 * vnode to a special vnode in the ops lookup() and create().
 *
 */

struct xsem {			/* XENIX semaphore */
	short  x_scount;	/* current semaphore count */
	short  x_eflag;		/* err flg */
	struct file *x_headw;	/* first waiter */
	struct file *x_tailw;	/* last waiter */
};

struct xsd {               /* XENIX shared data */
	union {
		struct xsd *x_chain; /* next available shared data structure */
		struct anon_map *xamp;  /* Pointer to shared data segment */
	} x_sun;
	unsigned x_len;	     /* limit of segment (seg size - 1) */
	short    x_snum;         /* serial # for sdgetv, sdwaitv */
	short    x_flags;        /* LOCKED, etc. */
}; 


struct xnamnode {
	struct	xnamnode *x_next;		/* must be first */
	struct	vnode x_vnode;		/* vnode associated with this xnamnode */
	struct	vnode *x_realvp;	/* vnode for the fs entry (if any) */
	ushort	x_flag;			/* flags, see below */
	dev_t	x_dev;			/* device the xnamnode represents */
	dev_t	x_fsid;			/* file system identifier */
	daddr_t	x_nextr;		/* next byte read offset (read-ahead) */
	long	x_size;			/* block device size in bytes */
	time_t  x_atime;		/* time of last access */
	time_t  x_mtime;		/* time of last modification */
	time_t  x_ctime;		/* time of last attributes change */
	int	x_count;		/* count of opened references */
	mode_t	x_mode;			/* file mode and type */
	uid_t	x_uid;			/* owner */
	gid_t	x_gid;			/* group */
	union x_u {
		struct  xsem *xsem;	/* ptr to XENIX semaphores */
		struct  xsd *xsd;	/* ptr to XENIX semaphores */
	} x_un;
	struct proc *x_powns;		/* XXX vm debugging */
};

#define x_sem	x_un.xsem	/* for v_type==VXNAM && v_rdev==XNAM_SEM */
#define x_sd	x_un.xsd	/* for v_type==VXNAM && v_rdev==XNAM_SD */

/* flags */
#define XNAMLOCKED		0x01		/* xnamnode is locked */
#define XNAMUPD		0x02		/* update device access time */
#define XNAMACC		0x04		/* update device modification time */
#define XNAMWANT		0x10		/* some process waiting on lock */
#define XNAMCHG		0x40		/* update device change time */

/* XENIX semaphore sub-types */
#define	XNAM_SEM		0x01
#define	XNAM_SD			0x02

/*
 * Convert between vnode and xnamnode
 */
#define	VTOXNAM(vp)	((struct xnamnode *)((vp)->v_data))
#define	XNAMTOV(xp)	(&(xp)->x_vnode)

extern struct proc *curproc;	/* XXX vm debugging */
/*
 * Lock and unlock xnamnodes.
 */
#define XNAMNLOCK(xp) { \
	while ((xp)->x_flag & XNAMLOCKED) { \
		(xp)->x_flag |= XNAMWANT; \
		(void) sleep((caddr_t)(xp), PINOD); \
	} \
	(xp)->x_flag |= XNAMLOCKED; \
	if (((xp)->x_vnode.v_flag & VISSWAP) != 0) { \
		curproc->p_swlocks++; \
		curproc->p_flag |= SSWLOCKS; \
	} \
	(xp)->x_powns = curproc; \
}

#define XNAMNUNLOCK(xp) { \
	(xp)->x_flag &= ~XNAMLOCKED; \
	if (((xp)->x_vnode.v_flag & VISSWAP) != 0) \
		if (--curproc->p_swlocks == 0) \
			curproc->p_flag &= ~SSWLOCKS; \
	if ((xp)->x_flag & XNAMWANT) { \
		(xp)->x_flag &= ~XNAMWANT; \
		wakeprocs((caddr_t)(xp), PRMPT); \
	} \
	(xp)->x_powns = NULL; \
}

/*
 * Construct an xnam vnode for a given device that shadows a particular
 * "real" vnode.
 */
extern struct vnode *xnamvp();


/*
 * Xnamnode lookup stuff.
 * These routines maintain a table of xnamnodes.
 */

#define	XNAMTABLESIZE	2
extern struct xnamnode *xnamtable[];

extern struct vnodeops xnam_vnodeops;

#endif	/* _FS_XNAMNODE_H */
