/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"memcpy.s"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.

	.ident	"@(#)boot:boot/at386/memcpy.s	1.1.3.1"

	.align	4

	.globl	memcpy

/	--------------------------------------------
/	memcpy(dest, src, cnt): works in exactly the same fashion as the 
/	libc memcpy; addresses are physaddr's.

memcpy:
	pushl	%edi
	pushl	%esi

	push	%es
	push	%ds

	cld

	mov	$0x08, %eax	/ set %ds and %es to point at flat
	movw	%ax, %es	/ descriptor, so that moves are done
	movw	%ax, %ds	/ using physical addresses

	movl	20(%esp), %edi	/ %edi = dest address
	movl	24(%esp), %esi	/ %esi = source address
	movl	28(%esp), %ecx	/ %ecx = length of string
	movl	%edi, %eax	/ return value from the call

	movl	%ecx, %edx	/ %edx = number of bytes to move
	shrl	$2, %ecx	/ %ecx = number of words to move
	rep ; smovl		/ move the words

	movl	%edx, %ecx	/ %ecx = number of bytes to move
	andl	$0x3, %ecx	/ %ecx = number of bytes left to move
	rep ; smovb		/ move the bytes

	pop	%ds
	pop	%es

	popl	%esi
	popl	%edi

	ret
