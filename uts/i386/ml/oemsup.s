/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.ident	"@(#)kern-ml:oemsup.s	1.3"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft 
/	Corporation and should be treated as Confidential.

/
/
/ *** oemsup.s - miscellaneous oem specific routines
/
/	Routines are:
/		oem_fclex - clear 287/387 exception

	.text

/ ******************************************************************
/
/ 	oem_fclex - clear 287/387 exception 
/
/ ******************************************************************

	.align	4
	.globl	oem_fclex
oem_fclex:
	/ The NDP's (287's) busy line is only held active while actually
	/ executing, but hardware that sits between the 2 chips latches
	/ the NDP error line and feeds it to the 286/386 busy line.  Since this
	/ prevents normal coprocessor communication, we must clear the NDP
	/ BUSY latch before attempting to examine the NDP status word.
	/ Else the 286 thinks the NDP is busy and hangs on any NDP access.
	movw	$0x0f0,%dx		/ outb(0xf0, 0)
	subb	%al,%al
	outb	(%dx)
	ret
