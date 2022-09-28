/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/brtoc.h	1.1.2.1"

/* This file contains definintions about the table-of-contents table */

/* SCHEDULE TABLE DEFINITIONS */
/* Field Names */
#define	TOC_FNAME	(unsigned char *)"fname"
#define	TOC_VOL		(unsigned char *)"vol"
#define	TOC_DEV		(unsigned char *)"dev"
#define	TOC_INODE	(unsigned char *)"inode"
#define	TOC_MODE	(unsigned char *)"mode"
#define	TOC_NLINK	(unsigned char *)"nlink"
#define	TOC_UID		(unsigned char *)"uid"
#define	TOC_GID		(unsigned char *)"gid"
#define	TOC_RDEV	(unsigned char *)"rdev"
#define	TOC_SIZE	(unsigned char *)"size"
#define	TOC_ATIME	(unsigned char *)"atime"
#define	TOC_MTIME	(unsigned char *)"mtime"
#define	TOC_CTIME	(unsigned char *)"ctime"

/* ENTRY FORMAT */
#define	TOC_ENTRY_F	(unsigned char *)\
	"fname:vol:dev:inode:mode:nlink:uid:gid:rdev:size:atime:mtime:ctime"
