/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_UADMIN_H
#define _SYS_UADMIN_H

#ident	"@(#)head.sys:sys/uadmin.h	11.7.7.1"

#define	A_REBOOT	1
#define	A_SHUTDOWN	2
#define	A_REMOUNT	4
#define A_CLOCK		8
#define A_SWAPCTL	16
#define A_SETCONFIG	128

#define	AD_HALT		0
#define	AD_BOOT		1
#define	AD_IBOOT	2

/*
 * fcn's for A_SETCONFIG
 */
#define AD_PANICBOOT	1

#if defined(__STDC__) && !defined(_KERNEL)
int uadmin(int, int, int);
#endif

#endif	/* _SYS_UADMIN_H */
