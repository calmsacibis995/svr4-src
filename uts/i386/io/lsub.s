/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.


/ Double long subtraction routine.

	.ident	"@(#)kern-io:lsub.s	1.3.1.1"
        .file   "lsub.s"
        .globl  lsub
        .text
        .align  4

	.set	lop,8
	.set	rop,16
	.set	ans,0

lsub:

	movl	lop(%esp),%ecx
	subl	rop(%esp),%ecx
	movl	lop+4(%esp),%edx
	sbbl	rop+4(%esp),%edx
	movl	%ecx,ans(%eax)
	movl	%edx,ans+4(%eax)

	ret	$4
