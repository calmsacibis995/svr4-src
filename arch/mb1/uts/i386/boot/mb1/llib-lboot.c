/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mb1:uts/i386/boot/mb1/llib-lboot.c	1.3"

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

/*
 *	This file contains stubs for lint corresponding to the routines
 *	which are implemented in assembly.
*/
#include "sys/types.h"

extern int end;
ushort unit;
ushort cyls;
ushort heads;
ushort sectors;
int dev_gran;
ushort alternates;
ulong debug_ck;
ulong text_len;		/* Length of text in bytes */
daddr_t text_addr;	/* Address of text on disk */
ulong data_len;		/* Length of data in bytes */

int
getCS()
{	return(0);
}

int
getDS()
{	return(0);
}

/* ARGSUSED */
void
iomove(src_off, dst_off, dst_sel, count)
char *src_off;
char *dst_off;
int dst_sel;
int count;
{
}

/* ARGSUSED */
touchmem(addr)
paddr_t addr;
{	return(0);
}

main()
{	iomove((caddr_t)0, (paddr_t)0, 0, (int)&end);
	bload((caddr_t)0);
	return(0);
}
