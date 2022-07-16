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
/
/	The _sdget() routine is called from libx/port/sys/sdget.c
/

	.file	"_sdget.s"

	.ident	"@(#)xcplibx:libx/i386/sys/_sdget.s	1.1"


	.globl	_sdget
	.globl	_cerror

_sdget:
	MCOUNT			/ subroutine entry counter if profiling
	movl	$SDGET,%eax
	lcall	SYSCALL
	jc	_cerror
	ret
