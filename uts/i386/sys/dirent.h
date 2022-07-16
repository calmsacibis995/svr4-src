/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#ident	"@(#)head.sys:sys/dirent.h	11.12.3.1"

/*
 * File-system independent directory entry.
 */
struct dirent {
	ino_t		d_ino;		/* "inode number" of entry */
	off_t		d_off;		/* offset of disk directory entry */
	unsigned short	d_reclen;	/* length of this record */
	char		d_name[1];	/* name of file */
};

typedef	struct	dirent	dirent_t;

#if !defined(_POSIX_SOURCE)
#if defined(__STDC__) && !defined(_KERNEL)
int getdents(int, struct dirent *, unsigned);
#else
int getdents( );
#endif
#endif /* !defined(_POSIX_SOURCE) */

#endif	/* _SYS_DIRENT_H */
