/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"float.s"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft 
/	Corporation and should be treated as Confidential.

	.ident	"@(#)x286emul:float.s	1.1"

	.set    USER_DS, 0x1f   / must match value in sys/seg.h

	.globl  TheStack	/ 386 virtual address of base of 286 stack seg
	.globl  Stacksel	/ 286 stack segment selector

	.globl	IDTTarget
/ This is the interface routine that catches int instructions, which may
/ preceed floating point coprocessor instructions.  All we have to do is
/ move from the 286 stack to the 386 and back.
/
/ IDT entry aims here
IDTTarget:
	/ save 286 registers
	pushw   %bp
	data16
	push    %ds
	data16
	push    %es
	pushw	%di
	pushw	%si
	pushw	%cx
	pushw	%bx
	movw	%sp,%bx
	pushw	%bx
	pushw	%ax
	pushw	%dx

	/ establish 386 data segment
	movw    $USER_DS, %bx
	movw    %bx, %ds
	movw    %bx, %es

	/ convert 286 SP to 386 equivalent
	movzwl  %sp, %ecx		/ zero-extend short SP
	addl    TheStack, %ecx		/ add base of 286 stack segment

	/ load 386 stack segment register and pointer
 	movw    %bx, %ss		/ interrupt cannot occur here
 	movl    %ecx, %esp

 	/ clear bp
 	xorl    %ebp, %ebp

/ The stack now looks like this:
/ (one word per line)
/	flags0
/	flags1
/	return0
/	return1
/	return2
/	return3
/	bp
/	ds
/	es
/	di
/	si
/	cx
/	bx
/	sp
/	ax
/	dx
/	

	/ call the C language system int handler
	call   FloatCommon

	/ convert 386 SP to 286 equivalent
	movl    %esp, %ecx
	subl    TheStack, %ecx		/ subtract base of 286 stack segment

	/ restore 286 stack segment register and pointer
	movw    Stacksel, %dx
	movw    %dx, %ss
		/ interrupt cannot occur here
	movw    %cx, %sp

	/ restore 286 registers
	popw	%dx
	popw	%ax
	popw	%bx
	popw	%bx
	popw	%cx
	popw	%si
	popw	%di
	data16
	pop	%es
	data16
	pop	%ds
	popw	%bp

	/ return back to 286 code
	iret
