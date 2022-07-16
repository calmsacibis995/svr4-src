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

	.file   "ics.s"

	.ident	"@(#)mbus:uts/i386/boot/msa/ics.s	1.3"

/ ********************************************************************
/
/	This is the Interconnect services module for the second stage 
/	bootstrap loader
/
/ ********************************************************************

	.set	LOCAL_SLOT,		0xF800
	.set	REGISTER_SHIFT,		0x2
	.set	ICS_HI_ADDR,		0x34
	.set	ICS_LO_ADDR,		0x30
	.set	ICS_DATA_ADDR,		0x3C
	.set	MAX_IC_REG,		512
	.set	INVALID_HOST_ID,	0xFFFF
	.set	END_OF_TEMPLATE,	0xFF
	.set	END_OF_IC_HEADER,	0x20
	.set	HOST_ID_REC_TYPE,	0x10
	.set	MEM_PAR_REC_TYPE,	0x04

	.text
	.globl get_host_id
	.globl ic_write

/ *********************************************************************
/
/	ic_read
/		Reads the interconnect space register
/
/ *********************************************************************

register	= 12

ic_read:
	push	%edx
	push	%ebp
	mov	%esp,%ebp

	mov	register(%ebp), %eax
	cmpw	$MAX_IC_REG, %ax
	jle	valid_reg
	mov	$0, %eax
	jmp	quit_read

valid_reg:
	shlw	$REGISTER_SHIFT, %ax
	orw	$LOCAL_SLOT, %ax
	xchgb	%ah, %al
	movw	$ICS_HI_ADDR, %dx
	outb	(%dx)
	xchgb	%al, %ah
	movw	$ICS_LO_ADDR, %dx
	outb	(%dx)
	movw	$ICS_DATA_ADDR, %dx
	xor	%eax, %eax		/ clear it for input
	inb	(%dx)

quit_read:
	pop	%ebp
	pop	%edx
	ret

/ *********************************************************************
/
/	get_host_id
/		Returns the host id from interconnect space
/
/ *********************************************************************

rec_type	= -4
index		= -8
host_id_val	= -12

get_host_id:
	enter 	$16, $0

	movw	$INVALID_HOST_ID, host_id_val(%ebp)
	movw	$END_OF_IC_HEADER, index(%ebp)
	movw	index(%ebp), %ax

search:
	push	%eax
	call	ic_read			/ get the record type
	add	$4, %esp		/ pop the parameter
	movb	%al, rec_type(%ebp)

	cmpb	$END_OF_TEMPLATE, %al	/ != EOT
	je	eot_reached
	cmpb	$HOST_ID_REC_TYPE, %al	/ && != HOST_ID_REC_TYPE
	je	record_found
	movw	index(%ebp), %ax
	incw	%ax
	push	%eax
	call	ic_read			/ get the size of this record
	add	$4, %esp		/ pop the parameter
	addw	$2, %ax
	addw	index(%ebp), %ax
	movw	%ax, index(%ebp)	/ skip to next record 
	jmp	search
	
record_found:
	movw	index(%ebp), %ax
	addw 	$2, %ax 		/ host id low byte
	push	%eax
	call	ic_read
	add	$4, %esp		/ pop the parameter
	movw	%ax, %bx
	movw	index(%ebp), %ax
	addw	$3, %ax			/ host id high byte
	push	%eax 
	call	ic_read
	add	$4, %esp		/ pop the parameter
	movb	%al, %bh
	movw	%bx, host_id_val(%ebp) / that's the host id

eot_reached:
	movw	host_id_val(%ebp), %ax
	leave
	ret

/ *********************************************************************
/
/	ic_write
/		Writes to the on-board interconnect space register
/
/ *********************************************************************

register	= 12
value		= 16

ic_write:
	push	%edx
	push	%ebp
	mov	%esp,%ebp

	mov	register(%ebp), %eax
	cmpw	$MAX_IC_REG, %ax
	jle	valid_wreg
	mov	$0, %eax
	jmp	quit_write

valid_wreg:
	shlw	$REGISTER_SHIFT, %ax
	orw	$LOCAL_SLOT, %ax
	xchgb	%ah, %al
	movw	$ICS_HI_ADDR, %dx
	outb	(%dx)
	xchgb	%al, %ah
	movw	$ICS_LO_ADDR, %dx
	outb	(%dx)
	movw	$ICS_DATA_ADDR, %dx
	xor	%eax, %eax		/ clear it 
	movb	value(%ebp), %al
	outb	(%dx)

quit_write:
	pop	%ebp
	pop	%edx
	ret
