/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/i354/space.c	1.3.2.1"

#include "sys/types.h"
#include "sys/tty.h"
#include "sys/cred.h"
#include "sys/ddi.h"
#include "sys/i354.h"
#include "config.h"

/*
 *	There are two io channels per 354 and you must configure them
 *	with adjacent entries in i354cfg[].
*/
struct i354cfg i354cfg[2*I354_UNITS] = {
	 /* ControlPort, DataPort */
	{ I354_0_SIOA+4, I354_0_SIOA+6 },
	{ I354_0_SIOA+0, I354_0_SIOA+2 },
#ifdef I354_1_SIOA
	{ I354_1_SIOA+4, I354_1_SIOA+6 },
	{ I354_1_SIOA+0, I354_1_SIOA+2 },
#endif
};

int i354_cnt = 2*I354_UNITS;			/* # of ports (must be multiple of 2) */
