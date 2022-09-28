/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/rlock_cfg.h	1.1"
SCCSID_H("@(#)rlock_cfg.h	1.2	15:46:33	11/20/89", rlock_cfg_h)

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
 *	rlock_cfg.h - internal RLOCK package configuration constants
 *
 *-------------------------------------------------------------------------
 */

/*
 * the shared memory has one or more version numbers.  note that these numbers
 * do NOT mean the same thing as the version number of the RLOCK library.  what
 * they do represent is the version of the shared memory format.  for example,
 * if the style of structure needs to change, the version number will change.
 * this will prevent old programs from working, but will allow new programs
 * to work in old environments.  luckily, at the present time, there will
 * probably be no use for this, but ...
 *
 * also note that there is a separate version number for each shared memory
 * 'set' of tables.  currently, only the basic tables exist, so only the
 * basic table versions are given.  future extensions may involve client
 * machines that are non-DOS for example.
 *
 * the known versions are:
 *
 *	VER_BASIC	- basic version
 */

#define VER_BASIC	((ushort) 1)

/*
 * these are the constants that are used internally.  they can't be changed
 * lightly!
 *
 *	DEF_MEM_BASE	- default memory base for attaching shared memory
 *	DEF_USER_KEY	- default user key (least significant 'short' ...)
 *	DEF_REC_LOCKS	- default # of record lock hash entries
 *	DEF_OPEN_TABLE	- default # of entries in the open file table
 *	DEF_FILE_TABLE	- default # of entries in the file header table
 *	DEF_HASH_TABLE	- default # of entries in the file hashed header table
 *	DEF_LOCK_TABLE	- default # of entries in the record lock table
 */

#define DEF_SHM_BASE		((char FAR *) 0)
#define DEF_USER_KEY		0x7372
#define DEF_REC_LOCKS		0
#define DEF_OPEN_TABLE		384
#define DEF_FILE_TABLE		256
#define	DEF_HASH_TABLE		60
#define DEF_LOCK_TABLE		512

/*
 * macros to create the required keys.  the BLD_KEY() macro should only
 * be used for the other macros in this section.
 *
 *	BLD_KEY(m, l)	- basis for the other macros
 *
 *	MAKE_SHM_KEY(k)	- make a shared memory key
 *	MAKE_LS_KEY(k)	- make a lockset key
 */

#define BLD_KEY(m, l)	(((ulong)(m) << 16) | ((ulong)(l) & 0x0000ffff))

#define MAKE_SHM_KEY(k)		((key_t)BLD_KEY(0x7372, k))
#define MAKE_LS_KEY(k)		((ulong)BLD_KEY(0x6c73, k))
