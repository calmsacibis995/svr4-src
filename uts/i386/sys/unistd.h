/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/unistd.h	1.3.3.1"
/* WARNING: This is an implementation-specific header,
** its contents are not guaranteed. Applications
** should include <unistd.h> and not this header.
*/

#ifndef _SYS_UNISTD_H
#define _SYS_UNISTD_H


/* command names for POSIX sysconf */

#define _SC_ARG_MAX	1
#define _SC_CHILD_MAX	2
#define _SC_CLK_TCK	3
#define _SC_NGROUPS_MAX 4
#define _SC_OPEN_MAX	5
#define _SC_JOB_CONTROL 6
#define _SC_SAVED_IDS	7
#define _SC_VERSION	8
#define _SC_PASS_MAX	9
#define _SC_LOGNAME_MAX	10
#define _SC_PAGESIZE	11
#define _SC_XOPEN_VERSION 12

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

#ifndef _POSIX_VERSION
#define _POSIX_VERSION	198808L
#endif

#ifndef _XOPEN_VERSION
#define _XOPEN_VERSION 3
#endif


#endif	/* _SYS_UNISTD_H */
