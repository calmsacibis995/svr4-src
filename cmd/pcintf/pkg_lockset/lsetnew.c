/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_lockset/lsetnew.c	1.1"
#include <sccs.h>

SCCSID("@(#)lsetnew.c	1.1	13:09:29	8/31/89");

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


/*
   lsetnew.c: Implementation of lock set creation function

   Exported functions:
	lockSet		*lsNew();
*/


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include <lockset.h>

#include <internal.h>

extern char FAR *malloc();

/* 
   lsNew: Create a new lock set

   Input Parameters:
	lockSetKey:		Key identifier for lock set
	permission:		Requested permissions
	minLocks:		Minimum acceptable number of locks in set
	maxLocks:		Maximum useful number of locks in set

   Tasks:
	- Create and initialize underlying lock structure
	  (file or semaphore set)
	- Initialize lockSet structure and return pointer to caller

   Outputs:
	Return Value: lockset ID for the new lock set
	lsetErr: Holds error code if return value is INVALID_LOCKSET
*/

lockSetT
lsNew(lockSetKey, permission, minLocks, maxLocks)
ulong			lockSetKey;
int			permission;
int			minLocks;
int			maxLocks;
{
key_t			semKey;		/* Semaphore set identifier */
int			semPerm;	/* Semaphore set permission code */
int			semDesc;	/* Semaphore set descriptor */
int			nLocks;		/* Number of locks in set */
int			initSem;	/* Initialize semaphores */
lockDataT FAR		*lockDataP;	/* Newly allocated lockSet */

	/* Validate arguments */
	if (minLocks <= 0 ||
	    maxLocks <= 0 ||
	    minLocks > maxLocks ||
	    lockSetKey == IPC_PRIVATE)
	{
		lsetErr = LSERR_PARAM;
		return INVALID_LOCKSET;
	}

	/*
	 * compute the semaphore identifier and permission code.
	 */

	semPerm = IPC_CREAT | IPC_EXCL;
	if (ANYSET(permission, LS_PRIVATE)) {
#ifdef	TBD
		/*
		 * check for conflicting permission bits
		 */

		if ((permission ^ LS_PRIVATE) != 0)
		{
			lsetErr = LSERR_PARAM;
			return INVALID_LOCKSET;
		}
#endif
		semKey = IPC_PRIVATE;
	} else {
		semKey	= (key_t)lockSetKey;

		if (ANYSET(permission, LS_O_WRITE)) semPerm |= UP_OW;
		if (ANYSET(permission, LS_G_WRITE)) semPerm |= UP_GW;
		if (ANYSET(permission, LS_W_WRITE)) semPerm |= UP_WW;
		if (ANYSET(permission, LS_O_READ)) semPerm |= UP_OR;
		if (ANYSET(permission, LS_G_READ)) semPerm |= UP_GR;
		if (ANYSET(permission, LS_W_READ)) semPerm |= UP_WR;
	}

	/* Try to get as many of maxLocks semaphores as available */
	for (nLocks = maxLocks; nLocks >= minLocks; nLocks--) {
		/* Attempt to create a new semaphore set */
		semDesc = semget(semKey, nLocks, semPerm);

		/* Break from loop on success */
		if (semDesc != FAIL)
			break;

		/* If semaphore set exists, return INVALID_LOCKSET */
		if (errno == EEXIST || errno == EINVAL)
		{
			lsetErr = LSERR_EXIST;
			return INVALID_LOCKSET;
		}
	}

	/* Check to see if semaphore set was successfully created */
	if (semDesc == FAIL) {
		if (errno == EACCES)
			lsetErr = LSERR_PERM;
		else
			lsetErr = LSERR_NOSPACE;
		return INVALID_LOCKSET;
	}

	/* Initialize all semaphores to maximum value */
	for (initSem = 0; initSem < nLocks; initSem++)
		if (semctl(semDesc, initSem, SETVAL, LS_SEMMAX) == FAIL) {
			/* "Can't happen" */
			/* Remove semaphore set and return INVALID_LOCKSET */
			semctl(semDesc, 0, IPC_RMID, 0);
			lsetErr = LSERR_SYSTEM;
			return INVALID_LOCKSET;
		}

	/* Allocate and initialize new lock set descriptor */
	lockDataP = (lockDataT FAR *)malloc(sizeof(lockDataT));
	if (nil(lockDataP))
	{
		lsetErr = LSERR_NOSPACE;
		return INVALID_LOCKSET;
	}
	lockDataP->semDesc = semDesc;
	lockDataP->nLocks = nLocks;
	lockDataP->nSet = 0;

	/* Return pointer to new lock set descriptor */
	return (lockSetT)lockDataP;
}
