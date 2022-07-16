/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mb1:uts/i386/boot/mb1/memsize.c	1.3"

#include "sys/types.h"
#include "../sys/boot.h"

#define	INC_SIZE	0x1000			/* 1 page increments */
#define	LO_MBUS_IO	0x00f80000
/*
 *	Size memory, assuming an integral number of pages
*/
paddr_t
memsize()
{	paddr_t		p;

	for (p = RESERVED_SIZE; p < LO_MBUS_IO; p += INC_SIZE) {
		if (!touchmem(p)) 
			break;
	}
	return(p);
}
