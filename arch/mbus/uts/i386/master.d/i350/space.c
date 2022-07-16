/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license agreement
 *	or nondisclosure agreement with Intel Corporation and may not be
 *	copied nor disclosed except in accordance with the terms of that
 *	agreement.
 *
 *	Copyright 1989  Intel Corporation
*/

#ident	"@(#)mbus:uts/i386/master.d/i350/space.c	1.1.1.1"

static char	prog_copyright[] = "Copyright 1989 Intel Corp. xxxxxx";
static char	prog_version[] = "@@(#) $File: space.c $ $Version: 1.3 $ $Date: 90/10/12 09:48:49 $ $State: unit-tested $";

/*
 * space.c
 *	Line-printer specific configuration.
*/
#include <sys/types.h>
#include <sys/cred.h>
#include <sys/ddi.h>
#include <sys/lp_i350.h>
#include "config.h"

/*
 * This table gives the interrupt level and  I/O addresses
 * for the line-printer. 
 */

struct	lp_cfg lp_i350_cfg = {
/*   Level, 	 port A, 	  port B, 	 	 port C, 		Control */
	I350_0_VECT, I350_0_SIOA, I350_0_SIOA+2, I350_0_SIOA+4, I350_0_SIOA+6
};
