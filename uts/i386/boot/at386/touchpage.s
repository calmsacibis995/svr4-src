/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"touchpage.s"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.

	.ident	"@(#)boot:boot/at386/touchpage.s	1.1.5.2"


/	--------------------------------------------
/	touchpage(p) returns TRUE if the physical address p responds 
/	to a memory probe, FALSE otherwise.

	.globl	touchpage

touchpage:
	pushl	%ebp
	movl	%esp,%ebp
	push	%es			/ save %es

	xorl	%eax, %eax		/ clear out %eax for return value

	movl	$0x08, %ecx		/ set up %es to point to 
	movw	%cx, %es		/ the 'flat' descriptor

	movl	8(%ebp), %ecx		/ grab pointer

	movl	$0xA5, %es:(%ecx)	/ write pattern
	movl	$0, %es:4(%ecx)		/ clear latch
	cmpl	$0xA5, %es:(%ecx)	/ value still there?
	jne	done			/ if not, we're done

	movl	$0x5A, %es:(%ecx)	/ write pattern
	movl	$0, %es:4(%ecx)		/ clear latch
	cmpl	$0x5A, %es:(%ecx)	/ value still there?
	jne	done

	movl	$1, %eax		/ memory is present, return true

done:	
	pop	%es			/ restore %es
	leave	
	ret	
