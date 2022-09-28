/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:tmeth.d/method.h	1.2.2.1"

#define	Aflag	0x1		/* automated operation */
#define dflag	0x2		/* do not put in backup history log */
#define lflag	0x4		/* long history log */
#define mflag	0x8		/* mount orig fs read only */
#define oflag	0x10		/* permit media label override */
#define rflag	0x20		/* include remote files */
#define tflag	0x40		/* create table of contents media */
#define vflag	0x80		/* validate archive as it is written */
#define Eflag	0x100		/* estimate media usage, proceed */
#define Nflag	0x200		/* estimate media usage, exit */
#define Sflag	0x400		/* print a dot per 100 512 byte recs */
#define Vflag	0x800		/* generate names of files */
#define iflag	0x1000		/* exclude files with inode change */
#define xflag	0x2000		/* ignore exception list */
#define cflag	0x4000		/* block count for data partition */
#define	uflag	0x8000	/* unmount file system before using. */
#define	gflag	0x10000	/* call get_volume */

#define IS_BACKUP	0x1	/* -B was specified */
#define IS_RFILE	0x2	/* -RF was specified */
#define IS_RCOMP	0x4	/* -RC was specified */
#define IS_RESTORE	0x8	/* -R was specified */

#define IS_BOTH 	(IS_BACKUP | IS_RESTORE)
#define IS_FC		(IS_RCOMP | IS_RFILE)
#define IS_B(x)		(x & IS_BACKUP)
#define IS_R(x)		(x & IS_RESTORE)

