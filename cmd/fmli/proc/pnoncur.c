/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:proc/pnoncur.c	1.4"

#include <stdio.h>
#include <sys/types.h>	/* EFT abs k16 */
#include "wish.h"
#include "terror.h"
#include "proc.h"
#include "procdefs.h"

extern struct proc_ref PR_all;

int
proc_noncurrent(p, all)
proc_id p;
bool all;
{
	/* suspend process */
#ifdef _DEBUG
	_debug(stderr, "proc_noncurrent not yet implemented\n");
#endif
}
