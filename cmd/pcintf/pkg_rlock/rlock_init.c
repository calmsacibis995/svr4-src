/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/rlock_init.c	1.1"
#include <sccs.h>

SCCSID("@(#)rlock_init.c	1.3	17:03:32	2/12/90");

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
 *	rlock_init.c - initialization for file/record locking package
 *
 *	routines included:
 *		rlockCreate()
 *		rlockAttach()
 *		rlockRemove()
 *
 *	comments:
 *		the basic shared memory segment consists of 5 parts:
 *
 *			1) basic table data (basicDataT structure)
 *			2) open file table
 *			3) file header table
 *			4) hashed file header table
 *			5) record lock table
 *
 *		table sizes in the basic data structure are used both to
 *		actually size the tables dynamically, and to allow the sizes
 * 		of the tables to be determined, so that pointers can be set
 *		to the proper locations.
 *
 *		if additions are made to the segment, they should follow a
 *		similar formula, but begin AFTER the last table listed above,
 *		in order to allow backward compatability.  this will necessitate
 *		additions to the rlockShmT structure.  it may also involve
 *		structures, such as basicDataT, which are local only to this
 *		file.  in either case, the same shared memory can be used,
 *		and the programs involved already know how and what to ignore.
 *
 *--------------------------------------------------------------------------
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#include <lockset.h>

#include <rlock.h>

#include <rlock_cfg.h>
#include <internal.h>

#if	!defined(SYS5_4)
extern int shmctl();
extern int shmget();
extern char FAR *shmat();
extern int shmdt();
#endif

/*
 * define a macro for the NIL shmid_ds structure pointer.
 */

#define NIL_SHMID_DS	((struct shmid_ds FAR *)0)

/*
 * externally visible variables.  these values may be checked by the user.
 */

int	rlockErr	= RLERR_NOERR;	/* error condition */

/*
 * globals, should be used only within the RLOCK functions themselves; the
 * user should NEVER access these values.
 *
 * WARNING: since the _rlCfgData structure is initialized here, any changes
 * to the structure must be checked, to be sure that the initialization
 * isn't invalidated.
 */

int		_rlNHashLocks	= 0;	/* # of hash record lock locks */
rlockShmT	_rlockShm;		/* shared memory data */
cfgDataT	_rlCfgData = {		/* configuration data */
	DEF_SHM_BASE,
	MAKE_SHM_KEY(DEF_USER_KEY),
	MAKE_LS_KEY(DEF_USER_KEY),
	DEF_REC_LOCKS,
	DEF_OPEN_TABLE,
	DEF_FILE_TABLE,
	DEF_HASH_TABLE,
	DEF_LOCK_TABLE
};

/*
 * this structure handles the information for the basic tables.  future
 * additions should add information to the end of the tables that already
 * exist.  that is, the area from the start of the segment to the end of
 * the last basic table should NEVER have any information except what is
 * part of the basic table information.
 *
 * the table counts are defined as indexT types to enforce the fact that the
 * maximum allowed count must be a legal index.
 *
 * the bdRecLocks field is not directly used by the shared memory segment.
 * it is present simply to provide data for run-time display.
 */

typedef struct {
	ushort	bdVersion;	/* basic version number */
	ushort	bdRecLocks;	/* number of 'lock' locks */
	indexT	bdCntOpen;	/* # of entries in the open file table */
	indexT	bdCntFile;	/* # of entries in the file header table */
	indexT	bdCntHash;	/* # of entries in the hashed file hdr table */
	indexT	bdCntLock;	/* # of entries in the record lock table */
	indexT	bdFreeOpen;	/* index of the open file table free list */
	indexT	bdFreeFhdr;	/* index of the file header table free list */
	indexT	bdFreeLock;	/* index of the record lock table free list */
	PAD		/* structure padding (see internal.h for def) */
} basicDataT;

/*
 * internal functions.
 */

static bool	doAttach();
static bool	getShmInfo();

/*
 *	rlockCreate() - create the RLOCK shared memory segment
 *
 *	input:	(none)
 *
 *	proc:	create the segment.  if it already exists, return the failure.
 *		once the segment has been created and initialized, detach it.
 *		it must be attached through the rlockAttach() function.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	rlockErr - may be set
 */

int
rlockCreate()
{	r0 openFileT FAR *openFileP;
	r1 fileHdrT FAR *fileHdrP;
	r2 indexT *hFileHdrP;
	r3 recLockT FAR *recLockP;
	r4 indexT index;
	int lockCount;
	basicDataT basicData;
	lockSetT lockSet;

	/*
	 * create the lockset for this memory, then initialize it and lock
	 * the memory.  we don't want anyone using it until the initialization
	 * has completed.  both _rlInitLocks() and _rlSetLocks() will set
	 * the rlockErr value.
	 */

	lockSet = lsNew(_rlCfgData.cfgLsetKey, LS_ALL_WRITE | LS_ALL_READ,
			NLOCKS, NLOCKS + _rlCfgData.cfgRecLocks);
	(void)lsUnuse(lockSet);
	if (lockSet == INVALID_LOCKSET)
	{
		if (lsetErr == LSERR_EXIST)
			rlockErr = RLERR_EXIST;
		else
			rlockErr = RLERR_LOCKSET;
		return FAIL;
	}
	lockCount = _rlInitLocks(_rlCfgData.cfgLsetKey);
	if ((lockCount == FAIL) || !_rlSetLocks(ALL_LOCK, UNUSED_LOCK))
	{
		(void)lsRmv(_rlCfgData.cfgLsetKey);
		return FAIL;
	}

	/*
	 * now, take care of all the stuff we have to do when attaching.
	 * doAttach() will set the rlockErr value.
	 */

	if (!doAttach(TRUE))
	{
		(void)lsRmv(_rlCfgData.cfgLsetKey);
		return FAIL;
	}

	/*
	 * first, initialize the basic data information.  the getShmInfo()
	 * function can then be used to determine the actual addresses of
	 * the tables, and to attach pointers.
	 */

	basicData.bdVersion	= VER_BASIC;
	basicData.bdRecLocks	= _rlCfgData.cfgRecLocks;
	basicData.bdCntOpen	= _rlCfgData.cfgOpenTable;
	basicData.bdCntFile	= _rlCfgData.cfgFileTable;
	basicData.bdCntHash	= _rlCfgData.cfgHashTable;
	basicData.bdCntLock	= _rlCfgData.cfgLockTable;
	basicData.bdFreeOpen	= (indexT) 0;
	basicData.bdFreeFhdr	= (indexT) 0;
	basicData.bdFreeLock	= (indexT) 0;
	*(basicDataT *)_rlockShm.segmentP = basicData;

	(void)getShmInfo(lockCount);

	/*
	 * next, some tables need to have each entry marked as unused.
	 */

	for (openFileP = &_rlockShm.openTableP[basicData.bdCntOpen - 1];
	     openFileP >= _rlockShm.openTableP;
	     openFileP--)
	{
		openFileP->ofFHdrIndex = INVALID_INDEX;
	}
	for (fileHdrP = &_rlockShm.fhdrTableP[basicData.bdCntFile - 1];
	     fileHdrP >= _rlockShm.fhdrTableP;
	     fileHdrP--)
	{
		CLEAR_UNIQUE_ID(fileHdrP->fhUniqueID);
	}
	for (hFileHdrP = &_rlockShm.hfhdrTableP[basicData.bdCntHash - 1];
	     hFileHdrP >= _rlockShm.hfhdrTableP;
	     hFileHdrP--)
	{
		*hFileHdrP = INVALID_INDEX;
	}
	for (recLockP = &_rlockShm.rlockTableP[basicData.bdCntLock - 1];
	     recLockP >= _rlockShm.rlockTableP;
	     recLockP--)
	{
		recLockP->rlLockHi = 0L;
	}

	/*
	 * some tables need their free lists built.
	 */

	index = 1;
	for (openFileP = _rlockShm.openTableP;
	     openFileP < &_rlockShm.openTableP[basicData.bdCntOpen];
	     openFileP++)
	{
		openFileP->ofNextIndex = index++;
	}
	_rlockShm.openTableP[basicData.bdCntOpen - 1].ofNextIndex =
		INVALID_INDEX;

	index = 1;
	for (fileHdrP = _rlockShm.fhdrTableP;
	     fileHdrP < &_rlockShm.fhdrTableP[basicData.bdCntFile];
	     fileHdrP++)
	{
		fileHdrP->fhHashIndex = index++;
	}
	_rlockShm.fhdrTableP[basicData.bdCntFile - 1].fhHashIndex =
		INVALID_INDEX;

	index = 1;
	for (recLockP = _rlockShm.rlockTableP;
	     recLockP < &_rlockShm.rlockTableP[basicData.bdCntLock];
	     recLockP++)
	{
		recLockP->rlNextIndex = index++;
	}
	_rlockShm.rlockTableP[basicData.bdCntLock - 1].rlNextIndex =
		INVALID_INDEX;

	/*
	 * all done.  unlock the segment and return.
	 */

	_rlUnlock();
	return SUCCESS;
}

/* 
 *	rlockAttach() - attach (create) to the RLOCK data
 *
 *	input:	(none)
 *
 *	proc:	attach the shared memory segment and init the lockset on it.
 *
 *	output:	(int) - SUCCESS or FAIL
 *
 *	global:	rlockErr - may be set
 */

int
rlockAttach()
{	int lockCount;

	/*
	 * for purpose of backwards compatability, try to attach to the old
	 * style memory.  this assumes the memory is present and initialized.
	 * if it isn't, we can either attach to or create the new style.  the
	 * _rlMAttach() function will set rlockErr.
	 */

	if (_rlMAttach() == SUCCESS)
		return SUCCESS;

	/*
	 * initialize the (existing) lockset.  if that works, attach to
	 * the (existing) shared memory.  if that works, get the global
	 * info and return; if not, report the error.  if initializing the
	 * lockset failed, we will need to create the segment (and lockset).
	 * doAttach() and getShmInfo() will set the rlockErr value on failure.
	 */

	lockCount = _rlInitLocks(_rlCfgData.cfgLsetKey);
	if (lockCount != FAIL)
	{
		if (!doAttach(FALSE))
			return FAIL;
		if (getShmInfo(lockCount))
			return SUCCESS;
		return FAIL;
	}

	/*
	 * the memory isn't yet there.  it must be created, attached to and
	 * initialized.  do that.  rlockCreate() will set the rlockErr value
	 * if necessary.
	 */

	return rlockCreate();
}

/*
 *	rlockRemove() - remove the RLOCK segment
 *
 *	input:	(none)
 *
 *	proc:	tear down what we had built up.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	rlockErr - may be set
 */

int
rlockRemove()
{	int shmDesc, saveErrno;
	bool shmError, lsError;

	/*
	 * lock the segments.  we don't want anyone accessing them.
	 */

	if ((_rlInitLocks(_rlCfgData.cfgLsetKey) == FAIL) ||
	    !_rlSetLocks(ALL_LOCK, UNUSED_LOCK))
	{
		rlockErr = RLERR_LOCKSET;
		return FAIL;
	}

	/*
	 * get the descriptor and remove the segment.
	 */

	shmError = FALSE;
	shmDesc = shmget(_rlCfgData.cfgShmKey, 0, UP_ORW);
	if (shmDesc == FAIL)
	{
		saveErrno = errno;
		shmError = TRUE;
	}
	else if (shmctl(shmDesc, IPC_RMID, NIL_SHMID_DS) == FAIL)
	{
		saveErrno = errno;
		shmError = TRUE;
	}

	/*
	 * now we can unlock the segment and remove the lockset.  we have
	 * to do this, even if the above failed, since we may have the case
	 * in which the shared memory is gone, but the lockset exists ...
	 */

	_rlUnlock();
	lsError = lsRmv(_rlCfgData.cfgLsetKey) != SUCCESS;

	/*
	 * now return.  if the shared memory failed, return that error.  if not,
	 * check the lockset error.
	 */

	if (shmError)
	{
		errno = saveErrno;
		rlockErr = RLERR_SYSTEM;
	}
	else if (lsError)
		rlockErr = RLERR_LOCKSET;
	else
		return SUCCESS;
	return FAIL;
}

/*
 *	STATIC	doAttach() - attach the shared memory
 *
 *	input:	create - creation?
 *
 *	proc:	attach the shared memory.
 *
 *	output:	(bool) - did we attach?
 *
 *	global:	rlockErr - may be set
 *		_rlockShm - set
 */

static bool
doAttach(create)
bool create;
{	int shmDesc, getFlags, saveErrno;
	struct shmid_ds sysShmInfo;

	/*
	 * first, set the flags for shmget(), and set a value for the total
	 * segment size.  for the latter, the true size is known only if
	 * we are creating it.  if not, set the size to 0 for now.  it will
	 * be corrected later.
	 */

	getFlags = UP_ORW | UP_GRW | UP_WRW;
	if (create)
	{
		getFlags |= IPC_CREAT;
		_rlockShm.segSize =
			(ulong)sizeof(basicDataT) +
			(ulong)(_rlCfgData.cfgOpenTable * sizeof(openFileT)) +
			(ulong)(_rlCfgData.cfgFileTable * sizeof(fileHdrT)) +
			(ulong)(_rlCfgData.cfgHashTable * sizeof(indexT)) +
			(ulong)(_rlCfgData.cfgLockTable * sizeof(recLockT));
	}
	else
		_rlockShm.segSize = 0;

	/*
	 * now, attach to the memory.
	 */

	shmDesc = shmget(_rlCfgData.cfgShmKey, _rlockShm.segSize, getFlags);
	if (shmDesc == FAIL)
	{
		switch (errno)
		{
			case EEXIST:	rlockErr = RLERR_EXIST;		break;
			case EACCES:	rlockErr = RLERR_PERM;		break;
			case ENOMEM:	rlockErr = RLERR_NOSPACE;	break;
			default:	rlockErr = RLERR_SYSTEM;
		}
		return FALSE;
	}
	_rlockShm.segmentP = shmat(shmDesc, _rlCfgData.cfgBaseP, 0);
	if (_rlockShm.segmentP == (char FAR *)FAIL)
	{
		switch (errno)	
		{
			case EINVAL:	rlockErr = RLERR_ADDR;		break;
			case EACCES:	rlockErr = RLERR_PERM;		break;
			case ENOMEM:	rlockErr = RLERR_NOSPACE;	break;
			default:	rlockErr = RLERR_SYSTEM;
		}
		saveErrno = errno;
		if (create) (void)shmctl(shmDesc, IPC_RMID, NIL_SHMID_DS);
		errno = saveErrno;
		return FALSE;
	}

	/*
	 * okay, so now we have the segment, and are attached.  if we didn't
	 * create it, we need to determine the size, then return.
	 */

	if (!create)
	{
		if (shmctl(shmDesc, IPC_STAT, &sysShmInfo) == FAIL)
		{
			saveErrno = errno;
			(void)shmdt(_rlockShm.segmentP);
			errno = saveErrno;
			rlockErr = RLERR_SYSTEM;
			return FALSE;
		}
		_rlockShm.segSize = sysShmInfo.shm_segsz;
	}
	return TRUE;
}

/*
 *	STATIC	getShmInfo() - set the global shared memory data
 *
 *	input:	lockCount - number of locks
 *
 *	proc:	this function simply sets the variables that are global
 *		throughout this package.  note that these values shouldn't
 *		be accessed by the package user.  this function will fail only
 *		if the version number of the existing shared memory is not
 *		recognized.
 *
 *		FUTURE EXPANSION - if the segment is extended, this function
 *		will have to be modified.  it will have to check the size of
 *		the segment, (unless we just created it, of course), and fill
 *		in the pointers to unused tables with NIL, and sizes with 0.
 *		(NOTE: also remove the setting of endBasicIndex and
 *		endSegmentP, and their definitions from the comments.)
 *
 *	output:	(bool) - could get information?
 *
 *	global:	rlockErr - may be set
 *		_rlockShm - set
 *		_rlNHashLocks - set
 */

static bool
getShmInfo(lockCount)
int lockCount;
{	off_t openOffset, fhdrOffset, rlockOffset, hfhdrOffset, endBasicOffset;
	basicDataT FAR *basicDataP;
	char FAR *currentEndP, *endSegmentP;

	/*
	 * fix up the number of available hash locks, and clear the 'old style'
	 * flag.  these manipulations are always handled, and are independent
	 * of the other tables.
	 */

	_rlNHashLocks = lockCount - (NLOCKS - 1);
	_rlockShm.useOldStyle = FALSE;

	/*
	 * get the basic data, verify the version, and set the 'end of the
	 * segment' pointer.
	 */

	basicDataP = (basicDataT FAR *)_rlockShm.segmentP;
	if (basicDataP->bdVersion != VER_BASIC)
	{
		rlockErr = RLERR_FORMAT;
		return FALSE;
	}
	endSegmentP = _rlockShm.segmentP + _rlockShm.segSize;

	/*
	 * get the sizes of each of the tables, and the addresses of the
	 * pointers to the free lists.  from the table sizes, determine the
	 * offset of each of the tables from the base of the memory, then
	 * set the appropriate pointers to the appropriate locations.
	 */

	_rlockShm.openSize		= basicDataP->bdCntOpen;
	_rlockShm.fhdrSize		= basicDataP->bdCntFile;
	_rlockShm.hfhdrSize		= basicDataP->bdCntHash;
	_rlockShm.lockSize		= basicDataP->bdCntLock;
	_rlockShm.recLocks		= basicDataP->bdRecLocks;
	_rlockShm.openFreeIndexP	= &basicDataP->bdFreeOpen;
	_rlockShm.fhdrFreeIndexP	= &basicDataP->bdFreeFhdr;
	_rlockShm.lockFreeIndexP	= &basicDataP->bdFreeLock;

	openOffset	= sizeof(basicDataT);
	fhdrOffset	= openOffset +
				(basicDataP->bdCntOpen * sizeof(openFileT));
	hfhdrOffset	= fhdrOffset +
				(basicDataP->bdCntFile * sizeof(fileHdrT));
	rlockOffset	= hfhdrOffset +
				(basicDataP->bdCntHash * sizeof (indexT));
	endBasicOffset	= rlockOffset +
				(basicDataP->bdCntLock * sizeof(recLockT));

	_rlockShm.openTableP	= (openFileT FAR *)(_rlockShm.segmentP +
							openOffset);
	_rlockShm.fhdrTableP	= (fileHdrT FAR *)(_rlockShm.segmentP +
							fhdrOffset);
	_rlockShm.hfhdrTableP	= (indexT FAR *)(_rlockShm.segmentP +
							hfhdrOffset);
	_rlockShm.rlockTableP	= (recLockT FAR *)(_rlockShm.segmentP +
							rlockOffset);

	/*
	 * we have the basic data (all products must have it).  get the first
	 * value for the end of the segment, and verify that we aren't in
	 * a bad way.
	 */

	currentEndP = _rlockShm.segmentP + endBasicOffset;
	if (currentEndP > endSegmentP)
	{
		rlockErr = RLERR_CORRUPT;
		return FALSE;
	}

	/*
	 * FUTURE EXPANSION: for each succeeding table or set of tables, do one
	 * of the following two things:
	 *
	 *	1) if there is enough room in the segment for the table or
	 *	   tables, then set the appropriate pointers in _rlockShm,
	 *	   and increase currentEndP by the size of the table or set
	 *	   of tables.
	 *	2) in any other case, set the appropriate pointers in _rlockShm
	 *	   to NIL, and set currentEndP to the end of the segment, to
	 *	   prevent small tables from 'sneaking in'.  the function
	 *	   should NOT return prematurely.  this allows the pointers to
	 *	   subsequent tables to be set to NIL, without adding the
	 *	   appropriate settings to all previous sets.
	 *
	 * the following pseudo-code may be used as an example:
	 *
	 *	tblSize = size of table or set of tables;
	 *	if ((currentEndP + tblSize) <= endSegmentP)
	 *	{
	 *		_rlockShm.(table pointers) = segment table(s);
	 *		currentEndP += tblSize;
	 *	}
	 *	else
	 *	{
	 *		_rlockShm.(table pointers) = NIL;
	 *		currentEndP = endSegmentP;
	 *	}
	 */

	/*
	 * if we got here, we could initialize, so return TRUE.
	 */

	return TRUE;
}
