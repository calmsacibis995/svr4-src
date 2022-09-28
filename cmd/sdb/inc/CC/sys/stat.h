/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/CC/sys/stat.h	1.2"
/*ident	"@(#)cfront:incl/sys/stat.h	1.7"*/
/*
 * Structure of the result of stat
 */

#ifndef STAT_H
#define STAT_H

struct	stat
{
	o_dev_t st_dev;
	o_ino_t st_ino;
	o_mode_t st_mode;
	o_nlink_t st_nlink;
	o_uid_t st_uid;
	o_gid_t st_gid;
	o_dev_t st_rdev;
	off_t	st_size;
	time_t	st_atime;
	time_t	st_mtime;
	time_t	st_ctime;
};

#define	S_IFMT	0170000		/* type of file */
#define		S_IFDIR	0040000	/* directory */
#define		S_IFCHR	0020000	/* character special */
#define		S_IFBLK	0060000	/* block special */
#define		S_IFREG	0100000	/* regular */
#define		S_IFIFO	0010000	/* fifo */
#define	S_ISUID	04000		/* set user id on execution */
#define	S_ISGID	02000		/* set group id on execution */
#define	S_ISVTX	01000		/* save swapped text even after use */
#define	S_IREAD	00400		/* read permission, owner */
#define	S_IWRITE	00200		/* write permission, owner */
#define	S_IEXEC	00100		/* execute/search permission, owner */

#ifdef _OVERLOAD_STAT
overload stat;
#endif

extern int stat (const char*, struct stat*),
           fstat (int, struct stat*);

#endif
