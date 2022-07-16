/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/iasy/space.c	1.3"

/*
 *	Reserve storage for Generic serial driver
*/
#include "sys/types.h"
#include "sys/stream.h"
#include "sys/termio.h"
#include "sys/strtty.h"
#include "sys/cred.h"
#include "sys/ddi.h"
#include "sys/iasy.h"

#define NSERIAL	256
int	iasy_num = NSERIAL;

struct	strtty iasy_tty[NSERIAL];	/* tty structs for each device */
struct iasy_hw iasy_hw[NSERIAL];	/* Hardware support routines */
