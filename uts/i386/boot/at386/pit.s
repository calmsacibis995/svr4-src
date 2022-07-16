/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file "pit.s"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft
/	Corporation and should be treated as Confidential.

	.ident	"@(#)boot:boot/at386/pit.s	1.1.2.1"

/
/ Call BIOS wait routine to wait for 1 second; programming the interval
/	timer directly does not seem to be reliable.
/ 	- decreased resolution to cut down on overhead of mode switching
/

	.globl	wait1s
wait1s:
	push	%ebp			/ C entry
	mov	%esp,%ebp
	push	%edi
	push	%esi
	push	%ebx

	call	goreal
	sti

	mov	$0x0f, %ecx
	mov	$0x4240, %edx
	movb	$0x86, %ah		/ setup for bios wait
	int	$0x15			/ BIOS utility function

	mov	$0, %eax

	cli
	data16
	call	goprot

	pop	%ebx
	pop	%esi			/ C exit
	pop	%edi
	pop	%ebp

	ret
