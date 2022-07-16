/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/eisa.h	1.1"
/*	Copyright (c) 1988, 1989 Intel Corp.		*/
/*	  All Rights Reserved  	*/
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license
 *	agreement or nondisclosure agreement with Intel Corpo-
 *	ration and may not be copied or disclosed except in
 *	accordance with the terms of that agreement.
 */

#define EISA_CFG0	0xc80	/* EISA configuration port 0 */
#define EISA_CFG1	0xc81	/* EISA configuration port 1 */
#define EISA_CFG2	0xc82	/* EISA configuration port 2 */
#define EISA_CFG3	0xc83	/* EISA configuration port 3 */

#define ELCR_PORT0	0x4d0	/* Edge/level trigger control register 0 */
#define ELCR_PORT1	0x4d1	/* Edge/level trigger control register 1 */

#define EDGE_TRIG	0	/* interrupt is edge triggered */
#define LEVEL_TRIG	1	/* interrupt is level triggered */
