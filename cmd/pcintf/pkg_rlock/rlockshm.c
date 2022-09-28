/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/rlockshm.c	1.1"
#include <sccs.h>

SCCSID("@(#)rlockshm.c	1.2	17:04:11	2/12/90"); 

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

/*---------------------------------------------------------------------------
 *
 *	rlockshm.c - initialize the RLOCK package's shared memory
 *
 *	usage:
 *		rlockshm [-cdmor] [-AFHLOUV]
 *	where:
 *		documented:
 *		-c	- create the shared memory segment
 *		-d	- show the default configuration (only)
 *		-m	- display miscellaneous existing segment information
 *		-o	- handle the old style (multi-segment) shared memory
 *		-r	- remove the shared memory segment
 *
 *		undocumented:
 *		-A	- display all (even unused) entries
 *		-F	- display the file header table
 *		-H	- display the hashed file header table
 *		-L	- display the record lock table
 *		-O	- display the open file table
 *		-U	- print usage with undocumented options (only)
 *		-V	- print the version/copyright (only)
 *
 *	the documented options must be supported for the customer's use.  the
 *	undocumented ones are only for Locus support, and need not be defined
 *	for the customer.
 *
 *	normal operation is to attach to the shared memory, not to display
 *	anything.  this has the effect of creating the default segment if it
 *	doesn't already exist, and verifing the ability to attach to it in
 *	any case.  however, some options will do the operation and immediately
 *	exit, doing nothing else (marked with 'only').
 *
 *---------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include <rlock.h>

#include <internal.h>

extern char *optarg;
extern int  optind;

/*
 * standard exit status values.
 */

#define EX_GOOD		0			/* all is well */
#define EX_BAD		1			/* something is screwed */

/*
 * internal variables.
 */

static char *usageP = "Usage: %s [-cdmor]\n";
static char *fullUsageP = "Usage: %s [-cdmor] [-AFHLOUV]\n";

/*
 * internal functions.
 */

static void displayConfig();
static void dispMisc();
static void dispFileTable();
static void dispHashTable();
static void dispLockTable();
static void dispOpenTable();

/*
 *	main() - main routine
 *
 *	input:	argc - count of arguments
 *		argV - vector of same
 *
 *	proc:	initialize shared memory.
 *
 *	output:	(int) - standard UNIX process return value
 */

int
main(argc, argV)
int argc;
char *argV[];
{	int optChar, retCode;
	char *actionStrP;
	bool createShm, attachShm, oldStyle;
	bool showAll, dMisc, dFileTable, dHashTable, dLockTable, dOpenTable;

	/*
	 * set global system needs and get the options.
	 */

	createShm	= FALSE;
	attachShm	= TRUE;
	oldStyle	= FALSE;
	showAll		= FALSE;
	dMisc		= FALSE;
	dFileTable	= FALSE;
	dHashTable	= FALSE;
	dLockTable	= FALSE;
	dOpenTable	= FALSE;
	while ((optChar = getopt(argc, argV, "cdmorAFHLOUV")) != EOF)
	{
		switch (optChar)
		{
			case 'c':	/* create the shared memory */
				createShm = TRUE;
				attachShm = FALSE;
				break;
			case 'd':	/* display default config (only) */
				displayConfig();
				return EX_GOOD;
			case 'm':	/* display misc. information */
				dMisc = TRUE;
				break;
			case 'o':	/* use old-style shared memory */
				oldStyle = TRUE;
				break;
			case 'r':	/* remove the shared memory */
				createShm = FALSE;
				attachShm = FALSE;
				break;
			case 'A':	/* display 'all' information */
				showAll = TRUE;
				break;
			case 'F':	/* display the file table */
				dFileTable = TRUE;
				break;
			case 'H':	/* display the hash table */
				dHashTable = TRUE;
				break;
			case 'L':	/* display the lock table */
				dLockTable = TRUE;
				break;
			case 'O':	/* display the open file table */
				dOpenTable = TRUE;
				break;
			case 'U':	/* full usage message (only) */
				printf(fullUsageP, argV[0]);
				return EX_GOOD;
			case 'V':	/* display the version (only) */
				printf("\nVersion 1.0\n");
				printf("Copyright (c) Locus Computing");
				printf(" Corporation 1986, 1987, 1988.\n");
				printf("All Rights Reserved.\n\n");
				return EX_GOOD;
			default:
				printf(usageP, argV[0]);
				return EX_BAD;
		}
	}
	if (argc != optind)
	{
		printf(usageP, argV[0]);
		return EX_BAD;
	}

	/*
	 * create or remove the shared table segment.
	 */

	if (createShm)
	{
		retCode = oldStyle ? _rlMCreate() : rlockCreate();
		actionStrP = "initialize";
	}
	else if (attachShm)
	{
		retCode = oldStyle ? _rlMAttach() : rlockAttach();
		actionStrP = "attach";
	}
	else
	{
		retCode = oldStyle ? _rlMRemove() : rlockRemove();
		actionStrP = "remove";
	}

	/*
	 * if we created or attached, and the function didn't fail, we may want
	 * to see the data.
	 */

	if ((createShm || attachShm) && (retCode == SUCCESS))
	{
		if (dMisc) dispMisc();
		if (dFileTable) dispFileTable(showAll);
		if (dHashTable) dispHashTable(showAll);
		if (dLockTable) dispLockTable(showAll);
		if (dOpenTable) dispOpenTable(showAll);
	}

	/*
	 * figure out what went on, tell the user if we failed.
	 */

	if (retCode == SUCCESS)
		return EX_GOOD;

	printf("Unable to %s common shared memory, ", actionStrP);
	if (rlockErr == RLERR_SYSTEM)
		printf("(system error: %d).\n", errno);
	else
		printf("%s.\n", rlockEString(rlockErr));
	return EX_BAD;
}

/*
 *	STATIC	displayConfig() - display the default configuration
 *
 *	input:	(none)
 *
 *	proc:	just print the default configuration.
 *
 *	output:	(void) - none
 *
 *	global:	_rlCfgData - checked
 */

static void
displayConfig()
{
	printf("\nDefault configuration information (new-style only):\n\n");
	if (nil(_rlCfgData.cfgBaseP))
		printf("\tsegment attach address\t\t(program selected)\n");
	else
		printf("\tsegment attach address\t\t%#.8lx\n",
			_rlCfgData.cfgBaseP);
	printf("\n");
	printf("\tshared memory key\t\t%#.8lx\n",
		_rlCfgData.cfgShmKey);
	printf("\tlockset key\t\t\t%#.8lx\n",
		_rlCfgData.cfgLsetKey);
	printf("\n");
	printf("\topen file table entries\t\t%d\n",
		_rlCfgData.cfgOpenTable);
	printf("\tfile header table entries\t%d\n",
		_rlCfgData.cfgFileTable);
	printf("\thashed file table entries\t%d\n",
		_rlCfgData.cfgHashTable);
	printf("\trecord lock table entries\t%d\n",
		_rlCfgData.cfgLockTable);
	printf("\n");
	printf("\tindividual record locks\t\t%d\n",
		_rlCfgData.cfgRecLocks);
}

/*
 *	STATIC	dispMisc() - display miscellaneous information
 *
 *	input:	(none)
 *
 *	proc:	display the information kept in the _rlockShm table.
 *
 *	output:	(void) - none
 *
 *	global:	_rlockShm - checked
 */

static void
dispMisc()
{
	/*
	 * tell the user what is being printed, then print it.
	 */

	printf("\nMiscellaneous existing memory information (%s style):\n\n",
		_rlockShm.useOldStyle ? "old" : "new");
	if (_rlockShm.useOldStyle)
	{
		printf("\topen file table entries\t\t%d @ %d bytes\n",
			_rlockShm.openSize, sizeof(oldOpenFile));
		printf("\tfile header table entries\t%d @ %d bytes\n",
			_rlockShm.fhdrSize, sizeof(oldFileHdr));
		printf("\thashed file table entries\t%d @ %d bytes\n",
			_rlockShm.hfhdrSize, sizeof(indexT));
		printf("\trecord lock table entries\t%d @ %d bytes\n",
			_rlockShm.lockSize, sizeof(oldRecLock));
		printf("\n");
		printf("\topen segment base\t\t%#.8lx\n",
			_rlockShm.oOpenSegmentP);
		printf("\tfile segment base\t\t%#.8lx\n",
			_rlockShm.oFhdrSegmentP);
		printf("\tlock segment base\t\t%#.8lx\n",
			_rlockShm.oLockSegmentP);
		printf("\n");
		printf("\topen file table\t\t\t%#.8lx\n",
			_rlockShm.oOpenTableP);
		printf("\tfile header table\t\t%#.8lx\n",
			_rlockShm.oFhdrTableP);
		printf("\thashed file table\t\t%#.8lx\n",
			_rlockShm.oHfhdrTablePP);
		printf("\trecord lock table\t\t%#.8lx\n",
			_rlockShm.oRlockTableP);
		printf("\n");
		printf("\topen table free list\t\t%#.8lx\n",
			*_rlockShm.oOpenFreePP);
		printf("\tfile table free list\t\t%#.8lx\n",
			*_rlockShm.oFhdrFreePP);
		printf("\tlock table free list\t\t%#.8lx\n",
			*_rlockShm.oLockFreePP);
	}
	else
	{
		printf("\topen file table entries\t\t%d @ %d bytes\n",
			_rlockShm.openSize, sizeof(openFileT));
		printf("\tfile header table entries\t%d @ %d bytes\n",
			_rlockShm.fhdrSize, sizeof(fileHdrT));
		printf("\thashed file table entries\t%d @ %d bytes\n",
			_rlockShm.hfhdrSize, sizeof(indexT));
		printf("\trecord lock table entries\t%d @ %d bytes\n",
			_rlockShm.lockSize, sizeof(recLockT));
      		printf("\ttotal segment size\t\t%ld bytes\n",
			_rlockShm.segSize);
		printf("\n");
		printf("\tindividual record locks\t\t%d\n",
			_rlockShm.recLocks);
		printf("\n");
		printf("\tattachment base\t\t\t%#.8lx",
			_rlockShm.segmentP);
		if (_rlCfgData.cfgBaseP == 0) printf(" (program selected)");
		printf("\n");
		printf("\topen file table\t\t\t%#.8lx\n",
			_rlockShm.openTableP);
		printf("\tfile header table\t\t%#.8lx\n",
			_rlockShm.fhdrTableP);
		printf("\thashed file table\t\t%#.8lx\n",
			_rlockShm.hfhdrTableP);
		printf("\trecord lock table\t\t%#.8lx\n",
			_rlockShm.rlockTableP);
		printf("\n");
		printf("\topen table free list index\t%ld\n",
			*_rlockShm.openFreeIndexP);
		printf("\tfile table free list index\t%ld\n",
			*_rlockShm.fhdrFreeIndexP);
		printf("\tlock table free list index\t%ld\n",
			*_rlockShm.lockFreeIndexP);
	}
}

/*
 *	STATIC	dispFileTable() - display the file header table
 *
 *	input:	showAll - display all data?
 *
 *	proc:	display the information kept in the file header table.
 *
 *	output:	(void) - none
 *
 *	global:	_rlockShm - checked
 */

static void
dispFileTable(showAll)
bool showAll;
{	fileHdrT *entryP, *lastP;
	oldFileHdr *oldEntryP, *oldLastP;

	/*
	 * what we display depends on the style of memory being used.
	 */

	printf("\nFile header table entries:\n");
	if (_rlockShm.useOldStyle)
	{
		printf("%12s%12s%12s%12s%12s\n",
			"entry", "open-list", "lock-list", "hash-link", 
			"unique-ID");
		oldLastP = _rlockShm.oFhdrTableP + _rlockShm.fhdrSize;
		for (oldEntryP = _rlockShm.oFhdrTableP;
		     oldEntryP < oldLastP;
		     oldEntryP++)
		{
			if (!showAll && (oldEntryP->uniqueID == 0L))
				continue;
			printf("%#12lx%#12lx%#12lx%#12lx%12ld\n",
				oldEntryP, oldEntryP->openList,
				oldEntryP->lockList, oldEntryP->hashLink,
				oldEntryP->uniqueID);
		}
	}
	else
	{
		printf("%6s%12s%12s%12s%12s\n",
			"entry", "open-index", "lock-index",
			"hash-index", "unique-ID");
		lastP = _rlockShm.fhdrTableP + _rlockShm.fhdrSize;
		for (entryP = _rlockShm.fhdrTableP;
		     entryP < lastP;
		     entryP++)
		{
			if (!showAll && !UNIQUE_ID_USED(entryP->fhUniqueID))
				continue;
			printf("%6d%12ld%12ld%12ld%12ld,%ld\n",
				(int)(entryP - _rlockShm.fhdrTableP),
				entryP->fhOpenIndex,
				entryP->fhLockIndex,
				entryP->fhHashIndex,
				entryP->fhUniqueID.fsysNumb,
				entryP->fhUniqueID.fileNumb);
		}
	}
}

/*
 *	STATIC	dispHashTable() - display the hashed file header table
 *
 *	input:	showAll - display all data?
 *
 *	proc:	display the information kept in the file header table.
 *
 *	output:	(void) - none
 *
 *	global:	_rlockShm - checked
 */

static void
dispHashTable(showAll)
bool showAll;
{	indexT *entryP, *lastP;
	oldFileHdr **oldEntryPP, **oldLastPP;

	/*
	 * what we display depends on the style of memory being used.
	 */

	printf("\nHashed file header table entries:\n");
	if (_rlockShm.useOldStyle)
	{
		printf("%12s%12s\n", "entry", "file-hdr");
		oldLastPP = _rlockShm.oHfhdrTablePP + _rlockShm.hfhdrSize;
		for (oldEntryPP = _rlockShm.oHfhdrTablePP;
		     oldEntryPP < oldLastPP;
		     oldEntryPP++)
		{
			if (!showAll && nil(*oldEntryPP))
				continue;
			printf("%#12lx%#12lx\n", oldEntryPP, *oldEntryPP);
		}
	}
	else
	{
		printf("%6s%12s\n", "entry", "file-index");
		lastP = _rlockShm.hfhdrTableP + _rlockShm.hfhdrSize;
		for (entryP = _rlockShm.hfhdrTableP;
		     entryP < lastP;
		     entryP++)
		{
			if (!showAll && (*entryP == INVALID_INDEX))
				continue;
			printf("%6d%12ld\n",
				(int)(entryP - _rlockShm.hfhdrTableP),
				*entryP);
		}
	}
}

/*
 *	STATIC	dispLockTable() - display the record lock table
 *
 *	input:	showAll - display all data?
 *
 *	proc:	display the information kept in the record lock table.
 *
 *	output:	(void) - none
 *
 *	global:	_rlockShm - checked
 */

static void
dispLockTable(showAll)
bool showAll;
{	recLockT *entryP, *lastP;
	oldRecLock *oldEntryP, *oldLastP;

	/*
	 * what we display depends on the style of memory being used.
	 */

	printf("\nRecord locking table entries:\n");
	if (_rlockShm.useOldStyle)
	{
		printf("%12s%12s%12s%12s%12s%12s\n",
			"entry", "next-lock", "lock-low", "lock-high", 
			"sess-ID", "dos-PID");
		oldLastP = _rlockShm.oRlockTableP + _rlockShm.lockSize;
		for (oldEntryP = _rlockShm.oRlockTableP;
		     oldEntryP < oldLastP;
		     oldEntryP++)
		{
			if (!showAll && (oldEntryP->lockHi == 0L))
				continue;
			printf("%#12lx%#12lx%#12ld%#12ld%#12d%12d\n",
				oldEntryP, oldEntryP->nextLock,
				oldEntryP->lockLow, oldEntryP->lockHi,
				oldEntryP->sessID, oldEntryP->dosPID);
		}
	}
	else
	{
		printf("%6s%12s%12s%12s%12s%12s%12s\n",
			"entry", "next-index", "open-index", "lock-low",
			"lock-high", "sess-ID", "dos-PID");
		lastP = _rlockShm.rlockTableP + _rlockShm.lockSize;
		for (entryP = _rlockShm.rlockTableP;
		     entryP < lastP;
		     entryP++)
		{
			if (!showAll && (entryP->rlLockHi == 0L))
				continue;
			printf("%6d%12ld%12ld%12ld%12ld%12ld%12ld\n",
				(int)(entryP - _rlockShm.rlockTableP),
				entryP->rlNextIndex,
				entryP->rlOpenIndex,
				entryP->rlLockLow,
				entryP->rlLockHi,
				entryP->rlSessID,
				entryP->rlDosPID);
		}
	}
}

/*
 *	STATIC	dispOpenTable() - display the open file table
 *
 *	input:	showAll - display all data?
 *
 *	proc:	display the information kept in the open file table.
 *
 *	output:	(void) - none
 *
 *	global:	_rlockShm - checked
 */

static void
dispOpenTable(showAll)
bool showAll;
{	openFileT *entryP, *lastP;
	oldOpenFile *oldEntryP, *oldLastP;

	/*
	 * what we display depends on the style of memory being used.
	 */

	printf("\nOpen file table entries:\n");
	if (_rlockShm.useOldStyle)
	{
		printf("%12s%12s%12s%12s%12s%8s%8s\n",
			"entry", "next-open", "header", "sess-ID",
			"dos-PID", "acc", "deny");
		oldLastP = _rlockShm.oOpenTableP + _rlockShm.openSize;
		for (oldEntryP = _rlockShm.oOpenTableP;
		     oldEntryP < oldLastP;
		     oldEntryP++)
		{
			if (!showAll && nil(oldEntryP->header))
				continue;
			printf("%#12lx%#12lx%#12lx%12d%12d%#8x%#8x\n",
				oldEntryP,
				oldEntryP->nextOpen,
				oldEntryP->header,	
				oldEntryP->sessID,
				oldEntryP->dosPID,
				oldEntryP->accMode,
				oldEntryP->denyMode);
		}
	}
	else
	{
		printf("%6s%12s%12s%12s%12s%12s%6s%6s\n",
			"entry", "next-index", "fhdr-index", "sess-ID",
			"dos-PID", "file-desc", "acc", "deny");
		lastP = _rlockShm.openTableP + _rlockShm.openSize;
		for (entryP = _rlockShm.openTableP;
		     entryP < lastP;
		     entryP++)
		{
			if (!showAll &&
			    (entryP->ofFHdrIndex == INVALID_INDEX))
				continue;
			printf("%6d%12ld%12ld%12ld%12ld%12ld%#6x%#6x\n",
				(int)(entryP - _rlockShm.openTableP),
				entryP->ofNextIndex,
				entryP->ofFHdrIndex,
				entryP->ofSessID,
				entryP->ofDosPID,
				entryP->ofFileDesc,
				entryP->ofAccMode,
				entryP->ofDenyMode);
		}
	}
}


/* 
	For shared memory debugging, this allows "log()" statements in
	the rlock library to come out like printf's
*/

#ifdef SHM_DEBUG
#include <varargs.h>
/*
   log: Log message to standard output
*/

int
log(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt;			/* printf-style format string */
	register int	prfRet;		/* Return value from printf */

	va_start(args);
	fmt = va_arg(args, char *);
	prfRet = vfprintf(stdout, fmt, args);
	va_end(args);

	/* Flush output immediately */
	fflush(stdout);
}

#endif	/* SHM_DEBUG */
