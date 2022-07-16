/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"trans.s"

	.ident	"@(#)kern-fp:trans.s	1.1"
/ *****************************************************************************
/
/			t r a n s . m o d
/                       =================
/
/	===============================================================
/               intel corporation proprietary information
/    this software  is  supplied  under  the  terms  of  a  license
/    agreement  or non-disclosure agreement  with intel corporation
/    and  may not be copied nor disclosed except in accordance with
/    the terms of that agreement.                                  
/	===============================================================
/
/	public procedures:
/		arctan, exp, log, splog, tan, move_10_bytes
/	move_constant, test_5w, ,test_6w, test_4w, test_3w, clear_5w, clear_6w
/		set_5w, set_6w,set_4w, set_3w, left_shift_result_cl
/		left_shift_frac1_cl, left_shift_frac2_cl
/		right_shift_result_cl,right_shift_frac1_cl
/		right_shift_frac2_cl,right_shift,left_shift
/		sticky_right_shift,addition_normalize
/		subtraction_normalize,one_left_normalize
/		gradual_underflow,left_shift_frac1_1,sin,cos,sincos
/
/ *****************************************************************************
/
/
/...october 16, 1986...
/
/$nolist
#include	"e80387.h"
/$list
	.text	/a_med	segment	er	public
/
/	extrn	put_si_result,do_exchange,addx,subx
/	extrn	divx,divid,round,underflow_response
/	extrn	log_divx,decompose,mult,mulx
/	extrn	overflow_response,sp_subadd,stack_full_
/	extrn	pop_free,put_op1_result,set_p_error
/	extrn	set_u_error,get_precision,do_exchange_leave_a_bit
/
	.globl	two_arg_trans
	.globl	exp
	.globl	log
	.globl	splog
	.globl	move_10_bytes
	.globl	tan
	.globl	move_constant
	.globl	test_5w
	.globl	test_6w
	.globl	test_4w
	.globl	test_3w
	.globl	clear_5w
	.globl	clear_6w
	.globl	set_5w
	.globl	set_6w
	.globl	set_4w
	.globl	set_3w
	.globl	left_shift_frac1_cl
	.globl	left_shift_result_cl
	.globl	left_shift_frac2_cl
	.globl	right_shift_frac1_cl
	.globl	right_shift_frac2_cl
	.globl	right_shift_result_cl
	.globl	right_shift
	.globl	left_shift
	.globl	gradual_underflow
	.globl	addition_normalize
	.globl	sticky_right_shift
	.globl	one_left_normalize
	.globl	subtraction_normalize
	.globl	left_shift_frac1_1
	.globl	sin
	.globl	cos
	.globl	sincos
/
/
add_half_pattern:
	.value	0x80,0,0,0,0
add_half_pattern_1:
	.value	0x8000,0,0,0,0
constant_ln2_ov_6:
	.value	0,0,0,0x1ff5,0x0ec98
constant_2_ov_ln2:
	.value	0x0bbc0,0x17f0,0x295c,0x0aa3b,0x00b8
constant_one:
	.value	0,0,0,0,0x100
constant_six:
	.value	0,0,0,0,0x0c000
constant_sqrt_of_2:
	.value	0x8400,0x0de64,0x33f9,0x04f3,0x0b5
constant_3_ov_ln2:
	.value	0,0x0f48d,0x0511,0x0ac5f,0x8a7f
constant_one_1:
	.value	0,0,0,0,0x8000
/
/ this constant has been decreased by 1 to compensate
/ for the bits lost during right shift.
/
constant_c0h:
	.value	0x0ff00,0x0ffff,0x0ffff,0x0ffff,0x0bf
/
log_constant:
	.value	0x09fa0,0x0d687,0x039fb,0x0c01a,0x095	/1
	.value	0x0f260,0x0dc57,0x05e68,0x0d3c2,0x0a4	/2
	.value	0x0fd20,0x0b43c,0x0cfde,0x000d1,0x0ae	/3
	.value	0x0e680,0x098b3,0x0d648,0x01fb7,0x0b3	/4
	.value	0x089c0,0x0ec39,0x0ac77,0x0d69b,0x0b5	/5
	.value	0x05380,0x0914c,0x02e16,0x03cb4,0x0b7	/6
	.value	0x0fc80,0x0428b,0x0b778,0x0f285,0x0b7	/7
	.value	0x056e0,0x063ba,0x06bd5,0x04e23,0x0b8	/8
	.value	0x031e0,0x0ab26,0x0f853,0x07c1f,0x0b8	/9
	.value	0x0fd60,0x0a2a0,0x0ba1f,0x09329,0x0b8	/a
	.value	0x057c0,0x0be18,0x07bca,0x09eb1,0x0b8	/b
	.value	0x070a0,0x0fe44,0x0150d,0x0a476,0x0b8	/c
	.value	0x0aa60,0x09b1b,0x08fd2,0x0a758,0x0b8	/d
	.value	0x09460,0x09ae9,0x0d8be,0x0a8c9,0x0b8	/e
	.value	0x03380,0x02572,0x08017,0x0a982,0x0b8	/f
/
tan_constant:
	.value	0x034c0,0x068c2,0x0a221,0x00fda,0x0c9	/0
	.value	0x04580,0x0da7b,0x02b0d,0x06338,0x0ed	/1
	.value	0x01560,0x006eb,0x0c964,0x0dbaf,0x0fa	/2
	.value	0x032c0,0x07b6e,0x0d561,0x0add4,0x0fe	/3
	.value	0x036c0,0x0ef4e,0x0b967,0x0aadd,0x0ff	/4
	.value	0x04280,0x0b125,0x0dd4b,0x0eaad,0x0ff	/5
	.value	0x0bbe0,0x094d5,0x0dddb,0x0faaa,0x0ff	/6
	.value	0x06800,0x0d4b9,0x0addd,0x0feaa,0x0ff	/7
	.value	0x04bc0,0x0ddb9,0x0aadd,0x0ffaa,0x0ff	/8
	.value	0x04ba0,0x0dddd,0x0aaad,0x0ffea,0x0ff	/9
	.value	0x0dba0,0x0dddd,0x0aaaa,0x0fffa,0x0ff	/a
	.value	0x0dde0,0x0addd,0x0aaaa,0x0fffe,0x0ff	/b
	.value	0x0dde0,0x0aadd,0x0aaaa,0x0ffff,0x0ff	/c
	.value	0x0dde0,0x0aaad,0x0eaaa,0x0ffff,0x0ff	/d
	.value	0x0dde0,0x0aaaa,0x0faaa,0x0ffff,0x0ff	/e
	.value	0x0ade0,0x0aaaa,0x0feaa,0x0ffff,0x0ff	/f
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
at_case_table:
	.long	at_round,case_1,case_2,case_3
/
two_arg_trans_table:
	.long	at_non_0,log,splog
/
/...the following jump tables refer to the following op1/op2 cases 
/
/   (where v=valid, z=zero, f=infinity, and d=denormal):
/		v/v,v/z,v/f,v/d,z/v,z/z,z/f,z/d,f/v,f/z,f/f,f/d,d/v,d/z,d/f and d/d,
/	 in that order ..
/
/ arctan2_table is first
/
special_table:
	.long	handle_non_special_cases,sign_y_hapi
	.long	sign_y_pi_or_0,only_op2_denormd
	.long	sign_y_pi_or_0,at_both_0
	.long	sign_y_pi_or_0,derror_sign_y_pi_or_0
	.long	sign_y_hapi,sign_y_hapi
	.long	handle_non_special_cases,derror_sign_y_hapi
	.long	only_op1_denormd,derror_sign_y_hapi
	.long	derror_sign_y_pi_or_0,both_ops_denormd
/
/ log_table is second, 64 bytes after arctan2_table
/
	.long	handle_non_special_cases,op2_signed_by_op1_vs_1
	.long	inv_or_op2_signed_by_op1_vs_1,only_op2_denormd
	.long	zerror_flip_sign_y_inf,op_unsupp
	.long	zerror_flip_sign_op2,zerror_flip_sign_y_inf
	.long	sign_y_op1,op_unsupp
	.long	op2,derror_sign_y_op1
	.long	only_op1_denormd,derror_flip_sign_op2
	.long	derror_flip_sign_op2,both_ops_denormd
/
/ splog_table is third (and last), 128 bytes after arctan2_table
/
	.long	handle_non_special_cases,exor_signed_op2
	.long	exor_signed_op2,only_op2_denormd
	.long	exor_signed_op1,exor_signed_op2
	.long	op_unsupp,derror_exor_signed_op1
	.long	sign_y_op1,op_unsupp
	.long	op2,derror_sign_y_op1
	.long	only_op1_denormd,derror_exor_signed_op2
	.long	derror_exor_signed_op2,both_ops_denormd
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			atan_core:
/			"""""""""
/
/	function:
/		calculates arc tangent of (y/x), where 
/		0 < y/x < 1 ;  op2 = x, op1 = y.
/
/	inputs:
/		assumes that operand1 and operand2 are set up.
/
/	outputs:
/		result in global record 12; possible underflow
/		and inexact errors.
/
/	data accessed:
/		- expon1		word_frac1
/		- lsb_frac1		msb_frac1
/		- offset_operand1		expon2
/		- lsb_frac2		offset_operand2
/		- offset_result		result_sign
/		- result_tag		result_expon
/		- result_word_frac	msb_result
/		- offset_of_result_frac	offset_cop
/		- offset_dop		siso
/
/	data changed:
/		- expon1		frac1
/		- frac2			result_sign
/		- result_expon		result_frac
/		- siso
/
/	procedures called:
/		put_si_result		right_shift		left_shift
/		move_10_bytes		move_constant		addx
/subx			mulx			accel_divx (replaces divx)
/		divid			one_left_normalize	round
/		addition_normalize	underflow_response	pop_free
/		test_5w			set_p_error		set_u_error
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
atan_core:	/proc
	push	%gs:sr_masks			/ save precision control
/	and	dword ptr %gs:sr_masks, 0fffff3ffh; set rounding control to nearest
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
	orb	$inexact_mask,%gs:sr_errors	/ always inexact
	movb	positive,result_sign(%ebp)/ result is always positive
	movw	$0,siso					/ clear siso
	mov	dword_expon2,%eax
	sub	dword_expon1,%eax
	mov	%eax,dword_exp_tmp
	cmp	$15,%eax
	jle	atn_pseudo_div
	cmp	$63,%eax
/
/tiny argument
/
	jg	atn_divide
	jmp	atn_rat_appx
atn_divide:
	call	divid				/ floating-point divide
/
/post operation rounding
/
	call	post_op_round		/ d.e. precision rounding
/
/error check
/
	mov	dword_result_expon,%eax
	cmp	$0,%eax
	jg	atn_put_atn_f
	jl	atn_put_atn		/ underflow of core has occurred.
	mov	$offset_result+frac64,%edi
	call	test_4w				/ note that ax is 0 here.
	jnz	atn_put_atn		/ underflow of core has occurred.
atn_zero:
	movb	special,result_tag(%ebp)
	jmp	atn_put_atn
/
atn_pseudo_div:
	call	right_shift_frac2_8
	call	right_shift_frac1_8
atn_loop_back_1:
	call	subx
	shlw	$1,siso
	cmpb	$0,msb_result
	jnz	atn_shift_left
	call	move_result_cop		/ cop is 67 bits temp
	mov	$offset_operand1,%edi
	call	right_shift_dword_exp_tmp
	call	addx
	call	move_result_frac1
	mov	$add_half_pattern,%esi
	call	add_frac2			/ add 1/2 round
	call	move_result_frac2
	and	$0x0ff000000,dword_frac2(%ebp)	/ frac2 = 64 bits x
	call	move_cop_frac1							/ frac1 = 67 bits y
	incw	siso
	cmpb	$0,msb_result
	je	atn_shift_left
	mov	$1,%ecx
	call	right_shift_frac2_cl
	and	$0x0ff000000,dword_frac2(%ebp)
	jmp	atn_merge_0
atn_shift_left:
	call	left_shift_frac1_1
atn_merge_0:
	inc	dword_exp_tmp
	cmp	$15,dword_exp_tmp
	jna	atn_loop_back_1
	mov	$7,%ecx
	call	left_shift_frac1_cl
	and	$0x0e0000000,dword_frac1(%ebp)
	mov	$15,dword_exp_tmp
	call	left_shift_frac2_8
atn_rat_appx:
/ the following call used to be to divx instead of accel_divx:
	call	accel_divx				/ y/x --> z (67)
	call	move_result_frac1
	mov	$9,%ecx
	call	right_shift_frac1_cl
	and	$0x0ffe00000,dword_frac1(%ebp)
	call	move_result_frac2
	call	right_shift_frac2_8
	and	$0x0ffe00000,dword_frac2(%ebp)
	and	$0x0e0000000,result_dword_frac(%ebp)/ mask result frac
	call	move_result_cop
	call	addx
	call	move_result_dop		/dop = new y (67)
	call	move_cop_frac1
	call	move_cop_frac2
	mov	$30,%ecx
	call	right_shift_frac1_cl	/ perform short multiply
	mov	$0,dword_frac1(%ebp)
	mov	$30,%ecx
	call	left_shift_frac1_cl
	call	mulx
	mov	$offset_result,%edi
	call	right_shift_dword_exp_tmp
	movb	$0x0c0,msb_result
	call	move_result_frac2		/ new x (64)
	and	$0x0e0000000,dword_frac2(%ebp)
	cmpb	$0,msb_dop
	movb	$1,%al
	mov	$7,%ecx
	jne	atn_merge_1
	movb	$0,%al
	mov	$8,%ecx
atn_merge_1:
	push	%eax				/save al
	mov	$offset_dop+frac80,%edi
	call	left_shift
	call	move_dop_frac1
/ the following call used to be to divx instead of accel_divx:
	call	accel_divx
	and	$0x0e0000000,result_dword_frac(%ebp)/ theta is here
	pop	%eax
	shrb	$1,%al
	mov	$7,%ecx
	jnb	atn_merge_2
	mov	$6,%ecx
atn_merge_2:
	call	right_shift_result_cl
	call	move_result_frac1
atn_loop_back_2:
	shrw	$1,siso
	jnb	atn_q_bit_not_set
	mov	$10,%eax
	mul	dword_exp_tmp
	add	$tan_constant,%eax
	mov	%eax,%esi
	call	add_frac2
	call	move_result_frac1
atn_q_bit_not_set:
	cmpw	$0,siso
	jz	atn_end_loop
	mov	$1,%ecx
	call	right_shift_frac1_cl
	and	$0x0ffe00000,dword_frac1(%ebp)
	dec	dword_exp_tmp
	jmp	atn_loop_back_2
atn_end_loop:
	cmpb	$0,msb_frac1
	mov	$8,%ecx
	je	atn_merge_3
	mov	$7,%ecx
	dec	dword_exp_tmp
atn_merge_3:
	call	left_shift_frac1_cl
	mov	$0x3ffe,dword_expon1
	mov	dword_exp_tmp,%eax
	sub	%eax,dword_expon1
	mov	$offset_operand1,%edi
	call	one_left_normalize
	call	move_frac1_result/ added when add_half1_frac below was cut
	mov	dword_expon1,%eax
	mov	%eax,dword_result_expon
/		call	add_half1_frac2
atn_put_atn_f:
	movb	valid,result_tag(%ebp)	/ valid result
atn_put_atn:
	pop	%gs:sr_masks			/ restore precision control
	ret
/atan_core	endp
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
move_unpacked_0_to_edi:	/proc

	mov	$0,(%ebp,%edi)
	mov	$0,frac64(%ebp,%edi)
	mov	$0,frac32(%ebp,%edi)
	mov	$0x10000\*SPECIAL+POSITIVE,sign(%ebp,%edi)
	mov	$0,expon(%ebp,%edi)
	ret

/move_unpacked_0_to_edi	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
move_unpacked_hapi_to_edi:	/proc

	mov	$0,(%ebp,%edi)
	mov	$0x2168c235,frac64(%ebp,%edi)
	mov	$0x0c90fdaa2,frac32(%ebp,%edi)
	mov	$0x10000\*VALID+POSITIVE,sign(%ebp,%edi)
	mov	$0x3fff,expon(%ebp,%edi)
	ret

/move_unpacked_hapi_to_edi	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

at_non_0:	/proc

	movb	sign1(%ebp),%dl
	movb	sign2(%ebp),%dh
	movb	positive,sign1(%ebp)
	movb	positive,sign2(%ebp)
	movb	%dh,%bl
	and	$8,%ebx
at_cmpr_ops:				/ this compare assumes same-
															/ signed, normalized comparands
	mov	dword_expon2,%eax
	cmp	dword_expon1,%eax
	jg	take_atan
	jl	at_switch_ops
	mov	dword_frac1+frac32(%ebp),%eax
	cmp	dword_frac2+frac32(%ebp),%eax
	jg	take_atan
	jl	at_switch_ops
	mov	dword_frac1+frac64(%ebp),%eax
	cmp	dword_frac2+frac64(%ebp),%eax
	jg	take_atan
	jl	at_switch_ops
at_ops_eql:
	mov	$offset_result,%edi
	call	move_unpacked_hapi_to_edi
	dec	dword_result_expon				/ make pi/4
	or	%ebx,%ebx
	jz	at_sign_adjust
	jmp	hapi_to_op1
at_switch_ops:
	push	%ebx
	call	swap_ops						/ edx preserved
	pop	%ebx
	or	$4,%ebx
take_atan:
	push	%ebx								/ save operand signs
	push	%edx								/ save table index
	call	atan_core
	pop	%edx
	pop	%ebx
	or	%ebx,%ebx
	jnz	hapi_to_op1
at_underflow_:
	mov	dword_result_expon,%eax
	cmp	$0,%eax
	jg	at_round
	jl	at_underflow
	cmpb	special,result_tag(%ebp)
	je	at_round_report
at_underflow:							/ underflow
	push	%edx						/ save sign info
	call	underflow_response
	pop	%edx					/ restore sign info
	orb	underflow_mask,%gs:sr_errors
	jmp	at_sign_adjust
hapi_to_op1:
	mov	$offset_operand1,%edi
	call	move_unpacked_hapi_to_edi
	jmp	*%cs:at_case_table(%ebx)
/
case_3:
	movb	$add_op,operation_type(%ebp)
	jmp	at_to_op2
case_2:
	inc	dword_expon1					/ make pi
case_1:
	movb	$sub_op,operation_type(%ebp)
at_to_op2:
	push	%edx
/	push	ebx
	lea	dword_frac2(%ebp),%edi
	lea	result_dword_frac(%ebp),%esi
	call	move_op_to_op
	call	subadd
/	pop		ebx
	pop	%edx
at_round:
	push	%gs:sr_masks
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
	push	%edx
	call	post_op_round
	pop	%edx
	pop	%gs:sr_masks
at_round_report:
	cmpw	$0,rnd_history
	je	at_sign_adjust
	orb	$inexact_mask,%gs:sr_errors
	cmpb	true,added_one
	jne	at_sign_adjust
	orb	$a_mask,%gs:sr_flags
at_sign_adjust:
	movb	%dl,result_sign(%ebp)
at_exit:
	ret

/at_non_0	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
/
two_arg_trans:	/proc

	jz	two_arg_special_case_	/ branch if no stack error
	call	set_stk_u_error			/ stack underflow occurred
	testb	invalid_mask,%gs:sr_masks/ if unmasked, just return
	jnz	return_indef			/ else, return indefinite
two_arg_unmasked_exit:
	ret
return_indef:
	call	put_indefinite
	jmp	get_result_ptr
two_arg_special_case_:
	andb	$ -1 ! a_mask,%gs:sr_flags		/ clear a-bit initially
	movzbl	tag1(%ebp),%ebx		/ both operands valid?
	orb	tag2(%ebp),%bl
	jnz	two_arg_special_case
handle_non_special_cases:
	movzbl	operation_type(%ebp),%ebx/ call log/splog/arctan2
	subb	$arctan_op,%bl			/ to do the operation
	je	main_call		/ ebx has offset of at_non_0 container
	cmpb	$log_op-arctan_op,%bl
	je	log_of_neg_num_
	cmp	$0x3fff,dword_expon1
	jg	op_unsupp
	jl	get_splog_offset
weed_out_neg_1:
	cmpb	positive,sign1(%ebp)
	je	op_unsupp
	cmp	$0x080000000,dword_frac1+frac32(%ebp)
	jne	op_unsupp
	cmp	$0,dword_frac1+frac64(%ebp)
	jne	op_unsupp
	jmp	zerror_flip_sign_y_inf
get_splog_offset:
	mov	$8,%ebx			/ ebx has offset of splog container
	jmp	main_call
log_of_neg_num_:
	cmpb	positive,sign1(%ebp)
	jne	op_unsupp
	mov	$4,%ebx		/ ebx has offset of log container
main_call:
	call	*%cs:two_arg_trans_table(%ebx)
get_result_ptr:
	mov	$offset_result,%edi
got_result_ptr:
	mov	offset_result_rec,%esi
	jmp	put_arith_result
/
both_ops_denormd:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
	mov	$offset_operand1,%edi
	call	norm_denorm
op2_denormd:
	mov	$offset_operand2,%edi
	call	norm_denorm
	jmp	handle_non_special_cases
only_op1_denormd:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
	mov	$offset_operand1,%edi
	call	norm_denorm
	jmp	handle_non_special_cases
only_op2_denormd:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
	jmp	op2_denormd
/
two_arg_special_case:
	testb	$0x10,%bl		/ al contains [ebp].tag1 or [ebp].tag2
	jz	op1_nan_
op_unsupp:
	orb	invalid_mask,%gs:sr_errors/ here, at least one op is unsupported.
	testb	invalid_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
	jmp	return_indef
op1_nan_:				/ here, neither op is unsupported.
	cmpb	inv,tag1(%ebp)
	je	op1_snan_
	cmpb	inv,tag2(%ebp)
	jne	non_nan_supp_ops
	jmp	signal_invalid_
op1_snan_:
	testb	$0x40,msb_frac1
	jz	invalid_operand
op2_also_nan_:
	cmpb	inv,tag2(%ebp)
	jne	jmp_set_up_nan_return
signal_invalid_:
	testb	$0x40,msb_frac2
	jnz	jmp_set_up_nan_return
invalid_operand:
	orb	invalid_mask,%gs:sr_errors/ invalid operand sets
	testb	invalid_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit	/ i-error, if unmasked
jmp_set_up_nan_return:
	jmp	set_up_nan_return
non_nan_supp_ops:
	movb	tag1(%ebp),%bh
	movzbl	operation_type(%ebp),%eax
	subb	$arctan_op,%al
	je	get_tag2				/ eax now holds base of arctan table
	cmpb	$log_op-arctan_op,%al
	je	only_neg_0_ok
	cmp	$0x3fff,dword_expon1
	jl	get_splog_table
	je	weed_out_neg_1
	cmp	$0x7fff,dword_expon1
	jne	op_unsupp
	cmpb	positive,sign1(%ebp)
	jne	op_unsupp
get_splog_table:
	mov	$128,%eax				/ eax now holds base of splog table
	jmp	get_tag2
only_neg_0_ok:
	cmpb	positive,sign1(%ebp)
	je	log_table_base_to_eax
	cmpb	special,%bh
	jne	op_unsupp
log_table_base_to_eax:
	mov	$64,%eax		/ eax now holds base of log table
get_tag2:
	movb	tag2(%ebp),%bl
	testb	$0x04,%bh
	jz	op2_denorm_
	movb	$3,%bh				/ indicate op1 is denorm
op2_denorm_:
	testb	$0x04,%bl
	jz	get_index
	movb	$3,%bl				/ indicate op2 is denorm
get_index:
	and	$0x0303,%ebx		/ form index to special operation table
															/ bx=4*(4*masked_tag1 + masked_tag2),
					/ where masked_tag =	0 for valid,
	shlb	$2,%bh			/1 for zero,
															/2 for infinity
	addb	%bh,%bl			/3 for denormd
	xorb	%bh,%bh
	shl	$2,%ebx
add_base:
	add	%eax,%ebx			/ (e)bx = case offset
	jmp	*%cs:special_table(%ebx)	/ jump to special case
						/
derror_sign_y_pi_or_0:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit		/ if unmasked, return
sign_y_pi_or_0:
	cmpb	positive,sign2(%ebp)
	je	sign_y_0
sign_y_pi:
	mov	$offset_result,%edi
	call	move_unpacked_hapi_to_edi
	inc	dword_result_expon			/ make pi
	jmp	at_spcl_sign_adjust

derror_sign_y_hapi:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit		/ if unmasked, return
sign_y_hapi:
	mov	$offset_result,%edi
	call	move_unpacked_hapi_to_edi
	jmp	at_spcl_sign_adjust

at_both_0:
	cmpb	positive,sign2(%ebp)
	jne	sign_y_pi
sign_y_0:
	mov	$offset_result,%edi
	call	move_unpacked_0_to_edi

at_spcl_sign_adjust:
	movb	sign1(%ebp),%dl
	movb	%dl,result_sign(%ebp)
	jmp	get_result_ptr

zerror_flip_sign_y_inf:
	orb	zero_divide_mask,%gs:sr_errors
	testb	zero_divide_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
flip_sign_y_inf:
	mov	$0x080000000,dword_frac1+frac32(%ebp)
	mov	$0x10000\*INFINITY+POSITIVE,sign1(%ebp)
	mov	$0x7fff,dword_expon1
flip_sign_y_op1:
	mov	$offset_operand1,%edi
flip_sign_y_to_edi:
	movb	sign2(%ebp),%dl
	notb	%dl
	movb	%dl,sign(%ebp,%edi)
	jmp	got_result_ptr

/zerror_derror_flip_sign_y_inf:
/		or		%gs:sr_errors,	zero_divide_mask
/		test	%gs:sr_masks,	zero_divide_mask
/		jz		two_arg_unmasked_exit
/		or		%gs:sr_errors,	denorm_mask
/		test	%gs:sr_masks,	denorm_mask
/		jz		two_arg_unmasked_exit
/		jmp	short	flip_sign_y_inf

derror_sign_y_op1:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
sign_y_op1:
	mov	$offset_operand1,%edi
sign_y_to_edi:
	movb	sign2(%ebp),%dl
	movb	%dl,sign(%ebp,%edi)
	jmp	got_result_ptr

inv_or_op2_signed_by_op1_vs_1:
	cmp	$0x3fff,dword_expon1
	jg	op2
	jl	flip_sign_op2
	cmp	$0x80000000,dword_frac1+frac32(%ebp)
	ja	op2
	cmp	$0,dword_frac1+frac64(%ebp)
	jne	op2
	jmp	op_unsupp

zerror_flip_sign_op2:
	orb	zero_divide_mask,%gs:sr_errors
	testb	zero_divide_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
	jmp	flip_sign_op2

op2_signed_by_op1_vs_1:
	cmp	$0x3fff,dword_expon1
	jge	op2
flip_sign_op2:
	notb	sign2(%ebp)
op2:
	mov	$offset_operand2,%edi
	jmp	got_result_ptr

derror_flip_sign_op2:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
	jmp	flip_sign_op2

derror_exor_signed_op2:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
exor_signed_op2:
	movb	sign1(%ebp),%dl
	xorb	%dl,sign2(%ebp)
	jmp	op2

derror_exor_signed_op1:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	two_arg_unmasked_exit
exor_signed_op1:
	movb	sign2(%ebp),%dl
	xorb	%dl,sign1(%ebp)
op1:
	mov	$offset_operand1,%edi
	jmp	got_result_ptr

/two_arg_trans	endp
/
push_5_dw_at_esi:	/proc

	pop	%ebx								/ ebx  <-- return address
	push	(%ebp,%esi)
	push	frac64(%ebp,%esi)
	push	frac32(%ebp,%esi)
	push	sign(%ebp,%esi)
	push	expon(%ebp,%esi)
	jmp	*%ebx

/push_5_dw_at_esi	endp

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

pop_5_dw_at_esi:	/proc

	pop	%ebx								/ ebx  <-- return address
	pop	expon(%ebp,%esi)
	pop	sign(%ebp,%esi)
	pop	frac32(%ebp,%esi)
	pop	frac64(%ebp,%esi)
	pop	(%ebp,%esi)
	jmp	*%ebx

/pop_5_dw_at_esi	endp

/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
move_unpacked_1_to_esi:	/proc

	mov	$0,(%ebp,%esi)
	mov	$0,frac64(%ebp,%esi)
	mov	$0x080000000,frac32(%ebp,%esi)
/	mov	positive+0x10000*valid,sign(%ebp,%esi)
	mov	$0x10000\*VALID+POSITIVE,sign(%ebp,%esi)
	mov	$0x3fff,expon(%ebp,%esi)
	ret

/move_unpacked_1_to_esi	endp
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			exp:
/			"""
/	function:
/		calculates 2**x - 1, where 0 <= x < 1/2.
/	inputs:
/		assumes that operand1 is set up.
/
/	outputs:
/		result in global record 12; possible underflow 
/		and inexact errors.
/
/	data accessed:
/		- offset_operand1		tag1
/		- expon1			word_frac1
/		- lsb_frac1			offset_operand1
/		- sign2				expon2
/		- offset_operand2		offset_result
/		- result_tag			result_expon
/		- lsb_result			msb_result
/		- offset_result		siso
/		- offset_cop
/
/	data changed:
/		- expon1			lsb_frac1
/		- sign2				expon2
/		- result_tag			msb_result
/		- siso
/
/	procedures called:
/		put_si_result		right_shift		left_shift
/		move_10_bytes		move_constant		addx
/		subx			mulx			divx
/		addition_normalize	round			divid
/		log_constant		underflow_response	set_p_error
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
exp_core:	/proc
/
/pseudo_division
/
	movw	$0,siso					/ clear siso
	mov	$0x3fff,%eax
	sub	dword_expon1,%eax		/ dword_expon1 < 3fffh
	cmp	$63,%eax
	jg	exp_tiny_argument
	mov	%eax,dword_expon1
	cmp	$15,%eax
	ja	exp_rat_appx			/ impossible that eax <= 0
	call	right_shift_frac1_8
exp_loop_back_1:
	mov	$10,%eax
	mul	dword_expon1
	add	$log_constant-10,%eax
	mov	%eax,%esi
	call	sub_frac2
	shlw	$1,siso
	cmpb	$0,msb_result
	jne	exp_merge_0
	call	move_result_frac1
	and	$0x0ffe00000,dword_frac1(%ebp)
	incw	siso
exp_merge_0:
	inc	dword_expon1
	cmp	$15,dword_expon1
	ja	exp_branch_1
	call	left_shift_frac1_1
	jmp	exp_loop_back_1
exp_tiny_argument:
	movb	positive,sign2(%ebp)
	mov	$0x3fff,dword_expon2
	mov	$constant_2_ov_ln2,%esi
	call	move_constant_frac2
	call	left_shift_frac2_8				/ tiny argument
	call	divid				/ floating-point divide
	call	post_op_round
exp_put_exp_jmp:
	jmp	exp_put_exp_f
/
/pre_rat_appx
/
exp_branch_1:
	mov	$15,dword_expon1
	call	left_shift_frac1_8
exp_rat_appx:
	call	move_frac1_cop
/
/80287 short multiply (does 67-bit by 34-bit multiplication only).
/
	movb	$30,%cl
	call	right_shift_frac1_cl
	and	$0,dword_frac1(%ebp)
	movb	$30,%cl
	call	left_shift_frac1_cl
	call	mulx
	call	move_result_frac1
	and	$0x0e0000000,dword_frac1(%ebp)
	mov	$constant_ln2_ov_6,%esi
	call	mul_frac2
	mov	dword_expon1,%ecx
	add	$10,%ecx
	call	right_shift_result_cl
	and	$0x0ffe00000,result_dword_frac(%ebp)
	call	move_cop_frac1
	call	right_shift_frac1_8
	call	move_result_frac2
	call	subx
	mov	dword_expon1,%ecx
	inc	%ecx
	call	right_shift_result_cl
	and	$0x0ffe00000,result_dword_frac(%ebp)
	call	move_result_frac2
	mov	$constant_2_ov_ln2,%esi
	call	sub_frac1
	call	move_cop_frac1
	call	move_result_frac2
	call	left_shift_frac2_8
/ the following call used to be to divx instead of accel_divx:
	call	accel_divx
	mov	$8,%ecx
	call	right_shift_result_cl
	and	$0x0ffe00000,result_dword_frac(%ebp)

/
/pseudo_multiply
/
exp_loop_back_2:
	shrw	$1,siso
	jnb	exp_merge_2
	call	move_result_frac1
	call	move_result_frac2
	mov	dword_expon1,%ecx
	call	right_shift_frac2_cl
	and	$0x0ffe00000,dword_frac2(%ebp)
	orw	$0x80,8+word_frac2
	call	addx
exp_merge_2:
	cmpw	$0,siso
	je	exp_almost_end
	call	right_shift_result_1
	and	$0x0ffe00000,result_dword_frac(%ebp)
	dec	dword_expon1
	jmp	exp_loop_back_2
exp_almost_end:
	movb	$8,%cl
	cmpb	$0,msb_result
	je	exp_merge_3
	movb	$7,%cl
	dec	dword_expon1
exp_merge_3:
	call	left_shift_result_cl
	mov	$0x3fff,%eax
	sub	dword_expon1,%eax
	mov	%eax,dword_result_expon
/	mov	positive+valid\*0x10000,result_sign(%ebp)
	mov	$0x10000\*VALID+POSITIVE,result_sign(%ebp)
	testb	$0x80,msb_result
	jne	exp_add_half_round
/
/normalize x
/
	movb	$1,%cl
	call	left_shift_result_cl
	dec	dword_result_expon
exp_add_half_round:
	call	move_result_frac1
	call	add_half1_frac2
exp_put_exp_f:
	ret

/exp_core	endp
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
neg_exp_core:	/proc
	notb	sign1(%ebp)
	call	exp_core
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op
	mov	$offset_operand1,%esi
	call	move_unpacked_1_to_esi
	movb	negative,sign1(%ebp)
	movb	$sub_op,operation_type(%ebp)
	mov	$offset_result,%esi
	call	push_5_dw_at_esi
	call	subadd
	call	post_op_round
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op
	mov	$offset_operand1,%esi
	call	pop_5_dw_at_esi
	call	divid				/ floating-point divide
	call	post_op_round
exp_neg_core_exit:
	ret

/neg_exp_core	endp
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
exp:	/proc
	call	one_result_op1_chk
	jnz	exp_non_zero
garb_out:
	jmp	put_op1_result
/
exp_non_zero:
	cmp	$0x3fff,dword_expon1
	jg	garb_out
	jl	more_than_half_
more_than_1_:
	cmp	$0x080000000,dword_frac1+frac32(%ebp)
	ja	garb_out
	cmp	$0,dword_frac1+frac64(%ebp)
	jne	garb_out
	cmpb	positive,sign1(%ebp)
	je	garb_out			/ answer is 1, already in op1
	dec	dword_expon1
	jmp	garb_out				/ answer is -1/2.
more_than_half_:
	orb	$inexact_mask,%gs:sr_errors	/ set inexact error flag
	push	%gs:sr_masks	/ save precision control
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
	and	$0x0fffff3ff,%gs:sr_masks	/ set rc = to_nearest
	cmp	$0x3ffe,dword_expon1
	je	maybe_half
	cmpb	positive,sign1(%ebp)
	jne	tween_neg_half_and_0
	jmp	tween_0_and_half
maybe_half:
	cmp	$0x080000000,dword_frac1+frac32(%ebp)
	ja	tween_half_and_1
	cmp	$0,dword_frac1+frac64(%ebp)
	jne	tween_half_and_1
	cmpb	positive,sign1(%ebp)
	jne	tween_neg_half_and_0
tween_0_and_half:
	call	exp_core
/
/error checking
/
exp_uflow_:
	mov	dword_result_expon,%ecx
	cmp	$0,%ecx
	jle	exp_underflow
	movb	valid,result_tag(%ebp)	/valid result
	jmp	exp_exit
exp_underflow:
	call	underflow_response
	orb	underflow_mask,%gs:sr_errors
exp_exit:
	cmpb	true,added_one
	jne	exp_restore
	orb	$a_mask,%gs:sr_flags
exp_restore:
	pop	%gs:sr_masks	/ restore precision and rounding controls
	mov	$offset_result,%edi
	jmp	put_si_result
tween_neg_half_and_0:
	call	neg_exp_core
	jmp	exp_uflow_
tween_half_and_1:
	cmpb	positive,sign1(%ebp)
	jne	tween_neg_1_and_neg_half
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	movb	$sub_op,operation_type(%ebp)
	call	subadd
	call	post_op_round
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op
	call	neg_exp_core
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	movb	$add_op,operation_type(%ebp)
	call	subadd
	call	post_op_round
	inc	dword_result_expon
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	movb	$sub_op,operation_type(%ebp)
	call	subadd
	call	post_op_round
	jmp	exp_exit
tween_neg_1_and_neg_half:
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	movb	$add_op,operation_type(%ebp)
	call	subadd
	call	post_op_round
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op
	call	exp_core
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	movb	$add_op,operation_type(%ebp)
	call	subadd
	call	post_op_round
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op
	dec	dword_expon1
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	movb	$sub_op,operation_type(%ebp)
	call	subadd
	call	post_op_round
	jmp	exp_exit

/exp	endp
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			log:
/			"""
/	function:
/		calculates y times log(x) or y times log(1 + x).
/
/	inputs:
/		assumes that operand1 and operand2 are set up;
/		op1 = x, op2 = y.
/
/	outputs:
/		possible underflow, overflow, or inexact errors
/
/	data accessed:
/		- operation_type		offset_result_rec
/		- sign1				expon1
/		- offset_operand1		offset_operand2
/		- tag2				sign2
/		- expon2			word_frac2
/		- msb_frac2			offset_operand2
/		- offset_result			result_tag
/		- result_sign			result_expon
/		- result_word_frac		msb_result
/		- msb_result			offset_result
/		- result2_tag			result2_sign
/		- msb_result2			log_loop_ct
/		- offset_cop			offset_dop
/		- siso
/
/	data changed:
/		- sign1				sign2
/		- msb_frac2			word_frac2
/		- result_tag			result_sign
/		- result_frac			offset_result
/		- result2_tag			result2_sign
/		- msb_result2			log_loop_ct
/		- siso
/
/	procedures called:
/		put_result		right_shift		left_shift
/		move_10_bytes		move_constant		addx
/		subx			mulx			accel_divx (for log_divx)
/		one_left_normalize	addition_normalize	log_constant
/		test_5w			subtraction_normalize	test_4w
/		underflow_response	overflow_response	decompose
/		pop_free		sp_subadd		mult
/		round			get_precision		set_p_error
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
log:	/proc
	movb	valid,result2_tag(%ebp)
	jmp	log_entry
splog:
	movb	special,result2_tag(%ebp) / flag for special log
log_entry:
	push	%gs:sr_masks			/ save precision control
/	and	dword ptr %gs:sr_masks, 0fffff3ffh; set rounding control to nearest
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
	mov	$offset_cop+frac80,%edi
	call	move_frac2			/ temp store y, cop <- frac(y)
	movb	tag2(%ebp),%al
	movb	%al,lsb_cop
	mov	dword_expon2,%eax	/ log_loop_ct <- expo(y)
	mov	%eax,log_loop_ct
	movb	sign2(%ebp),%al		/ lsb_result2 <- sign2
	movb	%al,lsb_result2
	movb	positive,result2_sign(%ebp)/ used for sign of result
	cmpb	valid,result2_tag(%ebp)	/ after log core
	jnz	log_special_log			/ result sign same as result2
	call	right_shift_frac1_8
	mov	$constant_sqrt_of_2,%esi
	call	sub_frac2
	cmpb	$0,msb_result
	jne	log_small_lg
	jmp	log_large_lg
log_sp_negative_x:
	call	move_frac1_result
	movb	negative,result2_sign(%ebp)
	jmp	log_core
log_special_log:
/	mov	edi,	offset_operand1	; weed out the special case
/	call	test_6w				; where x = 0.  result should
/	jz		log_splog_zero			; be exact signed zero.
	mov	dword_expon1,%eax	/ log x not 0
	mov	%eax,dword_result_expon
	cmpb	positive,sign1(%ebp)
	jne	log_sp_negative_x
	call	move_frac1_frac2
	mov	$0x3fff,%ecx
	sub	dword_expon1,%ecx
	cmp	$72,%ecx
	jbe	log_sp_label_1
	mov	$72,%ecx
log_sp_label_1:
	call	right_shift_frac2_cl
	movw	word_frac2,%cx
	and	$0x1fff,%ecx
	or	%ecx,%edx
	jz	log_non_sticky
	orw	$0x2000,word_frac2
log_non_sticky:
	orb	$0x80,msb_frac2
	and	$0x0e0000000,dword_frac2(%ebp)
	jmp	log_div
/
/ frac(x) < sqrt(2)
/
log_small_lg:
	call	left_shift_frac1_8
	call	move_frac1_result
	mov	$1,%ecx
	call	left_shift_result_cl
	mov	$offset_result+frac80,%edi
	call	test_5w
	jz	log_power_of_two
	mov	$0x3ffe,dword_result_expon
	mov	$offset_result,%edi
	call	subtraction_normalize
	call	move_frac1_frac2
	call	move_result_frac1
log_div:
/ the following call used to be to log_divx instead of accel_divx:
	push	log_loop_ct
	call	accel_divx
	pop	log_loop_ct
	and	$0x0e0000000,result_dword_frac(%ebp)/ mask result frac
	jmp	log_merge_lg
/
/ x = 2 ** n
/
log_power_of_two:
	call	decompose
log_mul:
	mov	$offset_operand1+frac80,%edi
	call	test_5w
	jnz	log_x_not_0
/	call	log_round				; round the result
log_splog_zero:
	movb	special,tag1(%ebp)			/ zero tag
	movb	lsb_result2,%al
	xorb	%al,sign1(%ebp)
	lea	dword_frac1(%ebp),%esi
	lea	result_dword_frac(%ebp),%edi
	call	move_op_to_op
	jmp	log_restore_user_controls
log_x_not_0:
/		cmp		lsb_cop,	special
/		jne		log_y_not_zero
/
/y=0
/
/		xor		eax,	eax
/		mov		edi,	offset result_dword_frac + 4
/		call	set_4w
/		mov		dword_result_expon,	eax
/		mov		dword ptr [ebp].result_sign,	special*10000h
/		mov		al,		[ebp].sign1
/		xor		al,		lsb_result2
/		mov		[ebp].result_sign,	al
/		jmp	short	log_finish_up
/log_y_not_zero:
	and	$0x0ff000000,dword_cop(%ebp)
	call	move_cop_frac2			/ restore y into op2
	mov	log_loop_ct,%eax
	mov	%eax,dword_expon2
	movb	lsb_result2,%al
	movb	%al,sign2(%ebp)
	call	mult
	call	post_op_round		/ round the result
/
/error checking
/
	mov	dword_result_expon,%eax
	cmp	$0x7ffe,%eax
	jg	log_overflow
	cmp	$0,%eax
	jl	log_underflow
	jg	log_set_tag_valid
	mov	$offset_result+frac64,%edi
	call	test_4w					/ eax = 0 here
	jnz	log_underflow
/
/zero result
/
	movb	special,result_tag(%ebp)
	jmp	log_round_report
log_set_tag_valid:
	movb	valid,result_tag(%ebp)
log_round_report:
	cmpw	$0,rnd_history
	je	log_restore_user_controls
	orb	$inexact_mask,%gs:sr_errors
	cmpb	true,added_one
	jne	log_restore_user_controls
	orb	$a_mask,%gs:sr_flags
log_restore_user_controls:
	pop	%gs:sr_masks
log_exit:
	ret
/
log_overflow:
	call	overflow_response
	jmp	log_restore_user_controls
log_underflow:
	call	underflow_response
	jmp	log_restore_user_controls
/
/frac(x) > sqrt(2)
/
log_large_lg:
	inc	dword_expon1
	call	move_frac1_frac2
	mov	$constant_one,%esi
	call	sub_frac1
	mov	$7,%ecx
	call	left_shift_result_cl
	movb	negative,result2_sign(%ebp)	/ set flag for large log
	mov	$0x3fff,dword_result_expon
log_merge_lg:
	mov	$offset_result+frac80,%edi	/ don't normalize a zero
	call	test_5w							/ is the result zero?
	jz	log_core			/ yes, no need to normalize
	mov	$offset_result,%edi
	call	subtraction_normalize
log_core:
	call	move_result_frac2
	movw	$0,siso						/ clear siso
	mov	$0x3fff,%eax
	sub	dword_result_expon,%eax
	mov	%eax,dword_exp_tmp
	cmp	$15,%eax
	ja	log_rat_apx
	call	move_result_frac1			/ log pseudo divide
	mov	$7,%ecx
	add	dword_exp_tmp,%ecx
	call	right_shift_frac1_cl
	call	right_shift_frac2_8
	call	addx
	cmpb	$0,msb_result
	je	log_loop_back_1
	and	$0x0ffe00000,result_dword_frac(%ebp)
	movb	$0,msb_result
	call	move_result_frac2
	movw	$1,siso
log_loop_back_1:
	shlw	$1,siso
	call	move_frac2_frac1
	mov	dword_exp_tmp,%ecx
	call	right_shift_frac1_cl
	or	$0x00800000,dword_frac1+frac32(%ebp)
	call	addx
	cmpb	$0,msb_result
	je	log_merge_2
	and	$0x0ffe00000,result_dword_frac(%ebp)
	movb	$0,msb_result
	call	move_result_frac2
	incw	siso
log_merge_2:
	mov	$1,%ecx
	call	left_shift_frac2_cl
	inc	dword_exp_tmp
	cmp	$15,dword_exp_tmp
	jbe	log_loop_back_1
	call	left_shift_frac2_8
/
/rational approximation
/
log_rat_apx:
	orb	$inexact_mask,%gs:sr_errors		/ set p flag
	call	move_frac2_frac1
	call	mulx
	mov	dword_exp_tmp,%ecx
	cmp	$72,%ecx
	jbe	log_label_4
	mov	$72,%ecx
log_label_4:
	call	right_shift_result_cl
	and	$0x0e0000000,result_dword_frac(%ebp)/ mask result frac
	call	move_result_frac2
	call	subx
	call	move_result_dop
	mov	dword_exp_tmp,%ecx
	cmp	$72,%ecx
	jbe	log_label_5
	mov	$72,%ecx
log_label_5:
	call	right_shift_frac1_cl
	and	$0x0e0000000,dword_frac1(%ebp)
	call	move_frac1_frac2
	mov	$constant_six,%esi
	call	sub_frac1
	call	move_result_frac1
	call	move_dop_frac2
	mov	dword_exp_tmp,%ecx
	inc	%ecx
	cmp	$72,%ecx
	jbe	log_label_6
	mov	$72,%ecx
log_label_6:
	call	right_shift_frac2_cl
	and	$0x0f0000000,dword_frac2(%ebp)
	call	subx
	and	$0x0e0000000,result_dword_frac(%ebp)/ mask result frac
	call	move_dop_frac1
	call	move_result_frac2
/ the following call used to be to log_divx instead of accel_divx:
	push	log_loop_ct
	call	accel_divx
	pop	log_loop_ct
	and	$0x0e0000000,result_dword_frac(%ebp)/ mask result frac
	call	move_result_frac1
	mov	$constant_3_ov_ln2,%esi
	call	mul_frac2
	mov	$7,%ecx
	call	right_shift_result_cl
	dec	dword_exp_tmp
log_loop_back_2:
	shrw	$1,siso
	jnb	log_no_carry
	mov	dword_exp_tmp,%esi
	dec	%esi
	mov	$10,%eax
	mul	%esi
	mov	%eax,%esi
	add	$log_constant,%esi
	call	move_constant_frac2
	call	move_result_frac1
	call	addx
log_no_carry:
	cmpw	$0,siso
	jz	log_end_loop
	call	right_shift_result_1
	dec	dword_exp_tmp
	jmp	log_loop_back_2
log_end_loop:
	mov	$0x4007,%eax			/4007 = 3fff + 8
	sub	dword_exp_tmp,%eax
	mov	%eax,dword_result_expon
	and	$0x0ffe00000,result_dword_frac(%ebp)
	mov	$offset_result,%edi	/ don't normalize a zero
	call	test_6w					/ is the result zero?
	jz	log_end_log_core	/ yes, no need to normalize
	mov	$offset_result,%edi
	call	subtraction_normalize
log_end_log_core:
	cmpb	valid,result2_tag(%ebp)
	jnz	log_special_log_end
	call	decompose
	call	move_result_frac2
	mov	dword_result_expon,%eax
	mov	%eax,dword_expon2
	movb	$add_op,operation_type(%ebp)
	movb	result2_sign(%ebp),%al
	movb	%al,sign2(%ebp)
	call	sp_subadd
log_merge_4:
	call	move_result_frac1
	movb	result_sign(%ebp),%al
	movb	%al,sign1(%ebp)
	mov	dword_result_expon,%eax
	mov	%eax,dword_expon1
	jmp	log_mul
log_special_log_end:
	movb	result2_sign(%ebp),%al
	movb	%al,result_sign(%ebp)
	jmp	log_merge_4
/
/do round				; made into common subroutine for all
/
log_round:
	movb	$1,lsb_result				/ fixed up such that
/	call	get_precision
post_op_round:
	movb	prec64,%dl
	mov	$offset_result,%edi		/ s = 1 always
	movb	false,%al				/ not second rounding
	call	round
	mov	$offset_result,%edi
	movb	$4,%ah					/ fall through
/log	endp
/
/
/		addition_normalize:
/
/	function:
/		performs normalization after addition if necessary.
/
/	inputs:
/		carry-out from addition in al upon entry.
/		fraction offset in di.
/
/	outputs:
/		increments exponent and shifts 1 bit in from left if al=1.
/
/	data accessed:
/
/	data changed:
/
/	procedures called:
/		sticky_right_shift
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
addition_normalize:	/proc
	testb	$0x01,%al				/if low bit of al = 1,
	jnz	right_shift_1			/ then shift right 1
	ret					/ else, exit
right_shift_1:
	inc	expon(%ebp,%edi)	/increment exponent
	movb	$1,%cl				/shift right 1 bit
/addition_normalize	endp				; sticky_right_shift
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			sticky_right_shift:
/			""""""""""""""""""
/	function:
/		shifts a 10-byte number right by amount in
/		cl register.  di contains offset of number.
/
/	inputs:
/		low bit of al contains bit to inject from left.
/		(for shifts of more than one, the result is undefined
/		unless al contains zero).
/
/	outputs:
/		the low-order bit of the number is "sticky", ie, it is
/		left as a one if any ones were shifted out.        
/
/	data accessed:
/
/	data changed:
/
/	procedures called:
/		right_shift
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
sticky_right_shift:	/proc
	call	right_shift			/do right shift
	and	%edx,%edx				/edx <> 0 if 1's lost
	jz	sticky_right_shift_done		/done if no 1's lost
	orb	$0x01,frac80(%ebp,%edi)		/else, set sticky bit
sticky_right_shift_done:
	ret
/sticky_right_shift	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			gradual_underflow:
/
/	function:
/		changes an "underflowed" number to a legal,
/		denormalized number.
/
/	input:
/		address of exponent and fraction in operands or result
/		(ptr in di register); minimum legal exponent(in
/		ax register).
/
/	output:
/		denormalized fraction and adjusted exponent (in input
/		record)
/
/	data accessed:;		- expon			word_frac
/
/	data changed:
/		- expon			word_frac
/
/	procedures called:
/		sticky_right_shift
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
gradual_underflow:	/proc
	mov	%eax,%ecx		           / set exponent to minimum
	xchg	expon(%ebp,%edi),%eax
	sub	%eax,%ecx			            / form shift amount
	cmp	$67,%ecx		        / (exponent - minimum exponent)
	jle	do_right_shift
	mov	$67,%ecx		        / shift amount must be <= 67
do_right_shift:
	xorb	%al,%al			            / byte to be shifted in from
	jmp	sticky_right_shift            / the left
/gradual_underflow	endp


/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/					two_result_op1_chk
/
/	action:
/	for sincos and (two result) tan, checks the following and responds as
/       described:
/		1.  whether fetch_an_op discovered that the source from which
/		operand1 was to have been taken was empty.  if so, sets stack
/		underflow exception, clears c-flag (indicating not incomplete
/		reduction), pops the return address of its caller so that it
/		returns directly to the e80387 executive routine, and, if the
/		invalid exception is unmasked, makes that return forthwith.
/          if the invalid exception is masked, this procedure prepares to
/		return via a jump to put_result by first setting edi so that
/		the nan indefinite put in operand1 by fetch_an_op will be
/		returned, and setting esi so that this returned value will be
/		pushed onto the stack.  note that after this push, the former
/		stack top is unchanged; i.e., if it was empty as stack top, it's
/		still empty as st(1).
/		2.  if operand1 was fetched from a non-empty source, clears
/		c-flag (indicating not incomplete reduction) and then checks
/		whether a push will be to a full position.  if so, sets zf so
/		that return to e80387 executive after popping caller's return
/		address and jumping to put_result does not falsely signal
/		masked stack underflow instead of correctly signaling stack
/		overflow.
/		3.  if push would be to an empty position, initializes the
/		before_error_signals used by put_result, clears the a-flag,
/		and checks tag of operand1.
/			a.  if tag is unsupp or infinty, replaces operand1 by
/		the nan indefinite, then checks whether invalid operation
/		is masked.  if not masked, sets invalid operation exception
/		flag and pops caller's return address so that return is to
/		e80387 executive.  if masked, copies the quiet nan in
/		operand1 to operand2, sets the signal_i_error? component of
/		before_error_signals to true, and returns to e80387
/		executive by popping caller's return address and jumping to
/		do_exchange.
/		b.  if tag is inv, checks whether the nan in operand1 is
/		signaling.  if not, sets zf, pops caller's return address,
/		sets edi and esi so that jump to put_result causes the
/		quiet nan in operand1 to be pushed onto '87 stack.  if the
/		nan is signaling, sets its leading fraction bit, thereby
/		"quieting" it, and proceeds as in "a" above, but with the
/		"quietized" nan in place of the nan indefinite.
/		c.  if tag is denormd,  checks whether denormal exception
/		is masked.  if not masked, sets denormal exception flag and
/		pops caller's return address so that return is to e80387
/		executive.  if masked, sets the signal_d_error component of
/		before_error_signals to true, normalizes operand1 and forces
/		zf = 0 (indicating non-zero operand1) before returning to
/		caller.
/		d.  if tag is valid or special (signed, true zero), returns
/			to caller with zf clear or set, respectively. 
/
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
two_result_op1_chk:	/proc
	jz	is_stack_full_			/ if no stack underflow, go on
	call	set_stk_u_error			/ else, prepare for error exit 
	andb	$ -1 ! c_mask,%gs:sr_flags		/ clear '87 c-flag
	testb	invalid_mask,%gs:sr_masks
	jnz	push_op1		/ masked stack error, so return indef
unmasked_exit:
	pop	%eax		/ pop address of transcendental caller so
two_res_quik_exit:		/ return is to e80387 executive procedure.
	ret
push_op1:
	mov	$offset_operand1,%edi
	mov	offset_result2_rec,%esi
	pop	%eax			/ pop address of transcendental caller
	lahf			/ save zf to show if masked stack underflow
	jmp	put_result	/ return is to e80387 executive procedure.
is_stack_full_:
	andb	$ -1 ! c_mask,%gs:sr_flags	/ clear '87 c-flag
	call	stack_full_
	jz	cntnu_chk
set_zf:
	xor	%eax,%eax
	jmp	push_op1
cntnu_chk:
	mov	$0,before_error_signals(%ebp)
	andb	$ -1 ! a_mask,%gs:sr_flags
	movb	tag1(%ebp),%al				/ load tag for op1
	cmpb	special,%al
	jbe	ok_op
	cmpb	denormd,%al
	jb	inv_op
	je	signal_d_error	/ elsif denormalized, then signal d_error
	mov	$offset_operand1,%edi	/ infinity or unsupported,
	call	set_up_indefinite		/ set up masked result
	jmp	test_invalid_mask
inv_op:
	testb	$0x40,msb_frac1	/ elsif op1 nan, find what kind
	jnz	set_zf			/ op1 is a  qnan, so load it to tos
	orb	$0x40,msb_frac1	/ make nan quiet
test_invalid_mask:
	testb	invalid_mask,%gs:sr_masks
	jnz	copy_nan
	orb	invalid_mask,%gs:sr_errors
	jmp	unmasked_exit
copy_nan:
	lea	dword_frac1(%ebp),%esi	/ copy op1 to op2
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op
signal_i_error:
	movb	true,signal_i_error_
	pop	%eax
	jmp	do_exchange
signal_d_error:
	testb	denorm_mask,%gs:sr_masks
	jnz	set_up_d_error
	orb	denorm_mask,%gs:sr_errors
	jmp	unmasked_exit
set_up_d_error:
	movb	true,signal_d_error_/ op1 is denormal or pseudo-denormal
	mov	$offset_operand1,%edi
	call	norm_denorm		/ if d_error masked, make valid
	or	$1,%eax					/ force zf = 0
ok_op:
	ret									/ zf = 1 iff op1 is true zero

/two_result_op1_chk	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/					one_result_op1_chk
/
/	action:
/		for sin and cos, checks the following and responds as
/       described:
/	1.  whether fetch_an_op discovered that the source from which
/	operand1 was to have been taken was empty.  if so, sets stack
/	underflow exception, clears c-flag (indicating not incomplete
/	reduction), pops the return address of its caller so that it
/	returns directly to the e80387 executive routine, and, if the
/	invalid exception is unmasked, makes that return forthwith.
/       if the invalid exception is masked, this procedure prepares to
/	return via a jump to put_result by first setting edi so that
/	the nan indefinite put in operand1 by fetch_an_op will be
/	returned, and setting esi so that this returned value will be
/	pushed onto the stack.  note that after this push, the former
/	stack top is unchanged; i.e., if it was empty as stack top, it's
/	still empty as st(1).
/	2.  if operand1 was fetched from a non-empty source, clears
/	c-flag (indicating not incomplete reduction) and then checks
/	whether a push will be to a full position.  if so, sets zf so
/	that return to e80387 executive after popping caller's return
/	address and jumping to put_result does not falsely signal
/	masked stack underflow instead of correctly signaling stack
/	overflow.
/	3.  if push would be to an empty position, initializes the
/	before_error_signals used by put_result, clears the a-flag,
/	and checks tag of operand1.
/		a.  if tag is unsupp or infinty, replaces operand1 by
/		the nan indefinite, then checks whether invalid operation
/		is masked.  if not masked, sets invalid operation exception
/		flag and pops caller's return address so that return is to
/		e80387 executive.  if masked, copies the quiet nan in
/		operand1 to operand2, sets the signal_i_error? component of
/		before_error_signals to true, and returns to e80387
/		executive by popping caller's return address and jumping to
/		do_exchange.
/		b.  if tag is inv, checks whether the nan in operand1 is
/		signaling.  if not, sets zf, pops caller's return address,
/		sets edi and esi so that jump to put_result causes the
/		quiet nan in operand1 to be pushed onto '87 stack.  if the
/		nan is signaling, sets its leading fraction bit, thereby
/		"quieting" it, and proceeds as in "a" above, but with the
/		"quietized" nan in place of the nan indefinite.
/		c.  if tag is denormd,  checks whether denormal exception
/		is masked.  if not masked, sets denormal exception flag and
/		pops caller's return address so that return is to e80387
/		executive.  if masked, sets the signal_d_error component of
/		before_error_signals to true, normalizes operand1 and forces
/		zf = 0 (indicating non-zero operand1) before returning to
/		caller.
/		d.  if tag is valid or special (signed, true zero), returns
/		to caller with zf clear or set, respectively. 
/
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
one_result_op1_chk:	/proc
	jz	check_tag1			/ if no stack underflow, go on
	call	set_stk_u_error			/ else, prepare for error exit 
	andb	$ -1 ! c_mask,%gs:sr_flags	/ clear '87 c-flag
	testb	invalid_mask,%gs:sr_masks
	jnz	give_op1		/ masked stack error, so return indef
one_res_unmasked_exit:
	pop	%eax		/ pop address of transcendental caller so
one_res_quik_exit:		/ return is to e80387 executive procedure.
	ret
give_op1:
	pop	%eax		/ pop address of transcendental caller so
	jmp	put_op1_result  / return is to e80387 executive procedure.
check_tag1:
	andb	$ -1 ! [a_mask+c_mask],%gs:sr_flags
	movb	tag1(%ebp),%al				/ load tag for op1
	cmpb	special,%al
	jle	op1_ok
	cmpb	denormd,%al
	jl	op1_inv
	je	op1_den				/ elsif denormalized, then signal d_error
	cmpb	$exp_op,operation_type(%ebp)
	jne	one_res_unsup
	cmpb	unsupp,%al
	je	one_res_unsup
	cmpb	positive,sign1(%ebp)		/ op1 is +/- infinity
	je	give_op1
	mov	$0x3fff,dword_expon1
	jmp	give_op1
one_res_unsup:
	mov	$offset_operand1,%edi	/ infinity or unsupported,
	call	set_up_indefinite		/ set up masked result
	jmp	invalid_masked_
op1_inv:
	testb	$0x40,msb_frac1	/ elsif op1 nan, find what kind
	jnz	give_op1			/ op1 is a  qnan, so return it
	orb	$0x40,msb_frac1	/ make nan quiet
invalid_masked_:
	orb	invalid_mask,%gs:sr_errors
	testb	invalid_mask,%gs:sr_masks
	jz	one_res_unmasked_exit
	jmp	give_op1
op1_den:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	one_res_unmasked_exit
	mov	$offset_operand1,%edi
	call	norm_denorm					/ if d_error masked, make valid
	or	$1,%eax					/ force zf = 0
op1_ok:
	ret									/ zf = 1 iff op1 is true zero

/one_result_op1_chk	endp
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
mod_qrtr_pi:	/proc
/ '87 c-flag has been cleared by this point
	xor	%eax,%eax
	movb	operation_type(%ebp),%al
	movb	sign1(%ebp),%ah
	mov	%eax,trans_info		/ initialize cofunc_flag false (00h)
						/ and cos_sign positive (00h)
	movb	positive,sign1(%ebp)/ take absolute value of op1
	movw	$0,exp_tmp			/ initialize q and hi_q to 0.
	mov	dword_expon1,%eax
	sub	$0x3ffe,%eax
	jl	reduction_done
	cmp	$63,%eax
	jle	do_reduction
	orb	$c_mask,%gs:sr_flags		/ indicate incomplete reduction
	pop	%eax			/ get rid of caller's return address
	ret
do_reduction:
	mov	$0x2168c235,dword_frac2+frac64(%ebp)
	mov	$0x0c90fdaa2,dword_frac2+frac32(%ebp)
	mov	$0x10000\*VALID+POSITIVE,dword_frac2+sign(%ebp)
	mov	$0x3ffe,dword_expon2
	sub	%eax,dword_expon1
	inc	%eax
	call	remrx
	mov	dword_frac1+frac64(%ebp),%eax
	or	dword_frac1+frac32(%ebp),%eax
	jnz	mod_not_0
	mov	%eax,dword_frac1(%ebp)		/ here, eax = 0
	mov	%eax,dword_expon1
	mov	special,dword_frac1+tag(%ebp)
	jmp	reduce_pi
mod_not_0:
	mov	$offset_operand1,%edi
	call	subtraction_normalize		/ returns with zf = 0.
reduction_done:
	orb	$inexact_mask,%gs:sr_errors	/ precision error unless zero mod
/check_for_tan_op:
/		cmpb		op_type,	tan_op
/		je		reduce_hapi
reduce_pi:
	testb	$4,q
	jz	reduce_hapi
	notw	res_signs		/ flip sign for both sin and cos
reduce_hapi:
	testb	$2,q
	jz	reduce_qrpi
	notb	cofunc_flag
	notb	cos_sign
/	cmpb		op_type,	tan_op
/	jne		reduce_qrpi
/		not		sin_sign	; use sin_sign as tan_sign
reduce_qrpi:
	testb	$1,q
	jnz	last_reduc
	ret										/ zf is set here
last_reduc:
	notb	cofunc_flag		/ a zero mod will be changed here
	cmpb	special,tag1(%ebp)
	jne	mod_set_inexact
	cmpb	$tan_op,op_type
/	je		complementary_angle		
	jne	mod_set_inexact
	movb	$0x80,msb_frac1
	movb	valid,tag1(%ebp)
	mov	$0x3fff,dword_expon1
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	add	$4,%esp				/ get rid of return address
	jmp	adjust_sign
mod_set_inexact:			/ a zero mod will be changed here
	orb	$inexact_mask,%gs:sr_errors	/ to qrtr_pi, so precision error
complementary_angle:
	mov	$0,dword_frac2(%ebp)
	mov	$0x2168c235,dword_frac2+frac64(%ebp)
	mov	$0x0c90fdaa2,dword_frac2+frac32(%ebp)
	mov	$0x10000\*VALID+POSITIVE,dword_frac2+sign(%ebp)
	mov	$0x3ffe,dword_expon2
	movb	negative,sign1(%ebp)
	movb	$add_op,operation_type(%ebp)
	call	subadd
	push	%gs:sr_masks
	and	$0x0fffff3ff,%gs:sr_masks	/ set rc = to_nearest
	call	post_op_round
	pop	%gs:sr_masks
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op
	ret
/mod_qrtr_pi	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
tan_pseudo_divide:	/proc

	movw	$0,siso				/ clear siso
	mov	%eax,dword_expon1
	cmp	$16,%eax
	jg	tan_rat_appx
	dec	dword_expon1		/ decrement expon1
	call	right_shift_frac1_8
tan_loop_back_1:
	mov	$10,%eax
	mul	dword_expon1
	add	$tan_constant,%eax
	mov	%eax,%esi
	call	sub_frac2
	shlw	$1,siso
	cmpb	$0,msb_result
	jne	tan_merge_0
	call	move_result_frac1
	and	$0x0ffe00000,dword_frac1(%ebp)
	incw	siso
tan_merge_0:
	inc	dword_expon1
	cmp	$15,dword_expon1
	ja	tan_branch_1
	call	left_shift_frac1_1
	jmp	tan_loop_back_1
tan_branch_1:
	call	left_shift_frac1_8
	mov	$0,dword_frac1(%ebp)
tan_rat_appx:
	call	move_frac1_cop			/ cop <- theta, cop tmp
	call	mulx					/ theta * theta
	mov	dword_expon1,%ecx
	shl	$1,%ecx
	add	$8,%ecx
	call	right_shift_result_cl
	and	$0x0ff000000,result_dword_frac(%ebp)	/ 64 bits only
	call	move_result_frac2
	mov	$constant_c0h,%esi
	call	sub_frac1			/ x <- 1.1 - theta*theta
	call	move_cop_frac2
	call	right_shift_frac2_8
	call	move_cop_frac1
	movb	$9,%cl
	call	right_shift_frac1_cl
	call	move_result_cop					/ cop <- x
	call	addx					/ y <- y' + theta
	call	right_shift_result_1
	call	move_result_dop					/ dop <- y, dop
	dec	dword_expon1			/ is a 67 bits temp
/
/pseudo_multiply
/
tan_loop_back_2:
	shrw	$1,siso
	jnb	tan_merge_2
	call	move_cop_frac1			/ x
	call	move_dop_frac2			/ y
	call	addx					/ x + y
	call	move_result_dop			/ -> dop
	mov	dword_expon1,%ecx
	shl	$1,%ecx
	call	right_shift_frac2_cl
	and	$0x0ffe00000,dword_frac2(%ebp)
	call	subx					/ x - y*2(-2exp)
	call	move_result_cop			/ -> cop
	and	$0x0ff000000,dword_cop(%ebp)
tan_merge_2:
	cmpw	$0,siso
	je	tan_almost_end
	movb	$1,%cl
	mov	$offset_dop,%edi
	call	right_shift_al0
	and	$0x0ffe00000,dword_dop(%ebp)
	dec	dword_expon1
	jmp	tan_loop_back_2
tan_almost_end:
	movb	$8,%cl
	cmpb	$0,msb_dop
	je	tan_merge_3
	movb	$7,%cl
	dec	dword_expon1
tan_merge_3:
	mov	$offset_dop+frac80,%edi
	call	left_shift
	call	move_dop_frac1
	mov	$0x3fff,%eax
	sub	dword_expon1,%eax
	mov	%eax,dword_expon1
/
/normalize y
/
	mov	$offset_operand1,%edi
	call	subtraction_normalize
/
/now x
/
	call	move_cop_frac2
	call	left_shift_frac2_8
	mov	$0x3fff,dword_expon2
	mov	$0x10000\*VALID+POSITIVE,sign2(%ebp)
/
/normalize x
/
	mov	$offset_operand2,%edi
	call	subtraction_normalize
	ret
/tan_pseudo_divide	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
/			tan:
/
/	function:
/		calculates y and x such that y/x = tan(theta)
/
/	inputs:
/		assumes that operand1 is set up (theta).
/
/	outputs:
/		results in result records;  y = result,  x = result2.
/ 		error checking done.
/
/	data accessed:
/		- result_rec_offset		result2_rec_offset
/		- offset_operand1		expon1
/		- word_frac1			lsb_frac1
/		- offset_operand1		offset_operand2
/		- sign2				tag2
/		- expon2			lsb_frac2
/		- msb_frac2			offset_operand2
/		- lsb_result			msb_result
/		- offset_result			siso
/		- offset_cop			lsb_cop
/		- offset_dop			lsb_dop
/		- msb_dop
/
/	data changed:
/		- expon1			word_frac1
/		- lsb_frac1			sign2
/		- tag2				expon2
/		- lsb_frac2			msb_result
/		- lsb_cop			lsb_dop
/		- siso
/
/	procedures called:
/		do_exchange		stack_full?		right_shift
/		left_shift		move_10_bytes		move_constant
/		addx			subx			mulx
/		subtraction_normalize	test_5w			set_p_error
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
tan:	/proc
	call	two_result_op1_chk
get_report:
	jnz	reduce_op1
zero_modulus:
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	jmp	do_exchange
reduce_op1:
	call	mod_qrtr_pi
	cmpb	special,tag1(%ebp)
	jne	tan_tiny_
	cmpb	true,cofunc_flag
	jne	zero_modulus
	mov	$0,dword_frac1(%ebp)
	mov	$0,dword_frac1+frac64(%ebp)
	mov	$0x080000000,dword_frac1+frac32(%ebp)
	mov	$0x10000\*INFINITY+POSITIVE,dword_frac1+sign(%ebp)
	mov	$0x7fff,dword_frac1+expon(%ebp)
	movb	true,signal_z_error_
	jmp	zero_modulus
tan_tiny_:
	mov	$0x3fff,%eax
	sub	dword_expon1,%eax
	cmp	$63,%eax
	jle	tan_calc
tan_tiny_argument:
	cmp	$0x3fff,%eax
	jl	op2_gets_1
	movb	sin_sign,%al
	movb	%al,sign1(%ebp)
	orb	underflow_mask,%gs:sr_errors
	testb	underflow_mask,%gs:sr_masks
	jnz	need_grad_uflow
	add	wrap_around_constant,dword_expon1
	jmp	zero_modulus
need_grad_uflow:
	mov	$offset_operand1,%edi
	mov	$1,%eax
	call	gradual_underflow
	mov	$0,dword_expon1
	movb	denormd,tag1(%ebp)
	jmp	zero_modulus
op2_gets_1:
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	cmpb	true,cofunc_flag
	jne	adjust_sign		/ straight divide superfluous
	movw	res_signs,%ax
	movb	%al,sign1(%ebp)
	movb	%ah,sign2(%ebp)
	push	%gs:sr_masks
/	and		dword ptr %gs:sr_masks, 0fffff3ffh	; set rc = to_nearest
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
	push	trans_info
	jmp	reverse_div
tan_calc:
	push	%gs:sr_masks
/	and		dword ptr %gs:sr_masks, 0fffff3ffh	; set rc = to_nearest
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
	push	trans_info
	call	tan_pseudo_divide
	pop	trans_info
cotan_:
	movw	res_signs,%ax
	movb	%al,sign1(%ebp)
	movb	%ah,sign2(%ebp)
	push	trans_info
	cmpb	true,cofunc_flag
	jne	straight_div
reverse_div:
	call	op1_into_op2
	jmp	get_a_bit
straight_div:
	call	op2_into_op1
get_a_bit:
	cmpb	true,added_one
	jne	move_tan_result
	orb	$a_mask,%gs:sr_flags
move_tan_result:
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
	pop	trans_info
	pop	%gs:sr_masks
adjust_sign:
	movb	sin_sign,%al
	xorb	cos_sign,%al
	movb	%al,sign1(%ebp)
jump_do_exchange:
	jmp	do_exchange_leave_a_bit
/tan	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
swap_ops:	/proc
	mov	$offset_operand1,%esi
	call	push_5_dw_at_esi
	lea	dword_frac2(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op
	mov	$offset_operand2,%esi
	call	pop_5_dw_at_esi
	ret
/swap_ops	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
op1_into_op2:	/proc
	call	swap_ops
op2_into_op1:
	push	%gs:sr_masks
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
	call	divid						/ floating-point divide
	call	post_op_round
	pop	%gs:sr_masks
	ret

/op1_into_op2	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
/
sin_specific:	/proc

	mov	$offset_result,%esi
	call	push_5_dw_at_esi			/ save (x/y)
	call	op2_into_op1				/ result <-- (y/x)
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op				/ op1 <-- (y/x)
	mov	$offset_operand2,%esi
	call	pop_5_dw_at_esi				/ op2 <-- (x/y)
	or	$0x0f00,%gs:sr_masks 	/ set rc to chop!
	movb	$add_op,operation_type(%ebp)
	call	subadd
	call	post_op_round
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op			/ op2 <-- chop(x/y + y/x)
	mov	$offset_operand1,%esi
	call	move_unpacked_1_to_esi		/ op1 <-- 1
	inc	dword_expon1				/ op1 <-- 2
	call	op2_into_op1		/ res <-- chop{2/chop(x/y + y/x)} 
	ret

/sin_specific	endp
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/
/				sin:
/
sin:	/proc
	call	one_result_op1_chk
sin_report:
	jnz	sin_reduce_op1
	movb	$sin_op,op_type
	movb	sign1(%ebp),%al
	movb	%al,sin_sign
sin_0_modulus:
	lea	dword_frac1(%ebp),%esi
	lea	result_dword_frac(%ebp),%edi
	call	move_op_to_op
	jmp	check_op_type
sin_reduce_op1:
	call	mod_qrtr_pi
	cmpb	special,tag1(%ebp)
	jne	sin_tiny_
/	testb	q,	1			; was operand (2k+1)*qrtr_pi?
/	jz		sin_cofunc?		; if so, set precision error
/	orb		%gs:sr_errors,	inexact_mask
/sin_cofunc?:
	cmpb	true,cofunc_flag
	jne	sin_0_modulus
	jmp	cos_0_modulus
sin_tiny_:
	cmpb	true,cofunc_flag
	je	check_cos_lim
check_sin_lim:
	mov	$0x3fff,%eax
	sub	dword_expon1,%eax
	cmp	$30,%eax
	jl	sin_calc
	jg	sin_tiny_argument
	cmp	$0x08d100000,dword_frac1+frac32(%ebp)
	ja	sin_calc
	jb	sin_0_modulus
	cmp	$0,dword_frac1+frac64(%ebp)
	je	sin_0_modulus
sin_tiny_argument:
	cmp	$0x3fff,%eax
	jl	sin_0_modulus
	orb	underflow_mask,%gs:sr_errors
	testb	underflow_mask,%gs:sr_masks
	jnz	sin_grad_uflow
	add	wrap_around_constant,dword_expon1
	jmp	sin_0_modulus
sin_grad_uflow:
	mov	$offset_operand1,%edi
	mov	$1,%eax
	call	gradual_underflow
	mov	$0,dword_expon1
	movb	denormd,tag1(%ebp)
	jmp	sin_0_modulus
sin_calc:
	push	trans_info
	push	%gs:sr_masks
	and	$0x0fffff3ff,%gs:sr_masks	/ set rc = to_nearest
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
/	dec		dword_expon1	; compute tan((op1)/2)
	inc	%eax			/ eax holds -(true_expon)
	call	tan_pseudo_divide
	mov	$offset_operand1,%esi
	call	push_5_dw_at_esi			/ save y
	mov	$offset_operand2,%esi
	call	push_5_dw_at_esi			/ save x
	call	op1_into_op2				/ result <-- x/y
	mov	$offset_operand2,%esi
	call	pop_5_dw_at_esi				/ op2 <-- x
	mov	$offset_operand1,%esi
	call	pop_5_dw_at_esi				/ op1 <-- y
	call	sin_specific
	pop	%gs:sr_masks
	pop	trans_info
	andb	$ -1 ! a_mask,%gs:sr_flags
	cmpb	true,added_one
	jne	check_op_type
	orb	$a_mask,%gs:sr_flags
	jmp	check_op_type

/sin	endp
/
/-----------------------------------------------------------------------
/				cos:
/
/-----------------------------------------------------------------------
/
cos_specific:	/proc

	lea	dword_frac1(%ebp),%esi
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op				/ op2 <-- x/y
	call	mult						/ res <-- (x/y)**2
	call	post_op_round
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op				/ op1 <-- (x/y)**2
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi		/ op2 <-- 1
	movb	$add_op,operation_type(%ebp)
	call	subadd
	call	post_op_round				/ res <-- 1 + (x/y)**2
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op				/ op2 <-- 1 + (x/y)**2
	mov	$offset_operand1,%esi
	call	move_unpacked_1_to_esi		/ op1 <-- 1
	inc	dword_expon1			/ op1 <-- 2
	call	op2_into_op1			/ res <-- 2/(1 + (x/y)**2)
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op			/ op2 <-- 2/(1 + (x/y)**2)
	mov	$offset_operand1,%esi
	call	move_unpacked_1_to_esi		/ op1 <-- 1
	movb	$sub_op,operation_type(%ebp)
	call	subadd
	call	post_op_round			/ res <-- 1 - {2/(1 + (x/y)**2)}
	ret

/cos_specific	endp
/
/-----------------------------------------------------------------------
/
cos:	/proc
	call	one_result_op1_chk
cos_report:
	jnz	cos_reduce_op1
	movb	$cos_op,op_type
	movb	positive,cos_sign
cos_0_modulus:
	mov	$offset_result,%esi
	call	move_unpacked_1_to_esi
check_op_type:
	cmpb	$cos_op,op_type
	je	attach_cos_sign
attach_sin_sign:
	movb	sin_sign,%al
	jmp	sign_done
attach_cos_sign:
	movb	cos_sign,%al
sign_done:
	movb	%al,result_sign(%ebp)
send_result:
	mov	$offset_result,%edi
	jmp	put_si_result
cos_reduce_op1:
	call	mod_qrtr_pi
	cmpb	special,tag1(%ebp)
	jne	cos_tiny_
/	testb	q,	1			; was operand (2k+1)*qrtr_pi?
/	jz		cos_cofunc?		; if so, set precision error
/	orb		%gs:sr_errors,	inexact_mask
/cos_cofunc?:
	cmpb	true,cofunc_flag
	jne	cos_0_modulus
	mov	$0,result_dword_frac(%ebp)
	mov	$0,result_dword_frac+frac64(%ebp)
	mov	$0,result_dword_frac+frac32(%ebp)
	mov	$0x10000\*SPECIAL+POSITIVE,result_sign(%ebp)
	mov	$0,dword_result_expon
	jmp	attach_cos_sign
cos_tiny_:
	cmpb	true,cofunc_flag
	je	check_sin_lim
check_cos_lim:
	mov	$0x3fff,%eax
	sub	dword_expon1,%eax
	cmp	$62,%eax
	jl	cos_calc
	jg	cos_0_modulus
	cmp	$0x080000000,dword_frac1+frac32(%ebp)
	ja	cos_calc
	cmp	$0,dword_frac1+frac64(%ebp)
	je	cos_0_modulus
cos_calc:
	push	trans_info
	push	%gs:sr_masks
	and	$0x0fffff3ff,%gs:sr_masks	/ set rc = to_nearest
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
/	dec	dword_expon1				; compute tan((op1)/2)
	inc	%eax				/ eax holds -(true_expon)
	call	tan_pseudo_divide
	call	op1_into_op2				/ result <-- x/y
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac1(%ebp),%edi
	call	move_op_to_op				/ op1 <-- x/y
	call	cos_specific				/ result <-- cos
	pop	%gs:sr_masks
	pop	trans_info
	andb	$ -1 ! a_mask,%gs:sr_flags
	cmpb	true,added_one
	jne	check_op_type
	orb	$a_mask,%gs:sr_flags
	jmp	check_op_type

/cos	endp
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			sincos:

sincos:	/proc
	call	two_result_op1_chk
combo_get_report:
	jnz	combo_reduce_op1
	xor	%eax,%eax
	movb	sign1(%ebp),%ah
	mov	%eax,trans_info
combo_0_mod:
	mov	$offset_operand2,%esi
	call	move_unpacked_1_to_esi
combo_cofunc_:
	cmpb	true,cofunc_flag
	jne	attach_signs
switch_ops:
	call	swap_ops
attach_signs:
	movw	res_signs,%ax
	movb	%al,sign1(%ebp)
	movb	%ah,sign2(%ebp)
results_sent:
	jmp	do_exchange
combo_reduce_op1:
	call	mod_qrtr_pi
	cmpb	special,tag1(%ebp)
/	jne		combo_tiny?
/	testb	q,		1
	jz	combo_0_mod
/	orb		%gs:sr_errors,	inexact_mask
/	jmp	short	combo_0_mod
combo_tiny_:
	mov	$0x3fff,%eax
	sub	dword_expon1,%eax
	cmp	$62,%eax
	jl	combo_sin_calc
	jg	combo_tiny_arg
	cmp	$0x080000000,dword_frac1+frac32(%ebp)
	ja	combo_sin_calc
	cmp	$0,dword_frac1+frac64(%ebp)
	jne	combo_sin_calc
combo_tiny_arg:
	cmp	$0x3fff,%eax
	jl	combo_0_mod
	orb	underflow_mask,%gs:sr_errors
	testb	underflow_mask,%gs:sr_masks
	jnz	combo_grad_uflow
	add	wrap_around_constant,dword_expon1
	jmp	combo_0_mod
combo_grad_uflow:
	mov	$offset_operand1,%edi
	mov	$1,%eax
	call	gradual_underflow
	mov	$0,dword_expon1
	movb	denormd,tag1(%ebp)
	jmp	combo_0_mod
combo_sin_calc:
	push	trans_info
	push	%gs:sr_masks
	and	$0x0fffff3ff,%gs:sr_masks	/ set rc = to_nearest
	or	$0x0300,%gs:sr_masks 	/ set precision control to prec64
	cmp	$30,%eax
	jl	push_flags
	jg	force_zf_1
	cmp	$0x08d100000,dword_frac1+frac32(%ebp)
	ja	push_flags
	jb	force_zf_1
	cmp	$0,dword_frac1+frac64(%ebp)
	jne	push_flags
force_zf_1:
	xor	%ebx,%ebx					/ zf set iff arg is tiny for sin
	mov	$offset_operand1,%esi		/ save tiny op1 as sin(op1)
	call	push_5_dw_at_esi		/ does not affect flags or eax
push_flags:
	pushf
/	dec	dword_expon1		; compute tan((op1)/2)
	inc	%eax				/ eax holds -(true_expon)
	call	tan_pseudo_divide
	popf
	jz	calc_x_over_y
	mov	$offset_operand1,%esi
	call	push_5_dw_at_esi			/ save y
	mov	$offset_operand2,%esi
	call	push_5_dw_at_esi			/ save x
calc_x_over_y:
	pushf
	call	op1_into_op2				/ result <-- x/y
	popf
	jz	save_x_over_y
	mov	$offset_operand2,%esi
	call	pop_5_dw_at_esi				/ op2 <-- x
	mov	$offset_operand1,%esi
	call	pop_5_dw_at_esi				/ op1 <-- y
save_x_over_y:
	mov	$offset_result,%esi
	call	push_5_dw_at_esi			/ save	x/y
	jz	combo_cos_calc
	pushf
	call	sin_specific				/ result <-- sin
	popf
combo_cos_calc:
	mov	$offset_operand1,%esi
	call	pop_5_dw_at_esi				/ op1 <-- x/y
	jz	tiny_sin_cos_entry
	mov	$offset_result,%esi
	call	push_5_dw_at_esi			/ save	computed sin
tiny_sin_cos_entry:
	call	cos_specific				/ result <-- cos
	mov	$offset_operand1,%esi
	call	pop_5_dw_at_esi				/ op1 <-- sin
	lea	result_dword_frac(%ebp),%esi
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op				/ op2 <-- cos
	pop	%gs:sr_masks
	pop	trans_info
	jmp	combo_cofunc_

/sincos	endp
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			test_5w, test_4w, test_3w
/
/	function:
/		test 3, 4, or 5 consecutive words for zero
/
/	inputs:
/       	least significant word pointed to by ss:[bp + di]
/		ax must be clear upon entry for test_4w and test_3w
/
/	outputs:
/		if variable = 0, ax = 00000h; zf = 1
/		otherwise,       ax = 0ffffh; zf = 0
/
/	data accessed:
/
/	data changed:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
test_5w:	/proc
	movw	8(%ebp,%edi),%ax	/ dump the data into ax
test_4w:
	orw	6(%ebp,%edi),%ax
test_3w:
	orw	4(%ebp,%edi),%ax
	orw	2(%ebp,%edi),%ax
	orw	(%ebp,%edi),%ax
	jnz	its_not_zero	/ branch if the number is nonzero
	ret			/ return with ax = 00000h, zf = 1
its_not_zero:
	movw	$0x0ffff,%ax	/ return with ax = 0ffffh, zf = 0
	ret
/test_5w	endp
/
test_6w:	/proc
	mov	(%ebp,%edi),%eax	/ dump the data into ax
	or	4(%ebp,%edi),%eax
	or	8(%ebp,%edi),%eax
	jnz	non_0	/ branch if the number is nonzero
	ret			/ return with eax = 000000000h, zf = 1
non_0:
	mov	$0x0ffffffff,%eax	/ return with eax = 0ffffffffh, zf = 0
	ret
/test_6w	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			clear_5w, set_5w, set__4w, set_3w
/
/	function:
/		sets 3, 4, or 5 consecutive words to zero or given value
/
/	inputs:
/       	least significant word pointed to by ss:[ebp + edi]
/		ax contains value given to store for set routines
/
/	outputs:
/		ax contains zero for the clear_5w routine
/		eax "        "    "   "  clear_6w    "
/	data accessed:
/
/	data changed:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
clear_6w:	/proc
	xor	%eax,%eax
set_6w:
	mov	%eax,(%ebp,%edi)	/ store the value in eax
	mov	%eax,4(%ebp,%edi)
	mov	%eax,8(%ebp,%edi)
	ret
/clear_6w	endp
/
clear_5w:	/proc
	xorw	%ax,%ax
set_5w:
	movw	%ax,8(%ebp,%edi)	/ store the value in ax
set_4w:
	movw	%ax,6(%ebp,%edi)
set_3w:
	movw	%ax,4(%ebp,%edi)
	movw	%ax,2(%ebp,%edi)
	movw	%ax,(%ebp,%edi)
	ret
/clear_5w	endp
/
/
/ common entries to shift and move subroutines
/
/ right_shift group
/
right_shift_dword_exp_tmp:
	mov	dword_exp_tmp,%ecx
	shlb	$1,%cl
	jmp	right_shift_al0
right_shift_result_1:
	movb	$1,%cl
right_shift_result_cl:
	mov	$offset_result,%edi
	jmp	right_shift_al0
right_shift_frac2_8:
	movb	$8,%cl
right_shift_frac2_cl:
	mov	$offset_operand2,%edi
	jmp	right_shift_al0
right_shift_frac1_8:
	movb	$8,%cl
right_shift_frac1_cl:
	mov	$offset_operand1,%edi
right_shift_al0:
	movb	$0,%al
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/            right_shift:
/
/    function:
/        shifts a 12-byte number right by amount in ecx register.
/
/    inputs:
/        low bit of al contains bit to inject from left.
/        edi contains the offset of the least significant byte
/        of the 12-byte number.
/
/    outputs:
/        on return, edx is non-zero if any ones have been shifted out.
/		 edi has the same value it had upon entry.
/    data accessed:
/
/    data changed:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
right_shift:	/proc
	xor	%edx,%edx  / initialize edx, the "sticky word"
	rorb	$1,%al    / move lsb to msb
	movsbl	%al,%eax      / transmit sign to ah and hi_word(eax)
	push	%eax             / stack insert bit replica
	movb	$2,%ch       / one less than # of dwords to get bit-shifted
	cmpb	$32,%cl      / does shift_count exceed 31?
	jb	bits_shft_rt    / if not, just do the bit shifting;
	cmpb	$96,%cl        / does shift_count exceed 95?
	jb	dwords_shft_rt    / if not, proceed with dword shifts;
	je	get_out_shfts   / else, if shift_count exceeds 96
	orw	%ax,%dx      /       then bit shifted in is also shifted out
get_out_shfts:
	or	(%ebp,%edi),%edx     / all 3 dwords are "shifted out,"
	or	4(%ebp,%edi),%edx / and or'd to edx
	or	8(%ebp,%edi),%edx
	movb	%ah,%al              / copy insert bit throughout eax
	mov	%eax,(%ebp,%edi)       /
	mov	%eax,4(%ebp,%edi)      /
	mov	%eax,8(%ebp,%edi)    / replace all 96 bits by insert bit.
	pop	%eax
	ret
/ 
dwords_shft_rt:
	push	%ds                / save a?msr
	push	%ss                / copy stack segment
	pop	%ds                / to ds
	push	%ss
	pop	%es              / and es
	add	%ebp,%edi
	mov	%edi,%esi
	movb	%cl,%bl      / save bit_shft_count in bl
	and	$0x60,%ecx     / get dword_shft_count
	shr	$5,%ecx       / normalize dword_shft_count
	movb	%cl,%al      / save dword_shft_count in al
dword_shft_chk:
	or	(%esi),%edx
	add	$4,%esi
	loop	dword_shft_chk
	movw	$3,%cx
	subb	%al,%cl
	movb	%cl,%bh
	rep	
	movsl
	movb	%al,%cl
	movb	%ah,%al
	rep	
	stosl
	sub	$12,%edi       / adjust edi
	sub	%ebp,%edi      / to value it had at entry
	pop	%ds
	movw	%bx,%cx
bits_shft_rt:
	pop	%eax
	push	%edi
	push	%eax
	mov	(%ebp,%edi),%eax
	xor	%ebx,%ebx
	shrdl	%eax,%ebx  / cl is interpreted mod 32
	or	%ebx,%edx
bit_shft_loop:
	mov	4(%ebp,%edi),%eax
	shrdl	%eax,(%ebp,%edi)
	add	$4,%edi
	decb	%ch
	jnz	bit_shft_loop
	pop	%eax
	movb	%ah,%al
	shrdl	%eax,(%ebp,%edi)
	pop	%edi
right_shift_done:
	clc
	ret
/
/right_shift	endp
/
/
/ left_shift group
/
left_shift_result_cl:
	mov	$offset_result+frac80,%edi
	jmp	left_shift
left_shift_frac2_8:
	movb	$8,%cl
left_shift_frac2_cl:
	mov	$offset_operand2+frac80,%edi
	jmp	left_shift
left_shift_frac1_8:
	movb	$8,%cl
	jmp	left_shift_frac1_cl
left_shift_frac1_1:
	movb	$1,%cl
left_shift_frac1_cl:
	mov	$offset_operand1+frac80,%edi
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			left_shift:
/
/	function:
/		shifts a 10-byte number left by amount in cl reg.
/
/	input:
/		di contains offset of low-byte.
/
/	outputs:
/		bits shifted off the left end are lost.
/
/	data accessed:
/
/	data changed:
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
left_shift:	/proc
	cmpb	$80,%cl				/ more than 80 bits?
	jle	left_count			/ no, form left count
	movb	$80,%cl				/ yes, 80 is the max
left_count:
	movb	%cl,%al				/ form byte count
	andb	$0x78,%al				/ = bits 3-6 of amount
	jz	leftover_bits			/ branch if count < 8
	push	%ecx				/ stack bit count
	push	%edi				/ stack frac offset
	push	%ds				/ save a?msr
	cbtw					/ clear ah
	shrw	$3,%ax				/ normalize byte count
	decw	%ax
	mov	$0x0009,%ecx			/ count = 10 - ax
	add	%ecx,%edi
	movswl	%ax,%eax
	sub	%eax,%ecx				/ dest = ss:bp+si
	add	%ebp,%edi
	mov	%edi,%esi				/ dest = ss:bp+di
	sub	%eax,%esi
	dec	%esi
	push	%ss				/ copy ss to ds and es
	pop	%ds
	push	%ss
	pop	%es
	std					/ set direction flag
	rep	
	movsb					/ shift bytes left
	movb	%cl,(%edi)			/ clear next byte
	mov	%eax,%ecx				/ form zero count
	mov	%edi,%esi				/ form string offsets
	dec	%edi
	rep	
	movsb					/ clear rest of string
	cld					/ clear direction flag
	pop	%ds				/ reload a?msr
	pop	%edi				/ reload frac offset
	pop	%ecx				/ reload bit count
leftover_bits:
	and	$0x00000007,%ecx			/ bits 0-2 = bit count
	jz	left_shift_done			/ exit if all done
left_shift_bits:
	shlw	$1,(%ebp,%edi)		/ shift five words
	rclw	$1,2(%ebp,%edi)	/ one bit left, using
	rclw	$1,4(%ebp,%edi)	/ the carry flag out
	rclw	$1,6(%ebp,%edi)
	rclw	$1,8(%ebp,%edi)
	loop	left_shift_bits			/ loop bit count times
left_shift_done:
	ret
/left_shift	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			one_left_normalize:
/
/	function:
/		normalizes following a multiplication.
/
/	inputs:
/		assumes fraction is not zero.  di = frac offset.
/
/	outputs:
/		shifts fraction left and decrements exponent until
/		fraction normalized, but no more than one left shift.
/               zf is reset if the fraction is normalized upon return.
/
/	data accessed:
/
/	data changed:
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
one_left_normalize:	/proc
	testb	$0x80,frac80+9(%ebp,%edi)	/normalized?
	jnz	one_left_norm_done		/quit if normalized.
	push	%edi				/ save frac offset
	mov	$0x00000001,%ecx			/ load loop count
	add	$2,%edi
	call	left_shift_bits			/shift 5 words left 1
	pop	%edi				/ reload frac offset
	dec	expon(%ebp,%edi)	/decrement exponent
	testb	$0x80,frac80+9(%ebp,%edi)	/normalized?
one_left_norm_done:
	ret
/one_left_normalize	endp
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			subtraction_normalize:
/
/	function:
/		normalizes following a subtraction.
/
/	inputs:
/		assumes fraction is not zero.  di = frac offset
/
/	outputs:
/		shifts fraction left and decrements exponent
/		until fraction normalized.
/
/	data accessed:
/
/	data changed:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
subtraction_normalize:	/proc
	call	one_left_normalize		/ normalize one bit
	jz	subtraction_normalize		/quit when normalized.
	ret
/subtraction_normalize	endp
/
/
/ move_10_bytes group
/
move_frac2_frac1:
	mov	$offset_operand1+2,%edi
move_frac2:
	mov	$offset_operand2+2,%esi
	jmp	move_10_bytes
move_frac1_result:
	mov	$offset_result+2,%edi
	jmp	move_frac1
move_frac1_cop:
	mov	$offset_cop+2,%edi
	call	move_frac1
move_frac1_frac2:
	mov	$offset_operand2+2,%edi
move_frac1:
	mov	$offset_operand1+2,%esi
	jmp	move_10_bytes
move_dop_frac2:
	mov	$offset_operand2+2,%edi
	jmp	move_dop
move_dop_frac1:
	mov	$offset_operand1+2,%edi
move_dop:
	mov	$offset_dop+2,%esi
	jmp	move_10_bytes
move_cop_frac2:
	mov	$offset_operand2+2,%edi
	jmp	move_cop
move_cop_frac1:
	mov	$offset_operand1+2,%edi
move_cop:
	mov	$offset_cop+2,%esi
	jmp	move_10_bytes
move_result_cop:
	mov	$offset_cop+2,%edi
	jmp	move_result
move_result_dop:
	mov	$offset_dop+2,%edi
	jmp	move_result
move_result_frac2:
	mov	$offset_operand2+2,%edi
	jmp	move_result
move_result_frac1:
	mov	$offset_operand1+2,%edi
move_result:
	mov	$offset_result+2,%esi
move_10_bytes:
	add	%ebp,%esi		/ add global record offset
	push	%ds		/ save a?msr
	push	%ss		/ load source segment register
set_destination:
	pop	%ds
	push	%ss		/ load destination segment register
	pop	%es		/ into es
	add	%ebp,%edi		/ add global record offset
	mov	$5,%ecx		/ move five words
	rep	
	movsw
	pop	%ds		/ reload a?msr
	ret
/
/ add, sub, mul, move_constant group
/
add_half1_frac2:
	mov	$add_half_pattern_1,%esi
	call	add_frac2
	mov	$offset_result,%edi
	jmp	addition_normalize
add_frac2:
	call	move_constant_frac2
	jmp	addx
sub_frac1:
	mov	$offset_operand1+2,%edi
sub_constant:
	call	move_constant
	jmp	subx
sub_frac2:
	mov	$offset_operand2+2,%edi
	jmp	sub_constant
mul_frac2:
	call	move_constant_frac2
	jmp	mulx
move_constant_frac2:
	mov	$offset_operand2+2,%edi
move_constant:
	push	%ds		/ save a?msr
	push	%cs		/ load code segment base into ds
	jmp	set_destination	/ move five words
/	.byte	'bernard	verreau,san	francisco,ca	.	1983'
/
/a_med	ends
/
/	end
