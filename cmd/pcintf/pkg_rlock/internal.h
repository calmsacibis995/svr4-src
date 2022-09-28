/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_rlock/internal.h	1.1"
SCCSID_H("@(#)internal.h	1.6	17:02:53	2/12/90", internal_h)

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

/*-------------------------------------------------------------------------
 *
 *	internal.h - internal definitions for the RLOCK package
 *
 *-------------------------------------------------------------------------
 */

/*
 * standard constants.
 */

#define SUCCESS		0			/* no error return code */
#define FAIL		(-1)			/* error return code */
#define NIL		((char FAR *) 0)	/* NIL pointer */
#define DONT_CARE	0			/* integer place-holder */

/*
 * useful macros:
 *
 *	ABS(x)		- return absolute value of x (needed since not all
 *			  systems have an abs() function/macro)
 *	ANYSET(x,y)	- are any of the bits set in y, also set in x?
 */

#define ABS(x)		((x) >= 0 ? (x) : -(x))
#define ANYSET(x,y)	(((x) & (y)) != 0)

/*
 * to prevent absolute addressing of the shared memory, indices are used
 * in place of pointers.  the following are definitions for the index type,
 * and for an invalid index (used like a NIL pointer).  note that the index
 * is returned to the user by some functions, as an 'int' type.  therefore,
 * the indexT type should never be larger than an 'int'.
 */

typedef short indexT;

#define INVALID_INDEX		((indexT) -1)

#ifdef DGUX	/* DG Aviion requires longword alignment of ints */
#define PAD	unsigned long LongWord_Align
#else
#define PAD
#endif	/* DGUX */

/*
 * the following primitive allows a pointer to be checked for NIL, without the
 * caller having to worry about casts.
 */

#define nil(pointer)	((char FAR *)(pointer) == NIL)

/*
 * give C some resemblence of a logical structure. CAVEAT!! don't check a
 * bool value explicitly for TRUE (i.e., (val == TRUE)), since a TRUE
 * value may actually be any non-0 value.  while we're at it, some systems
 * define TRUE and FALSE in the most obscene places, so make sure we have
 * our own definition.
 */

typedef int bool;
#if defined(ULTRIX) || defined(XENIX)
#	if !defined(vax) && !defined(XENIX)
	typedef unsigned short ushort;
#	endif /* !vax && !XENIX */
typedef unsigned long ulong;
typedef unsigned char uchar;
#endif	/* ULTRIX || XENIX */

#undef	TRUE
#undef	FALSE

#define TRUE	((bool) 1)
#define FALSE	((bool) 0)

/*
 * these are the standard permission bits, used for both files and IPC
 * objects.  note that some are defined here only for completeness; not all
 * modes are used.
 */

#define UP_SUID		04000		/* setUID bit */
#define UP_SGID		02000		/* setGID bit */
#define UP_TEXT		01000		/* save text bit */
#define UP_OR		00400		/* owner read permission */
#define UP_OW		00200		/* owner write permission */
#define UP_OX		00100		/* owner execute */
#define UP_ORW		00600		/* owner read & write */
#define UP_ORWX		00700		/* owner read, write & execute */
#define UP_GR		00040		/* group read */
#define UP_GW		00020		/* group write */
#define UP_GX		00010		/* group execute */
#define UP_GRW		00060		/* group read & write */
#define UP_GRWX		00070		/* group read, write & execute */
#define UP_WR		00004		/* world read */
#define UP_WW		00002		/* world write */
#define UP_WX		00001		/* world execute */
#define UP_WRW		00006		/* world read & write */
#define UP_WRWX		00007		/* world read, write & execute */

#define UP_OSHIFT	6		/* bit offset of owner permissions */
#define UP_GSHIFT	3		/* offset to group permissions */
#define UP_WSHIFT	0		/* world permissions */

/*
 * configuration is handled by this structure.
 *
 * WARNING: this structure is initialized statically.  thus, any changes
 * to this structure must be reflected in the initialization.
 */

typedef struct {
	char FAR	*cfgBaseP;		/* base of the shared memory */
	key_t		cfgShmKey;		/* shared memory key */
	ulong		cfgLsetKey;		/* lockset key */
	short		cfgRecLocks;		/* hashed record locks */
	short		cfgOpenTable;		/* open table size */
	short		cfgFileTable;		/* file hdr table size */
	short		cfgHashTable;		/* hash table size */
	short		cfgLockTable;		/* lock table size */
} cfgDataT;

/*
 * this structure represents an open file in the shared memory.  an unused
 * entry is indicated by a value of 0 in ofAccMode.  the ofNextIndex is the
 * index of the next open in the list of opens on a particular file.  the
 * first entry is indicated by the fhOpenIndex field in the fileHdrT
 * structure.  the ofFHdrIndex is a back-reference index to the file header
 * for this open.  thus the file header at the start of the chain of which
 * this open is a part, is referenced by ofFHdrIndex.
 */

typedef struct {
	indexT	ofNextIndex;	/* another open of the same file */
	indexT	ofFHdrIndex;	/* index of the appropriate file header */
	long	ofSessID;	/* ID of session containing open */
	long	ofDosPID;	/* process ID of opening process */
	long	ofFileDesc;	/* host's open file descriptor */
	char	ofAccMode;	/* access modes */
	char	ofDenyMode;	/* deny modes */
	PAD			/* structure padding */
} openFileT;

/*
 * flags for openFileT.ofAccMode and openFileT.ofDenyMode, respectively.
 */

#define OFA_READ	0x0001		/* read allowed */
#define OFA_WRITE	0x0002		/* write allowed */

#define OFD_READ	0x0001		/* deny read */
#define OFD_WRITE	0x0002		/* deny write */
#define OFD_DOS_COMPAT	0x0004		/* DOS compatibility (deny lots) */

/*
 * each file is represented in the shared memory segment with a unique ID.
 * this structure defines the format of that unique ID.  a value of 0 for the
 * fsysNumb signifies an unused entry.  however, the macros that follow should
 * be used for most access to this structure.
 */

typedef struct {
	ulong	fsysNumb;	/* file system (device, etc.) number */
	ulong	fileNumb;	/* file number */
} uniqueT;

/*
 * the following macros should be used for all access to the unique ID, except
 * where the unique ID is actually set for a specific file, or where the
 * ID is 'hashed'.  these tasks are a little too complex for macros, and ard
 * handled by actual functions.
 *
 *	CLEAR_UNIQUE_ID(x)	- 'clear' the specified unique ID
 *	UNIQUE_ID_USED(x)	- is the unique ID 'in use'?
 *	MATCH_UNIQUE_ID(x, y)	- do the unique IDs match?
 *	UNIQ_INT_HASH(u, lim)	- hash unique ID u into a signed integer (int)
 *				  value, between 0 and ABS(lim) - 1.  lim is
 *				  presumed to fit in a signed integer.
 *
 * in each case, the parameters are uniqueT structures, not pointers.  note
 * also that the fileNumb fields are checked first, since it is presumed that
 * these numbers will have a greater probability of difference than the
 * fsysNumb fields, so only the first of the equivallency tests will normally
 * be necessary for different files.
 */

#define CLEAR_UNIQUE_ID(x)	(x.fsysNumb = 0L)
#define UNIQUE_ID_USED(x)	(x.fsysNumb != 0L)
#define MATCH_UNIQUE_ID(x, y)	\
		((x.fileNumb == y.fileNumb) && (x.fsysNumb == y.fsysNumb))
#define UNIQ_INT_HASH(u, lim)	\
		(ABS((int)u.fsysNumb + (int)u.fileNumb) % ABS((int)(lim)))

/*
 * function as a handle to the open file chain and the record lock list for each
 * distinct open file.  an unused entry is indicated by a 'cleared' fhUniqueID
 * structure field.  the fhOpenIndex and fhLockIndex are indices to associated
 * open file and record lock structure lists.  the fhHashIndex field is the
 * index of the next file header whose unique ID 'hashed' to the same value
 * as the one being looked at.
 */

typedef struct {
	indexT	fhOpenIndex;	/* head of openFileT structure list */
	indexT	fhLockIndex;	/* head of recLockT structure list */
	indexT	fhHashIndex;	/* next file that hashed in this list */
	uniqueT	fhUniqueID;	/* unique ID of Unix file */
	PAD			/* structure padding */
} fileHdrT;

/*
 * structure representing a single record lock.  an unused entry is
 * represented by a value of 0 in the rlLockHi field.  the rlNextIndex field
 * is a pointer to the next lock on this particular file.  the list of locks
 * is kept in non-decreasing order.
 */

typedef struct {
	indexT	rlNextIndex;	/* next higher lock on the file */
	indexT	rlOpenIndex;	/* the open that created this lock */
	long	rlLockLow;	/* first byte locked */
	long	rlLockHi;	/* last byte locked */
	long	rlSessID;	/* ID of session applying lock */
	long	rlDosPID;	/* ID of DOS proc that applied lock */
} recLockT;

/*
 * these are the equivalent structures as above, but are for the old, multi
 * segment style, which used pointers.  this made them dependent on their
 * attachment addresses.
 */

typedef struct oldOpenFile {
	short			sessID;		/* ID of session containing open */
	short			dosPID;		/* Process ID of opening process */
	char			accMode;	/* Access modes */
	char			denyMode;	/* Deny modes */
	struct oldFileHdr FAR	*header;	/* Per file header */
	struct oldOpenFile FAR	*nextOpen;	/* Another open of the same file */
} oldOpenFile;

typedef struct oldFileHdr {
	struct oldOpenFile FAR	*openList;	/* List of all opens of file */
	struct oldRecLock FAR	*lockList;	/* List of record locks on file */
	long			uniqueID;	/* Unique ID of UNIX file */
	struct oldFileHdr FAR	*hashLink;	/* List of files that hash together */
} oldFileHdr;

typedef struct oldRecLock {
	long			lockLow;	/* First byte locked */
	long			lockHi;		/* Last byte locked */
	short			sessID;		/* ID of session applying lock */
	short			dosPID;		/* ID of DOS proc that applied lock */
	struct oldRecLock FAR	*nextLock;	/* Next higher lock on file */
} oldRecLock;

/*
 * this structure is the library visible information necessary to access the
 * shared memory data.  it will need to be updated if new tables are added to
 * the library (see rlock_init.c).
 *
 * in some of the following comments, the abbreviation, 'PTI' is used.  it
 * stands for 'pointer to index', and indicates that the value is a pointer
 * to a table index.  the pointer references into the shared memory so that
 * all processes know it.  in addition, the table pointers are actually used
 * as arrays in most of the code, for ease of readability.
 *
 * there are some fields that are not meant to be used in normal operation;
 * that is, the shared memory segment pointers.  they are used only for the
 * display of package operation.
 *
 * except for the old/new boolean and the table sizes, most of the fields are
 * duplicated for both new and old styles.  the exceptions are the segment
 * pointers (1 vs. 3), and for the segment size.  the latter can be figured
 * once for each of the mulitple segments, and will never change.  thus, it
 * isn't very useful.  for the single segment, which is run-time configurable,
 * it is a quick indication of size, without figuring all of the sub-totals,
 * and adding them up.
 */

typedef struct {
	/* new style memory pointers */
	openFileT FAR	*openTableP;		/* open file table */
	fileHdrT FAR	*fhdrTableP;		/* file header table */
	indexT		*hfhdrTableP;		/* hash table */
	recLockT FAR	*rlockTableP;		/* record lock table */
	indexT		*openFreeIndexP;	/* open file PTI */
	indexT		*fhdrFreeIndexP;	/* file header PTI */
	indexT		*lockFreeIndexP;	/* record lock PTI */
	char FAR	*segmentP;		/* shared memory base */
	ulong		segSize;		/* total segment size */

	/* old style memory pointers */
	oldOpenFile FAR	*oOpenTableP;		/* open file table */
	oldFileHdr FAR	*oFhdrTableP;		/* file header table */
	oldFileHdr FAR	**oHfhdrTablePP;	/* hash table */
	oldRecLock FAR	*oRlockTableP;		/* record lock table */
	oldOpenFile FAR	**oOpenFreePP;		/* open file free list */
	oldFileHdr FAR	**oFhdrFreePP;		/* file header free list */
	oldRecLock FAR	**oLockFreePP;		/* record lock free list */
	char FAR	*oOpenSegmentP;		/* open file segment base */
	char FAR	*oFhdrSegmentP;		/* file header segment base */
	char FAR	*oLockSegmentP;		/* record lock segment base */

	/* table sizes */
	short		openSize;		/* # of open files/host */
	short		fhdrSize;		/* # of file headers/host */
	short		hfhdrSize;		/* # of file hdr hash/host */
	short		lockSize;		/* # of locks/host */

	/* misc. data */
	short		recLocks;		/* record 'lock' locks */
	bool		useOldStyle;		/* use the old style? */
} rlockShmT;	

/*
 * pre-cast NIL pointers to share structures
 */

#define NIL_CFGDATA	((cfgDataT FAR *) 0)
#define NIL_OPENFILE	((openFileT FAR *) 0)
#define NIL_FILEHDR	((fileHdrT FAR *) 0)
#define NIL_RECLOCK	((recLockT FAR *) 0)
#define NIL_O_OPENFILE	((struct oldOpenFile FAR *) 0)
#define NIL_O_FILEHDR	((struct oldFileHdr FAR *) 0)
#define NIL_O_RECLOCK	((struct oldRecLock FAR *) 0)
#define NIL_RLOCKSHM	((rlockShmT FAR *) 0)

/*
 * lock numbers for shared memory.  due to the way they are handled, these
 * values must be signed integer (int) values.
 */

#define ALL_LOCK	0		/* special case - all locks */

#define OF_LOCK		1		/* open file & file header tables */
#define LT_LOCK		2		/* record lock table */
#define RL_LOCK0	3		/* first record lock lock */

#define NLOCKS		RL_LOCK0	/* number of required locks */

/*
 * prioritized register storage class modifiers.  some of these may be
 * unconfirmed rumors.  they should be checked.
 */

#if	defined(u3b) || defined(u3b2) || defined(u3b5) || defined(vax) || defined(mips)
#define r0	register
#define r1	register
#define r2	register
#define r3	register
#define r4	register
#define r5	register
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(u3b), et. al. */

#if	defined(m68k)
#define r0	register
#define r1	register
#define r2	register
#define r3	register
#define r4	register
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(m68k) */

#if	defined(u370)
#define r0	register
#define r1	register
#define r2	register
#define r3	register
#define r4
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(u370) */

#if	defined(i386)
#define r0	register
#define r1	register
#define r2	register
#define r3
#define r4
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(i386) */

#if	defined(iAPX286) || defined(M_I86)
#define r0	register
#define r1	register
#define r2
#define r3
#define r4
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* defined(iAPX286) */

#if	!defined(r0)
#define	r0
#define r1
#define r2
#define r3
#define r4
#define r5
#define r6
#define r7
#define r8
#define r9
#endif	/* !defined(r0) */

/*
 * variables known throughout the share table routines.  these data shouldn't
 * be accessed outside of the library.
 */

extern int		_rlNHashLocks;		/* number of locks in set */
extern rlockShmT	_rlockShm;		/* shared memory data */
extern cfgDataT		_rlCfgData;		/* configuration data */

/*
 * functions used by the share table library routines.
 */

/* shm_open.c */
extern openFileT FAR	*_rlGetOFile();	/* validate an open file slot */

/* rlock_prim.c */
extern int		_rlInitLocks();	/* init the locking data */
extern bool		_rlSetLocks();	/* set two sync locks */
extern void		_rlUnlock();	/* remove current lock(s) */

/* multi_init.c (archaic, need for this should go away) */
extern int		_rlMCreate();	/* create multi-segments */
extern int		_rlMAttach();	/* attach to multi-segments */
extern int		_rlMRemove();	/* remove multi-segments */

/* multi_fshm.c (archaic, need for this should go away) */
extern int		_rlAddOpen();	/* add an open to the table */
extern int		_rlIsOpen();	/* is a file open? (0 = no) */
extern int		_rlRmvOpen();	/* 'remove' an open from the table */
extern oldOpenFile FAR	*_rlOGetOFile();/* get a used open entry */

/* multi_lshm.c (archaic, need for this should go away) */
extern int		_rlAddLock();	/* add a lock to a files lock list */
extern int		_rlRmvLock();	/* remove a lock */
extern int		_rlIOStart();	/* exclude record lockers for I/O */
extern void		_rlIODone();	/* release I/O record lock exclusion */
extern int		_rlRstLocks();	/* reset locks */
