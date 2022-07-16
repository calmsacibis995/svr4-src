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

	.file   "bpsmgr.s"

	.ident	"@(#)mbus:uts/i386/boot/msa/bpsmgr.s	1.3"

/ ***************************************************************************
/
/	This is the Bootstrap Parameter String (bps) manager C to PL/M 386
/	interface.  This interface assumes that the BPS manager is implemented
/	through a call gate (entry 50 in the GDT).  It is assumed that the
/	call gate along with the code and data segment descriptor for the BPS
/	manager exist in the caller's GDT environment.
/	
/	All reference to pointers is assumed to be with respect to current 
/	data segment.
/
/	For a description of the BPS manager functions refer to the Bootstrap
/	functional specification.

/ ****************************************************************************

	.set	BP_INIT_CODE,		0
	.set	BP_ADD_CODE,		1
	.set	BP_REMOVE_CODE,		2
	.set	BP_DISPLAY_CODE,	3
	.set	BP_GET_CODE,		4
	.set	BP_TOKEN_CODE,		5
	.set	BP_NUMERIC_CODE,	6
	.set	BPX_INIT_CODE,		7
	.set	BPX_ADD_CODE,		8
	.set	BPX_REMOVE_CODE,	9
	.set	BPX_DISPLAY_CODE,	10
	.set	BPX_GET_CODE,		11
	.set	BPX_DUMP_CODE,		12
	.set	BPX_COPY_CODE,		13
	.set	BP_ASCII_CODE,		14
	.set	BPS_SEL,	 	0x190	/ entry 50 in the GDT


	.data

	.text

	.align	8

bps_mgr_ptr:
	.long	0
	.value	BPS_SEL
	
	
/ *********************************************************************
/
/	BP_init
/
/ *********************************************************************

	.globl	BP_init

BP_init:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp

	mov	%esp, %ebp
	push	$BP_INIT_CODE
	push	$0
	push	$0
	push	$0
	push	$0
	push	$0
	push	$0
	push	$0
	lcall	*bps_mgr_ptr

	jmp	cret


/ *********************************************************************
/
/	BP_add
/
/ *********************************************************************

	.globl	BP_add

param_off	= 32
source		= 36
fail_off	= 40

BP_add:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BP_ADD_CODE
	push	$0
	push	$0
	mov	param_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	fail_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	source(%ebp), %eax
	push	%eax
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BP_remove
/
/ *********************************************************************

	.globl	BP_remove

name_off	= 32
fail_off	= 40

BP_remove:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BP_REMOVE_CODE
	push	$0
	push	$0
	mov	name_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	fail_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BP_display
/
/ *********************************************************************

	.globl	BP_display

index		= 32
name_off	= 36
value_off	= 40

BP_display:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BP_DISPLAY_CODE
	push	$0
	push	$0
	mov	name_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	value_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	index(%ebp), %eax
	push	%eax
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BP_get
/
/ *********************************************************************

	.globl	BP_get

name_off	= 32
value_off	= 36
value_length	= 40

BP_get:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BP_GET_CODE
	push	$0
	push	$0
	mov	name_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	value_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	value_length(%ebp), %eax
	push	%eax
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/ BP_token
/	Note that for the parameter new_string_ptr_ptr, the caller must
/	allocate space for 48-bits because new_string_ptr is a pointer
/	to a position in the string_ptr.
/
/ *********************************************************************

	.globl	BP_token

string_off		= 32
token_off		= 36
new_string_p_off	= 40

BP_token:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BP_TOKEN_CODE
	mov	token_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	new_string_p_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	string_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BP_numeric
/
/ *********************************************************************

	.globl	BP_numeric

string_off	= 32
numeric_off	= 36

BP_numeric:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BP_NUMERIC_CODE
	mov	string_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	push	$0
	mov	numeric_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BP_ascii
/
/ *********************************************************************

	.globl	BP_ascii

numeric_off	= 32
string_off	= 36

BP_ascii:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BP_ASCII_CODE
	mov	numeric_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	push	$0
	mov	string_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BPX_init
/
/ *********************************************************************

	.globl	BPX_init

save_area_off	= 32
save_area_size	= 36

BPX_init:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BPX_INIT_CODE
	mov	save_area_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	push	$0
	push	$0
	push	$0
	mov	save_area_size(%ebp), %eax
	push	%eax
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BPX_add
/
/ *********************************************************************

	.globl	BPX_add

save_area_off	= 32
buffer_info_off	= 36
source		= 40
fail_off	= 44

BPX_add:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BPX_ADD_CODE
	mov	save_area_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	buffer_info_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	fail_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	source(%ebp), %eax
	push	%eax
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BPX_remove
/
/ *********************************************************************

	.globl	BPX_remove

save_area_off	= 32
name_off	= 36
fail_off	= 40

BPX_remove:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BPX_REMOVE_CODE
	mov	save_area_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	name_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	fail_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BPX_display
/
/ *********************************************************************

	.globl	BPX_display

save_area	= 32
index		= 36
name_off	= 40
value_off	= 44

BPX_display:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BPX_DISPLAY_CODE
	mov	save_area(%ebp), %eax
	push	%eax
	mov	name_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	value_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	index(%ebp), %eax
	push	%eax
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BPX_get
/
/ *********************************************************************

	.globl	BPX_get

save_area_off	= 32
name_off	= 36
value_off	= 40
value_length	= 44

BPX_get:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BPX_GET_CODE
	mov	save_area_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	name_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	value_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	value_length(%ebp), %eax
	push	%eax
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BPX_dump
/
/ *********************************************************************

	.globl	BPX_dump

save_area_off		= 32
text_buffer_off		= 36
buffer_size		= 40

BPX_dump:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BPX_DUMP_CODE
	mov	save_area_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	push	$0
	mov	text_buffer_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	buffer_size(%ebp), %eax
	push	%eax
	lcall	*bps_mgr_ptr

	jmp	cret

/ *********************************************************************
/
/	BPX_copy
/
/ *********************************************************************

	.globl	BPX_copy

save_area_off		= 32
new_save_area_off	= 36
new_size		= 40

BPX_copy:
	push	%es
	push	%ds
	push	%ecx
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	mov	%esp,%ebp

	push	$BPX_COPY_CODE
	mov	save_area_off(%ebp), %eax
	push	%ds
	push	%eax
	push	$0
	push	$0
	mov	new_save_area_off(%ebp), %eax
	push	%ds
	push	%eax
	mov	new_size(%ebp), %eax
	push	%eax
	lcall	*bps_mgr_ptr

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
