/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_lockset/lsetclr.c	1.1"
#include <sccs.h>

SCCSID("@(#)lsetclr.c	1.1	13:08:35	8/31/89");

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
   lsetclr.c: Implementation of inter-process lock clearing

   Exported functions:
	int		lsClr();
*/


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include <lockset.h>

#include <internal.h>

/* 
   lsClr: Remove locks previously applied to a lock set

   Input Parameters:
	lockSetT lockSet:	Lockset ID

   Tasks:
	(description of what's done)

   Outputs:
	Return Value: SUCCESS or FAIL
	rLockErr: Holds error code if return value == FAIL
*/

int
lsClr(lockSet)
lockSetT lockSet;
{	r0 lockDataT FAR *lockDataP;
	r1 struct sembuf FAR *opScan;
	r2 int nSet;

	/*
	 * get the internal format of the lockset data, return an error if
	 * no locks are set.
	 */

	lockDataP = (lockDataT FAR *)lockSet;
	if ((nSet = lockDataP->nSet) == 0) {
		lsetErr = LSERR_NOLOCK;
		return FAIL;
	}

	/* Translate list of lock setting semaphore ops to their inverses */
	opScan = lockDataP->semOps;
	for (nSet = lockDataP->nSet; nSet-- > 0; opScan++)
		/* Negate semaphore op code to invert locking action */
		opScan->sem_op = -opScan->sem_op;

	/* Remember and clear count of locks set */
	nSet = lockDataP->nSet;
	lockDataP->nSet = 0;

	/* Release locks by incrementing semaphores */
	if (semop(lockDataP->semDesc, lockDataP->semOps, nSet) != SUCCESS) {
		lsetErr = LSERR_SYSTEM;
		return FAIL;
	}

	/* Success */
	return SUCCESS;
}
