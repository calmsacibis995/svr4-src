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

#ident	"@(#)boot:boot/sys/error.h	1.1.2.1"

/* Error Codes */

#define STAGE2				2

#define E_OK				0x00L
#define	E_DEVICE_BUSY			0x01L	
#define	E_DEVICE_NOT_READY		0x02L
#define	E_NO_SERVER_FOUND		0x03L 
#define	E_SERVER_NOT_RESPONDING		0x04L
#define	E_TIMEOUT			0x05L

#define	E_ACCESS			0x11L
#define	E_BAD_2ND_STAGE			0x12L
#define	E_CONFIG_SYNTAX			0x13L
#define	E_CONFIG_NO_HOST_ID		0x14L
#define	E_CONTINUE			0x15L
#define	E_DESCRIPTOR_CONFLICT		0x16L
#define	E_DEVICE_GRANULARITY		0x17L
#define	E_DEVICE_IGNORED		0x18L
#define	E_DEVICE_NOT_EXIST		0x19L
#define	E_DEVICE_TYPE			0x1aL
#define	E_DIB_TYPE			0x1bL
#define	E_END_OF_DATA			0x1cL
#define	E_END_OF_MEDIUM			0x1dL
#define	E_EOF				0x1eL
#define	E_FILE_MARK			0x1fL

#define	E_FILE_SYSTEM			0x20L
#define	E_FIRM_REC_NOT_FOUND		0x21L
#define	E_FNEXIST			0x22L
#define	E_HW_ERROR			0x23L
#define	E_IO				0x24L
#define	E_IO_ERROR			0x25L
#define	E_NO_DEVICE_EXIST		0x26L
#define	E_OMF_TYPE			0x27L
#define	E_OMF_OVERWRITE			0x28L
#define	E_OMF_CHECKSUM			0x29L
#define	E_OVERFLOW			0x2aL
#define	E_PARAMETER_NOT_FOUND		0x2bL
#define	E_PRECEDENCE			0x2cL
#define	E_RETURN_FROM_2ND_STAGE		0x2dL
#define	E_SUBSTITUTION_FAILED		0x2eL
#define	E_SUBSTITUTION_LIMIT		0x2fL

#define	E_SUBSTITUTION_LOOP		0x30L
#define	E_SYNTAX			0x31L
#define	E_UNIT_ATTENTION		0x32L

#define	E_TRANSMISSION			0x34L
#define	E_BUFFER_SIZE			0x35L
#define	E_CANCELLED			0x36L
#define	E_Q1_MASTER			0x37L

#define	E_ALT_FORMAT			0x40L
#define	E_VTOC_FORMAT			0x41L
#define	E_BAD_MAGIC_NUM			0x42L
#define	E_MEM_OVERLAP			0x43L
#define	E_SCAN				0x44L
#define	E_NO_TSS			0x45L

#define	E_UNKNOWN_EXCEPTION		0xffL
