/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_ULIMIT_H
#define _SYS_ULIMIT_H

#ident	"@(#)head.sys:sys/ulimit.h	1.5.3.1"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */


/*
 *	@(#) ulimit.h 1.2 88/05/04 head.sys:ulimit.h
 */

/*
 * THIS FILE CONTAINS CODE WHICH IS DESIGNED TO BE
 * PORTABLE BETWEEN DIFFERENT MACHINE ARCHITECTURES
 * AND CONFIGURATIONS. IT SHOULD NOT REQUIRE ANY
 * MODIFICATIONS WHEN ADAPTING XENIX TO NEW HARDWARE.
 */

/*
 * The following are codes which can be
 * passed to the ulimit system call. (Xenix compatible.)
 */

#define UL_GFILLIM	1	/* get file limit */
#define UL_SFILLIM	2	/* set file limit */
#define UL_GMEMLIM	3	/* get process size limit */
#define UL_GDESLIM	4	/* get file descriptor limit */
#define UL_GTXTOFF	64	/* get text offset */

/*
 * The following are symbolic constants required for
 * X/Open Conformance.   They are the equivalents of
 * the constants above.
 */

#define UL_GETFSIZE	UL_GFILLIM	/* get file limit */
#define UL_SETFSIZE	UL_SFILLIM	/* set file limit */

#endif        /* _SYS_ULIMIT_H */
