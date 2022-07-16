/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)master:nfa/stubs.c	1.3"

/*
static char nfastubs_version[] = "@(#) $File: stubs.c $ $Version: 1.5 $ $Date: 89/11/30 19:35:23 $ $State: unit-tested $";
*/

/* 
 * this is the stubs file for OpenNet.
 * this file is used when OpenNet is not in the system.
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/pfdat.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/cmn_err.h"
#include "sys/errno.h"

nfc_nameibeg()
{
	return (0);
}

nfc_forkpar(nfc_forkpar_error)
int *nfc_forkpar_error;
{
}

nfc_forkch(nfc_forkch_error)
int *nfc_forkch_error;
{
}

nfc_exit()
{
}

nfa_sys()
{
	return(ENOPKG);
}

nfc_ustat()
{
	return(0);
}

