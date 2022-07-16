/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhead:unistd.h	1.1.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 
  
#include <sys/fcntl.h>

/* Symbolic constants for the "access" routine: */
#define	R_OK	4	/* Test for Read permission */
#define	W_OK	2	/* Test for Write permission */
#define	X_OK	1	/* Test for eXecute permission */
#define	F_OK	0	/* Test for existence of File */

#define F_ULOCK	0	/* Unlock a previously locked region */
#define F_LOCK	1	/* Lock a region for exclusive use */
#define F_TLOCK	2	/* Test and lock a region for exclusive use */
#define F_TEST	3	/* Test a region for other processes locks */


/* Symbolic constants for the "lseek" routine: */
#define	SEEK_SET	0	/* Set file pointer to "offset" */
#define	SEEK_CUR	1	/* Set file pointer to current plus "offset" */
#define	SEEK_END	2	/* Set file pointer to EOF plus "offset" */

/* Path names: */
#define	GF_PATH	"/etc/group"	/* Path name of the "group" file */
#define	PF_PATH	"/etc/passwd"	/* Path name of the "passwd" file */


/* command names for POSIX sysconf */
#define _SC_ARG_MAX	1
#define _SC_CHILD_MAX	2
#define _SC_CLK_TCK	3
#define _SC_NGROUPS_MAX 4
#define _SC_OPEN_MAX	5
#define _SC_JOB_CONTROL 6
#define _SC_SAVED_IDS	7
#define _SC_VERSION	8

/* command names for POSIX pathconf */

#define _PC_LINK_MAX	1
#define _PC_MAX_CANON	2
#define _PC_MAX_INPUT	3
#define _PC_NAME_MAX	4
#define _PC_PATH_MAX	5
#define _PC_PIPE_BUF	6
#define _PC_NO_TRUNC	7
#define _PC_VDISABLE	8
#define _PC_CHOWN_RESTRICTED	9

/* compile-time symbolic constants,
** Support does not mean the feature is enabled.
** Use pathconf/sysconf to obtain actual configuration value.
** 
*/

#define _POSIX_JOB_CONTROL	1
#define _POSIX_SAVED_IDS	1

#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE		0
#endif

/* Current version of POSIX */
#define _POSIX_VERSION		198808L


