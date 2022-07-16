/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_FSTYP_H
#define _SYS_FSTYP_H

#ident	"@(#)head.sys:sys/fstyp.h	11.6.7.1"

#define FSTYPSZ		16	/* max size of fs identifier */

/*
 * Opcodes for the sysfs() system call.
 */
#define GETFSIND	1	/* translate fs identifier to fstype index */
#define GETFSTYP	2	/* translate fstype index to fs identifier */
#define GETNFSTYP	3	/* return the number of fstypes */

#if defined(__STDC__) && !defined(_KERNEL)
int sysfs(int, ...);
#endif

#endif	/* _SYS_FSTYP_H */
