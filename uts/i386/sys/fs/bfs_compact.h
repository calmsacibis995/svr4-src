/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_BFS_COMPACT_H
#define _FS_BFS_COMPACT_H

#ident	"@(#)head.sys:sys/fs/bfs_compact.h	1.6.2.1"

#define BFS_CCT_READ(bvp, offset, len, buf,cr) \
	vn_rdwr(UIO_READ, bvp, (caddr_t)buf, len, offset, \
				UIO_SYSSPACE, 0, 0, cr, 0)

#define BFS_CCT_WRITE(bvp, offset, len, buf,cr) \
	vn_rdwr(UIO_WRITE, bvp, (caddr_t)buf, len, offset, \
				UIO_SYSSPACE, IO_SYNC, BFS_ULT, cr, (int *)0)

#define BFS_SANITYWSTART (BFS_SUPEROFF + (sizeof(long)*3))

#endif	/* _FS_BFS_COMPACT_H */
