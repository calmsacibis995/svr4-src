/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_NAMENODE_H
#define _FS_NAMENODE_H

#ident	"@(#)head.sys:sys/fs/namenode.h	1.4.3.1"

/*
 * This structure is used to pass a file descriptor from user
 * level to the kernel. It is first used by fattach() and then
 * be NAMEFS.
 */
struct namefd {
	int fd;
};

/*
 * Each NAMEFS object is identified by a struct namenode/vnode pair.
 */
struct namenode {
	struct vnode    nm_vnode;	/* represents mounted file desc. */
	ushort		nm_flag;	/* flags defined below */
	struct vattr    nm_vattr;	/* attributes of mounted file desc. */
	struct vnode	*nm_filevp;	/* file desc. prior to mounting */
	struct file	*nm_filep;  /* file pointer of nm_filevp */
	struct vnode	*nm_mountpt;	/* mount point prior to mounting */
	struct namenode *nm_nextp;	/* next link in the linked list */
	struct namenode *nm_backp;	/* back link in linked list */
};

/*
 * Valid flags for namenodes.
 */
#define NMLOCK        01	/* the namenode is locked */
#define NMWANT        02	/* a process wants the namenode */


/*
 * Macros to convert a vnode to a namenode, and vice versa.
 */
#define VTONM(vp) ((struct namenode *)((vp)->v_data))
#define NMTOV(nm) (&(nm)->nm_vnode)

extern struct namenode *namefind();
extern struct namenode *namealloc;
extern struct vnodeops nm_vnodeops;

#endif	/* _FS_NAMENODE_H */
