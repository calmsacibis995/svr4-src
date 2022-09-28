/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/rlock_prim.c	1.1"
#include <sccs.h>

SCCSID("@(#)rlock_prim.c	1.1	13:36:39	8/31/89"); 

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

/*--------------------------------------------------------------------------
 *
 *	lockprim.c - primitive interface to the LOCKSET package for RLOCK
 *
 *	exported functions:
 *		_rlInitLocks()
 *		_rlSetLocks()
 *		_rlUnlock()
 *
 *--------------------------------------------------------------------------
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include <lockset.h>

#include <rlock.h>

#include <internal.h>

/*
 * internal variables
 */

static lockSetT lockSet = INVALID_LOCKSET;	/* lockset */

/*
 *	_rlInitLocks() - initialize the lock data
 *
 *	input:	userKey - the user key
 *
 *	proc:	invoke lsUse() to set up the locks, lsCount to determine the
 *		number of locks available.  FAIL is returned if something goes
 *		wrong.
 *
 *	output:	(int) - count of locks or FAIL
 *
 *	global:	lockSet - set
 *		rlockErr - may be set
 */

int
_rlInitLocks(userKey)
ulong userKey;
{
	/*
	 * if the lockset has already been initialized, return the count.
	 */

	if (lockSet != INVALID_LOCKSET)
		return lsCount(lockSet);

	/*
	 * we need to really initialize.
	 */

	lockSet = lsUse(userKey);
	if (lockSet == INVALID_LOCKSET)
	{
		rlockErr = RLERR_LOCKSET;
		return FAIL;
	}
	return lsCount(lockSet);
}

/* 
 *	_rlSetLocks() - set two locks in the lockset
 *
 *	input:	lockNum1 - first lock to set or ALL_LOCK
 *		lockNum2 - second lock to set or UNUSED_LOCK
 *
 *	proc:	invoke lsSet() to set up the locks.  note that ALL_LOCK and
 *		UNUSED_LOCK can be used in the same call.
 *
 *	output:	(bool) - were the lock set?
 *
 *	global:	lockSet - checked
 *		rlockErr - may be set
 */

bool
_rlSetLocks(lockNum1, lockNum2)
int lockNum1, lockNum2;
{	int lsetRet;

	/*
	 * if the stuff wasn't initialized, return the error.
	 */

	if (lockSet == INVALID_LOCKSET)
	{
		rlockErr = RLERR_PARAM;
		return FALSE;
	}

	/*
	 * we have something, so try to lock, and return an appropriate error
	 * condition.
	 */

	if (lockNum1 == ALL_LOCK)
		lsetRet = lsSet(lockSet, LS_IGNORE, -OF_LOCK, -LT_LOCK,
				-RL_LOCK0, UNUSED_LOCK);
	else
		lsetRet = lsSet(lockSet, LS_IGNORE, lockNum1, lockNum2,
				UNUSED_LOCK);
	if (lsetRet == FAIL)
	{
		rlockErr = RLERR_LOCKSET;
		return FALSE;
	}
	return TRUE;
}

/*
 *	_rlUnlock() - remove all locks on the lockset
 *
 *	input:	(none)
 *
 *	proc:	invoke lsClr() to remove the locks.
 *
 *	output:	(void) - none
 *
 *	global:	lockSet - used
 */

void
_rlUnlock()
{
	if (lockSet != INVALID_LOCKSET) (void)lsClr(lockSet);
}

#if	defined(WATCH_LOCKS_TBD)
/*
 * this is old code from ancient sources.  its purpose is to force the 'set
 * locks' routine above to be more careful about the possibility of getting
 * hung in the locking calls.  however, it uses an alarm() call to make sure
 * a hang doesn't happen, but it is careless with the alarm clock setting.
 * this should be fixed if the code is to be used.
 */

{	time_t	startTime;		/* Time when called */
	time_t	nowTime;		/* Current time */

	/* Make sure lock doesn't hang forever */
	startTime = time((long *) 0);
	alarm((unsigned) MAX_LOCK_WAIT);

	for (;;) {
		/* Attempt to set the requested locks */
		if (lsSet(svrLocks, 0, lockNum1, lockNum2, 0) == 0)
			break;

		/* If the lock was interrupted, count retries */
		if (utilErr != EINTR) {
		   log("lock2: Error locking: %d, %d; utilErr: %d; errno: %d\n",
				lockNum1, lockNum2, utilErr, errno);
			break;
		} else {
			/* If too much time has elapsed, abort lock attempt */
			nowTime = time((long *) 0);
			if (nowTime - startTime >= MAX_LOCK_WAIT) {
				log("Timeout getting locks %d, %d\n",
					lockNum1, lockNum2);
				break;
			}
			alarm(MAX_LOCK_WAIT - (nowTime - startTime));
		}
	}

	/* Reset alarm */
	alarm(0);
}
#endif
