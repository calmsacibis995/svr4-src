/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/evc.h	1.1.2.1"
/*	Copyright (c) 1989 Intel Corp.		*/
/*	  All Rights Reserved  	*/
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license
 *	agreement or nondisclosure agreement with Intel Corpo-
 *	ration and may not be copied or disclosed except in
 *	accordance with the terms of that agreement.
 */

/* evc.h - board specific defines for Olivetti EVC-1 */

/* 
 * Copyright 1989 Ing. C. Olivetti & C. S.p.A.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Olivetti not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  Olivetti makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * OLIVETTI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL OLIVETTI BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* 
 *
 * (C) Copyright 1989 Olivetti Advanced Technology Center, Inc.
 * All rights reserved.
 */

/*
 * Register offsets to be used for accessing the video board.
 */

#define	OfsEVC1BoardEnableReg	0x0C84
#define	OfsEVC1BoardIdReg0	0x0C80
#define	OfsEVC1BoardIdReg1	0x0C81
#define	OfsEVC1BoardIdReg2	0x0C82
#define	OfsEVC1BoardIdReg3	0x0C83
#define	OfsEVC1BoardControlReg	0x0880
#define	OfsEVC1BoardConfigReg	0x0884

/***********  Board Enable Definitions ********/

#define	EISA_DISABLE	0x00	/* WRITE MASK that disables the board */
#define	EISA_ENABLE	0x01	/* RW 1= Enable EVC-1 board */
#define	EISA_IOCHKERR	0x02	/* RO 0= EVC-1 OK */
#define	EISA_STARTRS	0x04	/* WO 1-to-0 = Reset EVC-1 board (500 nanosec.) */
#define	EISA_STOPRS	0x00	/*    stop reset sequence. */


/***********  Board Identification Definitions ***********/

#define	EVC1Id0	0x3D
#define	EVC1Id1	0x89
#define	EVC1Id2	0x10
#define	EVC1Id3	0x11

/***********  Board Control Definitions ********/

#define	EVC1Setup	0x01	/* RW 1= Enable VGA, 0= Setup VGA */
#define	EVC1Access	0x02	/* RW 1= VGA path, 0= direct path */
#define	EVC1Address	0x04	/* RW 1= direct at 3.25 GB, 0= direct at 14 MB */
#define EVC1_MEMHIGH	0xD0000000
#define EVC1_MEMLOW	0x00E00000
#define	EVC1MonitorMask	0x70	/* RO monitor ID bits */

/***********  Monitor Identification Definitions ********/

#define	MONVGAMono	5	/* VGA Monochrome */
#define	MONVGAColor	3	/* VGA Color */
#define	MONHiRMono	4	/* High Resolution Monochrome */
#define	MONHiRColor	2	/* High Resolution Color */
#define	MONNone		7	/* No monitor attached */


/***********  Board Config Definitions ********/

#define	EVC1Buswidth	0x01	/* 1= 8 bits, 0= 16 bits */



/***********  Macros for Register Writes ********/

/*
#define	EVC1StartSetup	( (EVC1Setup & (~EVC1Setup)) | EVC1Access )
#define	EVC1StopSetup	( EVC1Setup | EVC1Access )
*/
#define	EVC1StartSetup	( (EVC1Setup & (~EVC1Setup)) | EVC1Access | EVC1Address )
#define	EVC1StopSetup	( EVC1Setup | EVC1Access | EVC1Address )
#define	EVC1Bus16	(EVC1Buswidth & (~EVC1Buswidth))
#define	EVC1Bus8	(EVC1Buswidth)

#define	EVC1DirectHIGH	( EVC1Setup | EVC1Address )
#define	EVC1DirectLOW	( EVC1Setup )


/* definitions used in 'vtables.c' */

#define	EVC_BASE	EVC1_MEMHIGH
#define	EVC_SIZE	0x00100000
#define	EVC_HGSIZE	307200L

