/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file "bstart.s"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.

	.ident	"@(#)boot:boot/at386/bstart.s	1.1.3.1"

#include "bsymvals.h"

/	-----------------------------------------------
/	bstart( addr, isprot) transfers control to another program in memory.
/	'addr' is a paddr_t, and 'isprot' is a flag which indicates if the
/	program is to be started in real or protected mode.
/	
/	Initprogs are run in protected mode, the kernel is started in real
/	mode.

	.globl	bstart

bstart:
	mov	8(%esp), %eax
	cmpl	$0x0, %eax
	je	realstart

/ 	protect mode startup

protstart:
	/ patch the extra 3 gdt entries to have the bstart address
	/ as their base. Reload the gdt.  

	push	%ebx

	mov	$initdata, %ecx
	mov	$initcode, %edx
	mov	$initcode16, %ebx

	mov	8(%esp), %eax

	movw	%ax, 2(%ebx)
	movw	%ax, 2(%ecx)
	movw	%ax, 2(%edx)

	shr	$16, %eax

	movb	%al, 4(%ebx)
	movb	%al, 4(%ecx)
	movb	%al, 4(%edx)

	lgdt	GDTptr

	/ push the %cs and %ip of the location to return to.

	push	%cs
	push	$protreturn

	/ push the %cs and %ip of the program we are jumping to

	push	$0x30			/ code segment for prot boot
	push	$0			/ offset in code segment

	/ jump to the program

	mov	$0x28, %eax		/ data segment for prot boot
	movw	%ax, %ds
	movw	%ax, %es

	/ put magic number in %edi

	mov	$BKI_MAGIC, %edi

	lret

protreturn:
	/ restore %ds and %es to the bootstrap data segment

	mov	$0x10, %eax
	movw	%ax, %ds
	movw	%ax, %es

	pop	%ebx

	ret

/	real mode startup

realstart:
	mov	4(%esp), %eax		/ get future %cs
	shr	$4, %eax		
	andl	$0xffff, %eax

	mov	4(%esp), %ecx		/ get future %ip
	andl	$0x000f, %ecx

	push	%ebp

	push 	%eax
	push	%ecx

	call	goreal
	sti

	data16
	pop	%ecx
	data16
	pop	%eax

	movw	%ax, %ds		/ set up %ds and %es for...
	movw	%ax, %es		/ program to be called

/ 	set up so that called program returns to us

	push 	%cs			/ our code segment

	data16
	movl	$realreturn, %edx
	push	%edx			/ %ip of location to return to

	push	%eax			/ %cs of program to call
	push	%ecx			/ %ip of program to call

/ 	put magic number (0xff1234ff) into %edi

	data16
	mov	$BKI_MAGIC, %edi

	cli				/ turn off interrupts (for kernel)

	lret

realreturn:
	movw	%cs, %ax
	movw	%ax, %ds
	movw	%ax, %es

	cli
	data16
	call	goprot

	pop 	%ebp

	ret
