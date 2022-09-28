/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/shm_open.c	1.1"
#include <sccs.h>

SCCSID("@(#)shm_open.c	1.1	13:38:20	8/31/89");

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
 *	shm_open.c - open file operations
 *
 *	routines included:
 *		addOpen()
 *		rmvOpen()
 *		_rlGetOFile()
 *
 *	comments:
 *
 *--------------------------------------------------------------------------
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <lockset.h>

#include <rlock.h>

#include <internal.h>

/*
 * this macro converts the file's unique id (fhUniqueID) into an index into
 * the hashed file header table.
 *
 * NOTE: in order to prevent problems, the result of the hashing function
 * should always be a signed integer (int) value.  since it is used as an
 * array index, the ABS() macro is also used.  it's probably not a good idea
 * to use an unsigned int, since that may not be portable as an index.  besides,
 * an int value should be more than large enough, especially since the
 * hfhdrSize field is a short.
 */

#define FH_NUM(uniqID)	UNIQ_INT_HASH(uniqID, _rlockShm.hfhdrSize)

/*
 * this macro converts an open file table entry's access and deny modes into
 * an index into the denyTable[].  the only parameter is a pointer to the
 * table entry.
 */

#define DENY_TABLE_INDEX(p)	((((p)->ofAccMode - 1) << 2) | (p)->ofDenyMode)

/*
 * internal constants
 */

#define NOPENS	12			/* number of distinct open types */
#define ALLOW	0			/* allow open (denyTable[] data) */
#define DENY	1			/* deny open (denyTable[] data) */

/*
 * this table is analogous to the one found in the DOS 3.0/3.1 technical
 * reference manual.  there are two main differences.  one is that the notation
 * is slightly different (a 'Y' in the manual's table appears as an 'A' in
 * this table and a 'N' in the manual's table is a 'D' here).
 *
 * more importantly, the order of the rows and columns is not the same.  the
 * order of the rows and columns here is dictated by the internal
 * representation of the access and deny modes.  the following mapping
 * can be applied to translate coordinates in the DOS manual's tables to
 * coordinates in this table:
 *
 *	DOS manual row/column		denyTable row/column
 *		1				3
 *		2				11
 *		3				7
 *		4				2
 *		5				10
 *		6				6
 *		7				1
 *		8				9
 *		9				5
 *		10				0
 *		11				8
 *		12				4
 *
 * the first subscript is the index of an existing open, while the second is
 * the index of the new open.  the intersection defines the situation.
 *
 * in order to avoid writing ALLOW and DENY in each of the following
 * entries, the following shorthand is used ONLY for this table (note that
 * they are undefined afterwards).
 *
 *	D -> deny
 *	A -> allow
 */

#define D	DENY
#define A	ALLOW

static char denyTable[NOPENS][NOPENS] = {
	/*0  1  2  3  4  5  6  7  8  9 10 11 */
	{ A, D, A, D, A, D, A, D, A, D, A, D }, /*  0 */
	{ D, D, D, D, A, D, A, D, D, D, D, D }, /*  1 */
	{ A, D, A, D, D, D, D, D, D, D, D, D }, /*  2 */
	{ D, D, D, D, D, D, D, D, D, D, D, D }, /*  3 */
	{ A, A, D, D, A, A, D, D, A, A, D, D }, /*  4 */
	{ D, D, D, D, A, A, D, D, D, D, D, D }, /*  5 */
	{ A, A, D, D, D, D, D, D, D, D, D, D }, /*  6 */
	{ D, D, D, D, D, D, D, D, D, D, D, D }, /*  7 */
	{ A, D, D, D, A, D, D, D, A, D, D, D }, /*  8 */
	{ D, D, D, D, A, D, D, D, D, D, D, D }, /*  9 */
	{ A, D, D, D, D, D, D, D, D, D, D, D }, /* 10 */
	{ D, D, D, D, D, D, D, D, D, D, D, D }	/* 11 */
};

#undef	D
#undef	A

/*
 * internal functions
 */

static openFileT FAR	*newOFile();
static void		freeOFile();
static void		fhAddOpen();
static void		fhRmvOpen();
static fileHdrT FAR	*getFHdr();
static fileHdrT FAR	*newFHdr();
static void		freeFHdr();
static void		getUniqueID();

/* 
 *	addOpen() - add an open to the open file table
 *
 *	input:	fileDesc - file descriptor
 *		fileStatP - pointer to file's stat(2) info or NIL
 *		sessID - session ID
 *		dosPID - process ID
 *		openModeP - maximum access (for DOS FCB opens)
 *		rwShare - reqested open/sharing modes
 *
 *	proc:	find a free slot in the open file table, check for
 *		compatability with existing opens of the same file, and add
 *		the new entry.
 *
 *	output:	(int) - entry index or FAIL
 *		*openModeP - actual access granted (for DOS FCB opens)
 *
 *	global:	rlockErr - may be set
 *		_rlockShm - checked
 */

int
addOpen(fileDesc, fileStatP, sessID, dosPID, openModeP, rwShare)
int fileDesc;
struct stat FAR *fileStatP;
long sessID, dosPID;
int FAR *openModeP;
int rwShare;
{	r0 openFileT FAR *scanOpenP;
	r1 fileHdrT FAR *fileHdrP;
	r2 openFileT FAR *openFileP;
	int accMode, denyMode, fcbAccess, oldDenyIndex, newDenyIndex, openSlot;
	bool dosCompat, dosFCB, rdOnlyFile, scanDosCompat;
	indexT openIndex;
	struct stat localStatus;

	/*
	 * if the file status isn't present, we need to get it.
	 */

	if (nil(fileStatP))
	{
		fileStatP = &localStatus;
		if (fstat(fileDesc, fileStatP) == FAIL)
		{
			rlockErr = RLERR_PARAM;
			return FAIL;
		}
	}

	/*
	 * if this is old style memory, we need to use a different function.
	 * _rlAddOpen() will set the rlockErr value.
	 */

	if (_rlockShm.useOldStyle)
		return _rlAddOpen((short)sessID, (short)dosPID, openModeP,
					rwShare, fileStatP);

	/*
	 * determine if the open is in DOS compatability or 'FCB' mode, and
	 * the access and deny modes.
	 */

	dosCompat = FALSE;
	dosFCB = FALSE;
	switch (SHR_BITS(rwShare))
	{
		case SHR_RDWRT:
			denyMode = OFD_READ | OFD_WRITE;
			break;
		case SHR_RD:
			denyMode = OFD_READ;
			break;
		case SHR_WRT:
			denyMode = OFD_WRITE;
			break;
		case SHR_NONE:
			denyMode = 0;
			break;
		case SHR_FCB:
			dosFCB = TRUE;
			/* fall through */
		case SHR_DOS:
			dosCompat = TRUE;
			denyMode = OFD_DOS_COMPAT;
			break;
		default:
			rlockErr = RLERR_PARAM;
			return FAIL;
	}
	switch (RW_BITS(rwShare))
	{
		case RW_RDONLY:
			accMode = OFA_READ;
			break;
		case RW_WRONLY:
			accMode = OFA_WRITE;
			break;
		case RW_RDWR:
			accMode = OFA_READ | OFA_WRITE;
			break;
		case RW_FCB:
			accMode = 0;
			break;
		default:
			rlockErr = RLERR_PARAM;
			return FAIL;
	}

	/*
	 * for FCB style opens, we need to get the access level specifid
	 * by the caller, and translate to open file style (OF_*) values.
	 * for all opens, we also need to determine the denyTable[] 'row'
	 * for it, and whether the file is read only.
	 */

	if (dosFCB)
	{
		if (*openModeP == O_RDONLY)
			fcbAccess = OFA_READ;
		else if (*openModeP == O_WRONLY)
			fcbAccess = OFA_WRITE;
		else
			fcbAccess = OFA_READ | OFA_WRITE;
	}
	rdOnlyFile = !ANYSET(fileStatP->st_mode, UP_OW | UP_GW | UP_WW);

	/*
	 * lock out anyone else from using the open file table.  _rlSetLocks()
	 * will set rlockErr.
	 */

	if (!_rlSetLocks(-OF_LOCK, UNUSED_LOCK))
		return FAIL;

	/*
	 * get a new open file table slot, and a new file header slot that
	 * corresponds to the file indicated via the status structure.  once
	 * we have the entries, update the open file table entry to the proper
	 * file header.
	 */

	openFileP = newOFile(fileDesc, sessID, dosPID, accMode, denyMode);
	if (nil(openFileP))
	{
		_rlUnlock();
		rlockErr = RLERR_NOSPACE;
		return FAIL;
	}
	fileHdrP = getFHdr(fileStatP, TRUE);
	if (nil(fileHdrP))
	{
		freeOFile(openFileP);
		_rlUnlock();
		rlockErr = RLERR_NOSPACE;
		return FAIL;
	}
	openFileP->ofFHdrIndex	= fileHdrP - _rlockShm.fhdrTableP;

	/*
	 * scan all existing opens to check for compatability.  in general, as
	 * soon as compatability between the request file and the one being
	 * checked in the table is assured, the section that verifies that will
	 * simply 'continue' the loop.  if an incompatability is discovered,
	 * the section will 'break' from the loop.
	 */

	newDenyIndex = DENY_TABLE_INDEX(openFileP);
	for (openIndex = fileHdrP->fhOpenIndex;
	     openIndex != INVALID_INDEX;
	     openIndex = scanOpenP->ofNextIndex)
	{
		/*
		 * convert the index into a pointer.
		 */

		scanOpenP = &_rlockShm.openTableP[openIndex];

		/*
		 * if DOS compatability mode is set for one file, it must be
		 * set for the other as well.  if not, then the file must be
		 * read only, and the open that ISN'T in DOS compatability
		 * mode must be set to 'deny-none' or 'deny-write'.  any other
		 * condition will fail.
		 */

		scanDosCompat = ANYSET(scanOpenP->ofDenyMode, OFD_DOS_COMPAT);
		if (!dosCompat && scanDosCompat)
		{
			if (!rdOnlyFile)
				break;
			if (openFileP->ofDenyMode == 0 ||
			    openFileP->ofDenyMode == OFD_WRITE)
			{
				continue;
			}
			break;
		}
		if (dosCompat && !scanDosCompat)
		{
			if (!rdOnlyFile)
				break;
			if (scanOpenP->ofDenyMode == 0 ||
			    scanOpenP->ofDenyMode == OFD_WRITE)
			{
				continue;
			}
			break;
		}

		/*
		 * okay, so now we know that both opens are either DOS
		 * compatible, or they are both not so.  if they are, then
		 * we need to check the following:
		 */

		if (dosCompat)
		{
			/*
			 * if the opener is the same for both files, the opens
			 * are compatable.  otherwise, if either has write
			 * access, the opens are incompatable.
			 */

			if (scanOpenP->ofSessID == sessID)
				continue;
			if (ANYSET(scanOpenP->ofAccMode, OFA_WRITE))
				break;
			if (ANYSET(openFileP->ofAccMode, OFA_WRITE))
				break;

			/*
			 * another open prohibits FCB write access.  if we
			 * have no other access requested, we can't handle
			 * the open.
			 */

			if (dosFCB)
			{
				fcbAccess &= ~OFA_WRITE;
				if (fcbAccess == 0)
					break;
			}

			/*
			 * we're okay in this mode, continue the loop.
			 */

			continue;
		}

		/*
		 * if we got here, we have to deal with the deny modes, and the
		 * algorithm is:
		 *
		 *	1) deny read/write will exclude ALL other opens; else
		 *	2) if opened by the same session, allow it; else
		 *	3) compare existing open with requested access.
		 */

		if (openFileP->ofDenyMode == (OFD_READ | OFD_WRITE) ||
		    scanOpenP->ofDenyMode == (OFD_READ | OFD_WRITE))
		{
			break;
		}
		if (scanOpenP->ofDosPID == dosPID && 
		    scanOpenP->ofSessID == sessID)
		{
			continue;
		}
		oldDenyIndex = DENY_TABLE_INDEX(scanOpenP);
		if (denyTable[oldDenyIndex][newDenyIndex] == DENY)
			break;
	}

	/*
	 * if we broke from the loop prematurely, a conflict was found, and
	 * the open request can't be honoured.  clean up and return the
	 * error.  this involves freeing the file header if its open list is
	 * empty (implying that it was added by this function), freeing the
	 * open file table entry, and unlocking the table itself.
	 */

	if (openIndex != INVALID_INDEX)
	{
		if (fileHdrP->fhOpenIndex == INVALID_INDEX)
			freeFHdr(fileHdrP);
		freeOFile(openFileP);
		_rlUnlock();
		rlockErr = RLERR_INUSE;
		return FAIL;
	}

	/*
	 * the open is allowed.  add the open file entry to the file header
	 * entry's list of opens, then release the table locks.  the rest of
	 * the processing doesn't require exclusive access to the tables.
	 */

	fhAddOpen(openFileP, fileHdrP);
	_rlUnlock();

	/*
	 * if this is an FCB open, record the granted modes in the open
	 * file table (we are the only process to use that entry, so we
	 * don't need the lock), then translate the bits back to the
	 * external version.
	 */

	if (dosFCB)
	{
		openFileP->ofAccMode = fcbAccess;
		if (fcbAccess == OFA_READ)
			*openModeP = O_RDONLY;
		else if (fcbAccess == OFA_WRITE)
			*openModeP = O_WRONLY;
		else
			*openModeP = O_RDWR;
	}

	/*
	 * return the table slot index.
	 */

	openSlot = openFileP - _rlockShm.openTableP;
	rlockState(openSlot, RLSTATE_OPENED);
	return openSlot;
}

/* 
 *	rmvOpen() - remove an entry from the open file table
 *
 *	input:	openEntry - index of the entry to be removed
 *
 *	proc:	reset the entry to an unused state, and return it to the
 *		free list.
 *
 *	output:	(int) - SUCCESS/FAIL
 *
 *	global:	_rlockShm - checked
 */

int
rmvOpen(openEntry)
int openEntry;
{	r0 openFileT FAR *openFileP;
	r1 fileHdrT FAR *fileHdrP;

	/*
	 * if this is old style memory, we need to use a different function.
	 * _rlRmvOpen() will set the rlockErr value.
	 */

	if (_rlockShm.useOldStyle)
		return _rlRmvOpen(openEntry);

	/*
	 * get the entry to remove.  _rlGetOFile() will set rlockErr.
	 */

	openFileP = _rlGetOFile(openEntry);
	if (nil(openFileP))
		return FAIL;

	/*
	 * guarantee exclusive table access.  _rlSetLocks() will set rlockErr.
	 */

	if (!_rlSetLocks(-OF_LOCK, UNUSED_LOCK))
		return FAIL;

	/*
	 * remove the open entry from its file header's open list.  if the file
	 * header's open list is now empty, remove the file header entry.
	 * finally, free the open entry itself.
	 */

	fhRmvOpen(openFileP);
	fileHdrP = &_rlockShm.fhdrTableP[openFileP->ofFHdrIndex];
	if (fileHdrP->fhOpenIndex == INVALID_INDEX)
		freeFHdr(fileHdrP);
	freeOFile(openFileP);

	/*	
	 * all done.  release the table and return.
	 */

	_rlUnlock();
	rlockState(openEntry, RLSTATE_CLOSED);
	return SUCCESS;
}

/*
 *	_rlGetOFile() - return a pointer to an indexed open file entry
 *
 *	input:	openEntry - the entry index
 *
 *	proc:	validate the index, get a pointer to the table, and check
 *		that the entry is actually in use.
 *
 *	output:	(openFileT *) - pointer to the entry
 *
 *	global:	rlockErr - may be set
 *		_rlockShm - checked
 */

openFileT FAR *
_rlGetOFile(openEntry)
int openEntry;
{	openFileT FAR *openFileP;

	/*
	 * validate the index.
	 */

	if (openEntry < 0 || openEntry >= _rlockShm.openSize)
	{
		rlockErr = RLERR_PARAM;
		return NIL_OPENFILE;
	}

	/*
	 * derive a pointer to the open file table, and verify that the
	 * slot is in use.  if it is, return the pointer.
	 */

	openFileP = &_rlockShm.openTableP[openEntry];
	if (openFileP->ofAccMode == 0)
	{
		rlockErr = RLERR_UNUSED;
		return NIL_OPENFILE;
	}
	return openFileP;
}

/*
 *	STATIC	newOFile() - get a free open file table entry
 *
 *	NOTE:	the open file table must be locked before this function is
 *		invoked.
 *
 *	input:	fileDesc - the host's file descriptor
 *		sessID - session ID of opener
 *		dosPID - DOS process ID of opener
 *		accMode - access mode
 *		denyMode - deny mode
 *
 *	proc:	find a free slot, initialize stuff and return a pointer to
 *		it.
 *
 *	output:	(openFileT *) - the entry or NIL
 *
 *	global:	_rlockShm - checked
 */

static openFileT FAR *
newOFile(fileDesc, sessID, dosPID, accMode, denyMode)
int fileDesc, accMode, denyMode;
long sessID, dosPID;
{	r0 openFileT FAR *openFileP;

	/*
	 * get the first entry on the free list.  if the free list is
	 * empty, return NIL.  if an entry is there, remove it from the
	 * list.
	 */

	if (*_rlockShm.openFreeIndexP == INVALID_INDEX)
		return NIL_OPENFILE;
	openFileP = &_rlockShm.openTableP[*_rlockShm.openFreeIndexP];
	*_rlockShm.openFreeIndexP = openFileP->ofNextIndex;

	/*
	 * initialize the structure and return a pointer to it.
	 */

	openFileP->ofFHdrIndex	= INVALID_INDEX;
	openFileP->ofNextIndex	= INVALID_INDEX;
	openFileP->ofSessID	= sessID;
	openFileP->ofDosPID	= dosPID;
	openFileP->ofFileDesc	= fileDesc;
	openFileP->ofAccMode	= accMode;
	openFileP->ofDenyMode	= denyMode;
	return openFileP;
}

/*
 *	STATIC	freeOFile() - free an open file table entry
 *
 *	NOTE:	the open file table must be locked before this function is
 *		invoked.
 *
 *	input:	openFileP - the entry to be freed
 *
 *	proc:	mark the entry and put it on the free list.
 *
 *	output:	(void) - none
 *
 *	global:	_rlockShm - checked
 */

static void
freeOFile(openFileP)
r0 openFileT FAR *openFileP;
{
	/*
	 * mark the entry as unused, and place it on the top of the free list.
	 */

	openFileP->ofFHdrIndex = INVALID_INDEX;
	openFileP->ofNextIndex = *_rlockShm.openFreeIndexP;
	*_rlockShm.openFreeIndexP = openFileP - _rlockShm.openTableP;
}

/*
 *	STATIC	fhAddOpen() - add an open to a file header entry
 *
 *	NOTE:	the open file table must be locked before this function is
 *		invoked.
 *
 *	input:	openFileP - the open file entry
 *		fileHdrP - the file header entry
 *
 *	proc:	push the entry onto the front of the file header entry's
 *		open list.
 *
 *	output:	(void) - none
 *
 *	global:	(none)
 */

static void
fhAddOpen(openFileP, fileHdrP)
r0 openFileT FAR *openFileP;
r1 fileHdrT FAR	*fileHdrP;
{
	/*
	 * simply put the open file entry on the front of the file header's
	 */

	openFileP->ofNextIndex = fileHdrP->fhOpenIndex;
	fileHdrP->fhOpenIndex = openFileP - _rlockShm.openTableP;
}

/*
 *	STATIC	fhRmvOpen() - remove an open file from a file header entry
 *
 *	NOTE:	the open file table must be locked before the function is
 *		invoked.
 *
 *	input:	openFileP - the open file
 *
 *	proc:	pull the entry out of the file header's list.
 *
 *	output:	(void) - none
 *
 *	global:	(none)
 */

static void
fhRmvOpen(openFileP)
r0 openFileT FAR *openFileP;
{	r1 openFileT FAR *scanOpenP;
	r2 fileHdrT FAR *fileHdrP;
	r3 openFileT FAR *nextOpenP;

	/*
	 * get a pointer to the open file's file header.
	 */

	fileHdrP = &_rlockShm.fhdrTableP[openFileP->ofFHdrIndex];

	/*
	 * if the file is the first in the file header's open list, it is a
	 * simple task to simply remove it, and return.
	 */

	scanOpenP = &_rlockShm.openTableP[fileHdrP->fhOpenIndex];
	if (scanOpenP == openFileP)
	{
		fileHdrP->fhOpenIndex = openFileP->ofNextIndex;
		openFileP->ofNextIndex = INVALID_INDEX;
		return;
	}

	/*
	 * not that simple, scan the list.  when the entry is found, remove
	 * it from the list and return.
	 */

	for (; !nil(scanOpenP); scanOpenP = nextOpenP)
	{
		/*
		 * get a pointer to the next entry.
		 */

		if (scanOpenP->ofNextIndex == INVALID_INDEX)
			nextOpenP = NIL_OPENFILE;
		else
			nextOpenP =
				&_rlockShm.openTableP[scanOpenP->ofNextIndex];

		/*
		 * if the next entry is what we are looking for, remove it
		 * from the list and return.
		 */

		if (nextOpenP == openFileP)
		{
			scanOpenP->ofNextIndex = openFileP->ofNextIndex;
			openFileP->ofNextIndex = INVALID_INDEX;
			return;
		}
	}

	/*
	 * we shouldn't ever get to this point.  if we do, it means the file
	 * header list didn't have the index of the open file entry that is
	 * being removed.  however, it really doesn't 'hurt' anything ...
	 */
}

/*
 *	STATIC	getFHdr() - get/add an entry into the file header table
 *
 *	NOTE:	the open file table must be locked before this function is
 *		invoked.
 *
 *	input:	fileStatP - file's status structure
 *		create - create the entry if it doesn't exist?
 *
 *	proc:	try to locate the file header table entry for the specified
 *		unique ID.  if one is found, return it.  if there is no
 *		corresponding entry and create is FALSE, return NIL.  if, on
 *		the other hand create is TRUE, and we didn't find an entry,
 *		try to create one.
 *
 *	output:	(fileHdrT *) - the new file header or NIL
 *
 *	global:	_rlockShm - checked
 */

static fileHdrT FAR *
getFHdr(fileStatP, create)
struct stat FAR *fileStatP;
bool create;
{	r0 fileHdrT FAR	*scanFHdrP;
	uniqueT uniqueID;
	fileHdrT FAR *newHeaderP;
	indexT FAR *hashedIndexP;
	indexT slotIndex;
	
	/*
	 * get the file's unique ID.
	 */

	getUniqueID(fileStatP, &uniqueID);

	/*
	 * get a pointer into the hashed file header table, and check if the
	 * 'hash bucket' is empty.  if it is, create and return a brand new
	 * file header entry if create is true; return NIL otherwise.
	 */

	hashedIndexP = &_rlockShm.hfhdrTableP[FH_NUM(uniqueID)];
	if (*hashedIndexP == INVALID_INDEX)
	{
		if (!create)
			return NIL_FILEHDR;
		newHeaderP = newFHdr(&uniqueID);
		if (!nil(newHeaderP))
			*hashedIndexP = newHeaderP - _rlockShm.fhdrTableP;
		return newHeaderP;
	}

	/*
	 * scan the chain indicated by the hashed index entry found above,
	 * for a file header with the requested unique ID.  if none is found,
	 * the last file header checked will be accessible via scanFHdrP, and
	 * its fhHashIndex value will be INVALID_INDEX.
	 */

	for (slotIndex = *hashedIndexP;
	     slotIndex != INVALID_INDEX;
	     slotIndex = scanFHdrP->fhHashIndex)
	{
		/*
		 * convert the index into a pointer.  then, if the entry's
		 * ID matches, return it.
		 */

		scanFHdrP = &_rlockShm.fhdrTableP[slotIndex];
		if (MATCH_UNIQUE_ID(scanFHdrP->fhUniqueID, uniqueID))
			return scanFHdrP;
	}

	/*
	 * if we didn't return, no matching ID was found.  if create is
	 * FALSE, we can't create a new one, so return NIL.  otherwise,
	 * take the last entry (which had the INVALID_INDEX as its
	 * fhHashIndex field data), and add a new file header to it.
	 */

	if (!create)
		return NIL_FILEHDR;
	newHeaderP = newFHdr(&uniqueID);
	if (!nil(newHeaderP))
		scanFHdrP->fhHashIndex = newHeaderP - _rlockShm.fhdrTableP;
	return newHeaderP;
}

/*
 *	STATIC	newFHdr() - get a free file header entry
 *
 *	NOTE:	the open file table must be locked before this function is
 *		invoked.
 *
 *	input:	uniqueIDP - pointer to the unique ID
 *
 *	proc:	find an available entry, and initialize it.  if no entries
 *		are available.
 *
 *	output:	(fileHdrT *) - the file header or NIL
 *
 *	global:	_rlockShm - checked
 */

static fileHdrT FAR *
newFHdr(uniqueIDP)
uniqueT *uniqueIDP;
{	r0 fileHdrT FAR	*fileHdrP;

	/*
	 * get the first entry on the free list.  if the free list is
	 * empty, return NIL.  if an entry is there, remove it from the
	 * list.
	 */

	if (*_rlockShm.fhdrFreeIndexP == INVALID_INDEX)
		return NIL_FILEHDR;
	fileHdrP = &_rlockShm.fhdrTableP[*_rlockShm.fhdrFreeIndexP];
	*_rlockShm.fhdrFreeIndexP = fileHdrP->fhHashIndex;

	/*
	 * initialize the structure and return a pointer to it.
	 */

	fileHdrP->fhOpenIndex	= INVALID_INDEX;
	fileHdrP->fhHashIndex	= INVALID_INDEX;
	fileHdrP->fhLockIndex	= INVALID_INDEX;
	fileHdrP->fhUniqueID	= *uniqueIDP;
	return fileHdrP;
}

/*
 *	STATIC	freeFHdr() - free a file header
 *
 *	NOTE:	the open file table must be locked when this function is
 *		invoked.
 *
 *	input:	fileHdrP - the file header to be freed
 *
 *	proc:	free the header.
 *
 *	output:	(void) - none
 *
 *	global:	_rlockShm - checked
 */

static void
freeFHdr(fileHdrP)
r0 fileHdrT FAR	*fileHdrP;
{	r1 fileHdrT FAR	*scanFHdrP;
	indexT paramIndex, slotIndex;
	indexT FAR *hashedIndexP;
	
	/*
	 * hash into the file header table.  there should be something in the
	 * hash table.  if not, there is an inconsistency, but return from the
	 * function anyway.
	 */

	hashedIndexP = &_rlockShm.hfhdrTableP[FH_NUM(fileHdrP->fhUniqueID)];
	if (*hashedIndexP == INVALID_INDEX)
		return;

	/*
	 * if the header to remove is the first in the chain, we need to handle
	 * it a little differently.
	 */

	paramIndex = fileHdrP - _rlockShm.fhdrTableP;
	if (paramIndex == *hashedIndexP)
	{
		/*
		 * remove the entry from the front of the chain.  this includes
		 * modifying the hash table as well as the free list index.
		 */

		*hashedIndexP = fileHdrP->fhHashIndex;
		fileHdrP->fhHashIndex = *_rlockShm.fhdrFreeIndexP;
		*_rlockShm.fhdrFreeIndexP = fileHdrP - _rlockShm.fhdrTableP;

		/*
		 * mark the entry unused and return.
		 */

		CLEAR_UNIQUE_ID(fileHdrP->fhUniqueID);
		return;
	}

	/*
	 * scan the hash chain for an entry with a matching ID.
	 */

	for (slotIndex = *hashedIndexP;
	     slotIndex != INVALID_INDEX;
	     slotIndex = scanFHdrP->fhHashIndex)
	{
		/*
		 * convert the index into a pointer.
		 */

		scanFHdrP = &_rlockShm.fhdrTableP[slotIndex];

		/*
		 * if the next element in the list is what we're looking for,
		 * remove it.
		 */

		if (scanFHdrP->fhHashIndex == paramIndex)
		{
			/*
			 * pull it out of the list, and stick it on the head
			 * of the free list.
			 */

			scanFHdrP->fhHashIndex = fileHdrP->fhHashIndex;
			fileHdrP->fhHashIndex = *_rlockShm.fhdrFreeIndexP;
			*_rlockShm.fhdrFreeIndexP = fileHdrP -
							_rlockShm.fhdrTableP;

			/*
			 * mark the entry as unused and return.
			 */

			CLEAR_UNIQUE_ID(fileHdrP->fhUniqueID);
			return;
		}
	}

	/*
	 * we shouldn't ever get to this point.  if we do, it means the file
	 * header list didn't have the index of the open file entry that is
	 * being removed.  however, it really doesn't 'hurt' anything ...
	 */
}

/* 
 *	STATIC	getUniqueID() - generate a unique ID from a status structure
 *
 *	input:	statP - the status structure
 *		uniqueIDP - pointer to the caller defined unique ID structure
 *
 *	proc:	construct a unique ID based on the information in the
 *		structure.  there is no failure return.
 *
 *	output:	(void) - none
 *
 *	global:	(none)
 */

static void
getUniqueID(statP, uniqueIDP)
r0 struct stat FAR *statP;
r1 uniqueT *uniqueIDP;
{
	/*
	 * this information is taken directly from the status structure.
	 * the casts are in there for no particularly good reason.
	 */

	uniqueIDP->fsysNumb	= (ulong)statP->st_dev;
	uniqueIDP->fileNumb	= (ulong)statP->st_ino;
}
