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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/common.h	1.3"

		/* General Constants */
		
#define		TRUE				1
#define		FALSE				0 

#define		CCI_PORTID			0x506

#define		E_OK				0
#define		E_ERROR				-1
#define		WAIT_FOREVER		0xFFFF

#define		BASE_TEN			10
#define 	DEF_PORT_ID			0x1ff

#ifndef MSGFILE
extern void on_intr();
#endif
