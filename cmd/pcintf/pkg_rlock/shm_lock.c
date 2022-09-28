/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/shm_lock.c	1.1"
#include <sccs.h>

SCCSID("@(#)shm_lock.c	1.3	10:53:04	1/30/90");

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
 *	shm_lock.c - record locking operations
 *
 *	routines included:
 *		addLock()
 *		rmvLock()
 *		ioStart()
 *		ioDone()
 *		rstLocks()
 *
 *	comments:
 *
 *--------------------------------------------------------------------------
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <values.h>
#include <errno.h>

#include <lockset.h>

#include <rlock.h>

#include <internal.h>

/*
 * since the ioStart() and ioDone() function pair locks out a specific
 * record lock for the duration of the period between the two calls, only one
 * process may do the I/O.  to relieve this, it is possible to create an
 * array of available locks, and to hash the file's unique ID to index one
 * of these.  this reduces the possiblity of conflict, up to the limit
 * defined by the system.  this macro handles that hashing function.
 */

#define RL_NUM(uniqID)	(RL_LOCK0 + UNIQ_INT_HASH(uniqID, _rlNHashLocks))

/*
 * internal functions
 */

static recLockT FAR	*newLock();
static void		freeLock();
static bool		hasLocks();
static bool		doHostLock();

/* 
 *	addLock() - add a lock to an open file
 *
 *	input:	openEntry - index into the global open file table
 *		dosPID - process id of the DOS process setting the lock
 *		lockLow - first byte to lock
 *		lockCount - number of bytes to lock
 *
 *	proc:	add the lock.  we need to verify that the file is open, and
 *		that the requested lock doesn't overlap an existing one.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	_rlockShm - checked
 */

int
addLock(openEntry, dosPID, lockLow, lockCount)
int openEntry;
long dosPID;
r3 unsigned long lockLow;
unsigned long lockCount;
{	r0 recLockT FAR *scanLockP;
	r1 recLockT FAR *newLockP;
	r2 recLockT FAR *nextLockP;
	r4 unsigned long lockHi;
	openFileT FAR *openFileP;
	fileHdrT FAR *fileHdrP;
	indexT lockIndex, newIndex;
	int saveRlockErr;

	/*
	 * if this is old style memory, we need to use a different function.
	 * _rlAddLock() will set the rlockErr value.
	 */

	if (_rlockShm.useOldStyle)
		return _rlAddLock(openEntry, (short)dosPID, lockLow, lockCount);

	/*
	 * validate the lock range, and convert the byte count into a high
	 * lock location.
	 */

	if (lockCount == 0)
	{
		rlockErr = RLERR_PARAM;
		return FAIL;
	}
	lockHi = lockLow + lockCount - 1;

	/*
	 * get a pointer to the (currently open) file which will have the
	 * locks set, and the pointer to the corresponding file header
	 * structure.  _rlGetOFile() will set the rlockErr value.
	 */

	openFileP = _rlGetOFile(openEntry);
	if (nil(openFileP))
		return FAIL;
	fileHdrP = &_rlockShm.fhdrTableP[openFileP->ofFHdrIndex];

	/*
	 * now the greedy part: exclude other programs from accessing the
	 * lock table.  _rlSetLocks() will set the rlockErr value.
	 */

	if (!_rlSetLocks(-LT_LOCK, -RL_NUM(fileHdrP->fhUniqueID)))
		return FAIL;

	/*
	 * get and initialize a new record lock structure.  newLock() will set
	 * rlockErr on failure.  since _rlUnlock() may also set it, the
	 * error must be saved.
	 */

	newLockP = newLock((indexT)openEntry, lockLow, lockHi,
				openFileP->ofSessID, dosPID);
	if (nil(newLockP))
	{
		saveRlockErr = rlockErr;
		_rlUnlock();
		rlockErr = saveRlockErr;
		return FAIL;
	}
	newIndex = newLockP - _rlockShm.rlockTableP;

	/*
	 * first existing lock, we just need to add the lock to the head of
	 * the lock list (signified by a NIL value for scanLockP).  if we
	 * can't get off that easy, we need to loop to look for the proper
	 * location in the list, and to verify that we don't overlap an
	 * existing lock.
	 */

	lockIndex = fileHdrP->fhLockIndex;
	if (lockIndex == INVALID_INDEX)
		scanLockP = NIL_RECLOCK;
	else
	{
		for (scanLockP = &_rlockShm.rlockTableP[lockIndex];
		     !nil(scanLockP);
		     scanLockP = nextLockP)
		{
			/*
			 * convert the next index into a pointer to the
			 * next data.
			 */

			if (scanLockP->rlNextIndex == INVALID_INDEX)
			    nextLockP = NIL_RECLOCK;
			else
			    nextLockP =
				&_rlockShm.rlockTableP[scanLockP->rlNextIndex];

			/*
			 * if the requested lock overlaps one that already
			 * exists, fail.
			 */

			if ((lockLow >= scanLockP->rlLockLow &&
			     lockLow <= scanLockP->rlLockHi) ||
			    (lockHi >= scanLockP->rlLockLow &&
			     lockHi <= scanLockP->rlLockHi))
			{
				freeLock(openFileP, newLockP);
				_rlUnlock();
				rlockErr = RLERR_INUSE;
				return FAIL;
			}

			/*
			 * we don't overlap, so check if the lock can go
			 * here.  if the new lock begins after the one we are
			 * currently indicating via scanLockP, and either
			 * there is no next lock, or the next lock's region
			 * is after the new one, we can add it here, break
			 * from the loop.
			 */

			if (lockLow > scanLockP->rlLockHi &&
			    (nil(nextLockP) || lockHi < nextLockP->rlLockLow))
			{
				break;
			}
		}
	}

	/*
	 * if scanLockP is NIL, the new lock will be entered at the head of
	 * the lock list.  otherwise, scanLock is the entry we currently are
	 * looking at in the list.  if nextLock is NIL, then scanLock is at
	 * the end of the list, so the new entry goes at the end of the
	 * list.  finally, if neither is NIL, the new entry goes between them.
	 */

	if (nil(scanLockP))
	{
		newLockP->rlNextIndex = fileHdrP->fhLockIndex;
		fileHdrP->fhLockIndex = newIndex;
	}
	else
	{
		if (nil(nextLockP))
			newLockP->rlNextIndex = INVALID_INDEX;
		else
			newLockP->rlNextIndex =
				nextLockP - _rlockShm.rlockTableP;
		scanLockP->rlNextIndex = newIndex;
	}

	/*
	 * we are done with the table, so release it.  then record the state
	 * change, and return.
	 */

	_rlUnlock();
	rlockState(openEntry, RLSTATE_LOCKED);
	return SUCCESS;
}

/*
 *	rmvLock() - remove a lock from a file
 *
 *	input:	openEntry - index into the global open file table
 *		dosPID - process id of the DOS process setting the lock
 *		lockLow - first byte to lock
 *		lockCount - number of bytes to lock
 *
 *	proc:	find the specified lock in the list and remove it.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	_rlockShm - checked
 */

int
rmvLock(openEntry, dosPID, lockLow, lockCount)
int openEntry;
long dosPID;
r1 unsigned long lockLow;
unsigned long lockCount;
{	r0 recLockT FAR	*scanLockP;
	r2 unsigned long lockHi;
	openFileT FAR *openFileP;
	fileHdrT FAR *fileHdrP;
	recLockT FAR *prevLockP;
	long sessID;
	bool isLocked;
	indexT rlockIndex;

	/*
	 * if this is old style memory, we need to use a different function.
	 * _rlRmvLock() will set the rlockErr value.
	 */

	if (_rlockShm.useOldStyle)
		return _rlRmvLock(openEntry, (short)dosPID, lockLow, lockCount);

	/*
	 * get a pointer to the open file, plus a pointer to the associated
	 * file header, and a local version of the session ID of the open
	 * file's owner.  while we're here, compute the high byte of the
	 * lock range.  _rlGetOFile() will set the rlockErr value.
	 */

	openFileP = _rlGetOFile(openEntry);
	if (nil(openFileP))
		return FAIL;
	fileHdrP = &_rlockShm.fhdrTableP[openFileP->ofFHdrIndex];
	sessID = openFileP->ofSessID;
	lockHi = lockLow + lockCount - 1;

	/*
	 * now, gain exclusive access to the lock table.  _rlSetLocks() will set
	 * the rlockErr value.
	 */

	if (!_rlSetLocks(-LT_LOCK, -RL_NUM(fileHdrP->fhUniqueID)))
		return FAIL;

	/*
	 * search the table for the lock to remove.  if this loop terminates
	 * 'normally', we didn't find the lock.
	 */

	prevLockP = NIL_RECLOCK;
	for (rlockIndex = fileHdrP->fhLockIndex;
	     rlockIndex != INVALID_INDEX;
	     rlockIndex = scanLockP->rlNextIndex)
	{
		/*
		 * convert the index into a table pointer.
		 */

		scanLockP = &_rlockShm.rlockTableP[rlockIndex];

		/*
		 * does this lock match the one requested?
		 */

		if (scanLockP->rlLockLow == lockLow &&
		    scanLockP->rlLockHi == lockHi)
		{
			/*
			 * was it applied by the caller?  if not, fail.
			 */

			if (scanLockP->rlDosPID != dosPID || 
			    scanLockP->rlSessID != sessID)
			{
				break;
			}

			/*
			 * the lock matched, and was applied by the caller,
			 * so remove it from the lock list, then free the
			 * table entry.
			 */

			if (nil(prevLockP))
				fileHdrP->fhLockIndex =
						scanLockP->rlNextIndex;
			else
				prevLockP->rlNextIndex =
						scanLockP->rlNextIndex;
			freeLock(openFileP, scanLockP);

			/*
			 * determine if this is the last lock, then release
			 * the table, record the non-locked condition of the
			 * file is appropriate, and return.  the recording
			 * is done after releasing the table, so we don't
			 * tie up the shared memory.
			 */

			isLocked = hasLocks(fileHdrP, (indexT)openEntry,
						sessID, dosPID);
			_rlUnlock();
			if (!isLocked)
				rlockState(openEntry, RLSTATE_ALL_UNLOCKED);
			return SUCCESS;
		}

		/*
		 * if the lock overlaps an existing one, fail.
		 */

		if ((lockLow >= scanLockP->rlLockLow &&
		     lockLow <= scanLockP->rlLockHi) ||
		    (lockHi >= scanLockP->rlLockLow &&
		     lockHi <= scanLockP->rlLockHi))
		{
			break;
		}

		/*
		 * yet another uneventful loop.  save a pointer to the current
		 * entry, so we can use it as the previous entry on the next
		 * iteration.
		 */

		prevLockP = scanLockP;
	}

	/*
	 * the loop terminated 'normally', so the requested lock wasn't found,
	 * and there were no other abnormallities.  return the appropriate
	 * failure.
	 */

	_rlUnlock();
	rlockErr = RLERR_NOUNLOCK;
	return FAIL;
}

/*
 *	ioStart() - handle the record locking checks
 *
 *	input:	openEntry - index into the global open file table
 *		dosPID - process id of the DOS process setting the lock
 *		lockLow - first byte to check
 *		lockCount - number of bytes to check
 *
 *	proc:	check that the requested range within the file hasn't been
 *		locked.  if it is, the function will fail.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	_rlockShm - checked
 */

int
ioStart(openEntry, dosPID, lockLow, lockCount)
int openEntry;
long dosPID;
r1 unsigned long lockLow;
unsigned long lockCount;
{	r0 recLockT FAR *scanLockP;
	r2 long ioHi;
	openFileT FAR *openFileP;
	fileHdrT FAR *fileHdrP;
	long sessID;
	indexT rlockIndex;

	/*
	 * if this is old style memory, we need to use a different function.
	 * _rlIOStart() will set the rlockErr value.
	 */

	if (_rlockShm.useOldStyle)
		return _rlIOStart(openEntry, (short)dosPID, lockLow, lockCount);

	/*
	 * get a pointer to the open file, and check if the file is in DOS
	 * compatability mode.  if that's the case, we don't need to deal
	 * with record locking, so just return.  _rlGetOFile() will set the
	 * rlockErr value.
	 */

	openFileP = _rlGetOFile(openEntry);
	if (nil(openFileP))
		return FAIL;
	if (openFileP->ofDenyMode == OFD_DOS_COMPAT)
		return SUCCESS;

	/*
	 * we have to deal with the locks.  get a pointer to the associated
	 * file header for this open file, and the session ID.  might as well
	 * get the high byte to lock while we're here.
	 */

	fileHdrP = &_rlockShm.fhdrTableP[openFileP->ofFHdrIndex];
	sessID = openFileP->ofSessID;
	ioHi = lockLow + lockCount - 1;

	/*
	 * now, gain exclusive access to the lock table.  _rlSetLocks() will set
	 * the rlockErr value.
	 */

	if (!_rlSetLocks(RL_NUM(fileHdrP->fhUniqueID), UNUSED_LOCK))
		return FAIL;

	/*
	 * see if any locks overlap the requested I/O range.
	 */

	for (rlockIndex = fileHdrP->fhLockIndex;
	     rlockIndex != INVALID_INDEX;
	     rlockIndex = scanLockP->rlNextIndex)
	{
		/*
		 * convert the index into a pointer.
		 */

		scanLockP = &_rlockShm.rlockTableP[rlockIndex];

		/*
		 * if this lock and the I/O request limits don't overlap, we
		 * can continue the loop, there isn't a conflict.  likewise, if
		 * the lock was set by the caller, there isn't a conflict,
		 * so just continue the loop.
		 */

		if (ioHi < scanLockP->rlLockLow || lockLow > scanLockP->rlLockHi)
			continue;
		if (scanLockP->rlDosPID == dosPID &&
		    scanLockP->rlSessID == sessID)
			continue;

		/*
		 * if we got to this point, there is a conflicting lock.  tell
		 * the user of his misfortune.
		 */

		_rlUnlock();
		rlockErr = RLERR_INUSE;
		return FAIL;
	}

	/*
	 * we didn't have any locking conflicts, so the user can access the
	 * memory.  in order to ensure that we don't get a lock applied in
	 * the middle of the write (which may take a while), leave the lock on
	 * the record lock table.  note that this means that the user is
	 * responsible for invoking ioDone() afterwards.
	 */

	return SUCCESS;
}

/*
 *	ioDone() - release the locking table lock set by ioStart()
 *
 *	input:	(none)
 *
 *	proc:	simply release the lock.  not much else to do.
 *
 *	output:	(void) - none
 *
 *	global:	_rlockShm - checked
 */

void
ioDone()
{
	/*
	 * if this is old style memory, we need to use a different function.
	 * _rlIODone() will set the rlockErr value.
	 */

	if (_rlockShm.useOldStyle)
	{
		_rlIODone();
		return;	
	}

	/*
	 * all we have to do is to release the record lock table.  note that
	 * if we never did the lock, due to being in DOS compatability mode
	 * (see ioStart()), _rlUnlock() will still work correctly.
	 */

	_rlUnlock();
}

/*
 *	rstLocks() - release all of a single user's file locks
 *
 *	input:	openEntry - index into the global open file table
 *		dosPID - process id of the DOS process setting the lock
 *
 *	proc:	scan thorough the lock list for the specified file, and
 *		release all locks on that file that were set by the specified
 *		PID.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	_rlockShm - checked
 */

int
rstLocks(openEntry, dosPID)
int openEntry;
long dosPID;
{	r0 recLockT FAR	*scanLockP;
	r1 recLockT FAR	*prevLockP;
	r2 recLockT FAR	*nextLockP;
	openFileT FAR *openFileP;
	fileHdrT FAR *fileHdrP;
	long sessID;

	/*
	 * if this is old style memory, we need to use a different function.
	 * _rlRstLocks() will set the rlockErr value.
	 */

	if (_rlockShm.useOldStyle)
		return _rlRstLocks(openEntry, (short)dosPID);

	/*
	 * get a pointer to the open file, plus a pointer to the associated
	 * file header, and a local version of the session ID of the open
	 * file's owner.  _rlGetOFile() will set the rlockErr value.
	 */

	openFileP = _rlGetOFile(openEntry);
	if (nil(openFileP))
		return FAIL;
	fileHdrP = &_rlockShm.fhdrTableP[openFileP->ofFHdrIndex];
	sessID = openFileP->ofSessID;

	/*
	 * if there are no locks set, return now.
	 */

	if (fileHdrP->fhLockIndex == INVALID_INDEX)
		return SUCCESS;

	/*
	 * now, gain exclusive access to the lock table.  _rlSetLocks() will set
	 * the rlockErr value.
	 */

	if (!_rlSetLocks(-LT_LOCK, -RL_NUM(fileHdrP->fhUniqueID)))
		return FAIL;

	/*	
	 * search the table for any looks that we can remove.
	 */

	prevLockP = NIL_RECLOCK;
	for (scanLockP = &_rlockShm.rlockTableP[fileHdrP->fhLockIndex];
	     !nil(scanLockP);
	     scanLockP = nextLockP)
	{
		/*
		 * get the next lock entry while the getting is good.
		 */

		if (scanLockP->rlNextIndex == INVALID_INDEX)
			nextLockP = NIL_RECLOCK;
		else
			nextLockP =
				&_rlockShm.rlockTableP[scanLockP->rlNextIndex];

		/*
		 * if this lock wasn't applied by the caller, continue the
		 * loop.  however, save the current entry pointer, so we
		 * can use it as the previous entry pointer the next time
		 * around.
		 */

		if (scanLockP->rlDosPID != dosPID ||
		    scanLockP->rlSessID != sessID ||
		    scanLockP->rlOpenIndex != (indexT)openEntry)
		{
			prevLockP = scanLockP;
			continue;
		}

		/*
		 * remove the lock from the file header's lock list, and
		 * free the entry.  if prevLockP is NIL, we were at the
		 * head of the list.
		 */

		if (nil(prevLockP))
			fileHdrP->fhLockIndex = scanLockP->rlNextIndex;
		else
			prevLockP->rlNextIndex = scanLockP->rlNextIndex;
		freeLock(openFileP, scanLockP);
	}

	/*
	 * all done, release the table, record the state change and return.
	 */

	_rlUnlock();
	rlockState(openEntry, RLSTATE_ALL_UNLOCKED);
	return SUCCESS;
}

/*
 *	STATIC	newLock() - get a lock structure from the free list
 *
 *	NOTE:	the table must be locked before this routine is called.
 *
 *	input:	openIndex - index of the open file entry setting the lock
 *		lockLow - low byte to lock
 *		lockHi - high byte to lock
 *		sessID - session ID of locker
 *		dosPID - DOS process ID of locker
 *
 *	proc:	if there are any free structures, return one, and update
 *		the free list pointers.  the indices in the returned
 *		structure are set to INVALID_INDEX.
 *
 *	output:	(recLockT *) - the structure or NIL
 *
 *	global:	_rlockShm - checked
 */

static recLockT FAR *
newLock(openIndex, lockLow, lockHi, sessID, dosPID)
indexT openIndex;
unsigned long lockLow, lockHi;
long sessID, dosPID;
{	r0 recLockT FAR	*recLockP;
	indexT rlockIndex;

	/*
	 * get the index of the free list.  if there is nothing there,
	 * return NIL.
	 */

	rlockIndex = *_rlockShm.lockFreeIndexP;
	if (rlockIndex == INVALID_INDEX)
	{
		rlockErr = RLERR_NOSPACE;
		return NIL_RECLOCK;
	}

	/*
	 * we have a free entry, can the host lock it?  doHostLock() will
	 * set rlockErr on failure.
	 */

	if (!doHostLock(&_rlockShm.openTableP[openIndex],
			lockLow, lockHi - lockLow, TRUE))
		return NIL_RECLOCK;

	/*
	 * okay, we have a free entry, and the host is locked.  pull the
	 * entry off the free list.
	 */

	recLockP = &_rlockShm.rlockTableP[rlockIndex];
	*_rlockShm.lockFreeIndexP = recLockP->rlNextIndex;

	/*
	 * initialize the structure.
	 */

	recLockP->rlNextIndex	= INVALID_INDEX;
	recLockP->rlOpenIndex	= openIndex;
	recLockP->rlLockLow	= lockLow;
	recLockP->rlLockHi	= lockHi;
	recLockP->rlSessID	= sessID;
	recLockP->rlDosPID	= dosPID;
	return recLockP;
}

/*
 *	STATIC	freeLock() - return a record lock to the free list
 *
 *	NOTE:	the table must be locked before this routine is called.
 *
 *	input:	openFileP - the open file associated with the lock
 *		recLockP - pointer to the entry to free
 *
 *	proc:	unlock the host locks, then mark the entry as 'unused', and
 *		stick it on the head of the record lock table's free list.
 *
 *	output:	(void) - none
 *
 *	global:	_rlockShm - set
 */

static void
freeLock(openFileP, recLockP)
openFileT FAR *openFileP;
r0 recLockT FAR	*recLockP;
{
	/*
	 * tell the host that we are unlocking this section.
	 */

	(void)doHostLock(openFileP, recLockP->rlLockLow,
			 recLockP->rlLockHi - recLockP->rlLockLow, FALSE);

	/*
	 * mark the entry as unused, and place it on the top of the free list.
	 */

	recLockP->rlLockHi = 0L;
	recLockP->rlNextIndex = *_rlockShm.lockFreeIndexP;
	*_rlockShm.lockFreeIndexP = recLockP - _rlockShm.rlockTableP;
}

/*
 *	STATIC	hasLocks() - does the specified file have any locks set?
 *
 *	WARN:	the lock table must be locked when this function is called.
 *
 *	input:	fileHdrP - pointer to the file header entry
 *		openIndex - the index of the 'open' entry
 *		sessID - the session ID
 *		dosPID - the DOS process ID
 *
 *	proc:	search through the locks on for the specified file, and find
 *		(if possible) at least one lock set by the specified process.
 *
 *	output:	(bool) - is an appropriate lock set?
 *
 *	global:	_rlockShm - checked
 */

static bool
hasLocks(fileHdrP, openIndex, sessID, dosPID)
fileHdrT FAR *fileHdrP;
r1 int	openIndex; 
r2 long sessID;
r3 long dosPID;
{	r0 recLockT FAR	*scanLockP;

	/*
	 * if there are no locks set, return now.
	 */

	if (fileHdrP->fhLockIndex == INVALID_INDEX)
		return FALSE;

	/*	
	 * search the table for any locks that were set by the caller.  if
	 * one is found, return TRUE.  if none are found by the time the
	 * loop is forced to terminate, return FALSE.
	 */

	scanLockP = &_rlockShm.rlockTableP[fileHdrP->fhLockIndex];
	while (TRUE)
	{
		if (scanLockP->rlDosPID == dosPID &&
		    scanLockP->rlSessID == sessID &&
		    scanLockP->rlOpenIndex == openIndex)
		{
			return TRUE;
		}
		if (scanLockP->rlNextIndex == INVALID_INDEX)
			break;
		scanLockP = &_rlockShm.rlockTableP[scanLockP->rlNextIndex];
	}
	return FALSE;
}

/*
 *	STATIC	doHostLock() - handle the actual host locking control
 *
 *	input:	openFileP - pointer to the open file structure
 *		lockLow - base byte offset to lock/unlock
 *		lockCount - number of bytes to lock/unlock
 *		setLock - set the lock? (if not, then unlock)
 *
 *	proc:	inform the host of the lock we want to set or clear.  if
 *		unlocking, there is no failure.  otherwise honor any
 *		existing locks.
 *
 *	output:	(bool) - could we lock/unlock?
 *
 *	global:	rlockErr - set on failure
 */

static bool
doHostLock(openFileP, lockLow, lockCount, setLock)
openFileT FAR *openFileP;
unsigned long lockLow, lockCount;
bool setLock;
{	struct flock hostLock;
	int retCode;

	/*
	 * set the host lock structure.  on Unix, simultaneous read and write
	 * locks aren't valid.  thus, we need to make some decisions.  for now,
	 * if the file has write access, we presume a write lock.  otherwise
	 * a read lock is set.  if we aren't setting the lock, we must be
	 * clearing it, so forget everything that I just said.
	 */

	if (!setLock)
		hostLock.l_type = F_UNLCK;
	else if (ANYSET(openFileP->ofAccMode, OFA_WRITE))
		hostLock.l_type = F_WRLCK;
	else
		hostLock.l_type = F_RDLCK;

	/*
	 * now, we come to the starting offset and length.  even though
	 * negative values aren't legal, the structure has signed types.
	 * we need to go through some gyrations to make things work, at
	 * least somewhat correctly.  for now, this means that if the
	 * requested lockLow + lockCount is larger than the maximum value of
	 * a signed long, the lockCount will be assumed to be 0, which will
	 * always extend to the end of the file.  no check is made of the start
	 * offset, since it is doubtful that it will ever be that big, and
	 * the fcntl() call will fail if it is.  there is no reasonable
	 * alternative, except possibly to set the start to MAXLONG ...
	 */

	hostLock.l_start	= (long)lockLow;
	if ((lockLow + lockCount) > (unsigned long)MAXLONG)
		hostLock.l_len	= 0L;
	else
		hostLock.l_len	= (long)lockCount;

	/*
	 * everything else is unused, or set to some default, so do it.
	 */

	hostLock.l_whence	= 0;
	hostLock.l_pid		= 0;

	/*
	 * we now have the information, perform the lock or unlock.  if we
	 * were unlocking, ignore the return code, presume that it all
	 * worked.  otherwise the lock is relevant, and any errors have to
	 * have the global error set.
	 */

	retCode = fcntl(openFileP->ofFileDesc, F_SETLK, &hostLock);
	if (!setLock || (retCode == SUCCESS))
		return TRUE;
	switch (errno)
	{
		case EACCES:	rlockErr = RLERR_LOCKED;	break;
		case EBADF:	rlockErr = RLERR_CORRUPT;	break;
		default:	rlockErr = RLERR_SYSTEM;
	}
	return FALSE;
}
