/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"arith.s"

	.ident	"@(#)kern-fp:arith.s	1.1"

/$tt("80387	emulator	+ + + arithmetic + + +")
/ ************************************************************************
/
/			a r i t h . m o d
/			=================
/ 
/	===============================================================
/               intel corporation proprietary information
/    this software  is  supplied  under  the  terms  of  a  license
/    agreement  or non-disclosure agreement  with intel corporation
/    and  may not be copied  or disclosed except in accordance with
/    the terms of that agreement.                                  
/	===============================================================
/
/	function:
/		implements add, subtract, multiply, and divide.
/
/	public procedures:
/		move_op_to_result		overflow_response
/		underflow_response		put_max_nan
/		put_indefinite			arith
/
/ *************************************************************************
/
/...March 3, 1987...
/
/	.file	"a_mar"
#include	"e80387.h"
/$eject
/...declare status register segment...
/
	.data	/a_msr	segment	rw	public
/	extrn	sr_masks
/a_msr	ends
/
	.text	/a_med	segment	er	public
/	assume	%ds:a_med
/	extrn	addition_normalize,round
/	extrn	gradual_underflow,put_result
/	extrn	pop_free,subadd,mult
/	extrn	divid,set_up_indefinite,test_4w
/	extrn	special_round_test,directed_round_test
/	extrn	get_precision,u_masked_,set_i_masked_
/	extrn	d_masked_,affine_infinity_,i_masked_
/	extrn	o_masked_,set_p_error,set_u_error
/	extrn	set_o_error,set_z_masked_,set_d_masked_
/	extrn	set_stk_u_error,d_error_,get_rnd_control
/
	.globl	put_indefinite
	.globl	move_op_to_result
	.globl	put_max_nan
	.globl	overflow_response
	.globl	underflow_response
	.globl	arith
	.globl	norm_denorm
	.globl put_arith_result
	.globl set_up_nan_return
/$eject
/
/...define some floating point constants...
/
indefinite_pattern:	
	.value	0,0,0,0x0c000,0x0ffff
infinity_pattern:	
	.value	0,0,0,0x8000,0x7fff
zero_pattern:	
	.value	0,0,0,0,0
max_valid_pattern:	
	.value	0x0ffff,0x0ffff,0x0ffff,0x0ffff,0x7ffe
arith_table:	
	.long	subadd,subadd,mult,divid
/
/...the following jump tables refer to the following op1/op2 cases 
/
/   (where v=valid, z=zero, f=infinity, and d=denormal):
/		v/v,v/z,v/f,v/d,z/v,z/z,z/f,z/d,f/v,f/z,f/f,f/d,d/v,d/z,d/f and d/d,
/	 in that order ..
/add_table
special_table:	
	.long	handle_non_special_cases,handle_non_special_cases
	.long	second_operand,only_op2_denormd
	.long	handle_non_special_cases,handle_non_special_cases
	.long	second_operand,only_op2_denormd
	.long	first_operand,first_operand
	.long	add_sub_infinities,derror_with_first_operand
	.long	only_op1_denormd,only_op1_denormd
	.long	derror_with_second_operand,both_ops_denormd
/sub_table
	.long	handle_non_special_cases,handle_non_special_cases
	.long	neg_second_operand,only_op2_denormd
	.long	handle_non_special_cases,handle_non_special_cases
	.long	neg_second_operand,only_op2_denormd
	.long	first_operand,first_operand
	.long	add_sub_infinities,derror_with_first_operand
	.long	only_op1_denormd,only_op1_denormd
	.long	derror_with_neg_second_operand,both_ops_denormd
/mul_table
	.long	handle_non_special_cases,exor_signed_zero
	.long	exor_signed_infinity,only_op2_denormd
	.long	exor_signed_zero,exor_signed_zero
	.long	invalid_error_detected,denormd_exor_signed_zero
	.long	exor_signed_infinity,invalid_error_detected
	.long	exor_signed_infinity,denormd_exor_signed_infinity
	.long	only_op1_denormd,denormd_exor_signed_zero
	.long	denormd_exor_signed_infinity,both_ops_denormd
/div_table
	.long	handle_non_special_cases,divide_by_zero
	.long	exor_signed_zero,only_op2_denormd
	.long	exor_signed_zero,invalid_error_detected
	.long	exor_signed_zero,denormd_exor_signed_zero
	.long	exor_signed_infinity,exor_signed_infinity
	.long	invalid_error_detected,denormd_exor_signed_infinity
	.long	only_op1_denormd,divide_by_zero
	.long	denormd_exor_signed_zero,both_ops_denormd
/
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			put_indefinite:
/
/	function:
/		sets unpacked result to indefinite.
/
/	inputs:
/
/	outputs:
/		result set to indefinite; tag set to invalid.
/
/	data accessed:
/		- result_sign			result_tag
/
/	data changed:
/		- result
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
put_indefinite:	/proc
	mov	$indefinite_pattern,%esi
	movb	negative,result_sign(%ebp)
/ note that entry points below do not affect result_sign.
put_inv_tag:
	movb	inv,result_tag(%ebp)	/ fall into set_constant_result
set_constant_result:
	lea	offset_result+4(%ebp),%edi
	push	%ds                       / save a?msr
	push	%cs
	pop	%ds
	push	%ss
	pop	%es
	mov	$0x0002,%ecx
	rep	
	movsl	                    / move fraction
	mov	%ecx,result_dword_frac(%ebp)        / clear ls dword
	mov	(%esi),%eax			            / move exponent
	movw	%ax,result_expon(%ebp)		/ to result
	pop	%ds				                    / reload a?msr
	ret
/put_indefinite	endp
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			move_op_to_result:
/
/	function:
/		moves operand to result.
/
/	inputs:
/		assumes ss:[ebp].esi points to op1 or op2
/
/	outputs:
/		result in result
/
/	data accessed:
/		- expon1			offset_result
/		- result_expon
/
/	data changed:
/		- result
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
move_op_to_result:	/proc
	push	%ds			                        / save a?msr
	add	%ebp,%esi
	push	%ss
	pop	%ds
	push	%ss
	pop	%es
	lea	offset_result(%ebp),%edi
	mov	$[offset_operand2-offset_operand1]\/4,%ecx
	rep	
	movsl		                        / move 5 dwords
	pop	%ds	                        / reload a?msr
	ret
/move_op_to_result	endp
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			overflow_response:
/
/	function:
/		responds to detected overflow.
/
/	inputs:
/
/	outputs:
/		overflow error indication(s) set when appropriate
/		correct masked or unmasked result
/
/	data accessed:
/		- result_sign			result_tag
/		- result_expon			msb_result
/
/	data changed:
/		- result
/
/	procedures called:
/		special_round_test		set_o_error
/		o_masked?			    set_p_error
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
overflow_response:	/proc
	orb	overflow_mask, %gs:sr_errors	/set overflow error
	testb	overflow_mask, %gs:sr_masks
	jnz	masked_overflow		   / branch if ovflw masked
/ note that p_error may have been set by the first rounding of the result,
/ and that rounded result will now have its exponent's bias adjusted:
reduce_exponent:
	sub	wrap_around_constant,dword_result_expon	/ subtract
	cmp	$0x7fff, dword_result_expon	/wrap around constant
	jge	masked_overflow	            / bad! should give qnan indefinite
unmasked_ov_un_rounds:
	cmpb	true, rnd1_inexact	/detect inexact result
	jne	give_valid_o_result	/ if exact, branch to replace source
	orb	$inexact_mask, %gs:sr_errors	/ else, set p_error
	cmpb	true, added_one	/was rounding done by adding one
	jne	give_valid_o_result	/ if not branch to replace source
	orb	$a_mask, %gs:sr_flags	/ else set A_bit
	jmp	give_valid_o_result
masked_overflow:
	orb	$inexact_mask, %gs:sr_errors	/set p_error
	call	get_rnd_control
	cmpb	rnd_to_zero,%al
	je	put_max_valid
	movb	result_sign(%ebp),%al	        / check for special
	call	special_round_test		            / rounding case
	jnz	put_max_valid
	orb	$a_mask, %gs:sr_flags	/set a_bit
put_infinity:
	mov	$infinity_pattern,%esi
	jmp	put_inv_tag
put_max_valid:
/	mov	    [ebp].result_tag,   valid
	mov	$0x00007ffe,dword_result_expon
	mov	$0,result_dword_frac(%ebp)
	mov	$0x0ffffffff,result_dword_frac+frac64(%ebp)
	mov	$0x0ffffffff,result_dword_frac+frac32(%ebp)
	cmpb	$add_op,operation_type(%ebp)
	jl	give_valid_o_result
	cmpb	$div_op,operation_type(%ebp)
	jg	give_valid_o_result
	movb	$precision_mask,%dl	/ load precision field mask
	andb	%gs:sr_controls,%dl		/ mask in precision control
	cmpb	prec53,%dl
	jg	give_valid_o_result
	je	purge_11_bits
	mov	$0,result_dword_frac+frac64(%ebp)
	mov	$0x0ffffff00,result_dword_frac+frac32(%ebp)
	jmp	give_valid_o_result
purge_11_bits:
	and	$0x0fffff800,result_dword_frac+frac64(%ebp)

/	mov	    esi,    offset max_valid_pattern	; set result to max
/	jmp	    set_constant_result		            ; valid number

give_valid_o_result:
	movb	valid,result_tag(%ebp)
	ret
/overflow_response	endp
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			underflow_response:
/
/	function:
/		responds to a detected underflow 
/
/	inputs:
/
/	outputs:
/		sets underflow error indication(s) if appropiate
/		correct masked or unmasked results in result
/
/	data accessed:
/		- offset_result			result_sign
/		- result_tag			result_expon
/		- result_word_frac
/
/	data changed:
/		- result
/
/	procedures called:
/		directed_round_test		gradual_underflow
/		round				    test_4w
/		set_u_error			    u_masked?
/		get_precision
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
underflow_response:	/proc
	testb	underflow_mask, %gs:sr_masks	/is underflow masked
	jnz	do_grad_underflow		/ yes, don't set flag
	orb	underflow_mask, %gs:sr_errors	/no, set error flag
increase_exponent:
	addl	wrap_around_constant,dword_result_expon / add wrap-around
	cmp	$0, dword_result_expon	/wrap_around constant
	jg	unmasked_ov_un_rounds	/ bad! shuld give qnan indef
	sub	wrap_around_constant, dword_result_expon / subtract
do_grad_underflow:
	mov	$offset_result,%edi                  / do gradual underflow
	mov	$0x0001,%eax			/ minimum expon is 0001
	call	gradual_underflow
	call	get_precision
	movb	true,%al
	mov	$offset_result,%edi		/ do second round
	call	round
	cmpw	$0, rnd_history		
	je	prepare_zero_expon
	orb	underflow_mask + inexact_mask, %gs:sr_errors /set both errors
	cmpb	true, added_one		/ was rounding done by adding one
	jne	prepare_zero_expon	/if not, branch for zero expon
	orb	$a_mask, %gs:sr_flags	/else, set a_bit & if incrementing
	testb	$0x80, msb_result	/The significand caused the most
	jz	prepare_zero_expon	/significant bit of result_frac to
	movb	valid, result_tag(%ebp) / be set, tag result "valid" & return
	ret
prepare_zero_expon:
	xor	%eax, %eax
clear_expon:
	mov	%eax,dword_result_expon		/ set exponent to zero
	mov	$result_dword_frac+4,%edi
	call	test_4w				/ if fraction nonzero,
	movb	denormd,result_tag(%ebp)
	jnz	accept_tag			/ tag as denormal
	movb	special,result_tag(%ebp)	/ else, tag as zero
accept_tag:
	ret
/underflow_response	endp
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			norm_denorm:
/
/	function:
/		normalizes an unpacked operand presumed to have been tagged
/		"denormd".  includes case in which operand is so tagged because
/		it was a single or double format denormal, which has already
/       been normalized in the course of extending it to unpacked format.
/		also includes case in which operand is a pseudo-denormal.
/
/	inputs:  edi points to the offset of the record to be normalized.
/
/	outputs:
/		8 bytes beginning at [ebp + edi + frac64] are left shifted until
/		the bit in position b63 is set.
/		the 32 bit exponent field is 1 - shift_count
/
/	data accessed:
/		- dword ptr [ebp + edi + expon]
/		- byte ptr [ebp + edi + msb]
/		- dword ptr [ebp + edi + frac32],	dword ptr [ebp + edi + frac64]
/
/	data changed:
/		- dword ptr [ebp + edi + expon]
/		- byte ptr [ebp + edi + msb]
/		- dword ptr [ebp + edi + frac32],	dword ptr [ebp + edi + frac64]
/
/	procedures called:
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
norm_denorm:	/proc
	cmp 	$0,expon(%ebp,%edi)
	jne	norm_denorm_done
	mov 	$1,expon(%ebp,%edi)
	testb	$0x80,msb(%ebp,%edi)
	jnz	norm_denorm_done
norm_64_bits:
	bsrl	frac32(%ebp,%edi),%ecx
	jz	top_32_zero
	mov	frac64(%ebp,%edi),%eax
	sub	$31,%ecx
	neg	%ecx
	shldl	%eax,frac32(%ebp,%edi)
	shll	%cl,frac64(%ebp,%edi)
adjust_dword_expon:
	sub	%ecx,expon(%ebp,%edi)	/ adjust exponent
norm_denorm_done:
	ret
top_32_zero:
	mov	frac64(%ebp,%edi),%eax
	bsrl	%eax,%ecx
	sub	$31,%ecx
	neg	%ecx
	shll	%cl,%eax
	mov	%eax,frac32(%ebp,%edi)
	mov	$0,frac64(%ebp,%edi)
	add	$32,%ecx
	jmp	adjust_dword_expon
/norm_denorm	endp
/$eject
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			arith:
/
/	function:
/		main procedure for implementation of 80387 add, subtract,
/		multiply, and divide instructions
/
/	inputs:
/		assumes operation type, operand(s), and unpacked
/		status variables are set up
/
/	outputs
/		result of operation in result
/
/	data accessed:
/		- operation_type		offset_operand1
/		- sign1				tag1
/		- expon1			offset_operand2
/		- sign2				tag2
/		- expon2			offset_result
/		- result_sign			result_tag
/		- result_expon			result_word_frac
/
/	data changed:
/		- result
/
/	procedures called:
/		put_indefinite		affine_infinity?
/		put_max_valid		move_op_to_result
/		overflow_response	underflow_response
/		put_max_nan		divid
/		set_up_indefinite	round
/		addition_normalized	put_result
/		pop_free		subadd
/		mult			test_4w
/		i_masked?		d_error?
/		d_masked?		i_error?
/		get_precision		set_i_masked?
/		set_z_masked?		set_d_masked?
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
arith:	/proc
/	assume	%ds:a_msr
	jz	weed_out_special_cases	/ branch if no stack error
	call	set_stk_u_error			/ stack underflow occurred
	testb	invalid_mask,%gs:sr_masks
	jz	go_home					/ if unmasked, return
	call	put_indefinite			/ otherwise, result = indefinite
	jmp	finish_up
weed_out_special_cases:
	andb	$ -1 ! a_mask, %gs:sr_flags		/clear a_bit initially
	movb	tag1(%ebp),%al			/ both operands valid?
	orb	tag2(%ebp),%al
	jnz	special_cases				/ no, special case
handle_non_special_cases:
	movzbl	operation_type(%ebp),%ebx/ call sub/add/mul/div
	subb	$add_op,%bl				/ to do the operation
	shlw	$2,%bx
	call	*%cs:arith_table(%ebx)
/ the following three instructions are residue of warning mode arithmetic:
/		call	i_error?		; invalid if attempted
/		jz		no_invalid_error; division by denormal
/		jmp		invalid_error_detected
no_invalid_error:
	mov	$offset_result,%edi	/ round result
	call	get_precision				/ (indicate first, not
	movb	false,%al			/ second, round)
	call	round
	mov	$offset_result,%edi	/ re-normalize
/	movb	$4,%ah				/ (num words in frac=4)
	call	addition_normalize			/ (al contains overflow
	mov	dword_result_expon,%eax	/ indication from round)
	cmp	$0x7ffe,%eax		/ overflow iff
	jg	overflow_happened			/ expon > 7ffeh
	cmp	$0,%eax			/ check underflow if
	jl	underflow_happened			/ expon < 0
	call	give_valid_o_result			/ set tag to valid
	and	%eax,%eax				/ check underflow if
	jnz	report_rounds		/ expon = 0 and frac <> 0
	mov	$result_dword_frac+4,%edi	/expon=0
	call	test_4w
	jnz	underflow_happened
	movb	special,result_tag(%ebp)	/ result is truly zero
report_rounds:
	cmpb	true, rnd1_inexact	/detect inexact result
	jne	finish_up		/if exact, branch
	orb	$inexact_mask, %gs:sr_errors  /else, set p_error
	cmpb	true, added_one		/was rounding done by adding one
	jne	finish_up		/if not, branch
	orb	$a_mask, %gs:sr_flags	/ else, set a_bit
finish_up:
	mov	$offset_result,%edi
	mov	offset_result_rec,%esi
	jmp	put_arith_result
/
both_ops_denormd:
	orb	denorm_mask, %gs:sr_errors 
	testb	denorm_mask, %gs:sr_masks
	jz	go_home
	mov	$offset_operand1,%edi
	call	norm_denorm
op2_denormd:
	mov	$offset_operand2,%edi
	call	norm_denorm
	jmp	handle_non_special_cases
only_op1_denormd:
	orb	denorm_mask, %gs:sr_errors 
	testb	denorm_mask, %gs:sr_masks
	jz	go_home
	mov	$offset_operand1,%edi
	call	norm_denorm
	jmp	handle_non_special_cases
only_op2_denormd:
	orb	denorm_mask, %gs:sr_errors 
	testb	denorm_mask, %gs:sr_masks
	jz	go_home
	jmp	op2_denormd
/
overflow_happened:
	call	overflow_response	/ here, overflow is certain.
	jmp	finish_up
go_home:
	ret
underflow_happened:
	call	underflow_response
	jmp	finish_up
special_cases:
	testb	$0x10,%al		/ al contains [ebp].tag1 or [ebp].tag2
	jz	op1_nan_
	orb	invalid_mask, %gs:sr_errors 
	testb	invalid_mask, %gs:sr_masks
	jz	go_home
	mov	$offset_operand1,%edi
	call	set_up_indefinite
	mov	offset_result_rec,%esi
	jmp	put_arith_result
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
	jne	set_up_nan_return
signal_invalid_:
	testb	$0x40,msb_frac2
	jnz	set_up_nan_return
invalid_operand:
	orb	invalid_mask, %gs:sr_errors 
	testb	invalid_mask, %gs:sr_masks
	jz	go_home					/ i-error, if unmasked
set_up_nan_return:
	mov	offset_result_rec,%esi	/ result=max(nan1,nan2)
	cmpb	inv, tag1(%ebp)
	je	check_tag2
	mov	$offset_operand2, %edi
	jmp	install_hi_bits
check_tag2:
	cmpb	inv, tag2(%ebp)
	je	put_max_nan
	mov	$offset_operand1, %edi
	jmp	install_hi_bits
put_max_nan:
	movb	sign1(%ebp),%al			/ set result to larger
	movb	sign2(%ebp),%ah			/ magnitude result
	push	%eax				/ save signs
	andb	$0x7f,msb_frac1
	andb	$0x7f,msb_frac2
subtract_fracs:
	movb	positive,sign1(%ebp)		/ subtract the absolute
	movb	positive,sign2(%ebp)		/ values of the two ops
	movb	$sub_op,operation_type(%ebp)
	push	%esi
	call	subadd
	pop	%esi
	pop	%eax
	movb	%al,sign1(%ebp)				/ restore sign1 and give
	mov	$offset_operand1,%edi	/ operand1 as answer
	cmpb	positive,result_sign(%ebp)		/ if difference is '+'
	je	install_hi_bits
	movb	%ah,sign2(%ebp)				/ restore sign2 and give
	mov	$offset_operand2,%edi	/ operand2 as answer
install_hi_bits:
	orb	$0x0c0,msb(%ebp,%edi)
put_arith_result:
	call	put_result
	jmp	pop_free
non_nan_supp_ops:
	movb	tag2(%ebp),%bl
	movb	tag1(%ebp),%bh
	testb	$0x04,%bh
	jz	op2_denorm_
	movb	$3,%bh				/ indicate op1 is denorm
op2_denorm_:
	testb	$0x04,%bl
	jz	get_index
	movb	$3,%bl				/ indicate op2 is denorm
get_index:
	and	$0x0303,%ebx	/ form index to special operation table
				/ bx=4*(4*masked_tag1 + masked_tag2),
				/ where masked_tag =	0 for valid,
	shlb	$2,%bh		/	1 for zero,
				/2 for infinity
	addb	%bh,%bl		/	3 for denormd
	xorb	%bh,%bh
	shl	$2,%ebx
	mov	$0x40fb,%eax		/ 64 byte/table -add_op
	addb	operation_type(%ebp),%al	/ al = normalized type
	mulb	%ah			/ (e)ax = operation offset
	add	%eax,%ebx		/ (e)bx = case offset
	jmp	*%cs:special_table(%ebx)			/ jump to special case
exit_arith:
	ret
derror_with_first_operand:
	orb	denorm_mask, %gs:sr_errors 
	testb	denorm_mask, %gs:sr_masks
	jz	exit_arith			/ if unmasked, return
first_operand:
	mov	$offset_operand1,%esi		/ give first operand as
	jmp	set_result_to_operand	/ result
derror_with_neg_second_operand:
	orb	denorm_mask, %gs:sr_errors 
	testb	denorm_mask, %gs:sr_masks
	jz	exit_arith			/ if unmasked, return
neg_second_operand:
	notb	sign2(%ebp)			/ negate second operand
	jmp	second_operand			/ result
derror_with_second_operand:
	orb	denorm_mask, %gs:sr_errors 
	testb	denorm_mask, %gs:sr_masks
	jz	exit_arith			/ if unmasked, return
second_operand:
	mov	$offset_operand2,%esi		/give second operand
set_result_to_operand:
	call	move_op_to_result
	jmp	go_to_finish_up
/divide_into_zero:	; this section, for pseudo-zeroes, was
/		xor	ax,ax		; commented out when they became unsupp.
/		mov	edi,offset word_frac2 + 2	; if frac2 = 0, invalid
/		call	test_4w		; else, zero and xor
/		jz	invalid_error_detected		; signs as the result
denormd_exor_signed_zero:
	orb	denorm_mask, %gs:sr_errors 
	testb	denorm_mask, %gs:sr_masks
	jz	exit_arith				/ if unmasked, return
exor_signed_zero:
	mov	$zero_pattern,%esi
	call	set_constant_result
	movb	special,result_tag(%ebp)
	jmp	set_exor_sign
denormd_exor_signed_infinity:
	orb	denorm_mask, %gs:sr_errors 
	testb	denorm_mask, %gs:sr_masks
	jz	exit_arith				/ if unmasked, return
	jmp	exor_signed_infinity
divide_by_zero:
	orb	zero_divide_mask, %gs:sr_errors 
	testb	zero_divide_mask, %gs:sr_masks
	jz	exit_arith				/ if unmasked, return
exor_signed_infinity:
	call	put_infinity			/ else, give infinity
set_exor_sign:
	movb	positive,result_sign(%ebp)	/ set sign to exclusive
	movb	sign1(%ebp),%ah	/ or of operand signs
	cmpb	sign2(%ebp),%ah
	je	go_to_finish_up
	notb	result_sign(%ebp)
go_to_finish_up:
	jmp	finish_up
add_sub_infinities:
	movb	sign2(%ebp),%ah	/ add or sub magnitude?
	cmpb	$sub_op,operation_type(%ebp)		/ add mag if add_op and
	jne	add_or_sub_mag_			/ signs same, else sub
	notb	%ah				/ comp sign2 if sub
add_or_sub_mag_:
	cmpb	sign1(%ebp),%ah
	je	first_operand		/ first op res if add_mag
invalid_error_detected:
	orb	invalid_mask, %gs:sr_errors 
	testb	invalid_mask, %gs:sr_masks
	jz	exit_arith
	call	put_indefinite			/ if masked, indefinite
	jmp	finish_up			/ otherwise, just return
/
/arith	endp
/
/a_med	ends
/
/	end
