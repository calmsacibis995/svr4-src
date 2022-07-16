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

	.file	"waitsem.s"

	.ident	"@(#)xcplibx:i386/sys/waitsem.s	1.1"


	.globl	waitsem
	.globl	_cerror

waitsem:
	MCOUNT			/ subroutine entry counter if profiling
	movl	$WAITSEM,%eax
	lcall	SYSCALL
	jc	_cerror
	ret
