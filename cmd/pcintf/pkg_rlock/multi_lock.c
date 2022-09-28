/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/multi_lock.c	1.1"
#include <sccs.h>

SCCSID("@(#)multi_lock.c	1.1	13:35:08	8/31/89");

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

/*
   multi_lock.c: Record Lock Table Operations

   Exported functions:
	_rlAddLock:		Apply a record lock to an openFile
	_rlRmvLock:		Remove a record lock
	_rlIOStart:		Check locks for I/O and exclude lockers
	_rlIODone:			Release locks for I/O and release lockers
	_rlRstLocks:		Reset a DOS process' record locks
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
 * internal functions
 */

static oldRecLock FAR	*newLock();
static void		lockFree();
static bool		hasLocks();

/*
 * since the _rlIOStart() and _rlIODone() function pair locks out a specific
 * record lock for the duration of the period between the two calls, only one
 * process may do the I/O.  to relieve this, it is possible to create an
 * array of available locks, and to hash the file's unique ID to index one
 * of these.  this reduces the possiblity of conflict, up to the limit
 * defined by the system.  this macro handles that hashing function.
*/

#define RL_NUM(uniqID)	(RL_LOCK0 + ((uniqID) % _rlNHashLocks))

/* 
   _rlAddLock: Add a lock to an open server file

   Input Parameters:
	openEntry: Index to global open file table
	dosPID: Process id of DOS process setting lock
	lockLow: First byte to lock
	lockCount: Number of bytes to lock

   Tasks:
	(description of what's done)

   Outputs:
	Return Value: SUCCESS/FAIL
	(global variable): (global side effect description)
*/

int
_rlAddLock(openEntry, dosPID, lockLow, lockCount)
int		openEntry;
short		dosPID;
r3 long		lockLow;
unsigned long	lockCount;
{
r4 long		lockHi;			/* Last byte of record lock */
oldOpenFile FAR	*oFile;			/* The open file to add a lock to */
oldFileHdr FAR	*fHdr;			/* File header for the open file */
oldRecLock FAR	*llAdd = NIL_O_RECLOCK;	/* Where to add new lock */
r0 oldRecLock FAR	*llScan;		/* Scan the lock list */
r1 oldRecLock FAR	*llNew;			/* New record lock structure */
r2 oldRecLock FAR	*llNext;		/* Next recLock in lockList */

	/* Validate lock range */
	if (lockCount == 0) {
		rlockErr = RLERR_PARAM;
		return FAIL;
	}

	/* Get a pointer to the openFile and its fileHdr */
	if (nil(oFile = _rlOGetOFile(openEntry))) {
		rlockErr = RLERR_PARAM;
		return FAIL;
	}
	fHdr = oFile->header;

	/* Compute high byte of lock range */
	lockHi = lockLow + lockCount - 1;

	/* Exclude other servers from the record lock table */
	if (!_rlSetLocks(-LT_LOCK, -RL_NUM(fHdr->uniqueID)))
		return FAIL;

	/* Get a new record lock structure */
	if (nil(llNew = newLock())) {
		_rlUnlock();
		rlockErr = RLERR_NOLOCK;
		return FAIL;
	}

	/* Record the lock information in the new recLock */
	llNew->lockLow = lockLow;
	llNew->lockHi = lockHi;
	llNew->sessID = oFile->sessID;
	llNew->dosPID = dosPID;

	/*
	   If there are no other locks or the new lock fits before the first
	   existing lock, add the lock at the beggining of the lock list
	*/
	if (nil(fHdr->lockList) || lockHi < fHdr->lockList->lockLow)
		llAdd = NIL_O_RECLOCK;
	else {
		/* See if any existing locks overlap the requested new lock */
		for (llScan = fHdr->lockList; !nil(llScan); llScan = llNext) {
			/* Get pointer to next recLock in list */
			llNext = llScan->nextLock;

			/* If new lock overlaps existing lock, fail */
			if ((lockLow >= llScan->lockLow
			     && lockLow <= llScan->lockHi)
			||  (lockHi >= llScan->lockLow
			     && lockHi <= llScan->lockHi))
			{
				lockFree(llNew);
				_rlUnlock();
				rlockErr = RLERR_INUSE;
				return FAIL;
			}

			/* If new lock fits here, add it */
			if (lockLow > llScan->lockHi
			&&  (nil(llNext) || lockHi < llNext->lockLow)) {
				llAdd = llScan;
				break;
			}
		}
	}

	/* If new lock goes first in the list, change the file header */
	if (nil(llAdd)) {
		llNew->nextLock = fHdr->lockList;
		fHdr->lockList = llNew;
	} else {
		llNew->nextLock = llNext;
		llAdd->nextLock = llNew;
	}

	/* Release lock table exclusion and return */
	_rlUnlock();
	rlockState(openEntry, RLSTATE_LOCKED);
	return SUCCESS;
}


int
_rlRmvLock(openEntry, dosPID, lockLow, lockCount)
int		openEntry;
short		dosPID;
r1 long		lockLow;
unsigned long	lockCount;
{
r2 long		lockHi;			/* Last byte of I/O */
oldOpenFile FAR	*oFile;			/* The open file to add a lock to */
oldFileHdr FAR	*fHdr;			/* File header for the open file */
r0 oldRecLock FAR	*llScan;		/* Scan the lock list & new lock */
oldRecLock FAR	*llPrev = NIL_O_RECLOCK;	/* RecLock preceding one to remove */
short		sessID;			/* Session ID of caller */
bool		isLocked;

	/* Get a pointer to the openFile */
	if (nil(oFile = _rlOGetOFile(openEntry))) {
		rlockErr = RLERR_PARAM;
		return FAIL;
	}
	fHdr = oFile->header;
	sessID = oFile->sessID;

	/* Compute high byte of lock range */
	lockHi = lockLow + lockCount - 1;

	/* Exclude other servers from the record lock table */
	if (!_rlSetLocks(-LT_LOCK, -RL_NUM(fHdr->uniqueID)))
		return FAIL;

	/* Search for lock to remove */
	for (llScan = fHdr->lockList; !nil(llScan); llScan = llScan->nextLock)
	{
		/* Does this lock match and was it applied by caller? */
		if (llScan->lockLow == lockLow && llScan->lockHi == lockHi)
			if (llScan->dosPID == dosPID && llScan->sessID == sessID)
				break;
			else
				goto badUnlock;

		/* If new lock overlaps existing lock, fail */
		if ((lockLow >= llScan->lockLow && lockLow <= llScan->lockHi)
		||  (lockHi >= llScan->lockLow && lockHi <= llScan->lockHi))
			goto badUnlock;

		llPrev = llScan;
	}

	/* If there was no lock to remove, fail */
	if (nil(llScan))
		goto badUnlock;

	/* Remove the lock from the file header's lock list */
	if (nil(llPrev))
		fHdr->lockList = llScan->nextLock;
	else
		llPrev->nextLock = llScan->nextLock;

	/* Free the removed lock */
	lockFree(llScan);

	/* Release lock table exclusion and return */
	isLocked = hasLocks(fHdr, sessID, dosPID);
	_rlUnlock();
	if (!isLocked) rlockState(openEntry, RLSTATE_ALL_UNLOCKED);
	return SUCCESS;

badUnlock:
	_rlUnlock();
	rlockErr = RLERR_NOUNLOCK;
	return FAIL;
}


int
_rlIOStart(openEntry, dosPID, ioLow, ioCount)
int		openEntry;
short		dosPID;
r1 long		ioLow;
long		ioCount;
{
r2 long		ioHi;			/* Last byte of I/O */
oldOpenFile FAR	*oFile;			/* The open file to add a lock to */
oldFileHdr FAR	*fHdr;			/* File header for the open file */
r0 oldRecLock FAR	*llScan;		/* Scan the lock list & new lock */
short		sessID;			/* Session ID of caller */

	/* Get a pointer to the openFile */
	if (nil(oFile = _rlOGetOFile(openEntry))) {
		rlockErr = RLERR_PARAM;
		return FAIL;
	}

	/* if in DOS compatability mode, don't bother checking locks. */
	if (oFile->denyMode == OFD_DOS_COMPAT)
		return SUCCESS;
	fHdr = oFile->header;
	sessID = oFile->sessID;

	/* Compute high byte of lock range */
	ioHi = ioLow + ioCount - 1;

	/* Exclude other servers from the record lock table */
	if (!_rlSetLocks(RL_NUM(fHdr->uniqueID), UNUSED_LOCK))
		return FAIL;

	/* See if any locks overlap the requested I/O range */
	for (llScan = fHdr->lockList; !nil(llScan); llScan = llScan->nextLock)
	{
		/* If lock doesn't overlap I/O request, go on */
		if (ioHi < llScan->lockLow || ioLow > llScan->lockHi)
			continue;

		/* If lock was applied by caller, it doesn't exclude the I/O */
		if (llScan->dosPID == dosPID && llScan->sessID == sessID)
			continue;

		_rlUnlock();
		rlockErr = RLERR_INUSE;
		return FAIL;
	}

	/* No lock conflicts - lock table exclusion remains! */
	return SUCCESS;
}


void
_rlIODone()
{
	/*
	 * release the lock table exclusion.  if we didn't lock (due to being
	 * in DOS compatability mode), _rlUnlock() will just return without
	 * doing anything.
	 */

	_rlUnlock();
}


int
_rlRstLocks(openEntry, dosPID)
int		openEntry;
short		dosPID;
{
oldOpenFile FAR	*oFile;			/* The open file to add a lock to */
oldFileHdr FAR	*fHdr;			/* File header for the open file */
r0 oldRecLock FAR	*llScan;		/* Scan the lock list & new lock */
r1 oldRecLock FAR	*llPrev = NIL_O_RECLOCK;	/* RecLock preceding one to remove */
r2 oldRecLock FAR	*llNext;		/* Next lock record in list */
short		sessID;			/* Session ID of caller */

	/* Get a pointer to the openFile */
	if (nil(oFile = _rlOGetOFile(openEntry))) {
		rlockErr = RLERR_PARAM;
		return FAIL;
	}
	fHdr = oFile->header;
	sessID = oFile->sessID;

	/* Exclude other servers from the record lock table */
	if (!_rlSetLocks(-LT_LOCK, -RL_NUM(fHdr->uniqueID)))
		return FAIL;

	/* Search for lock to remove */
	for (llScan = fHdr->lockList; !nil(llScan); llScan = llNext) {
		/* Get the next lock list element while the getting is good */
		llNext = llScan->nextLock;

		/* Was this lock applied by the caller? */
		if (llScan->dosPID != dosPID || llScan->sessID != sessID) {
			llPrev = llScan;
			continue;
		}

		/* Remove the lock from the file header's lock list */
		if (nil(llPrev))
			fHdr->lockList = llScan->nextLock;
		else
			llPrev->nextLock = llScan->nextLock;

		/* Free the removed lock */
		lockFree(llScan);
	}

	/* Release lock table exclusion and return */
	_rlUnlock();
	rlockState(openEntry, RLSTATE_ALL_UNLOCKED);
	return SUCCESS;
}


/*
   newLock: Get a recLock from the free list.
	NOTE: The lock table must be locked when newLock is called.
*/

static oldRecLock FAR *
newLock()
{
r0 oldRecLock FAR	*rLock;			/* The newly allocated recLock */

	if (nil(rLock = *_rlockShm.oLockFreePP))
		return NIL_O_RECLOCK;

	*_rlockShm.oLockFreePP = rLock->nextLock;
	rLock->nextLock = NIL_O_RECLOCK;
	return rLock;
}


/*
   lockFree: Return a recLock to the free list.
	NOTE: The lock table must be locked when lockFree is called.
*/

static void
lockFree(rLock)
r0 oldRecLock FAR	*rLock;			/* Free this recLock */
{
	rLock->nextLock = *_rlockShm.oLockFreePP;
	*_rlockShm.oLockFreePP = rLock;
	rLock->lockHi = 0L;
}

/*
 *	STATIC	hasLocks() - does the specified file have any locks?
 *
 *	WARN:	the lock table must be locked when this function is called.
 *
 *	input:	fHdrP - the file header
 *		sessID - the session ID
 *		dosPID - the DOS PID
 *
 *	proc:	return TRUE as soon as a lock is found on the file, that was
 *		set by the specified process.  return FALSE is no match is
 *		found.
 *
 *	output:	(bool) - was a lock set?
 *
 *	global:	(none)
 */

static bool
hasLocks(fHdrP, sessID, dosPID)
oldFileHdr FAR *fHdrP;
r1 long sessID;
r2 long dosPID;
{	r0 oldRecLock FAR *scanP;

	/*
	 * search for the specified lock.  if one isn't found, return FALSE.
	 * if one is found, return TRUE.
	 */

	for (scanP = fHdrP->lockList; !nil(scanP); scanP = scanP->nextLock) {
		if (scanP->dosPID == dosPID && scanP->sessID == sessID)
			return TRUE;
	}
	return FALSE;
}
