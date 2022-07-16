/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_FIFONODE_H
#define _FS_FIFONODE_H

#ident	"@(#)head.sys:sys/fs/fifonode.h	1.15.3.1"

/*
 * Each FIFOFS object is identified by a struct fifonode/vnode pair.
 */
struct fifonode {
	struct vnode	fn_vnode;	/* represents the fifo/pipe */
	struct vnode	*fn_mate;	/* the other end of a pipe */
	struct vnode	*fn_realvp;	/* node being shadowed by fifo */
	ushort		fn_ino;		/* node id for pipes */
	short		fn_wcnt;	/* number of writers */
	short		fn_rcnt;	/* number of readers */
	short		fn_open;	/* open count of node*/
	struct vnode	*fn_unique;	/* new vnode created by CONNLD */
	ushort		fn_flag;	/* flags as defined below */
	time_t		fn_atime;	/* creation times for pipe */
	time_t		fn_mtime;
	time_t		fn_ctime;
	struct fifonode	*fn_nextp;	/* next link in the linked list */
	struct fifonode	*fn_backp;	/* back link in linked list */
};

/*
 * Valid flags for fifonodes.
 */
#define ISPIPE		01	/* fifonode is that of a pipe */
#define FIFOLOCK	02	/* fifonode is locked */
#define FIFOSEND        04	/* file descriptor at stream head of pipe */
#define FIFOWRITE      010	/* process is blocked waiting to write */
#define FIFOWANT       020	/* a process wants to access the fifonode */
#define FIFOREAD       040	/* process is blocked waiting to read */
#define FIFOPASS      0100	/* CONNLD passed a new vnode in fn_unique */

/* XENIX Support */
extern int fifo_rdchk();
/* End XENIX Support */

/*
 * Macros to convert a vnode to a fifnode, and vice versa.
 */
#define VTOF(vp) ((struct fifonode *)((vp)->v_data))
#define FTOV(fp) (&(fp)->fn_vnode)

#endif	/* _FS_FIFONODE_H */
