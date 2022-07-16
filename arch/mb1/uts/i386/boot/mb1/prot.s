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

/	Copyright (c) 1988  Intel Corporation
/	All Rights Reserved

/	INTEL CORPORATION PROPRIETARY INFORMATION

/	This software is supplied to AT & T under the terms of a license
/	agreement with Intel Corporation and may not be copied nor
/	disclosed except in accordance with the terms of that agreement.

	.file	"prot.s"

	.ident	"@(#)mb1:uts/i386/boot/mb1/prot.s	1.3"

#include "../sys/prot.h"

/	----------------------------------------------------
/ Enter protected mode.
/
/ We must set up the GDTR
/
/ When we enter this routine, 	ss == ds == cs == "codebase", 
/	when we leave,  	ss == ds == es = DATADESC, cs = CODEDESC
/
/ Trashes %ax, %bx.
	
	.bss

	.globl	dataseg
destseg:
	. = . + 4
dataseg:
	. = . + 4
itsdmon:
	. = . + 4

	.text

	.globl	goprot
goprot:
	data16
	popl	%ebx			/ get return %eip, for later use
	data16
	push	%fs			/ save it
	push	%esi			/ save it

/ first check if DMON is initialized.  If yes, use it's GDT, else 
/ use our GDT.

	data16
	mov	$0, %eax
	movw	%ax, %fs
	addr16
	data16
	mov	%fs:0x400, %eax
/	data16
/	int	$3
	addr16
	data16
	cmpl	$0x2e54424a, %eax		/ JBT. ? - test for itsdmon
	jne	use_ourgdt

/	setup the base for code, and data descriptors in the GDT. Copy
/	GDT base, limit, IDT base and limit.

	addr16
	data16
	mov	$1, itsdmon	
	addr16
	data16
	mov	%fs:0x404, %eax
	addr16
	data16
	mov	%eax, gdtbase
	data16
	mov	%eax, %esi
	addr16
	mov	%fs:8(%esi), %eax
	addr16
	mov	%eax, gdtlimit

	addr16
	data16
	mov	%fs:0x408, %eax
	addr16
	data16
	mov	%eax, idtbase
	addr16
	data16
	mov	gdtbase, %esi
	addr16
	mov	%fs:0x10(%esi), %eax
	addr16
	mov	%eax, idtlimit
	jmp	setup_desc

use_ourgdt:
	addr16
	data16
	mov	$0, itsdmon	
	mov	$0, ourgdt	
	data16
	xor	%eax, %eax
	movw	%ds, %ax
	movw	%ax, %fs
	addr16
	data16
	shl	$4, %eax		/ get segment base
	addr16
	data16
	add	$ourgdt, %eax		/ add offset
	addr16
	data16
	mov	%eax, gdtbase
	addr16
	data16
	mov	$ourgdt, %esi
	addr16
	data16
	mov	$0x409200, %fs:12(%esi)
	addr16
	mov 	%eax, %fs:10(%esi)
	data16
	shr	$16, %eax
	addr16
	movb	%al, %fs:12(%esi)
	addr16
	data16
	mov	$gdtend, %eax
	data16
	sub	%esi, %eax
	dec	%eax
	addr16
	mov	%eax, gdtlimit
	addr16
	mov	%eax, %fs:8(%esi)

setup_desc:
	addr16
	data16
	mov	$0xffff, %fs:FLATDESC(%esi)
	addr16
	data16
	mov	$0xffff, %fs:DATADESC(%esi)
	addr16
	data16
	.globl _etext
	mov	$_etext, %fs:CODEDESC(%esi)
	addr16
	data16
	mov	$0xffff, %fs:CODE16DESC(%esi)
	addr16
	data16
	mov	$0xCF9200, %fs:FLATDESC+4(%esi)
	addr16
	data16
	mov	$0x409200, %fs:DATADESC+4(%esi)
	addr16
	data16
	mov	$0x409E00, %fs:CODEDESC+4(%esi)
	addr16
	data16
	mov	$0x09E00, %fs:CODE16DESC+4(%esi)

	movw	%ds, %ax
	addr16
	mov	%eax, dataseg
	movw	%cs, %ax
	addr16
	mov	%eax, destseg		/ save it now for return later
	data16
	shl 	$4, %eax
	addr16
	mov	%eax, %fs:2+CODEDESC(%esi)
	addr16
	mov	%eax, %fs:2+CODE16DESC(%esi)
	addr16
	mov	%eax, %fs:2+DATADESC(%esi)
	data16
	shr	$16, %eax
	addr16
	movb	%al, %fs:4+CODEDESC(%esi)
	addr16
	movb	%al, %fs:4+CODE16DESC(%esi)
	addr16
	movb	%al, %fs:4+DATADESC(%esi)
	data16
	shr	$8, %eax
	addr16
	movb	%al, %fs:7+CODEDESC(%esi)
	addr16
	movb	%al, %fs:7+CODE16DESC(%esi)
	addr16
	movb	%al, %fs:7+DATADESC(%esi)
	data16
	pop	%esi			/ restore it
	pop	%fs			/ restore it


/	load the GDTR

	addr16
	data16
	lgdt	GDTptr

	addr16
	data16
	cmp	$1, itsdmon
	jne	skip_idtr

/	load the IDTR

	addr16
	data16
	lidt	IDTptr

skip_idtr:
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
	mov	$DATADESC, %eax
	movw	%eax, %es
	movw	%eax, %ds
	movw	%eax, %ss		/ don't need to set %sp
	data16
	pushl	$CODEDESC		/ Reload CS
	data16
	pushl	$ret_here
	data16
	lret
ret_here:

#ifdef MB1
	push	%edx			/ save it
	movw	$PROTPORT, %dx		/ dx<- PORT addr for putting
					/ 386/20 into protected mode
	movb	$PROTVAL, %al		/ value to be output for putting
					/ 386/20 into protected mode
	outb	(%dx)

	movw	$MISSALLPORT, %dx	/ dx<- PORT addr for disabling
					/ cache on 386/20
	movb	$MISSALL, %al		/ value to be output for disabling
					/ cache on 386/20 
	outb	(%dx)
	pop	%edx			/ restore it

#endif /*MB1*/

/ 	Now, set up %cs by fiddling with the return stack and doing an lret
	pushl	$CODEDESC		/ push %cs
	pushl	%ebx			/ push %eip
	lret

/	----------------------------------------------------
/ 	Re-enter real mode.
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

	ljmp	$CODE16DESC, $set16cs
set16cs:			/ 16 bit addresses and operands 

	data16
	movl	$0x3ff, %eax
	addr16
	movw	%ax, idtlimit
	addr16
	data16
	mov	$0, idtbase

/	re-load the IDTR

	addr16
	data16
	lidt	IDTptr

	data16
	movl	$FLATDESC, %eax
	movw	%eax, %es
	addr16
	data16
	movb	$0x0f, %es:0x8b6

	data16
	movl	$DATADESC, %eax	/ need to have all segment regs sane ...
	movw	%eax, %es	/ ... before we can enter real mode

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
	movw	%cs, %eax
	movw	%eax, %ss
	movw	%eax, %ds
	movw	%eax, %es

#ifdef MB1
	push	%edx
/
/	work-around for a *feature* of DMON.  The monitor expects the
/	board to stay in protected mode eventhough the processor is in 
/	real mode.  This is because the monitor runs in pvam and it
/	does its initialization only once (to switch the board in pvam)
/
	addr16
	data16
	cmp	$1, itsdmon
	je	leave_bd_in_pvam

	data16
	mov	$PROTPORT, %edx		/ dx<- PORT addr for putting
					/ 386/20 into real mode
	data16
	movb	$0, %al			/ value to be output for putting
					/ 386/20 into real mode
	outb	(%dx)

leave_bd_in_pvam:

	data16
	mov	$MISSALLPORT, %edx	/ dx<- PORT addr for enabling
					/ cache on 386/20
	data16
	movb	$0, %al			/ value to be output for enabling
					/ cache on 386/20
	outb	(%dx)

	data16
	mov	$TIMEOUTPORT, %edx	/ dx<- PORT addr to clear timeout 
					/ on 386/20
	data16
	movb	$0, %al			/ value to clear timeout on 386/20  
	outb	(%dx)
	pop	%edx
#endif /* MB1 */

	data16
	ret			/ return to whence we came; it was a 32 bit call

	.align	8
	.globl	aliassel		/ points to flatdesc
aliassel:
	.value	0x208

	.bss

	.globl	GDTptr
	.globl	gdtlimit
	.globl	gdtbase
	.globl	IDTptr
	.globl	idtlimit
	.globl	idtbase
	

GDTptr:	
gdtlimit:
	. = . + 2			/ filled at runtime
gdtbase:
	. = . + 4			/ filled at runtime
	.align	8
IDTptr:	
idtlimit:
	. = . +	2			/ filled at runtime
idtbase:
	. = . + 4			/ filled at runtime
	.align	16
ourgdt:
	. = . + 4
	. = . + 4
	. = . + 560			/ max 70 entries
gdtend:					/ label to indicate end of gdt
