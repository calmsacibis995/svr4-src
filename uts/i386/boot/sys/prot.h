/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation */
/*	All Rights Reserved */

/*	INTEL CORPORATION PROPRIETARY INFORMATION */

/*	This software is supplied to AT & T under the terms of a license */
/*	agreement with Intel Corporation and may not be copied nor */
/*	disclosed except in accordance with the terms of that agreement. */

/*	.ident	"@(#)boot:boot/sys/prot.h	1.1.2.1"

/* Protected mode selectors */
#define	GDTSEL		0x008
#define	DATASEL		0x1b0
#define CODESEL		0x1b8
#define MEMALIAS	0x1e0

/* Real mode selectors */
#define FLATDESC	0X208
#define DATADESC	0X210
#define CODEDESC	0x218
#define CODE16DESC	0x220

/* Descriptor types */
#define CODE		0x409E
#define DATA		0x4092

#define PROTPORT	0xE0
#define TIMEOUTPORT	0xE2
#define PROTMASK	0x1
#define NOPROTMASK	0xfffffffe
#define PROTVAL		1

/* Cache management */
#define MISSALLPORT	0xEC
#define MISSALL		1

/* Physical address of real mode code after relocation */
#define MY_PADDR	0x0E000
/* 86 style selector of real mode code after relocation */
#define MY_86SEL	0x00E00
