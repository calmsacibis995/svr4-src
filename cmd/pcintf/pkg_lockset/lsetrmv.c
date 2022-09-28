/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_lockset/lsetrmv.c	1.1"
#include <sccs.h>

SCCSID("@(#)lsetrmv.c	1.1	13:09:41	8/31/89");

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
   lsetuse.c: Implementation of lock set removal function.

   Exported functions:
	lockSet		*lsRmv();
*/


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include <lockset.h>

#include <internal.h>

/* 
   lsRmv: Remove a lock set from the system

   Input Parameters:
	ulong lockSetKey;	Key identifier for the lockset to remove.

   Tasks:
	"Open" underlying lock structure (file or semaphore set)
	Remove it

   Outputs:
	Return Value: SUCCESS/FAIL
	lsetErr: Holds error code if return value is FAIL
*/

int
lsRmv(lockSetKey)
ulong			lockSetKey;
{
int			semDesc;	/* Semaphore set descriptor */

	/* Get a semaphore descriptor for the semaphore set */
	if ((semDesc = semget((key_t)lockSetKey, 0, UP_ORW)) == FAIL) {
		lsetErr = LSERR_SYSTEM;
		return FAIL;
	}

	/* Remove semaphore set from system */
	if (semctl(semDesc, 0, IPC_RMID, (struct semid_ds FAR *) 0) != FAIL)
		return SUCCESS;
	lsetErr = LSERR_SYSTEM;
	return FAIL;
}
