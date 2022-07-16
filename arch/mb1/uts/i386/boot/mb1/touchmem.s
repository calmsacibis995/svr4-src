/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"touchmem.s"

/	Copyright (c) 1988  Intel Corporation
/	All Rights Reserved

/	INTEL CORPORATION PROPRIETARY INFORMATION

/	This software is supplied to AT & T under the terms of a license
/	agreement with Intel Corporation and may not be copied nor
/	disclosed except in accordance with the terms of that agreement.

	.ident	"@(#)mb1:uts/i386/boot/mb1/touchmem.s	1.3"

	.text

/ *********************************************************************
/
/	routine to check if physical memory is present at the specified
/	address
/
/ *********************************************************************

	.globl	touchmem
phys_addr	= 8
PARERR		= 0xe8

touchmem:
	push	%ebp
	mov	%esp,%ebp
	push	%es
	push	%ebx
	push	%edx

	xor	%eax, %eax		
	movw	aliassel, %ax
	movw	%ax, %es
	xor	%eax, %eax		/ clear out %ax for return value
	
	mov	phys_addr(%ebp), %ebx
	movb	$0xA5, %es:(%ebx)	/ write pattern
	cmpb	$0xA5, %es:(%ebx)	/ value still there?
	je	done
#ifdef MB1
	movw	$PARERR, %dx		/ Parity error latch for 386/20
	movb	$0, %al			/ clear the latch - miss causes parity
	outb	(%dx)
#endif
alldone:	
	pop	%edx			/ restore %edx
	pop	%ebx			/ restore %ebx
	pop	%es			/ restore %es
	leave	
	ret	
done:
	mov	$1, %eax		/ memory is present, return true
	jmp	alldone
