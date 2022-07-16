/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_ACC_PRIV_PRIVILEGE_H
#define	_ACC_PRIV_PRIVILEGE_H

#ident	"@(#)head.sys:sys/privilege.h	1.1.4.1"

/**********************************************************
 *
 * The following is the set of all known privileges
 *
 * Also, the define NPRIVS is the number of privileges
 * currently in use.  It should be modified whenever a
 * privilege is added or deleted.
 *
 **********************************************************/

#define	NPRIVS		24

#define	P_OWNER		0x00000000
#define	P_AUDIT		0x00000001
#define	P_COMPAT	0x00000002
#define	P_DACREAD	0x00000003
#define	P_DACWRITE	0x00000004
#define	P_DEV		0x00000005
#define	P_FILESYS	0x00000006
#define	P_MACREAD	0x00000007
#define	P_MACWRITE	0x00000008
#define	P_MOUNT		0x00000009
#define	P_MULTIDIR	0x0000000a
#define	P_SETPLEVEL	0x0000000b
#define	P_SETSPRIV	0x0000000c
#define	P_SETUID	0x0000000d
#define	P_SYSOPS	0x0000000e
#define	P_SETUPRIV	0x0000000f
#define	P_DRIVER	0x00000010
#define	P_RTIME		0x00000011
#define	P_MACUPGRADE	0x00000012
#define	P_FSYSRANGE	0x00000013
#define	P_SETFLEVEL	0x00000014
#define	P_AUDITWR	0x00000015
#define	P_TSHAR		0x00000016
#define	P_PLOCK		0x00000017
#define	P_ALLPRIVS	0x00ffffff

#endif	/* _ACC_PRIV_PRIVILEGE_H */
