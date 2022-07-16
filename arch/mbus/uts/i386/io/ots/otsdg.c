/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/ots/otsdg.c	1.3"

#ifndef lint
static char otsdg_copyright[] = "Copyright 1988, 1989 Intel Corp. 464224";
#endif

/*
** ABSTRACT:	Open and close routines for the datagram side of OTS
**
** MODIFICATIONS:
*/

#include "sys/ots.h"
#include <sys/immu.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/debug.h>

int otsdgdevflag = 0;		/* V4.0 style driver */

/*
 * The following are standard STREAMS data structures.
 */
extern struct qinit otswinit, otsrinit;

int otsdgopen(), otsdgclose();
struct streamtab otsdginfo = {&otsrinit, &otswinit, NULL, NULL};

extern int ots_debug;

/* FUNCTION:		otsdgopen()
 *
 * ABSTRACT:	Required for datagram side of OTS driver
 *
 *	Stub - never called.
 */
int
otsdgopen()
{
	return(OPENFAIL);
}

/* FUNCTION:		otsdgclose()
 *
 * ABSTRACT:	Required for datagram side of OTS driver
 *
 *	Stub - never called.
 */
otsdgclose()
{
}
