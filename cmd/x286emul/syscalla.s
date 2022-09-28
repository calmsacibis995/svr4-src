/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"syscall.s"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft 
/	Corporation and should be treated as Confidential.

	.ident	"@(#)x286emul:syscalla.s	1.1"

	.set    USER_DS, 0x1f   / must match value in sys/seg.h

/ This file implements the 286 system call interface to 386 code.
/ The entry point "syscalla" is specified in the jam entry for
/ 286 system calls.
/

	.globl  TheStack	/ 386 virtual address of base of 286 stack seg
	.globl  Stacksel	/ 286 stack segment selector

	.globl  smallcalla
	.globl	largecalla
/
/ System call entry point for small model programs
/
smallcalla:
	/ save 286 registers
	pushfw
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

	/ establish 386 data segment
	movw    $USER_DS, %bx
	movw    %bx, %ds
	movw    %bx, %es

	/ convert 286 SP to 386 equivalent
	movzwl  %sp, %ecx	/ zero-extend short SP
	addl    TheStack, %ecx	/ add base of 286 stack segment

	/ load 386 stack segment register and pointer
	movw    %bx, %ss
		/ interrupt cannot occur here
	movl    %ecx, %esp

	/ clear bp
	xorl    %ebp, %ebp

/ The stack now looks like this:
/ (one word per line)
/       cs
/       ip (hi)     Dword ip because of jam area lcall
/       ip (lo)
/       flags
/       bp
/       ds
/       es
/	arg3	|
/       arg2    |
/       arg1    | 286 system call args
/       arg0    |
/	offset of arg0
/	system call #		<--- esp

	/ call the C language system call handler
	call   syscall

	/ the value returned by syscall in %eax is the value returned
	/ by the system call,
	/ so put it in %ax:%bx as expected by the 286 code
	movl    %eax, %ebx
	shrl    $16, %ebx       / hi-order word in %bx

	/ convert 386 SP to 286 equivalent
	movl    %esp, %ecx
	subl    TheStack, %ecx	/ subtract base of 286 stack segment

	/ restore 286 stack segment register and pointer
	movw    Stacksel, %dx
	movw    %dx, %ss
		/ interrupt cannot occur here
	movw    %cx, %sp

	/ restore 286 registers
	addw	$6,%sp	/ drop syscall#, don't restore ax,bx since they have
			/ the returned value
	popw	%cx
	popw	%si
	popw	%di
	data16
	pop	%es
	data16
	pop	%ds
	popw	%bp
	popfw           / flags - carry bit set in C code

	/ return to 286 code thru model dependent return after jam
	lret

/
/ System call entry point for large model programs
/
largecalla:
	/ save 286 registers
	pushfw
	pushw   %bp
	data16
	push    %ds
	data16
	push    %es
	pushw	%bx
	pushw	%ax

	/ establish 386 data segment
	movw    $USER_DS, %bx
	movw    %bx, %ds
	movw    %bx, %es

	/ convert 286 SP to 386 equivalent
	movzwl  %sp, %ecx	/ zero-extend short SP
	addl    TheStack, %ecx	/ add base of 286 stack segment

	/ load 386 stack segment register and pointer
	movw    %bx, %ss
		/ interrupt cannot occur here
	movl    %ecx, %esp

	/ clear bp
	xorl    %ebp, %ebp

/ The stack now looks like this:
/ (one word per line)
/       cs
/       ip (hi)     Dword ip because of jam area lcall
/       ip (lo)
/       flags
/       bp
/       ds
/       es
/	offset of args
/	system call #		<--- esp

	/ call the C language system call handler
	call   syscall

	/ the value returned by syscall in %eax is the value returned
	/ by the system call,
	/ so put it in %ax:%bx as expected by the 286 code
	movl    %eax, %ebx
	shrl    $16, %ebx       / hi-order word in %bx

	/ convert 386 SP to 286 equivalent
	movl    %esp, %ecx
	subl    TheStack, %ecx	/ subtract base of 286 stack segment

	/ restore 286 stack segment register and pointer
	movw    Stacksel, %dx
	movw    %dx, %ss
		/ interrupt cannot occur here
	movw    %cx, %sp

	/ restore 286 registers
	addw	$4,%sp	/ No need to restore ax or bx, they hold return value
	data16
	pop	%es
	data16
	pop	%ds
	popw	%bp
	popfw           / flags - carry bit set in C code

	/ return to 286 code thru model dependent return after jam
	lret
