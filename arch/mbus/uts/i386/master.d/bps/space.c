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

#ident	"@(#)mbus:uts/i386/master.d/bps/space.c	1.1"

/*
 * Configuration file for the BPS driver 
 */

#include "sys/types.h"
#include "sys/bps.h"

/* 
 * Use the BPS left in RAM by the f/w BPS manager.  This is the 
 * default feature.  The BPS contains a merge of parameters for this 
 * host from ROM, operator intervention, and configuration file.
 */
int		bps_use_native	= 1;	
/*
 * In the first release of MSA firmware, the BPS manager splits the 
 * value of the parameter into tokens. This flag enables the BPS driver 
 * to parse through the value of the parameter.
 */ 
char	bps_tokenized_value	= 1;
/*
 * The testing flag when set to 1, allows an application to test the 
 * BPSINIT function, that is, ability to specify a BPS.  This feature 
 * is not supported for the product and should be used for testing only.
 *
 * NOTE: the bps_testing flag in conjunction with  bps_ram_addr is used 
 * 		 to provide support for systems which do not have the new second 
 * 		 stage bootstrap loader.
 */ 
int		bps_testing = 1;
/*
 * The f/w BPS manager maintains it's data structures at this physical 
 * memory address.  The new second stage bootstrap loader will 
 * copy the start address of the BPS manager data segment in the 
 * bootinfo structure for the BPS driver to use.  This feature insulates
 * the BPS driver from changes in firmware.
 */
int		bps_ram_addr = 0x6000;	

int		bps_max_size = 0x1000;	

