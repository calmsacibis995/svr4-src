/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file   "sendsig.s"
	.ident	"@(#)i286emu:sendsig.s	1.1"

/ This file implements the 386 interface routine that invokes a
/ 286 signal call handler.
/ The address of the handler and the signal number are passed as arguments.
/ The entry point "sendsig" is called from signals_go_here() in C code.
/
/       sendsig(handler, signo, esds )
/               ulong handler;  /* handler CS/IP */
/               ulong signo;    /* signal number */


	.set    USER_CS, 0x17   / must match value in sys/seg.h
	.set    USER_DS, 0x1F   / must match value in sys/seg.h

	.globl  stacksegsel     / 286 stack segment selector
	.globl  uu_model        / large model flag

	.globl  sendsig
sendsig:

	/ establish stack frame and save registers
	pushl   %ebp
	movl    %esp, %ebp
	pushl   %ebx
	pushl   %edi
	pushl   %esi

	/ if small model, push a far return address on the stack.
	/ it is used by code at location zero in the 286 crt0.s
	/ that has been patched to do a far return.
	movl    uu_model, %edx
	cmpl    $0, %edx
	jne     pushargs
	pushw   $USER_CS
	movl    $restore, %eax  / 386 ld can't relocate short addr
	pushw   %ax

	/ push zero code and signal number args on stack
pushargs:
	subl    %eax, %eax
	pushw   %ax
	pushw   12(%ebp)

	/ convert 386 SP and BP to 286 equivalents
	movl    %esp, %ecx
	subl    stackbase, %ecx
	subl    stackbase, %ebp

	/ restore 286 stack segment register and pointer
	movw    stacksegsel, %bx
	movw    %bx, %ss
		/ interrupt cannot occur here
	movl    %ecx, %esp

	/ set ES and DS to stack segment selector for small model.
	/ they are always reloaded in large model code.
	movw    %bx, %ds
	movw    %bx, %es

	/ clear flags register
	pushw   %ax
	popfw

	/ clear registers to help debugging
	movl    %eax, %ebx
	movl    %eax, %ecx
	movl    %eax, %esi
	movl    %eax, %edi

	/ branch to large/small model call to 286 signal handler
	cmpl    $0, %edx        / test uu_model
	movl    %eax, %edx      / clear %edx
	jne     largemodel

	/ small model
	/ push zero return address on the stack,
	/ and then do an indirect, far jump to the 286 signal handler.
	/ the handler will return to location zero in crt0.s,
	/ which pops the args and does a far return to the restore label.
	pushw   $0
	/ note that 386 asm does not do 16-bit addressing correctly,
	/ so we must hand-code the ljmp instruction
	addr16                  / word address size
	data16                  / word operand size
	.byte   0xFF,0x6E,0x08  / ljmp 8(%bp) = handler CS/IP arg


	/ large model
	/ call 286 signal handler using indirect, far call.
	/ note that 386 asm does not do 16-bit addressing correctly,
	/ so we must hand-code the lcall instruction
largemodel:
	addr16                  / word address size
	data16                  / word operand size
	.byte   0xFF,0x5E,0x08  / lcall 8(%bp) = handler CS/IP arg
	addw    $4, %sp         / pop the args

	/ restore 386 environment
restore:
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

	/ restore registers
	popl    %esi
	popl    %edi
	popl    %ebx
	popl    %ebp

	/ return to 386 caller
	ret
