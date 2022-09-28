/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"syscall.s"
	.ident	"@(#)i286emu:syscalla.s	1.1"

	.set    USER_DS, 0x1f   / must match value in sys/seg.h

/ This file implements the 286 system call interface to 386 code.
/ The entry point "syscalla" is specified in the 386 call gate used by
/ 286 system calls.
/
/
/

	.globl  stackbase       / 386 virtual address of base of 286 stack
	.globl  stacksegsel     / 286 stack segment selector

	.globl  syscalla
syscalla:

	/ save 286 registers
	pushfw
	pushw   %bp
	data16
	push    %ds
	data16
	push    %es

	/ establish 386 data segment
	movw    $USER_DS, %bx
	movw    %bx, %ds
	movw    %bx, %es

	/ convert 286 SP to 386 equivalent
	movzwl  %sp, %ecx               / zero-extend short SP
	addl    stackbase, %ecx         / add base of 286 stack segment

	/ load 386 stack segment register and pointer
	movw    %bx, %ss
		/ interrupt cannot occur here
	movl    %ecx, %esp

	/ clear bp
	xorl    %ebp, %ebp

/ The stack now looks like this:
/
/ (one word per line)

/       ...
/       arg2    |
/       arg1    | 286 system call args
/       arg0    |
/       --          Dword cs because of 386 call gate
/       cs
/       ip (hi)     Dword ip because of 386 call gate
/       ip (lo)
/       flags
/       bp
/       ds
/       es      <-- esp

	/ call the C language system call handler
	call   syscall

	/ the value returned by syscall in %eax is the value returned
	/ by the system call,
	/ so put it in %ax:%dx as expected by the 286 code
	movl    %eax, %edx
	shrl    $16, %edx       / hi-order word in %dx

	/ convert 386 SP to 286 equivalent
	movl    %esp, %ecx
	subl    stackbase, %ecx         / subtract base of 286 stack segment

	/ restore 286 stack segment register and pointer
	movw    stacksegsel, %bx
	movw    %bx, %ss
		/ interrupt cannot occur here
	movw    %cx, %sp

	/ restore 286 registers
	data16
	pop    %es
	data16
	pop    %ds
	popw   %bp
	popfw           / flags - carry bit set in C code

	/ return to 286 code thru 386 call gate
	lret
