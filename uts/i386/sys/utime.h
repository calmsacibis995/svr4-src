/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* utimbuf is used by utime(2) */

#ifndef _SYS_UTIME_H
#define _SYS_UTIME_H

#ident	"@(#)head.sys:sys/utime.h	1.4.3.1"
#include <sys/types.h>

struct utimbuf {
	time_t actime;		/* access time */
	time_t modtime;		/* modification time */
};

#endif	/* _SYS_UTIME_H */
