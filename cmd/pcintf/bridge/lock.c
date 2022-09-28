/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/lock.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)lock.c	3.15	LCC);	/* Modified: 16:24:24 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/*
   (c) Copyright 1985 by Locus Computing Corporation.  ALL RIGHTS RESERVED.

   This material contains valuable proprietary products and trade secrets
   of Locus Computing Corporation, embodying substantial creative efforts
   and confidential information, ideas and expressions.	 No part of this
   material may be used, reproduced or transmitted in any form or by any
   means, electronic, mechanial, or otherwise, including photocopying or
   recording, or in connection with any information storage or retrieval
   system without permission in writing from Locus Computing Corporation.
*/

/*------------------------------------------------------------------------
 *
 *	lock.c - record locking and unlocking
 *
 *	routines included: coLockRec(), coUnlockRec(), validFid().
 *
 *	modification history: %MODHIST%
 *	
 *      Apr 1 1987  gregory  ver 1.0
 *      
 *      Initial coding followed starlan routines by same names.
 *      
 */

/*
 * All routines whose names begin with "co" are protocol service routines.
 * They all have identical parameter lists:
 *	coServiceRoutine(in, out, dosPID)
 *
 *	"in" is a pointer to a header structure for an incoming message
 *	which the routine is to service
 *
 *	"out" is a pointer to a header structure for the response, which the
 *	routine is to fill in
 *
 * The functions are "void". Any error status is recorded in the
 * "out" structure.
 *
 * Other functions are documented individually
 *-------------------------------------------------------------------------
 */

#ifdef RLOCK  	/* If record locking */
#include "pci_types.h"

#include <stdio.h>
#include <log.h>
#include <sys/stat.h>			/* stat(2) defs */
#include <errno.h>
#ifndef ULTRIX
#  include <mnttab.h>			/* mount table defs */
#  ifdef	SYS5_4
#    include <sys/fs/s5param.h>
#  endif
#  ifdef DGUX
#    include <sys/statfs.h>			/* superblock defs */
#  else
#    include <sys/filsys.h>			/* superblock defs */
#  endif	/* DGUX */
#endif	/* !ULTRIX */

#include <fcntl.h>			/* file operation defs */
#include <rlock.h>			/* record lock defs */

extern struct vFile *vfCache;		/* State table data on open files */

/*
 *	validFid: Check that the vfd supplied is in the current vfCache.
 *              Return the handle to the virtual file if found.
 *
 *	Input Parameters:
 *              Virtual file descriptor to be checked.
 *
 *	Tasks:	
 *		See above.
 *
 *	Outputs: 
 *		NIL if invalid, pointer to vf table entry if valid.
 */
struct vFile *
validFid(vfd, sessid)
int		vfd;
word		sessid;
{
	struct vFile *vfSlot;		/* Pointer to vfCache slot */
	extern unsigned vfCacheSize;

	if (vfd < 0 || vfd >= vfCacheSize)
		return (NIL_FIDINFO);

	vfSlot = &vfCache[vfd];
	if (vfSlot->flags && (vfSlot->sessID == sessid)) 
	{
		if((vfSlot->flags & VF_PRESET) != 0)
			log("validFid: I/O attempted on valid vfPreset file: vDesc %d\n", vfd);
		return vfSlot;
	}
	else
	{
		if((vfSlot->flags & VF_PRESET) != 0)
			log("validFid: I/O attempt FAILED on invalid vfPreset file: vDesc %d\n", vfd);
		return (NIL_FIDINFO);
	}
}



/*
 * Lock Record
 * place an IO lock on a string of bytes in a file
*/

int
coLockRec(vdescriptor, dospid, offset, length) 
int		vdescriptor;	/* PCI virtual file descriptor */
short		dospid;		/* Dos pid */
unsigned long	offset, length;		/* Position and size of lock */
{
word 		sessid;		/* svr pid */
struct vFile 	*fp;	 	/* pointer into vfCache */


        /* Verify the file */
	sessid = (word)(getpid());
	fp = validFid(vdescriptor, sessid);
	if (fp == NULL)
	{
		errno = EBADF;
		log("coLockrec: Invalid File ID\n");
       		return -1;
        }

	/* Don't lock tmp or spool files, or old-style fcb-openned files */
	if (fp->flags & FF_NOLOCK) {
		errno = EEXIST;
		log("coLockrec: Invalid - can't lock a tmp, spool, or fcb file\n");
       		return -1;
	}

	/* try to lock the bytes */
	if (addLock((int) fp->shareIndex, (long) dospid, offset, length) 
		== -1)
	{
		errno = rlockErr;   /* error from addLock */
		log("coLockrec: addLock() failed\n");
       		return -1;
	}

	fp->lockCount++;
	return SUCCESS;
}

/* Unlock Record */
/* remove an IO lock from a string of bytes in a file */

int
coUnlockRec (vdescriptor, dospid, offset, length) 
int		vdescriptor;	/* PCI virtual file descriptor */
short		dospid;		/* Dos pid */
unsigned long	offset, length;		/* Position and size of lock */
{
word 		sessid;		/* svr pid */
struct vFile 	*fp;	 	/* pointer into vfCache */
int		ret;		/* return from lock calls */
struct flock    *lock;		/* lock structure pointer */
struct flock    slock;		/* lock structure for fcntl() calls */

	lock = &slock;		/* set the pointer */
	
        /* Verify the file */
	sessid = (word)(getpid());
	fp = validFid(vdescriptor, sessid);
	if (fp == NULL)
	{
		errno = EBADF;
       		return -1;
        }

	/* Don't unlock tmp or spool files, or old-style fcb-openned files */
	if (fp->flags & FF_NOLOCK) {
		errno = EEXIST;
       		return -1;
	}

	/* The unix lock is in place... */
	/* try to unlock the bytes */
	if(rmvLock((int) fp->shareIndex, (long) dospid, offset, length)
		== -1)
	{
		errno = rlockErr;   	     /* error from rmvLock */
       		return -1;
	}

	fp->lockCount--;
	return SUCCESS;
}
#endif  /* RLOCK */
