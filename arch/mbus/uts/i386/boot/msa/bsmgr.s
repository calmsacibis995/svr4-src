/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

/	Copyright (c) 1988  Intel Corporation
/	All Rights Reserved

/	INTEL CORPORATION PROPRIETARY INFORMATION

/	This software is supplied to AT & T under the terms of a license
/	agreement with Intel Corporation and may not be copied nor
/	disclosed except in accordance with the terms of that agreement.

	.file   "bsmgr.s"

	.ident	"@(#)mbus:uts/i386/boot/msa/bsmgr.s	1.3"

/ ********************************************************************
/
/	This is the Bootserver file driver interface to the Unix V/386 R3.1 
/	second stage bootstrap loader for Intel's MBII system with the 
/	first stage executing in protected mode.
/
/ ********************************************************************

	.set	BS_DIB,	0x5963		/ MBII Bootserver DIB - must 
					/ match value in dib.h
	.set	E_DEVICE_TYPE,	0x1A	/ must match the first stage 
					/ error codes in error.h

	.bss
	.globl	file_id
file_id:
	. = . + 4

	.globl	gbuf_offset
gbuf_offset:
	. = . + 4

	.globl	end_of_file
end_of_file:
	. = . + 2
	
	.text

	.globl	bsmgr_copyright
bsmgr_copyright:
	.string  "Copyright 1988 Intel Corporation 462801";

	
/ *********************************************************************
/
/	BL_init
/		Just a stub to satisfy an i/f requirement
/
/ *********************************************************************

	.globl	BL_init

BL_init:
	ret

/ *********************************************************************
/
/	BL_seek
/		Just a stub to satisfy an i/f requirement
/
/ *********************************************************************

	.globl	BL_seek

BL_seek:
	ret

/ *********************************************************************
/
/	BL_file_open
/		This procedure implements the EPS level file open interface.
/		It opens the file, saves the file id for subsequent read calls.
/
/ *********************************************************************

	.globl	BL_file_open

path_offset		= 32
dib_off			= 36
status			= 40

BL_file_open:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	mov	dib_off(%ebp), %ebx
	movw	%ds, %ax
	movw	%ax, %es
	mov	$4, %ecx
	mov	%es:(%ebx, %ecx), %eax
	cmp	$BS_DIB, %eax		/ validate DIB.device_type
	jne	invalid_device
	
	movl	dev_gran, %eax
	movw	%ax, gbuf_offset	/ initialized; nothing has been read
	movw	$0, end_of_file

	mov	path_offset(%ebp), %eax
	push	%ds
	push	%eax
	mov	$file_id, %eax
	push	%ds
	push	%eax
	mov	status(%ebp), %eax
	push	%ds
	push	%eax
	lcall	*bs_file_open_ptr
	jmp	cret

invalid_device:
	mov	status(%ebp), %ebx
	mov	$E_DEVICE_TYPE, %es:(%ebx)	/ return the correct error code
	jmp	cret
	
	
/ *********************************************************************
/
/	bs_file_read
/		This procedure implements the EPS level file read function.
/		The file referenced by the file_id is assumed to be open and
/		data is read sequentially from the file.
/
/ *********************************************************************

	.globl	bs_file_read

buff_offset	= 32
buff_sel	= 36
buff_size	= 40
actual		= 44
status		= 48

bs_file_read:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	mov	file_id, %eax
	push	%eax
	movzwl	buff_sel(%ebp), %eax
	push	%eax
	mov	buff_offset(%ebp), %eax
	push	%eax
	mov	buff_size(%ebp), %eax
	push	%eax
	mov	actual(%ebp), %eax
	push	%ds
	push	%eax
	mov	status(%ebp), %eax
	push	%ds
	push	%eax
	lcall	*bs_file_read_ptr
	jmp	cret

/ *********************************************************************
/
/	BL_file_close
/		This procedure implements the EPS level file close function.
/		The file referenced by file_id is closed.
/
/ *********************************************************************

	.globl	BL_file_close

status		= 32

BL_file_close:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	movw	$1, end_of_file
	mov	file_id, %eax
	push	%eax
	mov	status(%ebp), %eax
	push	%ds
	push	%eax
	lcall	*bs_file_close_ptr
	jmp	cret

/ *********************************************************************
/
/	cret
/
/ *********************************************************************

cret:
	pop	%ebp
	pop	%edi
	pop	%esi
	pop	%ebx
	pop	%ecx
	pop	%ds
	pop	%es
	ret
