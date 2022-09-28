/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file   "run286.s"
	.ident	"@(#)i286emu:run286.s	1.1"

/ This file implements the 386 interface routine that runs 286 code.
/ The entry point "run286" is called from "???" in C code.
/
/ run286(stackpointer, entrypoint)
/   ushort *stackpointer;       /* 286 initial stack pointer */
/   ulong entrypoint;           /* 286 entry point CS/IP */
/
/ The 286 stack has the exec args on it.
/ We switch to the 286 stack,
/    push on the 286 entry point CS/IP,
/    clear the registers,
/    and return.
/
/ The 286 stack looks like this:
/
/ (one word per line)

/            <-- 0xFFFC
/       .
/       .
/       strings
/       NULL
/       .
/       .
/       environment string ptrs
/       NULL
/       .
/       .
/       exec arg string ptrs
/       argc                  <-- stackpointer


	.globl  stacksegsel     / ushort stack segment selector


	.globl  run286
run286:

	pushl   %ebp             / for debug stack tracing
	movl    %esp, %ebp

	/ get args
	movl    8(%ebp), %ecx    / stack pointer
	movl    12(%ebp), %eax   / entry point CS/IP

	/ The 386 kernel no longer fills the floating point stack with zeros,
	/ so the following 8 pop's are no longer needed.
	/ Uncomment them to run on an old kernel.

	/ flush the floating point stack
/       fstp    %st(0)
/       fstp    %st(0)
/       fstp    %st(0)
/       fstp    %st(0)
/       fstp    %st(0)
/       fstp    %st(0)
/       fstp    %st(0)
/       fstp    %st(0)

	/ load 286 stack segment register and pointer
	movw    stacksegsel, %bx
	movw    %bx, %ss
		/ interrupt cannot occur here
	movw    %cx, %sp

	/ push entry point on 286 stack
	pushl   %eax

	/ clear registers for 286 code
	subl    %eax, %eax
	movl    %eax, %ebx
	movl    %eax, %ecx
	movl    %eax, %edx
	movl    %eax, %ebp
	movl    %eax, %esi
	movl    %eax, %edi
	movw    %ax, %ds
	movw    %ax, %es
	pushw   %ax
	popfw

	/ return to 286 program entry point
	data16          / for 16-bit IP
	lret
