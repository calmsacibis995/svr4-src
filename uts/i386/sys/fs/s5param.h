/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_S5PARAM_H
#define _FS_S5PARAM_H

#ident	"@(#)head.sys:sys/fs/s5param.h	11.6.7.1"

/*
 * Filesystem parameters.
 */
#define	SUPERB	((daddr_t)1)	/* block number of the super block */
#define	DIRSIZ	14		/* max characters per directory */
#define	NICINOD	100		/* number of superblock inodes */
#define	NICFREE	50		/* number of superblock free blocks */
#define	S5ROOTINO	2	/* i-number of all roots */
#define NDPC		4		/* number of blocks/click */
#define Fs2BLK		0x8000		/* large block flag in bsize */

#define SUPERBOFF	512	/* superblock offset */

#endif	/* _FS_S5PARAM_H */
