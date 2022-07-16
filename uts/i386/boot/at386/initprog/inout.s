/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file "inout.s"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.

	.ident	"@(#)boot:boot/at386/initprog/inout.s	1.1.2.1"
/ The following routines read and write I/O adress space
/ outl(port address, val)
/ outw(port address, val)
/ outb(port address, val)
/ long  inw(port address)
/ ushort inw(port address)
/ unchar  inb(port address)

	.set	PORT, 8
	.set	VAL, 12

	.globl	outb
outb:	pushl	%ebp
	movl	%esp, %ebp

	movw	PORT(%ebp), %dx
	movb	VAL(%ebp), %al
	outb	(%dx)

	popl	%ebp
	ret

	.globl	inb
inb:	pushl	%ebp
	movl	%esp, %ebp

	subl    %eax, %eax
	movw	PORT(%ebp), %dx
	inb	(%dx)

	popl	%ebp
	ret

	.globl	longdelay
longdelay:

	movl	$0x80000, %ecx
delaycont:
	loop	delaycont

	ret
