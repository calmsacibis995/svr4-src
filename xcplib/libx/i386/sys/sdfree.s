/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.	  

	.file	"sdfree.s"

	.ident	"@(#)xcplibx:i386/sys/sdfree.s	1.1"


	.globl	sdfree
	.globl	_cerror

sdfree:
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SDFREE,%eax
	lcall	SYSCALL
	jc	_cerror
	ret
