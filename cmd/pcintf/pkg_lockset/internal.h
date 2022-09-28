/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_lockset/internal.h	1.1"
SCCSID_H("@(#)internal.h	1.5	10:33:37	1/30/90", internal_h)

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
 *	internal.h - internal definitions for the LOCKSET package
 *
 *-------------------------------------------------------------------------
 */

/*
 * standard constants.
 */

#define SUCCESS		0			/* no error return code */
#define FAIL		(-1)			/* error return code */
#define NIL		((char FAR *) 0)	/* NIL pointer */

/*
 * the following primitive allows a pointer to be checked for NIL, without the
 * caller having to worry about casts.
 */

#define nil(pointer)	((char FAR *)(pointer) == NIL)

/*
 * useful macros:
 *
 *	ANYSET(x,y)	- are any of the bits set in y, also set in x?
 */

#define ANYSET(x,y)	(((x) & (y)) != 0)

/*
 * give C some resemblence of a logical structure. CAVEAT!! don't check a
 * bool value explicitly for TRUE (i.e., (val == TRUE)), since a TRUE
 * value may actually be any non-0 value.  while we're at it, some systems
 * define TRUE and FALSE in the most obscene places, so make sure we have
 * our own definition.
 */

typedef int		bool;
#if defined(ULTRIX) || defined(XENIX)
typedef unsigned long ulong;
#	if !defined(vax) && !defined(XENIX)
typedef unsigned short ushort;
#	endif	/* !vax && !XENIX */
#endif

#undef	TRUE
#undef	FALSE

#define TRUE	((bool) 1)
#define FALSE	((bool) 0)

/*
 * permission bits for file and IPC objects.
 */

#define UP_SUID		04000		/* setUID bit */
#define UP_SGID		02000		/* setGID bit */
#define UP_TEXT		01000		/* save text bit */
#define UP_OR		00400		/* owner read permission */
#define UP_OW		00200		/* owner write permission */
#define UP_OX		00100		/* owner execute */
#define UP_ORW		00600		/* owner read & write */
#define UP_ORWX		00700		/* owner read, write & execute */
#define UP_GR		00040		/* group read */
#define UP_GW		00020		/* group write */
#define UP_GX		00010		/* group execute */
#define UP_GRW		00060		/* group read & write */
#define UP_GRWX		00070		/* group read, write & execute */
#define UP_WR		00004		/* world read */
#define UP_WW		00002		/* world write */
#define UP_WX		00001		/* world execute */
#define UP_WRW		00006		/* world read & write */
#define UP_WRWX		00007		/* world read, write & execute */

#define UP_OSHIFT	6		/* bit offset of owner permissions */
#define UP_GSHIFT	3		/* offset to group permissions */
#define UP_WSHIFT	0		/* world permissions */
#define UP_MASK		7		/* mask bits for permissions */

#define UP_PERM		0777		/* permission bits */

/*
 * the internal format of the lockSetT descriptor is provided as the
 * following data.  note that these fields hold values that are used by the
 * appropriate system calls, therefore 'int' is used.
 */

typedef struct {
	int		semDesc;	/* Semaphore descriptor */
	int		nLocks;		/* Number of locks in set */
	int		nSet;		/* Number of locks currently set */
	struct sembuf	semOps[LS_MAXSETS]; /* Semaphore operation array */
} lockDataT;

#define NIL_LOCKDATA	((lockDataT FAR *) 0)

/*
 * there are some internal constants needed by the lockset functions ...
 *
 *	LS_SEMMAX	- this is the maximum value in a semaphore
 */

#define LS_SEMMAX	255

/*
 * prioritized register storage class modifiers.  some of these may be
 * unconfirmed rumors.  they should be checked.
 */

#if	defined(u3b) || defined(u3b2) || defined(u3b5) || defined(vax)
#define r0	register
#define r1	register
#define r2	register
#define r3	register
#define r4	register
#define r5	register
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(u3b), et. al. */

#if	defined(m68k)
#define r0	register
#define r1	register
#define r2	register
#define r3	register
#define r4	register
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(m68k) */

#if	defined(u370)
#define r0	register
#define r1	register
#define r2	register
#define r3	register
#define r4
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(u370) */

#if	defined(i386)
#define r0	register
#define r1	register
#define r2	register
#define r3
#define r4
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(i386) */

#if	defined(iAPX286) || defined(M_I86)
#define r0	register
#define r1	register
#define r2
#define r3
#define r4
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(iAPX286) */

#if	!defined(r0)
#define	r0
#define r1
#define r2
#define r3
#define r4
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* !defined(r0) */
