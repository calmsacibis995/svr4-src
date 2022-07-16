/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"at386.s"

	.ident	"@(#)boot:boot/at386/initprog/at386.s	1.1.4.2"

	.globl	initprog
	.globl	cascade

initprog:

/	set up initprog's local stack

	movw	%ss, %cx		/ save existing stack segment
	mov	%esp, %edx		/ save the existing stack pointer
	movw	%ds, %ax
	movw	%ax, %ss		/ %ss = %ds 
	mov	$0xfffe, %esp		/ start a new stack

	push	%ecx			/ push old stack segment
	push	%edx			/ push old stack pointer 

	call	a20		/ turn on A20 addressing

	pop 	%edx		/ get old stack pointer
	pop	%ecx		/ get old stack segment

	movw	%cx, %ss	/ restore stack segment
	mov	%edx, %esp	/ restore stack pointer

	lret			/ return to boot()

/ ------------------------------
/	Check for Cascade hardware identification string

cas_id:	.string "IDNO"

cascade:
	push	%es

	mov	$0x8, %eax	/ set es to point to a 'flat' descriptor
	movw	%ax, %es

	lea	cas_id, %esi
	mov	$0xfed00, %edi
	mov	$0x4, %ecx
	cld
	rep
	cmpsb
	jne	cas_no

	mov	$0x1, %eax
	pop	%es
	ret

cas_no:
	mov	$0x0, %eax
	pop	%es
	ret
