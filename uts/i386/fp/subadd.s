/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"subadd.s"

	.ident	"@(#)kern-fp:subadd.s	1.1"
/$tt("80387	emulator  + + + s u b a	d d + + + ")
/ ************************************************************************
/
/			s u b a d d . m o d
/			===================
/
/	===============================================================
/               intel corporation proprietary information
/    this software  is  supplied  under  the  terms  of  a  license
/    agreement  or non-disclosure agreement  with intel corporation
/    and  may not be copied nor disclosed except in accordance with
/    the terms of that agreement.                                  
/	===============================================================
/
/	function:
/		performs floating point add, subtract, or compare
/		of unpacked, non-zero, valid numbers.
/
/	public procedures:
/		addx		subx		subadd
/		sp_subadd	compar		mov_neg_frac
/
/ **************************************************************************
/
/...March 3, 1987...
/
/$nolist
#include	"e80387.h"
/$list
/...declare status register segment...
/
	.data	/a_msr	segment	rw	public
/	extrn	%gs:sr_masks,%gs:sr_flags,%gs:sr_errors
/a_msr	ends
/
/	assume	%ds:a_msr
	.text	/a_med	segment	er	public
/
/	extrn	sticky_right_shift,right_shift,test_6w
/	extrn	addition_normalize,subtraction_normalize
/	extrn	pop_free,clear_6w,get_rnd_control
/	extrn	set_i_error,affine_infinity_,set_s_bit
/	extrn	clear_s_bit,set_z_bit,clear_z_bit
/	extrn	i_error_,set_d_error,set_stk_u_error
/
	.globl	addx
	.globl	subx
	.globl	subadd
	.globl	sp_subadd
	.globl	compar
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			addx:
/			"""""
/	function:
/		fractional add. result_frac <-- frac1 + frac2.
/
/	inputs:
/		assumes operand records are set up.
/
/	outputs:
/		al = 1 if there was a carry-out; al=0 otherwise. 
/
/	data accessed:
/		- word_frac1		word_frac2
/		- result_word_frac
/
/	data changed:
/		 result_word_frac
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
addx:	/proc

	mov	dword_frac1(%ebp),%eax		/do fractional add
	xorw	%ax,%ax				/clear extra word
	add	dword_frac2(%ebp),%eax
	mov	%eax,result_dword_frac(%ebp)
	mov	dword_frac1+4(%ebp),%eax
	adc	dword_frac2+4(%ebp),%eax
	mov	%eax,result_dword_frac+4(%ebp)
	mov	dword_frac1+8(%ebp),%eax
	adc	dword_frac2+8(%ebp),%eax
	mov	%eax,result_dword_frac+8(%ebp)

	movb	$0,%al
	rclb	$1,%al				/put carry into al
	ret
/addx	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			subx:
/			""""
/	function:
/		2's complement fractional subtract.
/		result_frac <-- frac1	- frac2. 
/
/	inputs:
/		assumes operand variables are set up.
/
/	outputs:
/		al=1 if borrow; al=0 otherwise.
/
/	data accessed:
/		- word_frac1		word_frac2
/		- result_word_frac
/
/	data changed:
/		- result_word_frac
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
subx:	/proc

	mov	dword_frac1(%ebp),%eax		/ fractional subtract
	and	$0x0ffff0000,dword_frac2(%ebp)	/ clear low word
	sub	dword_frac2(%ebp),%eax
	mov	%eax,result_dword_frac(%ebp)
	mov	dword_frac1+4(%ebp),%eax
	sbb	dword_frac2+4(%ebp),%eax
	mov	%eax,result_dword_frac+4(%ebp)
	mov	dword_frac1+8(%ebp),%eax
	sbb	dword_frac2+8(%ebp),%eax
	mov	%eax,result_dword_frac+8(%ebp)
	movb	$0,%al				/set al to borrow
	rclb	$1,%al
	ret
/subx	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			subadd:
/			"""""""
/	function:
/		floating point add or subtract.calculates unpacked result.
/		sp_subadd is the entry point for log functions.
/
/	inputs:
/		assumes operands are unpacked, valid, non-zero.
/
/	outputs:
/
/	data accessed:
/		- operation_type		sign1
/		- expon1			msb_frac
/		- offset_operand1		sign2
/		- expon2			msb_frac2
/		- offset_operand2		offset_result
/		- result_sign			result_expon
/		- result_word_frac
/
/	data changed:
/		- result_sign			result_expon
/		- result_word_frac
/
/	procedures called:
/		sticky_right_shift		right_shift
/		addition_normlized		subtraction_normalize
/		addx				subx
/		test_6w				get_rnd_control
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
subadd:	/proc
	mov	$sticky_right_shift,%esi	/ load shift proc ptr
	jmp	form_expon_difference	/ merge with sp_subadd
sp_subadd:
	mov	$sp_right_shift,%esi	/ load shift proc ptr
form_expon_difference:
	mov	dword_expon1,%ecx
	sub	dword_expon2,%ecx
	js	expon2_larger
	cmp	$67,%ecx			/set cl to shift amount =
	jbe	adjust_frac2		/min(67,expon1-expon2)
	mov	$67,%ecx
adjust_frac2:
	movb	$0x00,%al			/al = bit to inject from left
	mov	$offset_operand2,%edi
	push	%esi
	call	*%esi			/shift op2 fraction
	pop	%esi
	mov	expon1(%ebp),%eax		/set result expon to expon1
	mov	%eax,result_expon(%ebp)
	cmpb	special, tag1(%ebp)
	je	determine_operation_true
	testb	$0x80,msb_frac1
	jnz	determine_operation_true / branch if frac1 is normal
	jmp	frac2_normal_	/ test frac2
expon2_larger:
	neg	%ecx			/set cl to shift amount
	cmp	$67,%ecx
	jbe	adjust_frac1
	mov	$67,%ecx
adjust_frac1:
	push	%ecx			/save shift amount
	movb	$0x00,%al			/al = bit to inject
	mov	$offset_operand1,%edi
	call	*%esi
	mov	expon2(%ebp),%eax	/set result expon to expon2
	mov	%eax,result_expon(%ebp)
	pop	%ecx
frac2_normal_:
	cmpb	special, tag2(%ebp)
	je	determine_operation_true
	movb	false,%al
	testb	$0x80,msb_frac2	/ set al to true if frac2 is
	jz	determine_operation	/ normalized
determine_operation_true:
	movb	true,%al
determine_operation:
	cmpb	$sub_op,operation_type(%ebp) /if op=sub_op, negate op2
	jne	add_or_sub_
	notb	sign2(%ebp)
add_or_sub_:
	movb	sign1(%ebp),%ah
	cmpb	sign2(%ebp),%ah
	jne	do_subtraction
	call	addx			/do fractional add
	mov	$offset_result,%edi
	call	addition_normalize	/normalize result after add
	movb	sign1(%ebp),%al
	movb	%al,result_sign(%ebp)	/set result sign to sign1
	testb	$0x80, msb_result
	jnz	add_done
	mov	$offset_result, %edi
	call	test_6w
	jnz	norm_result
add_done:
	ret
do_subtraction:
	push	%eax			/save normalization indicator
	movb	sign1(%ebp),%al		/initially, set sign to sign1
	movb	%al,result_sign(%ebp)
	call	subx			/do fractional subtract
	mov	$offset_result,%edi
	cmpb	$0,%al
	mov	$0,%eax
	je	detect_zero

	not	(%ebp,%edi)		/form 2's comp of result_frac
	add	$1,(%ebp,%edi)
	not	4(%ebp,%edi)
	adc	%eax,4(%ebp,%edi)
	not	8(%ebp,%edi)
	adc	%eax,8(%ebp,%edi)
	notb	result_sign(%ebp)	/and complement result_sign
detect_zero:
	call	test_6w			/find true zero result
	jnz	do_norm_		/ if not zero, test for denorm
	call	get_rnd_control		/ sign is + unless rnd down
	xorb	rnd_down,%al
	decw	%ax
	cbtw
	movb	%ah,result_sign(%ebp)
	pop	%eax			/if al=true, then result should
	cmpb	true,%al			/ be true zero
	jne	subtract_done
	movw	$0,result_expon(%ebp)
	ret
do_norm_:
	pop	%eax			/if al=false, don't normalize
	cmpb	true,%al			/ the result
	jne	subtract_done
norm_result:
	mov	$offset_result,%edi
	call	subtraction_normalize	/normalize the result
subtract_done:
	ret
sp_right_shift:
	call	right_shift
	mov	(%ebp,%edi),%eax	/ ss:ebp+edi -> dword_frac after call
	and	$0x1fffffff,%eax	/ mask dword fraction
	or	%eax,%edx		/ or it to the right_shift output
	jz	sp_nonsticky	/ if zero, it must be nonsticky
	or	$0x20000000,(%ebp,%edi)	/ if nonzero, set bit of dword_frac
sp_nonsticky:
	and	$0x0e0000000,(%ebp,%edi) / now mask off most of dword_frac
	ret
/subadd	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			compar:
/			"""""""
/
/	function:
/		main procedure for compare and test instructions.
/
/	inputs:
/		op1 and op2 operand records.
/
/	outputs:
/		sets zero and sign status bits to indicate the result
/		of the compare.	sets the invalid and denorm error bits
/		if these errors are detected.
/
/	data accessed:
/		- operation_type		tag1
/		- expon1			tag2
/		- expon2			result_sign
/		- result_word_frac
/
/	data changed:
/
/	procedures called:
/		pop_free			subadd
/		test_6w				clear_6w
/		set_s_bit			clear_s_bit
/		set_z_bit			clear_z_bit
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
compar:	/proc
	jz	no_stack_error			/ branch if no stack error
	andb	$-1 ! a_mask,%gs:sr_flags	/ clear a-bit
	orb	invalid_mask+zero_mask,%gs:sr_errors/ clears zf, to 0
	jmp	invalid_error_signal
no_stack_error:
	mov	$0,before_error_signals(%ebp)
	andb	$-1 ![a_mask+c_mask],%gs:sr_flags
	cmpb	$test_op,operation_type(%ebp)	/ if not test_op, then
	jne	set_up_check			/ it's compar_op or ucom_op
	movb	positive,sign2(%ebp)		/ for test_op, set operand2
	movb	special,tag2(%ebp)			/ to zero
	mov	$dword_frac2,%edi
	call	clear_6w						/ returns with eax = 0
	mov	%eax,dword_expon2
set_up_check:
	movb	tag1(%ebp),%al			/ both operands valid?
	orb	tag2(%ebp),%al
	jz	ordered_cases				/ yes, ops are ordered.
	cmpb	special,%al			/ one op is zero, and the
	je	ordered_cases			/ other is zero or valid.
	testb	$0x10,%al			/ is either op unsupported?
	jnz	invalid_error_detected
op1_nan_:
	movb	tag1(%ebp),%al			/ al = value of tag1
	movb	tag2(%ebp),%ah			/ ah = value of tag2
	cmpb	inv,%al
	je	op1_snan_
	cmpb	inv,%ah
	jne	non_nan_supp_ops
	jmp	signal_invalid_
op1_snan_:
	testb	$0x40,msb_frac1
	jz	invalid_error_detected
op2_also_nan_:
	cmpb	inv,%ah
	jne	set_up_nan_return
signal_invalid_:
	testb	$0x40,msb_frac2
	jz	invalid_error_detected
set_up_nan_return:
	cmpb	$ucom_op,operation_type(%ebp)	/ if ucom_op, then
	je	set_uncertain			/ no invalid signal.	
	jmp	invalid_error_detected
non_nan_supp_ops:
	cmpb	denormd,tag1(%ebp)
	jne	op2_denormd_
	movb	true,signal_d_error_
	cmp	$0,expon1(%ebp)
	jne	op2_denormd_
	mov	$1,expon1(%ebp)
op2_denormd_:
	cmpb	denormd,tag2(%ebp)
	jne	ordered_cases
	movb	true,signal_d_error_
	cmp	$0,expon2(%ebp)
	jne	ordered_cases
	mov	$1,expon2(%ebp)
ordered_cases:
	movb	$sub_op,operation_type(%ebp)	/ subtract op2 from op1
	call	subadd
	mov	$offset_result,%edi			/ detect a zero fraction
	call	test_6w							/ in result
	jz	operands_equal
	andb	$ -1 ! [zero_mask+sign_mask],%gs:sr_flags / set zero to false
	testb	$0x01,result_sign(%ebp)			/ set sign to result
	jz	finish_up
	orb	$sign_mask,%gs:sr_flags
finish_up:
	cmp	$0,before_error_signals(%ebp)
	jne	check_i_error
	jmp	pop_free
check_i_error:
	cmpb	false,signal_i_error_		/ recall that false = 00h
	je	set_denorm_error
test_invalid_mask:
	testb	invalid_mask,%gs:sr_masks
	jz	unmasked_exit
	jmp	pop_free
unmasked_exit:
	ret
check_d_error:
	cmpb	false,signal_d_error_		/ recall that false = 00h
	jne	set_denorm_error
	jmp	pop_free
set_denorm_error:
	orb	denorm_mask,%gs:sr_errors
test_denorm_mask:
	testb	denorm_mask,%gs:sr_masks
	jz	unmasked_exit
	jmp	pop_free

invalid_error_detected:
	orb	invalid_mask,%gs:sr_errors
invalid_error_signal:
	mov	$0x000000ff,before_error_signals(%ebp)
	jmp	set_uncertain

also_d_error_:
	cmpb	denormd,tag1(%ebp)
	je	d_error_also
	cmpb	denormd,tag2(%ebp)
	jne	set_uncertain
d_error_also:
	movb	true, signal_d_error_
set_uncertain:
	orb	$[sign_mask + c_mask], %gs:sr_flags
	jmp	zero_true	/ set zero to true
operands_equal:
	andb	$-1 ! sign_mask,%gs:sr_flags	/ set sign positive
zero_true:
	orb	$zero_mask,%gs:sr_flags
	jmp	finish_up
/compar	endp
/
/a_med	ends
/
/	end
