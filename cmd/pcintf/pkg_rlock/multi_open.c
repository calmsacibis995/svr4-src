/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/multi_open.c	1.1"
#include <sccs.h>

SCCSID("@(#)multi_open.c	1.1	13:35:25	8/31/89");

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
   multi_open.c: Implementation of openFile table operators

   Exported functions:
	_rlAddOpen():	Add an open to the openFile table
	_rlIsOpen();	Is it open?
	_rlRmvOpen():	Remove an open file from table
	_rlOGetOFile():	Validate a slot # and return pointer to its slot
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
 * this function is used for accessing the hash table.
 */

#define FH_HASH(id)	((int)((ulong)(id) % _rlockShm.hfhdrSize))

/*
 * internal variables
 */

#define NOPENS	12			/* Number of distinct open types */
#define ALLOW	0			/* denyTable value that allows open */

/*
 * denyTable: The following table is analogous the the table found
 * in the DOS 3.0/3.1 Technical reference except that the notation
 * is slightly different (A 'Y' in the manual's table appears as an
 * 'A' in this table and a 'N' in the manual's table is a 'D' here).
 * More importantly, the order of the rows and collumns is not the
 * same.  The order of the rows and collumns here is dictated by
 * the internal representation of the access and deny modes.  The
 * following mapping can be applied to translate coordinates in the
 * DOS manual's table to coordinates in this table:
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
 * to make the following table more clear, the following shorthand is used
 * for each entry:
 *
 *	D -> deny
 *	A -> allow
 */

#define D	1
#define A	0

static char denyTable[NOPENS][NOPENS] = {
	/* +->	1st subscript - existing open
	 * |
	 * 0 1 	2  3  4	 5  6  7  8  9 10 11
	 */
	{ A, D, A, D, A, D, A, D, A, D, A, D }, /*  0	--> 2nd subscript */
	{ D, D, D, D, A, D, A, D, D, D, D, D }, /*  1		new open */
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

static oldOpenFile FAR	*newOFile();
static void		freeOFile();
static void		fhAddOpen();
static void		fhRmvOpen();
static oldFileHdr FAR	*getFHdr();
static oldFileHdr FAR	*chkFHdr();
static oldFileHdr FAR	*newFHdr();
static void		fhdrFree();
static long		getUniqueID();

/* 
   _rlAddOpen: Add an open to openFile table enforcing exclusion relationships

   Input Parameters:
	sessID: Session ID of file opener
	dosPID: Process ID of file opener
	*openMode: Current maximum access (for DOS FCB opens)
	rwShare: Requested open and sharing modes (smb_vwv[0] from Open File
		requests - faked/constructed for creates)
	fileStat: Pointer to file's stat info
	uniqueID: File's unique identifier

   Tasks:
	Find free slot in openFile table
	Check compatibility of new open with existing opens of same file
	Add new open entry if compatible

   Outputs:
	Return Value: openFile table index or FAIL
	*openMode: Actual access granted (for DOS FCB mode opens)
	openFile table: May have new entry added
*/

int
_rlAddOpen(sessID, dosPID, openMode, rwShare, fileStat)
short		sessID;			/* Session ID */
short		dosPID;			/* DOS process ID */
int FAR		*openMode;		/* Access modes granted */
int		rwShare;		/* Access and share modes requested */
struct stat FAR	*fileStat;		/* File's status information */
{
r2 oldOpenFile FAR	*oFile;			/* Pointer to new table slot */
r0 oldOpenFile FAR	*oScan;			/* Check opens for compatibility */
r1 oldFileHdr FAR	*fHdr;			/* File header for uniqeID */
int		fcbAccess;		/* Access granted to FCB open */
bool		dosCompat = FALSE;	/* Attempting DOS compat. mode open */
bool		dosFCB = FALSE;		/* Attempting DOS FCB style open */
bool		rdOnlyFile;		/* File to be opened is read-only */
int		oldX;			/* Deny table index for existing open */
int		newX;			/* Deny table index for new open */
int		openSlot;		/* The open file slot */

	/* Exclude other servers from any access to openFile table */
	if (!_rlSetLocks(-OF_LOCK, UNUSED_LOCK))
		return FAIL;

	/* Get a new openFile table slot */
	if (nil(oFile = newOFile())) {
		_rlUnlock();
		rlockErr = RLERR_NOSPACE;
		return FAIL;
	}

	/* Get the fileHdr table entry for this uniqueID */
	if (nil(fHdr = getFHdr(getUniqueID(fileStat)))) {
		freeOFile(oFile);
		_rlUnlock();
		rlockErr = RLERR_NOSPACE;
		return FAIL;
	}

	/* Initialize openFile table entry */
	oFile->sessID = sessID;
	oFile->dosPID = dosPID;
	oFile->header = fHdr;

	/* Compute denyMode field for openFile table entry */
	switch (SHR_BITS(rwShare)) {
	case SHR_RDWRT:				/* Deny both read and write */
		oFile->denyMode = OFD_READ | OFD_WRITE;
		break;

	case SHR_RD:				/* Deny read */
		oFile->denyMode = OFD_READ;
		break;

	case SHR_WRT:				/* Deny write */
		oFile->denyMode = OFD_WRITE;
		break;

	case SHR_NONE:				/* Deny none */
		oFile->denyMode = 0;
		break;

	case SHR_FCB:				/* DOS FCB open */
		dosFCB = TRUE;
		/* Fall through */

	case SHR_DOS:				/* DOS compatibility mode */
		dosCompat = TRUE;
		oFile->denyMode = OFD_DOS_COMPAT;
		break;

	default:
		freeOFile(oFile);
		if (nil(fHdr->openList))
			fhdrFree(fHdr);
		_rlUnlock();
		rlockErr = RLERR_PARAM;
		return FAIL;
	}

	/* Compute openFile.accMode */
	switch (RW_BITS(rwShare)) {
	case RW_RDONLY:		
		oFile->accMode = OFA_READ;
		break;

	case RW_WRONLY:
		oFile->accMode = OFA_WRITE;
		break;

	case RW_RDWR:
		oFile->accMode = OFA_READ | OFA_WRITE;
		break;

	case RW_FCB:				/* FCB opens */
		oFile->accMode = 0;
		break;

	default:
		freeOFile(oFile);
		if (nil(fHdr->openList))
			fhdrFree(fHdr);
		_rlUnlock();
		rlockErr = RLERR_PARAM;
		return FAIL;
	}

	/* For FCB opens, get access level allowed by higher level code */
	if (dosFCB) {
		/* Translate open() style bits to oFile access bits */
		if (*openMode == O_RDONLY)
			fcbAccess = OFA_READ;
		else if (*openMode == O_WRONLY)
			fcbAccess = OFA_WRITE;
		else
			fcbAccess = OFA_READ | OFA_WRITE;
	}

	/* Find out table row of new open and whether file is read-only */
	newX = ((oFile->accMode - 1) << 2) | oFile->denyMode;
	rdOnlyFile = !(fileStat->st_mode & (UP_OW | UP_GW | UP_WW));

	/* Scan existing opens on same file checking for compatibility */
	for (oScan = fHdr->openList; !nil(oScan); oScan = oScan->nextOpen) {
		/*
		   DOS compatibility mode incompatible with all deny modes
		   except when file is read-only, then deny none and deny
		   write are compatible with DOS compatible opens.
		*/
		if (!dosCompat && (oScan->denyMode & OFD_DOS_COMPAT)) {
			if (rdOnlyFile
			&&  (oFile->denyMode == 0 ||
			     oFile->denyMode == OFD_WRITE))
				continue;
			break;
		}
		if (dosCompat && (oScan->denyMode & OFD_DOS_COMPAT) == 0) {
			if (rdOnlyFile
			&&  (oScan->denyMode == 0 ||
			    oScan->denyMode == OFD_WRITE))
				continue;
			break;
		}

		/* DOS compatibility mode checks */
		if (dosCompat) {
			/* DOS compat mode only excludes between sessions */
			if (oScan->sessID != sessID) {
				/* DOS mode writer use is exclusive */
				if (oScan->accMode & OFA_WRITE)
					break;
				if (oFile->accMode & OFA_WRITE)
					break;

				/* Another open prohibits FCB write access */
				if (dosFCB) {
					fcbAccess &= ~OFA_WRITE;
					if (fcbAccess == 0)
						break;
				}
			}
		}
		/* Deny mode checks */
		else {
			/* Deny read/write excludes ALL other opens */
			if (oFile->denyMode == (OFD_READ | OFD_WRITE)
			||  oScan->denyMode == (OFD_READ | OFD_WRITE))
				break;

			/* Only deny opens from different openers */
			if (oScan->dosPID == dosPID && oScan->sessID == sessID)
				continue;

			oldX = ((oScan->accMode - 1) << 2) | oScan->denyMode;
			/* Is existing open compatible with requested access? */
			if (denyTable[oldX][newX] != ALLOW)
				break;
		}
	}

	/* If search ran off end of openList, requested open is allowable */
	if (nil(oScan)) {
		/* Add new openFile entry to open list of fileHdr */
		fhAddOpen(oFile, fHdr);

		/* Release exclusion on openFile table */
		_rlUnlock();

		/* Record and return access modes granted for FCB mode opens */
		if (dosFCB) {
			oFile->accMode = fcbAccess;
			/* Translate access() style bits to open() style bits */
			if (fcbAccess == OFA_READ)
				*openMode = O_RDONLY;
			else if (fcbAccess == OFA_WRITE)
				*openMode = O_WRONLY;
			else
				*openMode = O_RDWR;
		}

		/* Return openFile table slot index */
		openSlot = oFile - _rlockShm.oOpenTableP;
		rlockState(openSlot, RLSTATE_OPENED);
		return openSlot;
	}

	/* Exclusion disallowed open - clean up and return FAIL */
	/*
	   Only free fHdr if its openList is empty, which
	   implys it was added by the above call to getFHdr()
	*/
	if (nil(fHdr->openList))
		fhdrFree(fHdr);

	/* Free newly allocated openFile slot, release exclusion and return */
	freeOFile(oFile);
	_rlUnlock();
	rlockErr = RLERR_INUSE;
	return FAIL;
}


/* 
   _rlIsOpen: Check to see if a file is open

   Input Parameters:
	uniqueID: File's unique identifier

   Tasks:
	Look up file header for uniqueID and see if its openList is non-empty

   Outputs:
	Return Value: TRUE if file is open, FALSE if not
*/

int
_rlIsOpen(uniqueID)
long		uniqueID;		/* Unique ID of file to check */
{
bool		reallyOpen;		/* Result value */

	/* Set read lock */
	if (!_rlSetLocks(OF_LOCK, UNUSED_LOCK))
		return FAIL;

	/* Look up file header, if found, there's and open, otherwise not */
	reallyOpen = !nil(chkFHdr(uniqueID));

	/* Release lock and return result */
	_rlUnlock();
	return reallyOpen;
}


/* 
   _rlRmvOpen: Remove an entry from the openFile table

   Input Parameters:
	openEntry: Index of entry to be removed from openFile table

   Tasks:
	Reset entry to unused state and return to free list

   Outputs:
	Return Value: SUCCESS/FAIL
	openFile table: An entry may be removed from table
*/

int
_rlRmvOpen(openEntry)
int		openEntry;		/* Index of entry to remove */
{
r0 oldOpenFile FAR	*oFile;			/* Pointer to table slot */

	/* Validate openEntry */
	if (nil(oFile = _rlOGetOFile(openEntry)))
	{
		rlockErr = RLERR_PARAM;
		return FAIL;
	}

	/* Exclude other servers from openFile table */
	if (!_rlSetLocks(-OF_LOCK, UNUSED_LOCK))
		return FAIL;

	/* Remove openEntry from its fileHdr's openList */
	fhRmvOpen(oFile);

	/* If fileHdr's openList is now empty, remove it from fileHdr table */
	if (nil(oFile->header->openList))
		fhdrFree(oFile->header);

	/* Free the openFile table entry */
	freeOFile(oFile);

	/* Release exclusion on openFile table and return */
	_rlUnlock();
	rlockState(openEntry, RLSTATE_CLOSED);
	return SUCCESS;
}


/*
   _rlOGetOFile: Validate open file slot # and return pointer to that slot
	The openFile table need not be locked.
*/

oldOpenFile FAR *
_rlOGetOFile(openEntry)
int		openEntry;		/* Index of entry to remove */
{
oldOpenFile FAR	*oFile;			/* Pointer to table slot */

	/* Validate openEntry */
	if (openEntry < 0 || openEntry >= _rlockShm.openSize) {
		rlockErr = RLERR_PARAM;
		return NIL_O_OPENFILE;
	}

	/* Derive pointer to openFile table entry */
	oFile = &_rlockShm.oOpenTableP[openEntry];

	/* Verify that openFile table slot is in use */
	if (oFile->accMode == 0) {
		rlockErr = RLERR_UNUSED;
		return NIL_O_OPENFILE;
	}

	return oFile;
}


/*
   newOFile: Get a free openFile table slot
	Returns pointer to allocated slot or NIL if no slots are free
	NOTE: The openFile table must be locked when newOFile() is called
*/

static oldOpenFile FAR *
newOFile()
{
r0 oldOpenFile FAR	*ofNew;			/* Newly allocated openFile */

	/* Get first openFile on free list, if none, return NIL */
	if (nil(ofNew = *_rlockShm.oOpenFreePP))
		return NIL_O_OPENFILE;

	/* Remove openFile from free list, initialize and return it */
	*_rlockShm.oOpenFreePP = ofNew->nextOpen;
	ofNew->header = NIL_O_FILEHDR;
	ofNew->nextOpen = NIL_O_OPENFILE;
	return ofNew;
}

/*
   freeOFile: Free an entry in the openFile table
	NOTE: The openFile table must be locked when freeOFile() is called.
*/

static void
freeOFile(oFile)
r0 oldOpenFile FAR *oFile;			/* OpenFile table entry to free */
{
	oFile->nextOpen = *_rlockShm.oOpenFreePP;
	*_rlockShm.oOpenFreePP = oFile;
	oFile->header = NIL_O_FILEHDR;
}


/*
   fhAddOpen: Add and openFile entry to a fileHdr's openList
	NOTE: The openFile table must be locked when fhAddOpen() is called
*/

static void
fhAddOpen(oFile, fHdr)
r0 oldOpenFile FAR	*oFile;			/* Add this OpenFile.. */
r1 oldFileHdr FAR	*fHdr;			/* .. to this fileHdr */
{
	/* Push oFile onto front of fHdr's openList */
	oFile->nextOpen = fHdr->openList;
	fHdr->openList = oFile;
}


/*
   fhRmvOpen: Remove openFile from a fileHdr openList
	NOTE: The openFile table must be locked when fhRmvOpen() is called
*/

static void
fhRmvOpen(oFile)
r0 oldOpenFile FAR		*oFile;		/* openFile to remove */
{
r1 oldOpenFile FAR		*oScan;		/* Scan the fileHdr's openList */
r2 oldFileHdr FAR		*fHdr;		/* The fileHdr for openFile */

	/* Get oFile's fileHdr */
	fHdr = oFile->header;

	/* If oFile is first in fHdr's openList, change fHdr->openList */
	if ((oScan = fHdr->openList) == oFile) {
		fHdr->openList = oFile->nextOpen;
		oFile->nextOpen = NIL_O_OPENFILE;
		return;
	}

	/* Scan the openList */
	for (; !nil(oScan); oScan = oScan->nextOpen) {
		/* If entry after oScan is oFile, unlink and return */
		if (oScan->nextOpen == oFile) {
			oScan->nextOpen = oFile->nextOpen;
			oFile->nextOpen = NIL_O_OPENFILE;
			return;
		}
	}
}


/*
   getFHdr: Get/add entry for uniqueID in/to fileHdr table
	NOTE: The openFile table must be locked when getFHdr() is called
*/

static oldFileHdr FAR *
getFHdr(uniqueID)
r1 long		uniqueID;		/* File's unique identifier */
{
r0 oldFileHdr FAR	*fhScan;		/* File header pointer */
r2 oldFileHdr FAR	**fhHash;		/* Hash bucket for uniqueID */

	/* Hash into file header table */
	fhHash = &_rlockShm.oHfhdrTablePP[FH_HASH(uniqueID)];

	/* If hash bucket is empty, create add and return new fileHdr */
	if (nil(*fhHash))
		return *fhHash = newFHdr(uniqueID);

	/* Scan hash chain for entry with matching uniqueID */
	for (fhScan = *fhHash; ; fhScan = fhScan->hashLink) {
		/* When the matching entry is found, return it */
		if (fhScan->uniqueID == uniqueID)
			return fhScan;

		/* When the end of the hash list is found, add a new entry */
		if (nil(fhScan->hashLink))
			return fhScan->hashLink = newFHdr(uniqueID);
	}
}


/*
   chkFHdr: Get entry for uniqueID if it is in fileHdr table
	NOTE: The openFile table must be locked when getFHdr() is called
*/

static oldFileHdr FAR *
chkFHdr(uniqueID)
r1 long		uniqueID;		/* File's unique identifier */
{
r0 oldFileHdr FAR	*fhScan;		/* File header pointer */
r2 oldFileHdr FAR	**fhHash;		/* Hash bucket for uniqueID */

	/* Hash into file header table */
	fhHash = &_rlockShm.oHfhdrTablePP[FH_HASH(uniqueID)];

	/* If hash bucket is empty, return NIL */
	if (nil(*fhHash))
		return NIL_O_FILEHDR;

	/* Scan hash chain for entry with matching uniqueID */
	for (fhScan = *fhHash; !nil(fhScan); fhScan = fhScan->hashLink)
		if (fhScan->uniqueID == uniqueID)
			return fhScan;

	return NIL_O_FILEHDR;
}


/*
   newFHdr: Get a free fileHdr table slot
	Returns pointer to allocated slot or NIL if no slots are free
	NOTE: The openFile table must be locked when newFHdr() is called
*/

static oldFileHdr FAR *
newFHdr(uniqueID)
long		uniqueID;		/* Unique ID to add to new fileHdr */
{
r0 oldFileHdr FAR	*fhNew;			/* Scan table for a free slot */

	/* Get a fileHdr from the free list - if none, return NIL */
	if (nil(fhNew = *_rlockShm.oFhdrFreePP))
		return NIL_O_FILEHDR;

	/* Remove fhNew from free list, initialize and return it */
	*_rlockShm.oFhdrFreePP = fhNew->hashLink;
	fhNew->uniqueID = uniqueID;
	fhNew->openList = NIL_O_OPENFILE;
	fhNew->hashLink = NIL_O_FILEHDR;
	fhNew->lockList = NIL_O_RECLOCK;
	return fhNew;
}


/*
   fhdrFree: Free file header entry if not in use (openList is NIL)
	NOTE: The openFile table must be locked when fhdrFree() is called
*/

static void
fhdrFree(fHdr)
r0 oldFileHdr FAR	*fHdr;			/* File header slot to free */
{
r1 oldFileHdr FAR	*fhScan;		/* Scan hash chain */
r2 oldFileHdr FAR	**hashSlot;		/* Hash bucket pointer */

	/* Hash into file header table */
	hashSlot = &_rlockShm.oHfhdrTablePP[FH_HASH(fHdr->uniqueID)];

	/* If there is nothing in the hash bucket, there's an inconsistency */
	if (nil(*hashSlot))
		return;

	/* See if fHdr is first in hash chain */
	if (*hashSlot == fHdr) {
		/* Remove fHdr from front of hash chain */
		*hashSlot = fHdr->hashLink;
		fHdr->hashLink = *_rlockShm.oFhdrFreePP;
		fHdr->uniqueID = 0L;
		*_rlockShm.oFhdrFreePP = fHdr;
		return;
	}

	/* Scan hash chain for entry with matching uniqueID */
	for (fhScan = *hashSlot; !nil(fhScan); fhScan = fhScan->hashLink) {
		/* When next element in hash list is fHdr, unlink it */
		if (fhScan->hashLink == fHdr) {
			fhScan->hashLink = fHdr->hashLink;
			fHdr->hashLink = *_rlockShm.oFhdrFreePP;
			fHdr->uniqueID = 0L;
			*_rlockShm.oFhdrFreePP = fHdr;
			return;
		}
	}
}

/* 
 *	STATIC	getUniqueID() - generate a unique ID from a status structure
 *
 *	input:	statP - the status structure
 *
 *	proc:	construct a unique ID based on the information in the
 *		structure.  there is no failure return.
 *
 *	output:	(long) - the unique identifier
 *
 *	global:	(none)
 */

static long
getUniqueID(statP)
struct stat FAR *statP;
{
	/*
	 * for the time being, just use the st_dev and st_ino fields in
	 * the high and low 'shorts' of the output data.
	 */

	return ((long)statP->st_dev << 16) | statP->st_ino;
}
