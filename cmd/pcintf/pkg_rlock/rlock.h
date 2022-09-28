/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/rlock.h	1.1"
/* SCCSID_H("@(#)rlock.h	1.3	13:39:15	11/17/89", rlock_h) */

/*
   (c) Copyright 1985 by Locus Computing Corporation.  ALL RIGHTS RESERVED.

   This material contains valuable proprietary products and trade secrets
   of Locus Computing Corporation, embodying substantial creative efforts
   and confidential information, ideas and expressions.	 No part of this
   material may be used, reproduced or transmitted in any form or by any
   means, electronic, mechanical, or otherwise, including photocopying or
   recording, or in connection with any information storage or retrieval
   system without permission in writing from Locus Computing Corporation.
*/

/*-------------------------------------------------------------------------
 *
 *	rlock.h - definitions for the file/record locking library
 *
 *-------------------------------------------------------------------------
 */

/*
 * some machines with segmented architectures require that pointers be
 * different sizes, depending on how far away the reference can be.  the
 * following macro provides a default to be used for ALL pointers in the
 * RLOCK package.  if an architecture requires a special keyword, it should
 * be defined in the makefile as part of the compilation options.
 */

#if !defined(FAR)
#	define FAR
#endif

/*
 * the following macros dissect or build the 'open mode' byte.  the meaning
 * of each macro is:
 *
 *	RW_BITS(rwShare)	- extract 'access' bits from rwShare
 *	SHR_BITS(rwShare)	- extract the 'sharing mode' bits from rwShare
 *	RW_SHARE(share, rw)	- construct byte from 'sharing' and 'access'
 *				  field data.
 */

#define RW_BITS(rwShare)	((int)((rwShare) & 0x0f))
#define SHR_BITS(rwShare)	((int)((unsigned short)((rwShare) & 0x70) >> 4))

#define RW_SHARE(share, rw)	((unsigned char)(((share) << 4) | (rw)))

/*
 * 'access' values.
 */

#define RW_RDONLY	0		/* read only mode */
#define RW_WRONLY	1		/* write only */
#define RW_RDWR		2		/* read/write */
#define RW_FCB		0xf		/* RW bits for FCB opens */

/*
 * 'sharing mode' values.
 */

#define SHR_RDWRT	1		/* deny read and write */
#define SHR_WRT		2		/* deny write */
#define SHR_RD		3		/* deny read */
#define SHR_NONE	4		/* deny none */
#define SHR_DOS		0		/* DOS compatibility mode */
#define SHR_FCB		7		/* DOS FCB open mode */

/*
 * when a function is invoked on a file, it's state change causes the
 * rlockState() function to be invoked.  this normally does nothing.  If the
 * caller requires special handling, it must be redefined.  the calling
 * sequence is:
 *	void rlockState(openDesc, state)
 * where openDesc is the value returned by the addOpen() function, and which
 * is used to identify the file, and state is one of the following state
 * change indicators.
 */

#define RLSTATE_OPENED		1	/* the file has been opened */
#define RLSTATE_CLOSED		2	/* the file has been closed */
#define RLSTATE_LOCKED		3	/* the file was just locked */
#define RLSTATE_ALL_UNLOCKED	4	/* all file locks were removed */

/*
 * specific error codes returned from the RLOCK library, via rlockErr (defined
 * below).
 */

#define RLERR_NOERR		0	/* not an error */
#define RLERR_SYSTEM		1	/* system error */
#define	RLERR_PARAM		2	/* parameter error */
#define RLERR_VERSION		3	/* uknown memory version */

#define	RLERR_LOCKED		10	/* data is locked */
#define RLERR_UNLOCKED		11	/* data is unlocked */
#define RLERR_NOLOCK		12	/* can't lock table */
#define RLERR_NOUNLOCK		13	/* can't unlock table */
#define RLERR_GETKEY		14	/* can't get IPC identifier */
#define RLERR_LOCKSET		15	/* can't create/access lock set */
#define RLERR_FORMAT		16	/* unknown shared memory format */
#define RLERR_NOSPACE		17	/* no space available */
#define RLERR_INUSE		18	/* specified slot is in use */
#define RLERR_UNUSED		19	/* specified slot is unused */
#define RLERR_CORRUPT		20	/* shared memory is corrupted */
#define RLERR_ADDR		21	/* invalid attach address */
#define RLERR_EXIST		22	/* segment already exists */
#define RLERR_PERM		23	/* permission denied */

/*
 * external global variables.
 */

extern int	rlockErr;		/* specific error code */

/*
 * external functions.
 */

/* rlock_estr.c */
extern char FAR	*rlockEString();	/* return a string for the error */

/* rlock_init.c */
extern int	rlockCreate();		/* create the RLOCK segment */
extern int	rlockAttach();		/* attach (create) to the RLOCK data */
extern int	rlockRemove();		/* remove the RLOCK data */

/* shm_lock.c */
extern int	addLock();		/* add a lock to a files lock list */
extern int	rmvLock();		/* remove a lock */
extern int	ioStart();		/* exclude record lockers for I/O */
extern void	ioDone();		/* release I/O record lock exclusion */
extern int	rstLocks();		/* remove a single user's locks */

/* shm_open.c */
extern int	addOpen();		/* add an open to the table */
extern int	rmvOpen();		/* 'remove' an open from the table */

/* state.c */
extern void	rlockState();		/* handle a file's state change */
