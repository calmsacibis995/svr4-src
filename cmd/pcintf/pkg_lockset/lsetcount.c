/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:pkg_lockset/lsetcount.c	1.1"
#include <sccs.h>

SCCSID("@(#)lsetcount.c	1.1	13:09:21	8/31/89");

/*
   (c) Copyright 1985 by Locus Computing Corporation.  ALL RIGHTS RESERVED.

   This material contains valuable proprietary products and trade secrets
   of Locus Computing Corporation, embodying substantial creative efforts
   and confidential information, ideas and expressions.  No part of this
   material may be used, reproduced or transmitted in any form or by any
   means, electronic, mechanical, or otherwise, including photocopying or
   recording, or in connection with any information storage or retrieval
   system without permission in writing from Locus Computing Corporation.
*/


/*
   lsetcount.c: Implementation of lock set counting primitive

   Exported functions:
	int		lsCount();
*/


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include <lockset.h>

#include <internal.h>

/* 
   lsCount: Returns number of locks in an open lock set

   Input Parameters:
	lockSet:		The lockset ID

   Tasks:
	Return count field of lock set structure

   Outputs:
	Return Value: Number of locks in set
*/

int
lsCount(lockSet)
lockSetT lockSet;
{	lockDataT FAR *lockDataP;

	lockDataP = (lockDataT FAR *)lockSet;
	return lockDataP->nLocks;
}
