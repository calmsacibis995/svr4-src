/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_lockset/lockset.h	1.1"
/* SCCSID_H("@(#)lockset.h	1.1	13:08:19	8/31/89", lockset_h) */

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
 *	lockset.h - definitions and declaration for the LOCKSET package
 *
 *-------------------------------------------------------------------------
 */

/*
 * some machines with segmented architectures require that pointers be
 * different sizes, depending on how far away the reference can be.  the
 * following macro provides a default to be used for ALL pointers in the
 * LOCKSET package.  if an architecture requires a special keyword, it should
 * be defined in the makefile as part of the compilation options.
 */

#if !defined(FAR)
#	define FAR
#endif

/*
 * implementation constants that may be used by the user of the LOCKSET
 * package.
 *
 *	UNUSED_LOCK	- this defines the end of a list of locks, as used
 *			  by lsSet().
 *	INVALID_LOCKSET	- an invalid lockSetT value, used for error returns
 *	LS_MAXSETS	- the maximum number of locks per lockset
 */

#define UNUSED_LOCK		0
#define INVALID_LOCKSET		((lockSetT) 0L)
#if	defined(XENIX)
#	define LS_MAXSETS	5
#else
#	define LS_MAXSETS	8
#endif

/*
 * this type give the caller a handle to access the a lockset that is currently
 * being used.
 */

typedef char * lockSetT;

/*
 * argument values for permission argument to lsNew()
 */

#define LS_O_WRITE	0x0001		/* owner may write lockset */
#define LS_G_WRITE	0x0002		/* group may write lockset */
#define LS_W_WRITE	0x0004		/* world may write lockset */
#define LS_O_READ	0x0010		/* owner may read lockset */
#define LS_G_READ	0x0020		/* group may read lockset */
#define LS_W_READ	0x0040		/* world may read lockset */
#define LS_PRIVATE	0x8000		/* lockset invisible to useLocks() */

#define LS_ALL_WRITE	(LS_O_WRITE | LS_G_WRITE | LS_W_WRITE)
#define LS_ALL_READ	(LS_O_READ  | LS_G_READ  | LS_W_READ)

/*
 * argument values for lockOptions argument to lsSet()
 */

#define LS_NOWAIT	0x0001		/* Don't wait for unavailable locks */
#define LS_IGNORE	0x0002		/* Ignore signals */
#define LS_CONFIDENT	0x0004		/* Allow messy terminations */

/*
 * specific error codes returned from the LOCKSET library, via lsetErr (defined
 * below).
 */

#define LSERR_NOERR		0	/* not an error */
#define LSERR_SYSTEM		1	/* system error */
#define	LSERR_PARAM		2	/* parameter error */

#define	LSERR_LOCKED		10	/* table is locked */
#define LSERR_UNLOCKED		11	/* table is unlocked */
#define LSERR_NOLOCK		12	/* can't lock table */
#define LSERR_NOUNLOCK		13	/* can't unlock table */
#define LSERR_GETKEY		14	/* can't get IPC identifier */
#define LSERR_LOCKSET		15	/* can't create/access lock set */
#define LSERR_ATTACH		16	/* can't attach share mem segment */
#define LSERR_NOSPACE		17	/* no memory/table space */
#define LSERR_INUSE		18	/* specified slot is in use */
#define LSERR_UNUSED		19	/* specified slot is unused */
#define LSERR_EXIST		20	/* lockset already exists */
#define LSERR_PERM		21	/* permission denied */

/*
 * externally visible globals.
 */

extern int	lsetErr;		/* global error number */

/*
 * lockset primitive declarations
 */

extern lockSetT	lsNew();		/* Create a new lock set */
extern lockSetT	lsUse();		/* Open an existing lock set */
extern int	lsRmv();		/* Remove a lock set */
extern int	lsCount();		/* Discover # of locks in set */
extern int	lsSet();		/* Apply locks to set */
extern int	lsClr();		/* Remove existing locks from set */
extern char FAR	*lsEString();		/* string version of an error */
extern void	lsUnuse();		/* 'unuse' lockset data */
