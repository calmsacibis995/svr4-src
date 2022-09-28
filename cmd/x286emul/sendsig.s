/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file   "sendsig.s"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft 
/	Corporation and should be treated as Confidential.

	.ident	"@(#)x286emul:sendsig.s	1.1"


/ This file implements the 386 interface routine that invokes a
/ 286 signal call handler.
/ The address of the handler and the signal number are passed as arguments.
/ The entry point "sendsig" is called from signals_go_here() in C code.
/
/       sendsig(handler, signo )
/               ulong handler;  /* handler CS/IP */
/               ulong signo;    /* signal number */


	.set    USER_CS, 0x17   / must match value in sys/seg.h
	.set    USER_DS, 0x1F   / must match value in sys/seg.h

	.globl  Stacksel	/ 286 stack segment selector
	.globl	TheStack	/ base of 286 stack segment
	.globl  Ltext	        / large model text flag
	.globl	Jam_cs		/ selector of 286 entry point
	.globl	Sig_cal		/ offset of sig_cal in jam area

	.globl	restore
	.globl  sendsig
sendsig:

	/ establish stack frame and save registers
	pushl   %ebp
	movl    %esp, %ebp

	pushfw				/ flags
	movl	Jam_cs, %eax		/ push selector of sig_cal in jam
	pushw	%ax
	movl	Sig_cal, %eax		/ offset of sig_cal in jam
	pushw	%ax

	/ convert 386 SP and BP to 286 equivalents
	movl    %esp, %ecx
	subl    TheStack, %ecx
	subl    TheStack, %ebp

	/ restore 286 stack segment register and pointer
	movw    Stacksel, %bx
	movw    %bx, %ss
		/ interrupt cannot occur here
	movl    %ecx, %esp

	movw    %bx, %ds
	movw    %bx, %es

	/ clear flags register
	subl	%eax, %eax		/ get a zero
	pushw   %ax
	popfw

	/ clear registers to help debugging
	movl    %eax, %ebx
	movl    %eax, %ecx
	movl    %eax, %esi
	movl    %eax, %edi

	/ note that 386 asm does not do 16-bit addressing correctly,
	/ so we must hand-code the ljmp instruction
	addr16                  / word address size
	data16                  / word operand size
	.byte   0xFF,0x6E,0x08  / ljmp 8(%bp) = handler CS/IP arg

	/ restore 386 environment
	/ iret back from 286 signal handler goes here
restore:
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

	/ remove return address (left by jam code)
	addl	$8,%esp

	/ restore registers
	popl    %ebp

	/ return to 386 caller
	ret
