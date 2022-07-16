/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.

	.file	"prot.s"

	.ident	"@(#)boot:boot/at386/prot.s	1.1.3.1"

#include "bsymvals.h"

/	----------------------------------------------------
/ Enter protected mode.
/
/ We must set up the GDTR
/
/ When we enter this routine, 	ss == ds == cs == "codebase", 
/	when we leave,  	ss == ds = 0x10, es = 0x08, cs = 0x18
/
/ Trashes %ax, %bx.

	.globl	goprot
goprot:

	data16
	popl	%ebx			/ get return %eip, for later use

/	load the GDTR

	addr16
	data16
	lgdt	GDTptr

	mov	%cr0, %eax

	data16
	or	$PROTMASK, %eax

	mov	%eax, %cr0 

	jmp	qflush			/ flush the prefetch queue

/ 	Set up the segment registers, so we can continue like before;
/ 	if everything works properly, this shouldn't change anything.
/ 	Note that we're still in 16 bit operand and address mode, here, 
/ 	and we will continue to be until the new %cs is established. 

qflush:
	data16
	mov	$0x10, %eax
	movw	%ax, %es
	movw	%ax, %ds
	movw	%ax, %ss		/ don't need to set %sp

/ 	Now, set up %cs by fiddling with the return stack and doing an lret

	data16
	pushl	$0x18			/ push %cs

	data16
	pushl	%ebx			/ push %eip

	data16
	lret

/	----------------------------------------------------
/ 	Re-enter real mode.
/ 
/ 	We assume that we are executing code in a segment that
/ 	has a limit of 64k. Thus, the CS register limit should
/ 	be set up appropriately for real mode already. We also
/ 	assume that paging has *not* been turned on.
/ 	Set up %ss, %ds, %es, %fs, and %gs with a selector that
/ 	points to a descriptor containing the following values
/
/	Limit = 64k
/	Byte Granular 	( G = 0 )
/	Expand up	( E = 0 )
/	Writable	( W = 1 )
/	Present		( P = 1 )
/	Base = any value

	.globl	goreal
goreal:

/ 	To start off, transfer control to a 16 bit code segment

	ljmp	$0x20, $set16cs
set16cs:			/ 16 bit addresses and operands 

	data16
	movl	$0x10, %eax	/ need to have all segment regs sane ...
	movw	%ax, %es	/ ... before we can enter real mode

	data16
	mov	%cr0, %eax

	data16
	and 	$NOPROTMASK, %eax	/ clear the protection bit

	data16
	mov	%eax, %cr0

/ 	We want to do a long ret here, to reestablish %cs in real mode
/	Check destseg to find out where we want to go.

	addr16
	data16
	pushl	destseg

	data16
	pushl	$restorecs

	data16
	lret

/ 	Now we've returned to real mode, so everything is as it 
/	should be. Set up the segment registers and so on.
/	The stack pointer can stay where it was, since we have fiddled
/	the segments to be compatible.

restorecs:

	movw	%cs, %ax
	movw	%ax, %ss
	movw	%ax, %ds
	movw	%ax, %es

	data16
	ret			/ return to whence we came; it was a 32 bit call
