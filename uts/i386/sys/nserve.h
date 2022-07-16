/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_NSERVE_H
#define _SYS_NSERVE_H

#ident	"@(#)head.sys:sys/nserve.h	11.6.7.1"
/*
 * Contains definitions needed both in the kernel and in user programs 
 * for RFS adv, mount, and name service functions
 */
#define TPNSPID	  "/etc/rfs/%s/nspid" /* TP lock file for ns, also has pid */
#define	TPNS_PIPE "/etc/rfs/%s/nspip" /* transport-specific ns pipe */
#define TPNETMASTER "/etc/rfs/%s/rfmaster"	/* TP master file for nudnix network */
#define TPDOMMASTER "/etc/rfs/%s/dom.master"	/* TP file for outside domains  */
#define	TPNSERVE  "/usr/lib/rfs/TPnserve"	/* TP name serve executable */

#define A_RDWR		0	/* read/write flag */
#define A_RDONLY	1	/* read only flag */
#define A_CLIST		2	/* client list flag */
#define A_MODIFY	4	/* modify (really replace) clist flag */
#define A_INUSE		8	/* advertise table entry in use */
#define A_FREE		0	/* advertise table entry free */
#define A_MINTER	16	/* unadv -- but not free yet */
#define SEPARATOR	'.'
#define MAXDNAME	64
#define RFS_NMSZ	15

#define R_NOERR	0	/* no error */
#define R_FORMAT 1	/* format error */
#define R_NSFAIL 2	/* name server failure */
#define R_NONAME 3	/* name does not exist */
#define R_IMP	 4	/* request type not implemented or bad */
#define R_PERM	 5	/* no permission for this operation */
#define R_DUP	 6	/* name not unique (for advertise) */
#define R_SYS	 7	/* a system call failed in name server */
#define R_EPASS  8	/* error accessing primary passwd file */
#define R_INVPW  9   	/* invalid password */
#define R_NOPW   10	/* no passwd in primary passwd file */
#define R_SETUP  11	/* error in ns_setup() */
#define R_SEND   12	/* error in ns_send() */
#define R_RCV    13	/* error in ns_rcv() */
#define R_INREC	 14	/* in recovery, try again */
#define R_FAIL	 15	/* unknown failure */

#endif	/* _SYS_NSERVE_H */
