/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/ftw.h	1.1"
/*ident	"@(#)cfront:incl/ftw.h	1.5"*/

#ifndef FTWH
#define FTWH

/*
 *	Codes for the third argument to the user-supplied function
 *	which is passed as the second argument to ftw
 */

#define	FTW_F	0	/* file */
#define	FTW_D	1	/* directory */
#define	FTW_DNR	2	/* directory without read permission */
#define	FTW_NS	3	/* unknown type, stat failed */

#ifndef PFSEEN
#define PFSEEN
typedef int (*PF) ();
#endif

extern int ftw (const char*, PF, int);

#endif
