/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

/ Double long add routine.
	.ident	"@(#)kern-io:ladd.s	1.3.1.1"
        .file   "ladd.s"
        .text

        .globl  ladd
        .align  4
	.set	lop,8
	.set	rop,16
	.set	ans,0

ladd:
	movl	lop(%esp),%ecx
	addl	rop(%esp),%ecx
	movl	lop+4(%esp),%edx
	adcl	rop+4(%esp),%edx
	movl	%ecx,ans(%eax)
	movl	%edx,ans+4(%eax)

	ret	$4
