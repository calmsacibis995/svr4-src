/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/multi_cfg.h	1.1"
SCCSID_H("@(#)multi_cfg.h	1.2	15:46:14	11/20/89", multi_cfg.h)

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
 *	multi_cfg.h - multiple segment RLOCK package configuration constants
 *
 *	NOTE: this is the archaic method of shared memory.  any new table
 *	constants should go into rlock_cfg.h.  if there is justice in the
 *	world, this file, and references to it, will eventually be removed.
 *
 *-------------------------------------------------------------------------
 */

/*
 * these are the configuration constants for the multi-segment memory.  if
 * they need to be changed, it is probably a better idea to use the new
 * style.
 *
 *	SHM_OF_KEY	- open file shm key	('s', 'r', 'f', 'o')
 *	SHM_FH_KEY	- file header shm key	('s', 'r', 'h', 'f')
 *	SHM_LT_KEY	- lock table shm key	('s', 'r', 't', 'l')
 *	LS_KEY		- lockset key		('l', 's', 's', 'r')
 *	MAX_REC_LOCK	- maximum # of record lock hash entries
 *	MAX_OPEN_TABLE	- maximum # of entries in the open file table
 *	MAX_FILE_TABLE	- maximum # of entries in the file header table
 *	MAX_HASH_TABLE	- maximum # of entries in the file hashed header table
 *	MAX_LOCK_TABLE	- maximum # of entries in the record lock table
 */

#define SHM_OF_KEY		((key_t) 0x7372666f)
#define SHM_FH_KEY		((key_t) 0x73726866)
#define SHM_LT_KEY		((key_t) 0x7372746c)
#define LS_KEY			((ulong) 0x6c737372)
#define MAX_REC_LOCK		0
#define MAX_OPEN_TABLE		319
#define MAX_FILE_TABLE		368
#define	MAX_HASH_TABLE		61
#define MAX_LOCK_TABLE		255

/*
 * the base of the shared memory is a magic cookie.  there is no specific
 * algorithm that can be used to find it, and many machines use different
 * values.  if we could guarantee that a system selected value will always
 * be at the same virtual address, regardless of the program that created
 * or attached to it, all would be well.  however, that isn't always the
 * case, and we need to attach to the same virtual address, due to the
 * fact that pointers exist in the shared memory.  the following are the
 * values for the shared memory base for a variety of machines.
 */

#if	defined(ATT3B2)
#define SHM_BASE	((char FAR *)0xc00c0000)
#endif 
#if	defined(i386)
#define SHM_BASE	((char FAR *)0x80400000)
#endif 
#if	defined(AIX_RT)
#define SHM_BASE	((char FAR *)0x80000000)
#endif	
#if	defined(RIDGE) || defined(MOTOROLA) || defined(NCRTOWER) || \
	defined(CTIX) || defined(SYS19) || defined(ICM3216) || defined(SGM2)
#define SHM_BASE	((char FAR *)0x800000)
#endif 
#if	defined(XENIX)
#define SHM_BASE	((char FAR *)0x6f0000)
#endif 

#if	!defined(SHM_BASE)
#define SHM_BASE	((char FAR *)0xffffffff)
#endif 
