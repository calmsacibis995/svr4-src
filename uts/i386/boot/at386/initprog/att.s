/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.ident	"@(#)boot:boot/at386/initprog/att.s	1.1.4.2"

	PROTMASK	= 0x1
	NOPROTMASK	= 0xfffe

	.globl	initprog
	.globl	cascade
	.globl	rst_vga
	.globl	rst_ega

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

/	if we are running on a SuperVu or an EGA, we have to 
/	re-initialize the display BIOS, since, by default, it 
/	depends on the presence of shadow RAM

	call	ck_video

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

/ ------------------------------
/	Reset VGA BIOS

rst_vga:
	push	%ebp
	call	goreal
	sti
	.byte	0x9a
	.value	0x03
	.value	0xc000		/ lcall 0xc000:3 resets the video BIOS
	cli
	data16
	call	goprot
	pop	%ebp

	ret

/ ------------------------------
/	Reset EGA BIOS

rst_ega:
	mov	$0x8, %eax		/ set es to point to a 'flat' descriptor
	movw	%ax, %es

	/ if the fonts are at c000, don't change anything
	mov	$0x10e, %eax
	cmpw	$0xc000, %es:(%eax)
	je	done

	/ Point int 0x10 at c000:0cd7
	mov	$0x42, %eax
	movw	$0xc000, %es:(%eax)	/ 0:42 <- 0xC000	

	/ Point ??? at c000:3560
	mov	$0x7e, %eax
	movw	$0xc000, %es:(%eax)	/ 0:7e <- 0xC000	

	/ Fonts are at c000:3160
	mov	$0x10e, %eax
	movw	$0xc000, %es:(%eax)	/ 0:10e <- 0xC000	

done:
	ret

/ ------------------------------
/ 	Return to protected mode; stolen from prot.s

goprot:
	data16
	popl	%ebx			/ get return %eip, for later use

/	return to protected mode

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
	mov	$0x28, %ecx	/ hack, hack; this shouldn't be hard coded
	movw	%cx, %es
	movw	%cx, %ds
	movw	%cx, %ss		
	
/ 	Now, set up %cs by fiddling with the return stack and doing an lret

	data16
	mov	$0x30, %edx		/ hack, hack; ditto
	data16
	pushl	%edx			/ push %cs

	data16
	pushl	%ebx			/ push %eip

	data16
	lret

/	----------------------------------------------------
/ 	Re-enter real mode; stolen in large part from prot.s

	.globl	goreal
goreal:

/ 	transfer control to a 16 bit code segment

	ljmp	$0x38, $set16cs	/ hack, hack.
set16cs:			

/ 	16 bit addresses and operands, here.

	data16
	mov	%cr0, %eax

	data16
	and 	$NOPROTMASK, %eax	/ clear the protection bit

	data16
	mov	%eax, %cr0

/ 	Do a long jump to reestablish %cs in real mode

	addr16
	data16
	ljmp	$0xA00, $restorecs 		/ initprogs are always 
						/ loaded at 0xA000 physical. 
						/ So mote it be.

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
