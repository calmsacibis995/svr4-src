/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)master:ramd/space.c	1.3.1.1"

/*
 * RAM disk configuration file
 */
#include "sys/types.h"
#include "sys/param.h"
#include "sys/buf.h"
#include "sys/uio.h"
#include "sys/cred.h"
#include "sys/ramd.h"
#include "config.h"
#include "sys/ddi.h"

#ifdef RAMD_BOOT
#if defined (MB1) || defined (MB2)

#define RAMD_BOOT_MAJOR		3	/* Major dev to load from */
#define RAMD_BOOT_MINOR		125	/* Minor dev to load from */
#define RAMD_TAPE_FS_FILE_MARK	4	/* File mark offset of load image */
#define RAMD_NUM	2		/* Number of ram disks */
#define RAMD_SIZE 0x0015E000L		/* Size of ramd root file system */
#define RAMD_SWAP 0x000B0000L		/* Size of ramd swap partition */
int ramd_load_type = RAMD_TAPE_LOAD;	/* Flag to use device ioctls */

#else /* AT386 */

#define RAMD_TAPE_FS_FILE_MARK	1
#define RAMD_BOOT_MAJOR		1
#define RAMD_BOOT_MINOR		132
#define RAMD_NUM	2
#define RAMD_SIZE	0x00128400L
#define RAMD_SWAP	0x00080000L
int ramd_load_type = RAMD_FLOPPY_LOAD;

#endif /* AT386 */

struct ramd_info ramd_info[RAMD_NUM] = {
	{	RAMD_SIZE,		/* Ram disk size */
		RAMD_LOAD,		/* Ram disk flags */
		RAMD_BOOT_MAJOR,	/* Major number to load from */
		RAMD_BOOT_MINOR, 	/* Minor number to load from */
		0,			/* State info (set at runtime) */
		0			/* Ram disk vaddr (set at runtime) */
	},
	{	RAMD_SWAP,		/* Ram disk size */
		RAMD_STATIC,		/* Ram disk flags */
		NODEV,			/* Major number to load from */
		NODEV,			/* Minor number to load from */
		0,			/* State info (set at runtime) */
		0			/* Ram disk vaddr (set at runtime) */
	}
};
#else /* RAMD_BOOT */
#define RAMD_TAPE_FS_FILE_MARK	0
#define RAMD_BOOT_MAJOR		-1
#define RAMD_BOOT_MINOR		-1
#define RAMD_NUM	1
#define RAMD_SIZE	0x0
#define RAMD_SWAP	0x0
int ramd_load_type = RAMD_NO_LOAD;

struct ramd_info ramd_info[RAMD_NUM] = {
	{	-1,		/* Ram disk size */
		RAMD_RUNTIME,	/* Ram flags */
		NODEV,		/* Major number to load from */
		NODEV,		/* Minor number to load from */
		0,		/* State info (set at runtime) */
		0		/* Ram disk vaddr (set at runtime) */
	}
};
#endif /* RAMD_BOOT */

struct  ramd_info	ramd_info[RAMD_NUM];
struct  buf		ramd_buf[RAMD_NUM];
minor_t  ramd_num   = 	RAMD_NUM ;
int  ramd_tape_loc = 	RAMD_TAPE_FS_FILE_MARK;

