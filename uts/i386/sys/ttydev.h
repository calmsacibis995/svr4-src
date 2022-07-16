/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_TTYDEV_H
#define _SYS_TTYDEV_H

#ident	"@(#)head.sys:sys/ttydev.h	1.1.2.1"

/*	@(#)ttydev.h 2.6 88/02/08 SMI; from UCB 4.3 83/05/18	*/

/*
 * Terminal definitions related to underlying hardware.
 */

/*
 * Speeds
 */
#define B0	0
#define B50	1
#define B75	2
#define B110	3
#define B134	4
#define B150	5
#define B200	6
#define B300	7
#define B600	8
#define B1200	9
#define	B1800	10
#define B2400	11
#define B4800	12
#define B9600	13
#define B19200	14
#define B38400	15
#define EXTA	14
#define EXTB	15

#if 0
/*
 * Hardware bits.
 * SHOULD NOT BE HERE.
 */
#define	DONE	0200
#define	IENABLE	0100

/*
 * Modem control commands.
 */
#define	DMSET		0
#define	DMBIS		1
#define	DMBIC		2
#define	DMGET		3

#endif /* end if 0 comment */

#endif /*_SYS_TTYDEV_H */
