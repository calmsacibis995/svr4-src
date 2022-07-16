/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/sys/filsys.h	11.3.4.1"
/*
 *	Structure of the super-block.
 */

struct	filsys
{
	ushort	s_isize;	/* size in blocks of i-list */
	daddr_t	s_fsize;	/* size in blocks of entire volume */
	short	s_nfree;	/* number of addresses in s_free */
	daddr_t	s_free[NICFREE];/* free block list */
	short	s_ninode;	/* number of i-nodes in s_inode */
	ino_t	s_inode[NICINOD];/* free i-node list */
	char	s_flock;	/* lock during free list manipulation */
	char	s_ilock;	/* lock during i-list manipulation */
	char  	s_fmod; 	/* super block modified flag */
	char	s_ronly;	/* mounted read-only flag */
	time_t	s_time; 	/* last super block update */
	short	s_dinfo[4];	/* device information */
	daddr_t	s_tfree;	/* total free blocks*/
	ino_t	s_tinode;	/* total free inodes */
	char	s_fname[6];	/* file system name */
	char	s_fpack[6];	/* file system pack name */
	long	s_fill[12];	/* adjust to make sizeof filsys */
	long	s_state;	/* file system state */
	long	s_magic;	/* magic number to indicate new file system */
	long	s_type;		/* type of new file system */
} ;

#define FsMAGIC	0xfd187e20	/* s_magic */

#define Fs1b	1	/* 512 byte block */
#define Fs2b	2	/* 1024 byte block */

#define	FsOKAY	0x7c269d38	/* s_state: clean */
#define	FsACTIVE	0x5e72d81a	/* s_state: active */
#define	FsBAD	0xcb096f43	/* s_state: bad root */
#define FsBADBLK	0xbadbc14b	/* s_state: bad block corrupted it */

#define getfs(mp)	mp->m_bufp->b_un.b_filsys
