/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_lockset/lsetuse.c	1.1"
#include <sccs.h>

SCCSID("@(#)lsetuse.c	1.1	13:10:12	8/31/89");

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
   lsetuse.c: Implementation of lock set access function.

   Exported functions:
	lockSet		*lsUse();
*/


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include <lockset.h>

#include <internal.h>

extern char FAR *malloc();

/* 
   lsUse: Open a lock set for synchronization use

   Input Parameters:
	lockSetKey:		Identifier for the lockset to open

   Tasks:
	"Open" underlying lock structure (file or semaphore set)
	Initialize lockSet structure and return pointer to caller

   Outputs:
	Return Value: lockset ID
	lsetErr: Holds error code if return value is INVALID_LOCKSET
*/

lockSetT
lsUse(lockSetKey)
ulong lockSetKey;
{
int			semDesc;	/* Semaphore set descriptor */
struct semid_ds		semData;	/* Semaphore description info */
r0 lockDataT FAR	*lockDesc;	/* Lock set descriptor */
key_t			semKey;		/* Semaphore set key (name) */

	/* Compute semaphore set's key (its name) */
	semKey = (key_t)lockSetKey;

	/* Get a semaphore descriptor for the semaphore set */
	if ((semDesc = semget(semKey, 0, UP_ORW)) == FAIL)
		if ((semDesc = semget(semKey, 0, UP_WRW)) == FAIL)
			if ((semDesc = semget(semKey, 0, UP_GRW)) == FAIL) {
				lsetErr = LSERR_SYSTEM;
				return INVALID_LOCKSET;
			}

	/* Fetch semaphore set description information */
	if (semctl(semDesc, 0, IPC_STAT, &semData) == FAIL)
	{
		lsetErr = LSERR_SYSTEM;
		return INVALID_LOCKSET;
	}

	/* Allocate and initialize new lock set descriptor */
	lockDesc = (lockDataT FAR *)malloc(sizeof(lockDataT));
	if (nil(lockDesc))
	{
		lsetErr = LSERR_NOSPACE;
		return INVALID_LOCKSET;
	}
	lockDesc->semDesc = semDesc;
	lockDesc->nLocks = semData.sem_nsems;
	lockDesc->nSet = 0;

	/* Return pointer to new lock set descriptor */
	return (lockSetT)lockDesc;
}
