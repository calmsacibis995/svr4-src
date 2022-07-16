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

#ident	"@(#)mbus:uts/i386/master.d/mpc/space.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/mpc.h"
#include "sys/cred.h"
#include "sys/ddi.h"
#include "config.h"

/* MPC ADMA programming parameters */
unsigned long impc_base = IMPC_0_SIOA;

/*
 * Controls the fail-safe counter in the mpc for solicited transfers.
 * To enable, set this value to 1. To disable, set this value to 0.
 *
 */
int impc_fs_enabled = 1;

/*
 * Controls the retries for a NACK on sending messages. To disable set to 0.
 * Maximum value not to exceed 4 for performance considerations
 */

int impc_retries = 4;
