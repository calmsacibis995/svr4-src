/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.


/ Determine the sign of a double-long number.

	.ident	"@(#)kern-io:lsign.s	1.3.1.1"
        .file   "sign.s"
        .globl  lsign
        .text
        .align  4

lsign:	movl	8(%esp),%eax
	roll	%eax
	andl	$1,%eax

	ret
