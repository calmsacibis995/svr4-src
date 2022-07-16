/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PRIOCNTL_H
#define _SYS_PRIOCNTL_H

#ident	"@(#)head.sys:sys/priocntl.h	1.6.7.1"

#define	PC_VERSION	1	/* First version of priocntl */

#define priocntl(idtype, id, cmd, arg)\
	__priocntl(PC_VERSION, idtype, id, cmd, arg)

#define priocntlset(psp, cmd, arg)\
	__priocntlset(PC_VERSION, psp, cmd, arg)

extern long	__priocntl(), __priocntlset();

/*
 * The following are the possible values of the command
 * argument for the priocntl system call.
 */

#define PC_GETCID	0	/* Get class ID */
#define	PC_GETCLINFO	1	/* Get info about a configured class */
#define	PC_SETPARMS	2	/* Set scheduling parameters */
#define	PC_GETPARMS	3	/* Get scheduling parameters */
#define PC_ADMIN	4	/* Scheduler administration (used by     */
				/*   dispadmin(1M), not for general use) */

#define PC_CLNULL	-1

#define	PC_CLNMSZ	16
#define	PC_CLINFOSZ	(32 / sizeof(long))
#define	PC_CLPARMSZ	(32 / sizeof(long))

typedef struct pcinfo {
	id_t	pc_cid;			/* class id */
	char	pc_clname[PC_CLNMSZ];	/* class name */
	long	pc_clinfo[PC_CLINFOSZ];	/* class information */
} pcinfo_t;

typedef struct pcparms {
	id_t	pc_cid;			    /* process class */
	long	pc_clparms[PC_CLPARMSZ];    /* class specific parameters */
} pcparms_t;

/*
 * The following is used by the dispadmin(1M) command for
 * scheduler administration and is not for general use.
 */

typedef struct pcadmin {
	id_t	pc_cid;
	caddr_t	pc_cladmin;
} pcadmin_t;

#endif	/* _SYS_PRIOCNTL_H */
