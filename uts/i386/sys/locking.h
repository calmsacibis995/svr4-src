/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_LOCKING_H
#define _SYS_LOCKING_H

#ident	"@(#)head.sys:sys/locking.h	1.2.4.1"

/*
 * THIS FILE CONTAINS CODE WHICH IS DESIGNED TO BE
 * PORTABLE BETWEEN DIFFERENT MACHINE ARCHITECTURES
 * AND CONFIGURATIONS. IT SHOULD NOT REQUIRE ANY
 * MODIFICATIONS WHEN ADAPTING XENIX TO NEW HARDWARE.
 */
/*
 *   Flag values for XENIX locking() system call (os/xsys.c)
 */


#define LK_UNLCK  0	/* unlock request */
#define LK_LOCK   1	/* lock request */
#define LK_NBLCK  20	/* non-blocking lock request */
#define LK_RLCK   3	/* read permitted only lock request */
#define LK_NBRLCK 4	/* non-blocking read only lock request */

#endif	/* _SYS_LOCKING_H */
