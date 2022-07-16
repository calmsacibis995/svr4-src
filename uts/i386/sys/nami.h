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

#ident	"@(#)head.sys:sys/nami.h	11.1.4.1"

/*
 * Structure used by system calls to pass parameters
 * to the file system independent namei and attribute
 * functions.
 */
struct argnamei {	/* namei's flag argument */
	ushort	cmd;	/* command type (see below) */
	short	rcode;	/* a scratch for return codes (see below) */
	long	ino;	/* ino for link */
	long	mode;	/* mode for creat and chmod */
	ushort	ftype;	/* file type */
	ushort	uid;	/* uid for chown */
	ushort	gid;	/* gid for chown */
	dev_t	idev;	/* dev for link and creat */
};

/*
 * Possible values for argnamei.cmd field 
 */
#define	NI_DEL		0x1	/* unlink this file */
#define	NI_CREAT	0x2	/* create */
#define	NI_XCREAT	0x3	/* Exclusive create, error if */
				/* the file exists */
#define	NI_LINK		0x4	/* make a link */
#define	NI_MKDIR	0x5	/* mkdir */
#define	NI_RMDIR	0x6	/* rmdir */
#define	NI_MKNOD	0x7	/* mknod */

/* Requests to fs_setattr */
#define	NI_CHOWN	0x1	/* change owner */
#define	NI_CHMOD	0x2	/* change mode (permissions and 
				/* ISUID, ISGID, ISVTX) */

/* Codes to fs_notify */
#define	NI_OPEN		0x1	/* open - some fstyps may want */
				/* to know when a file is opened */
#define	NI_CLOSE	0x2	/* close */
#define	NI_CHDIR	0x3	/* let fstyp know that a cd */
				/* is happening (e.g., some fstyp */
				/* may need to know so that */
				/* directory cache can be */
				/* flushed - if it has one) */

/*
 * Return Codes for argnamei.rcode
 */
#define	FSN_FOUND	0x1	/* The file was found by namei (it exists) */
#define	FSN_NOTFOUND	0x2	/* The file was not found by namei  */
				/* (i.e., it does not exist */

/*
 * Return Codes for file sytem dependent namei's
 */
#define	NI_PASS		0	/* Error free FS specific namei */
#define	NI_FAIL		1	/* Error encountered in FS specific namei */
#define	NI_RESTART	2	/* The fs dependent code overwrote */
				/* the buffer and the namei must */
				/* begin again. Used mostly for */
				/* symbolic links. */
#define	NI_DONE		3	/* The fs dependent operation is */
				/* complete. There is no need to do */
				/* any further pathnme processing. */
				/* This is equivalent to NI_PASS for */
				/* those commands that require some */
				/* action from the fs dependent code */
#define NI_NULL 	4	/* Tell fs independent to return NULL to */
				/* the calling procedure without doing /*
				/* an iput */
#define NI_SYMRESTART	5	/* The fs dependent code overwrote the */
				/* buffer and namei must reparse */
				/* starting at the current directory */
				/* in the parse.  Used mostly for */
				/* symbolic links */

/*
 * Data that is passed from the file system independent namei to the
 * file system dependent namei.
 */
struct nx {
	struct	inode *dp;	/* inode of matched file */
				/* characters */
	caddr_t	comp;		/* pointer to beginning of current */
				/* pathname component */
	caddr_t	bufp;		/* pointer to the beginning of the */
				/* pathname buffer */
	long	ino;		/* inode number returned by fs dep code */
	long	flags;		/* Flag field */
};

#define	NX_ISROOT	0x1	/* Inode is root of fs */

/* Values for fsinfo[].fs_notify. If an fstyp wishes to be */
/* notified of an action the appropriate flag should be set */
/* in fsinfo[].fs_notify and fstypsw[].fs_notify should point */
/* to the desired fs dependent function */
#define	NO_CHDIR	0x1	/* chdir */
#define	NO_CHROOT	0x2	/* chroot */
#define	NO_SEEK		0x4	/* seek */
#define NO_CHSIZE	0x8000	/* XENIX chsize */

struct argnotify {
	long 	cmd;	/* command - see above */
	long	data1;	/* Allow caller to pass two pieces of data. */
	long 	data2;	/* These should be caste appropriately in */
			/* the fs_notify routine. They are declared */
			/* as longs here. However, they should be */
			/* large enough to hold the largest machine */
			/* specific data types (e.g., if a pointer is */
			/* larger than a long then these should be */
			/* int *). */
};
