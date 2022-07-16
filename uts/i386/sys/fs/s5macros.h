/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_S5MACROS_H
#define _FS_S5MACROS_H

#ident	"@(#)head.sys:sys/fs/s5macros.h	11.7.7.1"

#define FsLTOP(fs, b)	((b) << (fs)->vfs_ltop)
#define FsPTOL(fs, b)	((b) >> (fs)->vfs_ltop)
#define FsITOD(fs, x)	(daddr_t)(((unsigned)(x)+(2*(fs)->vfs_inopb-1)) >> (fs)->vfs_inoshift)
#define FsITOO(fs, x)	(daddr_t)(((unsigned)(x)+(2*(fs)->vfs_inopb-1)) & ((fs)->vfs_inopb-1))
#define FsINOS(fs, x)	((((x) >> (fs)->vfs_inoshift) << (fs)->vfs_inoshift) + 1)

#endif	/* _FS_S5MACROS_H */
