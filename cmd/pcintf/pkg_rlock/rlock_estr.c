/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/rlock_estr.c	1.1"
#include <sccs.h>

SCCSID("@(#)rlock_estr.c	1.1	13:36:15	8/31/89");

/*
   (c) Copyright 1985 by Locus Computing Corporation.  ALL RIGHTS RESERVED.

   This material contains valuable proprietary products and trade secrets
   of Locus Computing Corporation, embodying substantial creative efforts
   and confidential information, ideas and expressions.  No part of this
   material may be used, reproduced or transmitted in any form or by any
   means, electronic, mechanical, or otherwise, including photocopying or
   recording, or in connection with any information storage or retrieval
   system without permission in writing from Locus Computing Corporation.
*/

/*------------------------------------------------------------------------
 *
 *	rlock_estr.c - error strings for specific RLERR errors
 *
 *	routines included:
 *		rlockEString()
 *
 *-------------------------------------------------------------------------
 */

#include <rlock.h>

/*
 * return the maximum number of characters required (w/o the EOS) for an
 * int, in decimal format.  this is usually too big, but who cares?
 */

#define MAX_CHARS_INT		(sizeof(int) * 3)

/*
 * the unknown string (actually, a format).
 */

#define UNKNOWN_STR	"unknown RLERR error (%d)"

/*
 *	rlockEString() - print a string for a specific RLERR error
 *
 *	input:	error - the RLERR error number
 *
 *	proc:	just switch and return
 *
 *	output:	(char *) - the error string
 *
 *	global:	(none)
 */

char FAR *
rlockEString(error)
int error;
{	static char unknownStr[sizeof(UNKNOWN_STR)+MAX_CHARS_INT+1];

	/*
	 * if the error was added correctly, it will be in this list.
	 */

	switch (error)
	{
		case RLERR_NOERR:	return "not an error";
		case RLERR_SYSTEM:	return "system error";
		case RLERR_PARAM:	return "parameter error";
		case RLERR_VERSION:	return "unknown memory version";

		case RLERR_LOCKED:	return "data is locked";
		case RLERR_UNLOCKED:	return "data is unlocked";
		case RLERR_NOLOCK:	return "can't lock table";
		case RLERR_NOUNLOCK:	return "can't unlock table";
		case RLERR_GETKEY:	return "can't get IPC identifier";
		case RLERR_LOCKSET:	return "can't create/access lock set";
		case RLERR_NOSPACE:	return "no space available";
		case RLERR_INUSE:	return "specified slot is in use";
		case RLERR_UNUSED:	return "specified slot is unused";
		case RLERR_CORRUPT:	return "shared memory is corrupted";
		case RLERR_ADDR:	return "invalid attach address";
		case RLERR_EXIST:	return "segment already exists";
		case RLERR_PERM:	return "permission denied";
		case RLERR_FORMAT:	return "unknown shared memory format";
	}

	/*
	 * if we got here, the error was added, but no corresponding string
	 * exists.  try to give the user some information.
	 */

	sprintf(unknownStr, UNKNOWN_STR, error);
	return unknownStr;
}
