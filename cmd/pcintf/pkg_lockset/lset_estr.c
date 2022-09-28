/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_lockset/lset_estr.c	1.1"
#include <sccs.h>

SCCSID("@(#)lset_estr.c	1.1	13:08:28	8/31/89");

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
 *	lset_estr.c - error strings for specific LSERR errors
 *
 *	routines included:
 *		lsEString()
 *
 *-------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <lockset.h>

/*
 * return the maximum number of characters required (w/o the EOS) for an
 * int, in decimal format.  this is usually too big, but who cares?
 */

#define MAX_CHARS_INT		(sizeof(int) * 3)

/*
 * the unknown string (actually, a format).
 */

#define UNKNOWN_STR	"unknown LSERR error (%d)"

/*
 *	lsEString() - print a string for a specific LSERR error
 *
 *	input:	error - the LSERR error number
 *
 *	proc:	just switch and return
 *
 *	output:	(char *) - the error string
 *
 *	global:	(none)
 */

char FAR *
lsEString(error)
int error;
{	char unknownStr[sizeof(UNKNOWN_STR)+MAX_CHARS_INT+1];

	/*
	 * if the error was added correctly, it will be in this list.
	 */

	switch (error)
	{
		case LSERR_NOERR:	return "not an error";
		case LSERR_SYSTEM:	return "system error";
		case LSERR_PARAM:	return "parameter error";

		case LSERR_LOCKED:	return "table is locked";
		case LSERR_UNLOCKED:	return "table is unlocked";
		case LSERR_NOLOCK:	return "can't lock table";
		case LSERR_NOUNLOCK:	return "can't unlock table";
		case LSERR_GETKEY:	return "can't get IPC identifier";
		case LSERR_LOCKSET:	return "can't create/access lock set";
		case LSERR_ATTACH:	return "can't attach share mem segment";
		case LSERR_NOSPACE:	return "no memory/table space";
		case LSERR_INUSE:	return "specified slot is in use";
		case LSERR_UNUSED:	return "specified slot is unused";
		case LSERR_EXIST:	return "lockset already exists";
		case LSERR_PERM:	return "permission denied";
	}

	/*
	 * if we got here, the error was added, but no corresponding string
	 * exists.  try to give the user some information.
	 */

	sprintf(unknownStr, UNKNOWN_STR, error);
	return unknownStr;
}
