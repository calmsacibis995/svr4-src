/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/sys/lock.h	1.1"
/*ident	"@(#)cfront:incl/sys/lock.h	1.4"*/
/*
 * flags for locking procs and texts
 */

#ifndef LOCK_H
#define LOCK_H

#define	UNLOCK	 0
#define	PROCLOCK 1
#define	TXTLOCK	 2
#define	DATLOCK	 4

extern int plock (int);

#endif

