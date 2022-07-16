/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_S5DIR_H
#define _FS_S5DIR_H

#ident	"@(#)head.sys:sys/fs/s5dir.h	11.4.7.1"
#ifndef	DIRSIZ
#define	DIRSIZ	14
#endif
struct	direct
{
	o_ino_t	d_ino;		/* s5 inode type */
	char	d_name[DIRSIZ];
};

#define	SDSIZ	(sizeof(struct direct))

#endif	/* _FS_S5DIR_H */
