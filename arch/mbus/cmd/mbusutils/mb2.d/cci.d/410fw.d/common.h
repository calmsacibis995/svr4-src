/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/410fw.d/common.h	1.3"

#define 	MAX_WAIT 			16
	
	/* Interconnect Space Constants */

#define 	MY_SLOT 			0x1f
#define 	PSB_CONTROL_REC 	0x06
#define 	IC_FIRST_REC_OFFSET	32
#define 	IC_EOT				0xff
#define 	IC_LEN_OFFSET		1
#define 	IC_LOCK_OFFSET		17
#define 	IC_FW_OFFSET		2
#define 	IC_FW_REC			0xf
#define 	INIT_DONE			0x81
#define 	INIT_NOT_DONE		0x80
#define 	VENDOR_ID_LO		0
#define 	VENDOR_ID_HI		1

