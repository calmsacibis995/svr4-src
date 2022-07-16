/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/sys/inode.h	11.2.4.1"
/* this inode struct is optimized for bootstrap only
 * it is a minimal subset of s5 inode structure to save memory
 * expansion is allowed at the end for future bootstrap
 */

#define	NADDR	13
#define	NSADDR	(NADDR*sizeof(daddr_t)/sizeof(short))

typedef	struct	inode
{
	ino_t	i_number;	/* i number, 1-to-1 with device address */
	ushort	i_ftype;	/* otherwise known as mode	*/
	off_t	i_size;		/* size of the file	*/
	time_t	i_mtime;	/* last modification time */
} inode_t;

/* ftype or modes */
#define	IFMT	0xf000		/* type of file */
#define		IFDIR	0x4000	/* directory */
#define		IFCHR	0x2000	/* character special */
#define		IFBLK	0x6000	/* block special */
#define		IFREG	0x8000	/* regular */
#define		IFMPC	0x3000	/* multiplexed char special */
#define		IFMPB	0x7000	/* multiplexed block special */
#define		IFIFO	0x1000	/* fifo special */
#define	ISUID	0x800		/* set user id on execution */
#define	ISGID	0x400		/* set group id on execution */
#define ISVTX	0x200		/* save swapped text even after use */
#define	IREAD	0x100		/* read, write, execute permissions */
#define	IWRITE	0x080
#define	IEXEC	0x040
