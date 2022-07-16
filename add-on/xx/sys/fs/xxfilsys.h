/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ifndef _FS_XXFILSYS_H
#define _FS_XXFILSYS_H

#ident	"@(#)xx:sys/fs/xxfilsys.h	1.2.1.1"

/*
 * Structure of the super-block
 */

#pragma pack(2)

#define NSBFILL 371             /* aligns s_magic, .. at end of super block */
#undef  NICFREE
#define	NICFREE	100		/* number of superblock free blocks */

struct	filsys
{
	u_short	s_isize;	/* size in blocks of i-list */
	daddr_t	s_fsize;	/* size in blocks of entire volume */
	short	s_nfree;	/* number of addresses in s_free */
	daddr_t	s_free[NICFREE];	/* free block list */
	short	s_ninode;	/* number of i-nodes in s_inode */
	o_ino_t	s_inode[NICINOD];	/* free i-node list */
	char	s_flock;	/* lock during free list manipulation */
	char	s_ilock;	/* lock during i-list manipulation */
	char  	s_fmod; 	/* super block modified flag */
	char	s_ronly;	/* mounted read-only flag */
	time_t	s_time; 	/* last super block update */
	daddr_t	s_tfree;	/* total free blocks*/
	o_ino_t	s_tinode;	/* total free inodes */
	short   s_dinfo[4];     /* device information */
	char	s_fname[6];	/* file system name */
	char	s_fpack[6];	/* file system pack name */
	/* remainder is maintained for xenix */
	char   	s_clean;   	/* S_CLEAN if structure is properly closed */
	char    s_fill[NSBFILL];/* space to make sizeof filsys be BSIZE */
	long    s_magic;        /* indicates version of filsys */
	long	s_type;		/* type of new file system */
};

#pragma pack()

#define	S_CLEAN	0106        	/* arbitrary magic value  */

/* s_magic, magic value for file system version */
#define	S_S3MAGIC	0x2b5544	/* system 3 arbitrary magic value */

/* s_type, block size of file system */
#define	S_B512		1	/* 512 byte block */
#define	S_B1024		2	/* 1024 byte block */

/* codes for file system version (for utilities) */
#define	S_V2		1		/* version 7 */
#define	S_V3		2		/* system 3 */

#define getfs(vfsp)  ((struct filsys *)((struct s5vfs *)vfsp->vfs_data)->vfs_bufp->b_un.b_addr)

#define	XXBSIZE		1024	/* XENIXFS only support 1024 byte blocks */

#endif	/* _FS_XXFILSYS_H */
