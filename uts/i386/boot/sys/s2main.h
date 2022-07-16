/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)boot:boot/sys/s2main.h	1.1.2.1"

/* String constants */

#define MAX_BPS_IMAGE		6144	/* 6k of BPS image */
#define	MAX_PARAM_VALUE 	80	/* 80 characters maximum */
#define	MAX_BDEVICE_VALUE 	16	/* 16 characters maximum */
#define MAX_DEV_GRAN		4096	/* 4k bytes/sector */
#define	FALSE			0
#define	TRUE			1
#define	ONE_BLOCK		1L

#define	CF_GET_SELECTED_HOST	0x0000L
#define	CF_SEQUENTIAL		0x0000L
#define	CF_OLD_FILE		0x0000L
#define	CF_SEEKED		0x0001L
#define	CF_NEW_FILE		0x0002L
#define	CF_GET_NEXT_HOST	0x0004L

#define	BP_CONFIG		1	/* code for config file BPS */
#define	BP_RUNTIME		3	/* code for runtime generated BPS */
