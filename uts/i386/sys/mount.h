/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#ident	"@(#)head.sys:sys/mount.h	11.10.7.1"
/*
 * Flags bits passed to mount(2).
 */
#define	MS_RDONLY	0x01	/* Read-only */
#define	MS_FSS		0x02	/* Old (4-argument) mount (compatibility) */
#define	MS_DATA		0x04	/* 6-argument mount */
#define MS_HADBAD	0x08	/* File system incurred a bad block */
				/* so set s_state to FsBADBLK on umount */
#define	MS_NOSUID	0x10	/* Setuid programs disallowed */
#define MS_REMOUNT	0x20	/* Remount */
#define MS_NOTRUNC	0x40	/* Return ENAMETOOLONG for long filenames */

#if defined(__STDC__) && !defined(_KERNEL)
int mount(const char *, const char *, int, ...);
int umount(const char *);
#endif

#endif	/* _SYS_MOUNT_H */
