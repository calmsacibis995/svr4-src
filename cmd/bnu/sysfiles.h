/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:sysfiles.h	1.7.3.1"

#define SYSDIR		"/etc/uucp"
#define SYSFILES	"/etc/uucp/Sysfiles"
#define SYSTEMS		"/etc/uucp/Systems"
#define DEVICES		"/etc/uucp/Devices"
#define DIALERS		"/etc/uucp/Dialers"
#define	DEVCONFIG	"/etc/uucp/Devconfig"
#define CONFIG		"/etc/uucp/Config"

#define	SAME	0
#define	TRUE	1
#define	FALSE	0
#define	FAIL	-1

/* flags to check file access for REAL user id */
#define	ACCESS_SYSTEMS	1
#define	ACCESS_DEVICES	2
#define	ACCESS_DIALERS	3

/* flags to check file access for EFFECTIVE user id */
#define	EACCESS_SYSTEMS	4
#define	EACCESS_DEVICES	5
#define	EACCESS_DIALERS	6

