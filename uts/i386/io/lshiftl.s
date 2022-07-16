/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.


/ Shift a double long value.

	.ident	"@(#)kern-io:lshiftl.s	1.3.1.1"
        .file   "lshiftl.s"
        .globl  lshiftl
        .text
        .align  4

	.set	arg,12
	.set	cnt,20
	.set	ans,0

lshiftl:	pushl	%eax
	movl	arg(%esp),%eax
	movl	arg+4(%esp),%edx
	movl	cnt(%esp),%ecx
	orl	%ecx,%ecx
	jz	.lshiftld
	jns	.lshiftlp

/ We are doing a negative (right) shift

	negl	%ecx

.lshiftln:
	sarl	$1,%edx
	rcrl	$1,%eax
	loop	.lshiftln
	jmp	.lshiftld

/ We are doing a positive (left) shift

.lshiftlp:
	shll	$1,%eax
	rcll	$1,%edx
	loop	.lshiftlp

/ We are done.

.lshiftld:
	movl	%eax,%ecx
	popl	%eax
	movl	%ecx,ans(%eax)
	movl	%edx,ans+4(%eax)

	ret	$4
