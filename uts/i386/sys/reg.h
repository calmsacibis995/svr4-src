/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_REG_H
#define _SYS_REG_H

#ident	"@(#)head.sys:sys/reg.h	11.2.8.1"

/*
 * Location of the users' stored registers relative to EAX.
 * Usage is u.u_ar0[XX].
 *
 * NOTE: ERR is the error code.
 */

#define SS	18
#define UESP	17
#define EFL	16
#define CS	15
#define EIP	14
#define ERR	13
#define TRAPNO	12
#define EAX	11
#define ECX	10
#define EDX	9
#define EBX	8
#define ESP	7
#define EBP	6
#define ESI	5
#define EDI	4
#define DS	3
#define ES	2
#define FS	1
#define GS	0

#endif	/* _SYS_REG_H */
