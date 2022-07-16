/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.ident	"@(#)kern-ml:cswitch.s	1.3"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft 
/	Corporation and should be treated as Confidential.

	.text

	.align	4
	.globl	setjmp
setjmp:
	movl	4(%esp), %edx		/ address of save area
	movl	%edi, (%edx)
	movl	%esi, 4(%edx)
	movl	%ebx, 8(%edx)
	movl	%ebp, 12(%edx)
	movl	%esp, 16(%edx)
	movl	(%esp), %ecx		/ %eip (return address)
	movl	%ecx, 20(%edx)
	subl	%eax, %eax		/ retval <- 0
	ret

	.align	4
	.globl	longjmp
longjmp:
	movl	4(%esp), %edx		/ address of save area
	movl	(%edx), %edi
	movl	4(%edx), %esi
	movl	8(%edx), %ebx
	movl	12(%edx), %ebp
	movl	16(%edx), %esp
	movl	20(%edx), %ecx		/ %eip (return address)
	movl	$1, %eax
	addl	$4, %esp		/ pop ret adr
	jmp	*%ecx			/ indirect




/	setdscrlim(dscr, limit)
/	struct dscr *dscr;
/	unsigned int limit;
/	{
/		dscr->a_lim0015 = (ushort)limit;
/		dscr->a_lim1619 = (limit>>16)&0x0000000F;
/	}

	.set	dscr, 0x8
	.set	limit, 0xc

	.align	4
	.globl	setdscrlim
setdscrlim:
	pushl	%ebp
	movl	%esp, %ebp

	movl	dscr(%ebp), %eax
        movl	limit(%ebp),%edx
	movw	%dx, (%eax)		/ dscr->a_lim0015
	andl	$0xf0000, %edx
	movl	0x4(%eax), %ecx
	andl	$0xfff0ffff, %ecx
	orl	%ecx, %edx
	movl	%edx, 0x4(%eax)		/ dscr->a_lim1619

	leave  
	ret    
