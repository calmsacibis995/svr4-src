/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file   "msa.s"

/	Copyright (c) 1988  Intel Corporation
/	All Rights Reserved

/	INTEL CORPORATION PROPRIETARY INFORMATION

/	This software is supplied to AT & T under the terms of a license
/	agreement with Intel Corporation and may not be copied nor
/	disclosed except in accordance with the terms of that agreement.

	.ident	"@(#)mb1:uts/i386/boot/mb1/msa.s	1.3"
#include "../sys/prot.h"

/
/	This is the far-call interface to the drivers.
/

/
/	void disk_init(unit_id, flag, devinfo, status)
/	POINTER unit_id;
/	ulong flag;						/* 1 => hunt mode */
/	POINTER devinfo;
/	POINTER status;
/
	.globl disk_init
disk_init:
	lret	$8+4+8+8

/
/	void disk_read(stat, act, buff, blks, blk)
/	POINTER stat;
/	POINTER act;
/	POINTER buff;
/	ulong blks;
/	ulong blk;
/
	.globl	disk_read
disk_read:
	pushl	%ebp
	movl	%esp, %ebp
stat	= 12
act		= stat+8
buff	= act+8
blks	= buff+8
blk		= blks+4
	push	%ds
	movw	$DATADESC, %ax
	movw	%ax, %ds
	push	stat+4(%ebp)
	push	stat(%ebp)
	push	act+4(%ebp)
	push	act(%ebp)
	push	buff+4(%ebp)
	push	buff(%ebp)
	push	blks(%ebp)
	push	blk(%ebp)
	call	dread
	add		$4+4+8+8+8, %esp
	pop		%ds
	leave
	lret	$4+4+8+8+8

/
/	void tape_init (status, devinfo, flag, unit_id)
/	POINTER unit_id;
/	ulong flag;
/	POINTER devinfo;
/	POINTER status;
/
	.globl tape_init
tape_init:
	lret	$8+4+8+8

/
/	void tape_read (stat, act, buff, blks)
/	POINTER stat;
/	POINTER act;
/	POINTER buff;
/	ulong blks;
/
	.globl	tape_read
tape_read:
	pushl	%ebp
	movl	%esp, %ebp
stat	= 12
act		= stat+8
buff	= act+8
blks	= buff+8
	push	%ds
	movw	$DATADESC, %ax
	movw	%ax, %ds
	push	stat+4(%ebp)
	push	stat(%ebp)
	push	act+4(%ebp)
	push	act(%ebp)
	push	buff+4(%ebp)
	push	buff(%ebp)
	push	blks(%ebp)
	call	tread
	add		$4+8+8+8, %esp
	pop		%ds
	leave
	lret	$4+8+8+8

/
/	void tape_rewind(stat)
/	POINTER stat;
/
	.globl	tape_rewind
tape_rewind:
	pushl	%ebp
	movl	%esp, %ebp
stat	= 12
	push	%ds
	movw	$DATADESC, %ax
	movw	%ax, %ds
	push	stat+4(%ebp)
	push	stat(%ebp)
	call	trewind
	add		$8, %esp
	pop		%ds
	leave
	lret	$8

/
/	void tape_read_fm(stat)
/	POINTER stat;
/
	.globl	tape_read_fm
tape_read_fm:
	pushl	%ebp
	movl	%esp, %ebp
stat	= 12
	push	%ds
	movw	$DATADESC, %ax
	movw	%ax, %ds
	push	stat+4(%ebp)
	push	stat(%ebp)
	call	tread_fm
	add		$8, %esp
	pop		%ds
	leave
	lret	$8

/
/	void error(msg, sev, stage)
/	POINTER msg;
/	ulong sev;
/	ulong stage;
/
	.globl	error
error:
	pushl	%ebp
	movl	%esp, %ebp
msg		= 12
sev		= msg+8
stage	= sev+4
	push	%ds
	movw	$DATADESC, %ax
	movw	%ax, %ds
	push	stage(%ebp)
	push	sev(%ebp)
	push	msg+4(%ebp)
	push	msg(%ebp)
	call	cmessage
	add		$8+4+4, %esp
	pop		%ds
	leave
	lret	$8+4+4
