/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_S5INO_H
#define _FS_S5INO_H

#ident	"@(#)head.sys:sys/fs/s5ino.h	1.3.7.1"
/*
 * Inode structure as it appears on a disk block.  Of the 40 address
 * bytes, 39 are used as disk addresses (13 addresses of 3 bytes each)
 * and the 40th is used as a file generation number.
 */
struct	dinode {
	o_mode_t	di_mode;	/* mode and type of file */
	o_nlink_t	di_nlink;    	/* number of links to file */
	o_uid_t		di_uid;      	/* owner's user id */
	o_gid_t		di_gid;      	/* owner's group id */
	off_t		di_size;     	/* number of bytes in file */
	char  		di_addr[39];	/* disk block addresses */
	unsigned char	di_gen;		/* file generation number */
	time_t		di_atime;   	/* time last accessed */
	time_t		di_mtime;   	/* time last modified */
	time_t		di_ctime;   	/* time created */
};

#endif	/* _FS_S5INO_H */
