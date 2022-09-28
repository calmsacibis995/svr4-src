/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/multi_init.c	1.1"
#include <sccs.h>

SCCSID("@(#)multi_init.c	1.3	13:44:04	11/17/89");

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
 *	multi_init.c - multiple segment initializion
 *
 *	NOTE: this is the archaic method of shared memory.  any new table
 *	initialization should go into rlock_init.c.  if there is justice in
 *	the world, this file, and references to it, will eventually be removed.
 *
 *	routines included:
 *		_rlMCreate()
 *		_rlMAttach()
 *		_rlMRemove()
 *
 *	comments:
 *		there are three segments in this version of the shared memory,
 *		and each has a compiled size.  this makes the code simpler,
 *		but less adaptable to the user's special needs.  these segments
 *		are as follows:
 *
 *			1) open file table, with free list pointer
 *			2) file header table, with free list pointer and
 *			   hash table of pointers
 *			3) record lock table, with free lock pointer
 *
 *		each segment must be attached in it's own location, with it's
 *		own key.
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
#include <multi_cfg.h>
#include <internal.h>
#ifdef ULTRIX
#include <machine/pte.h>
#endif

#if	!defined(SYS5_4)
extern int shmctl();
extern int shmget();
extern char FAR *shmat();
extern int shmdt();
#endif

/*
 * this macro computes the base of the next shared memory page.  it is
 * unused by XENIX systems.  for everyone else, it rounds the segment address
 * passed as the parameter DOWN to a multiple of SHMLBA (like the IPC_RND
 * option does), then adds SHMLBA, so we get the next, completely unused,
 * segment.
 */

#define NEXT_SEG(addr)	 (((addr) - ((ulong)(addr) % SHMLBA)) + SHMLBA)

/*
 * define a macro for the NIL shmid_ds structure pointer.
 */

#define NIL_SHMID_DS	((struct shmid_ds FAR *)0)

/*
 * the following structures are specific to the multi-segment implementation
 * of the RLOCK shared memory.
 */

typedef struct {
	oldOpenFile FAR	*freeOpenP;			/* free list */
	oldOpenFile	ofTable[MAX_OPEN_TABLE];	/* open file table */
} ofShmT;

typedef struct {
	oldFileHdr FAR	*freeHdrP;			/* free list */
	oldFileHdr FAR	*hashTable[MAX_HASH_TABLE];	/* the hash table */
	oldFileHdr		fhTable[MAX_FILE_TABLE];	/* file header table */
} fhShmT;

typedef struct {
	oldRecLock FAR	*freeLockP;			/* free list */
	oldRecLock		ltTable[MAX_LOCK_TABLE];	/* record lock table */
} ltShmT;

/*
 * internal variables.
 */

static ofShmT FAR *ofShmP;	/* pointer to the open file segment */
static fhShmT FAR *fhShmP;	/* pointer to the file header segment */
static ltShmT FAR *ltShmP;	/* pointer to the record lock segment */

/*
 * internal functions.
 */

static bool doAttach();

/*
 *	_rlCreate() - create a multi-segment memory
 *
 *	input:	(none)
 *
 *	proc:	create the multi-segment memory, but don't attach to it.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	rlockErr - may be set
 */

int
_rlMCreate()
{	r0 oldOpenFile FAR *ofFree;
	r1 oldFileHdr FAR *fhFree;
	r2 oldFileHdr FAR **fhHClr;
	r3 oldRecLock FAR *ltFree;
	int lockCount;
	lockSetT lockSet;

	/*
	 * create the lockset for this memory, then initialize it and lock
	 * the memory.  we don't want anyone using it until the initialization
	 * has completed.
	 */

	lockSet = lsNew(LS_KEY, LS_ALL_WRITE | LS_ALL_READ,
			NLOCKS, NLOCKS + MAX_REC_LOCK);
	(void)lsUnuse(lockSet);
	if (lockSet == INVALID_LOCKSET)
	{
		rlockErr = RLERR_LOCKSET;
		return FAIL;
	}
	lockCount = _rlInitLocks(LS_KEY);
	if ((lockCount == FAIL) || _rlSetLocks(ALL_LOCK, UNUSED_LOCK) == FAIL)
	{
		(void)lsRmv(LS_KEY);
		rlockErr = RLERR_LOCKSET;
		return FAIL;
	}

	/*
	 * now, take care of all the stuff we have to do when attaching.  this
	 * function creates the segment if it doesn't exist (which should be
	 * the case).  the TRUE parameter tells it to do some special stuff
	 * associated with creation.  doAttach() will set rlockErr.
	 */

	if (!doAttach(TRUE, lockCount))
	{
		(void)lsRmv(LS_KEY);
		return FAIL;
	}

	/*
	 * the following information initializes the tables.  since the new
	 * method does the same thing, we could have a common function.
	 * however, if the new style is ever expanded, we would run into
	 * problems.  we could have a function just to do this basic stuff,
	 * but I think that might be more confusing, so don't worry about it.
	 * besides, it isn't that big, compared to what this is used in.
	 *
	 * first, set up the free list pointers.
	 */

	ofShmP->freeOpenP = ofShmP->ofTable;
	fhShmP->freeHdrP  = fhShmP->fhTable;
	ltShmP->freeLockP = ltShmP->ltTable;

	/*
	 * some tables simply need to be marked as unused.
	 */

	for (ofFree = _rlockShm.oOpenTableP;
	     ofFree < &_rlockShm.oOpenTableP[MAX_OPEN_TABLE];
	     ofFree++)
	{
		ofFree->header = NIL_O_FILEHDR;
	}
	for (fhFree = _rlockShm.oFhdrTableP;
	     fhFree < &_rlockShm.oFhdrTableP[MAX_FILE_TABLE];
	     fhFree++)
	{
		fhFree->uniqueID = 0L;
	}
	for (fhHClr = &_rlockShm.oHfhdrTablePP[MAX_HASH_TABLE - 1];
	     fhHClr >= _rlockShm.oHfhdrTablePP;
	     --fhHClr)
	{
		*fhHClr = NIL_O_FILEHDR;
	}
	for (ltFree = _rlockShm.oRlockTableP;
	     ltFree < &_rlockShm.oRlockTableP[MAX_LOCK_TABLE];
	     ltFree++)
	{
		ltFree->lockHi = 0L;
	}

	/*
	 * others need to have free lists built up.
	 */

	for (ofFree = _rlockShm.oOpenTableP;
	     ofFree < &_rlockShm.oOpenTableP[MAX_OPEN_TABLE];
	     ofFree++)
	{
		ofFree->nextOpen = ofFree + 1;
	}
	for (fhFree = _rlockShm.oFhdrTableP;
	     fhFree < &_rlockShm.oFhdrTableP[MAX_FILE_TABLE];
	     fhFree++)
	{
		fhFree->hashLink = fhFree + 1;
	}
	for (ltFree = _rlockShm.oRlockTableP;
	     ltFree < &_rlockShm.oRlockTableP[MAX_LOCK_TABLE];
	     ltFree++)
	{
		ltFree->nextLock = ltFree + 1;
	}

	/*
	 * all done.  unlock the segment and return.
	 */

	_rlUnlock();
	return SUCCESS;
}

/*
 *	_rlAttach() - attach to the multi-segment memory
 *
 *	input:	(none)
 *
 *	proc:	attach to the multi-segment memory.  if it doesn't exist,
 *		create it.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	(none)
 */

int
_rlMAttach()
{	int lockCount;

	/*
	 * initialize the lockset, then use doAttach() (which will set the
	 * rlockErr value) to do the details.
	 */

	lockCount = _rlInitLocks(LS_KEY);
	if (lockCount == FAIL)
	{
		rlockErr = RLERR_LOCKSET;
		return FAIL;
	}
	return doAttach(FALSE, lockCount) ? SUCCESS : FAIL;
}

/*
 *	_rlRemove() - remove multi-segment memory
 *
 *	input:	(none)
 *
 *	proc:	remove the multi-segment memory.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	rlockErr - may be set
 */

int
_rlMRemove()
{	int ofDesc, fhDesc, ltDesc, ofErr, fhErr, ltErr, saveErrno;
	bool shmError, lsError;

	/*
	 * lock the segments.  we don't want anyone accessing them.
	 */

	if ((_rlInitLocks(LS_KEY) == FAIL) ||
	    (_rlSetLocks(ALL_LOCK, UNUSED_LOCK) == FAIL))
	{
		rlockErr = RLERR_LOCKSET;
		return FAIL;
	}

	/*
	 * get the descriptors and remove the segments.  errors in getting the
	 * memory will be reported when cleaning it.  note that the removals
	 * are not done in an 'if' expression, to force each to be tried
	 * independently of the others.  the idea is to be able to clear
	 * things up, even if one of the segments has vanished for some
	 * reason.
	 */

	shmError = FALSE;
	ofDesc = shmget(SHM_OF_KEY, sizeof(ofShmT), UP_ORW);
	fhDesc = shmget(SHM_FH_KEY, sizeof(fhShmT), UP_ORW);
	ltDesc = shmget(SHM_LT_KEY, sizeof(ltShmT), UP_ORW);
	if ((ofDesc == FAIL) || (fhDesc == FAIL) || (ltDesc == FAIL))
	{
		saveErrno = errno;
		shmError = TRUE;
	}
	ofErr = shmctl(ofDesc, IPC_RMID, NIL_SHMID_DS);
	fhErr = shmctl(fhDesc, IPC_RMID, NIL_SHMID_DS);
	ltErr = shmctl(ltDesc, IPC_RMID, NIL_SHMID_DS);
	if (!shmError &&
	    ((ofErr == FAIL) || (fhErr == FAIL) || (ltErr == FAIL)))
	{
		saveErrno = errno;
		shmError = TRUE;
	}

	/*
	 * now we can unlock the segments and remove the lockset.
	 */

	_rlUnlock();
	lsError = lsRmv(LS_KEY) != SUCCESS;

	/*
	 * now return.  if the shared memory failed, return that error.  if
	 * not, check the lockset error.
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
 *	STATIC	doAttach() - attach the shared memory segments
 *
 *	input:	create - were the segments just created?
 *		lockCount - number of locks allocated
 *
 *	proc:	do all the functions necessary for attaching to the segments,
 *		and initializing the global data structures.
 *
 *	output:	(bool) - did it work?
 *
 *	global:	rlockErr - may be set
 *		_rlNHashLocks - set
 *		_rlockShm - set
 */

static bool
doAttach(create, lockCount)
bool create;
int lockCount;
{	int ofDesc, fhDesc, ltDesc, getFlags, saveErrno;

	/*
	 * the tables are kept in consecutive segments, in the order:
	 *	1) open file table
	 *	2) file header table (and hash table)
	 *	3) record lock table
	 * determine the base address of each.
	 */

#if	defined(XENIX)
       	ofShmP = (ofShmT FAR *)SHM_BASE;
       	fhShmP = (fhShmT FAR *)(SHM_BASE + (8 * SHMLBA));
       	ltShmP = (ltShmT FAR *)(SHM_BASE + (16 * SHMLBA));
#else
       	ofShmP = (ofShmT FAR *)SHM_BASE;
       	fhShmP = (fhShmT FAR *)NEXT_SEG((char FAR *)ofShmP + sizeof(ofShmT));
       	ltShmP = (ltShmT FAR *)NEXT_SEG((char FAR *)fhShmP + sizeof(fhShmT));
#endif

	/*
	 * get a descriptor for each of the segments.
	 */

	getFlags = UP_ORW | UP_GRW | UP_WRW;
	if (create) getFlags |= IPC_CREAT;
	ofDesc = shmget(SHM_OF_KEY, sizeof(ofShmT), getFlags);
	fhDesc = shmget(SHM_FH_KEY, sizeof(fhShmT), getFlags);
	ltDesc = shmget(SHM_LT_KEY, sizeof(ltShmT), getFlags);
	if ((ofDesc == FAIL) || (fhDesc == FAIL) || (ltDesc == FAIL))
	{
		switch (errno)
		{
			case EEXIST:	rlockErr = RLERR_EXIST;		break;
			case EACCES:	rlockErr = RLERR_PERM;		break;
			case ENOMEM:	rlockErr = RLERR_NOSPACE;	break;
			default:	rlockErr = RLERR_SYSTEM;
		}
		saveErrno = errno;
		if (create)
		{
			(void)shmctl(ofDesc, IPC_RMID, NIL_SHMID_DS);
			(void)shmctl(fhDesc, IPC_RMID, NIL_SHMID_DS);
			(void)shmctl(ltDesc, IPC_RMID, NIL_SHMID_DS);
		}
		errno = saveErrno;
		return FALSE;
	}

	/*
	 * now that we have the descriptors, try to attach everything.
	 */

	ofShmP = (ofShmT FAR *)shmat(ofDesc, (char FAR *)ofShmP, 0);
	fhShmP = (fhShmT FAR *)shmat(fhDesc, (char FAR *)fhShmP, 0);
	ltShmP = (ltShmT FAR *)shmat(ltDesc, (char FAR *)ltShmP, 0);
	if ((ofShmP == (ofShmT FAR *)FAIL) ||
	    (fhShmP == (fhShmT FAR *)FAIL) ||
	    (ltShmP == (ltShmT FAR *)FAIL))
	{
		switch (errno)
		{
			case EINVAL:	rlockErr = RLERR_ADDR;		break;
			case ENOMEM:	rlockErr = RLERR_NOSPACE;	break;
			default:	rlockErr = RLERR_SYSTEM;
		}
		saveErrno = errno;
		(void)shmdt((char FAR *)ofShmP);
		(void)shmdt((char FAR *)fhShmP);
		(void)shmdt((char FAR *)ltShmP);
		if (create)
		{
			(void)shmctl(ofDesc, IPC_RMID, NIL_SHMID_DS);
			(void)shmctl(fhDesc, IPC_RMID, NIL_SHMID_DS);
			(void)shmctl(ltDesc, IPC_RMID, NIL_SHMID_DS);
		}
		errno = saveErrno;
		return FALSE;
	}

	/*
	 * fix up the number of available hash locks, and set the 'old style'
	 * flag.
	 */

	_rlNHashLocks = lockCount - (NLOCKS - 1);
	_rlockShm.useOldStyle = TRUE;

	/*
	 * initialize the global segment description structure.
	 */

	_rlockShm.openSize	= MAX_OPEN_TABLE;
	_rlockShm.fhdrSize	= MAX_FILE_TABLE;
	_rlockShm.hfhdrSize	= MAX_HASH_TABLE;
	_rlockShm.lockSize	= MAX_LOCK_TABLE;

	_rlockShm.oOpenFreePP	= &ofShmP->freeOpenP;
	_rlockShm.oFhdrFreePP	= &fhShmP->freeHdrP;
	_rlockShm.oLockFreePP	= &ltShmP->freeLockP;

	_rlockShm.oOpenTableP	= ofShmP->ofTable;
	_rlockShm.oFhdrTableP	= fhShmP->fhTable;
	_rlockShm.oHfhdrTablePP	= fhShmP->hashTable;
	_rlockShm.oRlockTableP	= ltShmP->ltTable;

	_rlockShm.oOpenSegmentP = (char FAR *)ofShmP;
	_rlockShm.oFhdrSegmentP = (char FAR *)fhShmP;
	_rlockShm.oLockSegmentP = (char FAR *)ltShmP;

	/*
	 * all done, return.
	 */

	return TRUE;
}
