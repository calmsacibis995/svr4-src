/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

/	Copyright (c) 1988  Intel Corporation
/	All Rights Reserved

/	INTEL CORPORATION PROPRIETARY INFORMATION

/	This software is supplied to AT & T under the terms of a license
/	agreement with Intel Corporation and may not be copied nor
/	disclosed except in accordance with the terms of that agreement.

	.file   "util.s"

	.ident	"@(#)mbus:uts/i386/boot/msa/util.s	1.3"

	.text
target_vector:
	. = . + 8 		/ place to store the jump vector for xfer

/ ********************************************************************
/
/	gdt_move
/
/ *********************************************************************

	.globl	gdt_move

our_gdt_desc	=	32
save_flag	=	36

gdt_move:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp

	/ check the flag to see if we restore or save

	cmpl	$0, save_flag(%ebp)
	jne	restore_gdt

	/ move descriptor to eax and save

save_gdt:
	movl	our_gdt_desc(%ebp),%eax
	sgdt	(%eax)	
	jmp 	gdtdone

	/ move descriptor to eax and restore 

restore_gdt:
	movl	our_gdt_desc(%ebp),%eax
	lgdt	(%eax)	

	/ return C-style
gdtdone:		
	jmp	cret

/ *********************************************************************
/
/	idt_move
/
/ *********************************************************************

	.globl	idt_move

our_idt_desc	=	32
save_flag	=	36

idt_move:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp


	/ check the flag to see if we restore or save

	cmpl	$0, save_flag(%ebp)
	jne	restore_idt

	/ move descriptor to eax and save 

save_idt:
	movl	our_idt_desc(%ebp),%eax
	sidt	(%eax)	
	jmp 	idtdone

	/ move descriptor to eax and restore 

restore_idt:
	movl	our_idt_desc(%ebp),%eax
	lidt	(%eax)	

	/ return C-style
idtdone:		
	jmp	cret

/ *********************************************************************
/
/	check_entry
/
/ *********************************************************************

	.globl	check_entry

entry_off		=32 
entry_sel		=36 

check_entry:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	movl	%esp,%ebp

      / put the selector and offset in handy places

	movw	entry_sel(%ebp),%es
	movl	entry_off(%ebp),%ebx

      / compare entry with 0 - 4 bytes at a time

	cmpl	$0, %es:(%ebx)
	jne	not_empty
	cmpl	$0, %es:4(%ebx)
	jne	not_empty
	movl	$1, %eax
	jmp 	ckdone

not_empty:
	movl	$0, %eax

      / return C-style     
    
ckdone:
	jmp	cret

/ *********************************************************************
/
/	cret
/
/ *********************************************************************

cret:
	leave
	pop	%edi
	pop	%esi
	pop	%ebx
	pop	%ecx
	pop	%ds
	pop	%es
	ret

/ *********************************************************************
/
/	jump_tss
/
/ *********************************************************************

	.globl	jump_tss

tss_off		= 0 
tss_sel		= 4 

jump_tss:
	pop	%eax			/ pop ret address off the stack
	pop	target_vector 		/ selector of target
	pop	[target_vector + 4]	/ offset of target
	ljmp	*target_vector		/ head out into the wild 
					/ blue yonder
