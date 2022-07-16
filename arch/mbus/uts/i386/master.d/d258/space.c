/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/master.d/d258/space.c	1.3.1.1"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/dma.h"
#include "sys/i82258.h"
#include "config.h"

unsigned long d258_base = D258_0_SIOA;

/* 16 bit I/O, 16 bit MEM bus width, Local mode, ch. 3 normal,
 * 2 cycle, fixed priority, interrupts disabled, common int. disabled
#define D258_GMR_DEFAULT 0x7C03	
*/
#define D258_GMR_DEFAULT 0x7F03	/*	changed to rot. pri 1 cyc */	

unsigned long d258_gmr = D258_GMR_DEFAULT;

unsigned char d258_gbr = 0;		/* maximum # of contiguous bus cycles*/
unsigned char d258_gdr = 0;		/* minimum # of clocks between bursts*/

unsigned long d258_chan0_base = 0;	/* For chan0 transfers. */
unsigned long d258_chan1_base = 0;	/* For chan1 transfers. */
unsigned long d258_chan2_base = IMPC_0_SIOA;	/* For chan2 transfers. */
unsigned long d258_chan3_base = IMPC_0_SIOA;	/* For chan3 transfers. */
