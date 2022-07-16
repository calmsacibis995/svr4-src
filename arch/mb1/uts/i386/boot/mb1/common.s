/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file   "common.s"

/	Copyright (c) 1988  Intel Corporation
/	All Rights Reserved

/	INTEL CORPORATION PROPRIETARY INFORMATION

/	This software is supplied to AT & T under the terms of a license
/	agreement with Intel Corporation and may not be copied nor
/	disclosed except in accordance with the terms of that agreement.

	.ident	"@(#)mb1:uts/i386/boot/mb1/common.s	1.3"
#include "../sys/prot.h"

/ ********************************************************************
/
/	This file contains the common functions used by the disk and 
/	tape bootstrap loader for loading Unix System V/386
/
/ ********************************************************************

	.text

/ *********************************************************************
/
/	iomove
/
/ *********************************************************************

	.globl	iomove

src_off	=	24
dst_off	=	28
dst_seg	=	32
count	=	36

iomove:
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp
	movw	dst_seg(%ebp),%es
	mov	dst_off(%ebp),%edi
	mov	src_off(%ebp),%esi
	mov	count(%ebp),%ecx
	jcxz	iomdone			/ 0 ==> done
	rep				/ mult-times...
	smovb				/ move it.
iomdone:				/ return C-style
	jmp	cret

halt:
	sti
	hlt
	jmp	halt

/ *********************************************************************
/
/	getCS
/
/ *********************************************************************

	.globl	getCS			/

getCS:
	xor	%eax, %eax
	movw	%cs,%ax
	ret				/ that's it.

/ *********************************************************************
/
/	getDS
/
/ *********************************************************************

	.globl	getDS			/

getDS:
	xor	%eax, %eax
	movw	%ds,%ax
	ret				/ that's it.

/ *********************************************************************
/
/	getFLAGS
/
/ *********************************************************************

	.globl	getFLAGS

getFLAGS:
	pushf
	pop	%eax
	ret				/ that's it.

/ *********************************************************************
/
/	monitor
/
/ *********************************************************************

	.globl	monitor

monitor:

 	int	$3
	ret

/ *********************************************************************
/
/	cret
/
/ *********************************************************************

cret:
	leave
	pop	%edi
	pop	%esi
	pop	%ebx
	pop	%ecx
	ret

/
/	asm void bzero(sel, addr, count)
/
	.globl bzero
bzero:
	push	%ebp
	movl	%esp, %ebp
sel		=	8
addr	=	sel+4
count	=	addr+4
	push	%es
	push	%edi
	push	%ecx		/ save ecx
	movl	count(%ebp),%ecx
	pushl	sel(%ebp)	/ point es:di at addr
	pop		%es
	movl	addr(%ebp), %edi
	xorl	%eax, %eax	/ zero a string
	repz
	sstob
	popl	%ecx		/ restore ecx
	popl	%edi
	popl	%es
	leave
	ret

/
/	asm void pboot(blbp, code, data)
/
	.globl	pboot
pboot:
	push	%ebp
	movl	%esp, %ebp
blbp	=	8
code	=	blbp+4
data	=	code+4
	push	%ds			/ Pass the blb
	pushl	blbp(%ebp)
	pushl	$0			/ Dummy return address
	pushl	$0
	pushl	code(%ebp)	/ We are going to code:0
	pushl	$0
	pushl	data(%ebp)	/ with ds == data
	pop		%ds
	lret				/ Be there now

/
/	asm void fixup_sel(sel, addr, len, typ)
/
	.globl	fixup_sel
fixup_sel:
	push	%ebp
	movl	%esp, %ebp
sel		=	8
addr	=	sel+4
len		=	addr+4
typ		=	len+4
	push	%es			/ save es and di
	push	%edi
	pushl	$GDTSEL		/ point es:di at GDT entry
	pop		%es
	movl	sel(%ebp), %edi
	movl	len(%ebp), %eax
	stosw
	movl	addr(%ebp), %eax
	stosl
	dec		%edi		/ lose top of addr
	movl	typ(%ebp), %eax
	stosw
	xorl	%eax, %eax	/ assume top of addr was zero
	stosb
	popl	%edi		/ restore es and di
	popl	%es
	leave
	ret
