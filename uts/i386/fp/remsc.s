/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file 	"remsc.s"

	.ident	"@(#)kern-fp:remsc.s	1.1"

/ ********************************************************************
/
/			r e m s c . m o d
/			=================
/
/	===============================================================
/               intel corporation proprietary information
/    this software  is  supplied  under  the  terms  of  a  license
/    agreement  or non-disclosure agreement  with intel corporation
/    and  may not be copied nor disclosed except in accordance with
/    the terms of that agreement.                                  
/	===============================================================
/
/	functions:
/		implements 80387 fprem  instruction.
/		implements 80387 fprem1 instruction.
/		implements 80387 fscale instruction.
/
/	public procedures:
/		remr			scale
/		add_to_frac		add_to_frac_2
/
/	internal procedures:
/		remrx
/
/ ************************************************************************
/
/...March 3, 1987...
/
/
/$nolist
#include	"e80387.h"
/$list
	.data	/a_msr	segment	rw	public
/	extrn	%gs:sr_masks,%gs:sr_flags,%gs:sr_errors,%gs:sr_controls
/a_msr	ends
/
/	assume	%ds:a_msr
/
	.text	/a_med	segment	er	public
/
/	extrn	put_result,set_up_indefinite,test_4w
/	extrn	subtraction_normalize,move_op_to_result
/	extrn	underflow_response,right_shift_frac1_cl
/	extrn	right_shift_frac2_cl,left_shift_frac1_1
/	extrn	fix32,overflow_response,set_stk_u_error
/	extrn	subadd,put_arith_result,set_up_nan_return
/	extrn	test_6w,clear_6w,norm_denorm
/	extrn	left_shift_frac2_cl
	.globl	remr
	.globl	add_to_frac
	.globl	add_to_frac_2
	.globl	scale
	.globl	remrx
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			remrx:
/			""""""
/	function:
/		 fractional remainder.
/
/	inputs:
/		assumes dividend is in op1, divisor is in op2,
/		number of quotient bits to generate is in ax,
/		and q is cleared to all zeroes.
/
/	outputs:
/		it returns the fractional remainder in op1, and
/		the low bits of the quotient in q.
/
/	data accessed:
/		- offset_operand1		offset_operand2
/		- offset_result
/
/	data changed:
/		- frac1			frac2
/
/	procedures called:
/		right_shift_frac1_cl	right_shift_frac2_cl
/		left_shift_frac1_1		add_to_frac		
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
remrx:	/proc
	push	%eax				/ save the loop count
	movb	$1,%cl			/ shift frac1 right 1 bit
	call	right_shift_frac1_cl
	movb	$1,%cl			/ shift frac2 right 1 bit
	call	right_shift_frac2_cl		/ returns cf clear, ch = 0
	mov	$offset_operand2,%esi		/ result_frac <--  - frac2
	mov	$offset_result,%edi
	movw	$0,(%ebp,%esi)	/ clear extra bytes
	mov	$0x03,%ecx			/ load loop count
complement_frac2:
	mov	$0x0000,%eax				/ clear ax, leave cf intact
	sbb	(%ebp,%esi),%eax			/ 0 - frac2
	mov	%eax,(%ebp,%edi)			/ store into result frac
	inc	%esi						/ bump offsets
	inc	%esi					/ (doesn't affect cf)
	inc	%esi
	inc	%esi
	inc	%edi
	inc	%edi
	inc	%edi
	inc	%edi
	loop	complement_frac2		/ loop until result = -frac2
	jmp	enter_loop
form_next_frac1:
	push	%ecx					/ stack loop count
	call	left_shift_frac1_1		/ shift frac1 left one bit
	mov	$offset_operand2,%esi
	testb	$0x01,q				 	/ if lsb of q = 1 then
	jz	do_add				/ frac1 <-- frac1 + frac2
enter_loop:
	mov	$offset_result,%esi		/ else, frac1 <-- frac1 - frac2
do_add:
	mov	$offset_operand1,%edi
	call	add_to_frac			/ move carry-out from add
	rclb	$1,q				/ shift carry-out into q
/	adc	hi_q,	0		; hi_q counts non-0 quotient bits
	pop	%ecx			/ above bit b7 of the quotient.
	loop	form_next_frac1			/ if looping done,
	testb	$0x01,q				/ then one last iteration
	jnz	last_shift			/ frac1 <-- frac1 + frac2
	mov	$offset_operand2,%esi
	mov	$offset_operand1,%edi
	call	add_to_frac
last_shift:
	jmp	left_shift_frac1_1		/ shift frac1 left 1 bit
/remrx	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			add_to_frac:
/			"""""""""""
/	function:
/		adds a 10-byte fraction to another 10-byte fraction.
/
/	inputs:
/		ss:ebp+esi points to the source fraction, and ss:ebp+edi points
/		to the destination fraction.
/
/	outputs:
/		carry flag set if there was a carry out, else reset.
/
/	data accessed:
/
/	data changed:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
add_to_frac:	/proc
	mov	(%ebp,%esi),%eax
	xorw	%ax,%ax
add_to_frac_2:
	add	%eax,(%ebp,%edi)
	mov	frac64(%ebp,%esi),%eax
	adc	%eax,frac64(%ebp,%edi)
	mov	frac32(%ebp,%esi),%eax
	adc	%eax,frac32(%ebp,%edi)
simple_return:
	ret
/add_to_frac	endp
/ *************************************************************************
mov_esi_edi:	/proc
	add	%ebp, %edi	/add global record offsets
	add	%ebp, %esi
	push	%ds	/save a_msr ??
	push	%ss	/load source segment register
	pop	%ds
	push	%ss	/load destination segment register
	pop	%es	/into es
	mov	$3, %ecx
	rep
	movsl		/move three words
	pop	%ds	/reload a_msr
	ret
/mov_esi_edi	endp
/ *************************************************************************


/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			remr:
/			"""""
/	function:
/		80387 remainder instruction
/
/	inputs:
/		assumes the operand records are set up.
/
/	outputs:
/		results in result record.
/
/	data accessed:
/		- result_rec_offset		offset_operand1
/		- tag1				expon1
/		- word_frac1			tag2
/		- expon2			msb_frac2
/		- offset_result
/
/	data changed:
/		- tag1				expon1
/
/	procedures called:
/		set_up_indefinite	set_stk_u_error
/		remrx				subtraction_normalize
/		move_op_to_result	underflow_response
/		put_result			test_4w
/		subadd				mov_esi_edi
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
remr:	/proc
	lahf				/ save stack empty flag ('86 z-flag)
	andb	$-1 ! [a_mask+c_mask],%gs:sr_flags/ clear '87 a-flag, and also
					/ c-flag for 387 compatibility
	movw	$0,exp_tmp		/ initialize q and hi_q to 0.
	sahf				/ restore stack empty flag ('86 z-flag)
	jz	catch_special_cases		/ branch if no stack error
	call	set_stk_u_error			/ stack underflow occurred
	jmp	unmasked_i_error_
catch_special_cases:
	movb	tag1(%ebp),%al		/ both operands valid?
	orb	tag2(%ebp),%al
	jnz	special_cases_handler	/ no, branch to handler of special cases
valid_case:
	mov	dword_expon1,%eax	/ eax <- expon_diff = exp1 - exp2
	sub	dword_expon2,%eax		/ if expon_diff < zero,
	mov	$offset_operand1,%edi		/ then op1 is the modulus
	jl	rem_or_mod_
	cmp	$63,%eax			/ if expon_diff > 63,
	jle	calc_exponent			/ then set c-bit for incomplete
	orb	$c_mask,%gs:sr_flags	/ reduction and replace expon_diff
	or	$32,%eax		/ by [(expon_diff or 32) mod 64]
	and	$63,%eax
calc_exponent:
	sub	%eax,dword_expon1		/ expon1 <- remainder exponent
	inc	%eax			/ num_quotient_bits = expon_diff + 1
	call	remrx			/ calculate remainder fraction
	xor	%eax,%eax			/ detect zero result
	mov	$dword_frac1+frac64,%edi
	call	test_4w
	jnz	do_normalize
	testb	$c_mask,%gs:sr_flags		/ was reduction complete?
	jz	remr_zero			/ if so, process q as well
	movb	%al,q			/ else, make q 0 (value of al)
	jmp	remr_zero
do_normalize:
	mov	$offset_operand1,%edi
	call	subtraction_normalize
/	mov	[ebp].tag1,	valid	; unneeded if denormals retagged
reduction_incomplete_:
	testb	$c_mask,%gs:sr_flags
	jz	rem_or_mod_	/ rem or mod matters only if complete
	movb	$0,q			/ if incomplete, make q 0.
	jmp	check_rem_underflow	/ branch to check unmasked underflow
rem_or_mod_:
	lahf			/save zf to indicate whether 
				/op2 needs restoration
	cmpb	$rem1_op,operation_type(%ebp)
	jne	check_rem_underflow
	sahf		
	jnz	op2_restored
	movb	$1, %cl
	call 	left_shift_frac2_cl
op2_restored:
	
	dec	dword_expon2
	push	sign1(%ebp)
	movb	positive,sign2(%ebp)
	movb	positive,sign1(%ebp)
	movb	$sub_op,operation_type(%ebp)
	mov	dword_expon1, %eax
	cmp	dword_expon2, %eax
	mov	$offset_operand1, %esi
	jle	save_shiftable_op
	mov	$offset_operand2, %esi
save_shiftable_op:
	mov	$offset_cop, %edi
	call	mov_esi_edi
	call	subadd
	mov	$offset_result,%edi
	call	test_6w
	pop	sign1(%ebp)
	mov	$offset_operand1, %edi
	jnz	check_sign
tiebreaker:
	testb	$1,q
	jz	check_rem_underflow
	incb	q
	notb	sign1(%ebp)
	jmp	check_rem_underflow
check_sign:
	mov	$offset_cop, %esi
	cmpb	positive,result_sign(%ebp)
	je	restore_op2_
	call	mov_esi_edi
	mov	$offset_operand1, %edi
	jmp	check_rem_underflow
restore_op2_:
	mov	dword_expon1, %eax
	cmp	dword_expon2, %eax
	je	reduce_modulus
	mov	$offset_operand2, %edi
	call	mov_esi_edi
reduce_modulus:
	incb	q
	inc 	dword_expon2
	movb	sign1(%ebp),%al
	movb	%al,sign2(%ebp)
	call	subadd
	mov	$offset_result,%edi
check_rem_underflow:
	cmp	$0x0001,expon(%ebp,%edi)	/ if expon1 < 1, then underflow
	jge	do_put_result
	mov	%edi,%esi			/ give std underflow response
	call	move_op_to_result
	push	%gs:sr_masks
	orb	prec64,%gs:sr_controls		/ "or" works only because prec64
	movb	false,rnd1_inexact		/ sets all bits in the pc field
	call	underflow_response
	pop	%gs:sr_masks
move_result_to_result:
	mov	$offset_result,%edi
	jmp	do_put_result
remr_zero:
	mov	%eax,dword_expon1		/ if remainder fraction = 0,
	movb	special,tag1(%ebp)			/ then set result to 0
	cmpb	$rem1_op,operation_type(%ebp)
	jne	put_op1
	movb	positive,sign1(%ebp)
	testb	rnd_down,%gs:sr_controls
	jz	put_op1
	testb	rnd_up,%gs:sr_controls
	jnz	put_op1
	movb	negative,sign1(%ebp)
put_op1:
	mov	$offset_operand1,%edi
do_put_result:
	mov	offset_result_rec,%esi
	call	put_result
	andb	$-1![zero_mask+a_mask+sign_mask],%gs:sr_flags
	movb	q,%al
	shlb	$6,%al			/ move 3 low bits of q to
	jnc	fix_z_bit		/ s, z, and a bits
	orb	$sign_mask,%gs:sr_flags
fix_z_bit:
	shlb	$1,%al
	jnc	fix_a_bit
	orb	$zero_mask,%gs:sr_flags
fix_a_bit:
	shlb	$1,%al
	jnc	remr_done
	orb	$a_mask,%gs:sr_flags
remr_done:
	ret
special_cases_handler:
	testb	$0x10,%al	/ al contains [ebp].tag1 or [ebp].tag2
	jz	op1_nan_	/ no branch if at least one op unsupported
invalid_operand:
	orb	invalid_mask,%gs:sr_errors
unmasked_i_error_:
	testb	invalid_mask,%gs:sr_masks/ if unmasked, just exit
	jz	remr_done		/ (c-bit remains clear here,
					/ to indicate complete reduction.)
					/ else, return indefinite
remr_indef:
	mov	$offset_operand1,%edi
	call	set_up_indefinite
load_esi_for_result:
	mov	offset_result_rec,%esi
	jmp	put_arith_result
op1_nan_:			/ here, neither op is unsupported.
	movb	tag2(%ebp),%al
	movb	tag1(%ebp),%ah
	cmpb	inv,%ah
	je	op1_snan_
	cmpb	inv,%al
	jne	non_nan_supp_ops
	jmp	signal_invalid_
op1_snan_:
	testb	$0x40,msb_frac1
	jz	signal_for_snan
op2_also_nan_:
	cmpb	inv,%al
	jne	jmp_set_up_nan_return
signal_invalid_:
	testb	$0x40,msb_frac2
	jnz	jmp_set_up_nan_return
signal_for_snan:
	orb	invalid_mask,%gs:sr_errors/ set i-error for signaling nan operand
	testb	invalid_mask,%gs:sr_masks/ if unmasked, return forthwith
	jz	remr_done				/ else, masked, so return max nan.
jmp_set_up_nan_return:
	jmp	set_up_nan_return
non_nan_supp_ops:
	cmpb	$scale_op,operation_type(%ebp)
	je	scale_non_nan_supp_ops
rem_non_nan_supp_ops:
	cmpb	infinty,%ah
	je	invalid_operand
	cmpb	special,%al
	je	invalid_operand
	cmpb	denormd,%ah
	jne	op2_denorm_
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	remr_done
	mov	$offset_operand1,%edi
	cmpb	infinty,%al
	jne	norm_op1
	testb	$0x80,msb_frac1
	jz	op1_true_denormal
	mov	$1,dword_expon1
	movb	valid,tag1(%ebp)
	jmp	load_esi_for_result
op1_true_denormal:
	testb	underflow_mask,%gs:sr_masks
	jnz	load_esi_for_result
	call	norm_denorm
	add	wrap_around_constant,dword_expon1/ add wrap-around
	orb	underflow_mask,%gs:sr_errors
	jmp	load_esi_for_result
norm_op1:
	push	%eax
	call	norm_denorm
	pop	%eax
op2_denorm_:
	cmpb	denormd,%al
	jne	non_error_specials
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	remr_done
	mov	$offset_operand2,%edi
	push	%eax
	call	norm_denorm
	pop	%eax
non_error_specials:
	cmpb	special,%ah
	je	put_op1
op2_inf_:
	cmpb	infinty,%al
	je	put_op1
	jmp	valid_case
/
scale_non_nan_supp_ops:
	cmpb	infinty,%ah
	jne	scaler_denormd_
	cmpb	positive,sign1(%ebp)
	jne	op2_also_inf_
	cmpb	special,%al
	je	invalid_operand
	cmpb	denormd,%al
	jne	xfer_sign
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	exit_scale
xfer_sign:
	movb	sign2(%ebp),%al
	movb	%al,sign1(%ebp)
	mov	$offset_operand1,%edi
	jmp	put_scaled_result
op2_also_inf_:
	cmpb	infinty,%al
	je	invalid_operand
	cmpb	denormd,%al
	jne	zero_op2
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	exit_scale
zero_op2:
	mov	$offset_operand2,%edi
	call	clear_6w
	mov	%eax,dword_expon2		/ clear eax
	movb	special,tag2(%ebp)
	jmp	put_scaled_result
scaler_denormd_:
	cmpb	denormd,%ah
	jne	scalend_denormd_
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jnz	scalend_denormd_
exit_scale:
	ret
scalend_denormd_:
	cmpb	denormd,%al
	jne	give_op2
	orb	denorm_mask, %gs:sr_errors
	testb	denorm_mask, %gs:sr_masks
	jz	exit_scale
	cmpb	valid,%ah
	jne	check_unfl_mask
	mov	$offset_operand2,%edi
	call	norm_denorm
	jmp	valid_scale_case
check_unfl_mask:
	testb	underflow_mask,%gs:sr_masks
	jnz	give_op2
	jmp	scale_underflow

/remr	endp
/
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			scale:
/			""""""
/	function:
/		emulates the 80387 scale instruction.
/
/	inputs:
/		scale term (scaler) from st(1) in operand1, and 
/		scalend ("to be scaled") from st(0) in operand2
/
/	outputs:
/		scaled operand in operand2 record or result_record.
/		error indicators set.
/
/	data accessed:
/		- result_rec_offset		tag1
/		- word_frac1			offset_operand2
/		- tag2				expon2
/		- offset_result			extra_word_reg
/
/	data changed:
/		- expon2
/
/	procedures called:
/		set_up_indefinite	fix32		put_result
/		move_op_to_result	underflow_response
/		overflow_response
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
scale:	/proc
	jz	catch_spcl_scale_cases		/ branch if no stack error
	call	set_stk_u_error			/ stack underflow occurred
	jmp	unmasked_i_error_
catch_spcl_scale_cases:
	andb	$-1 ! [a_mask],%gs:sr_flags/ clear '87 a-bit
	movb	tag1(%ebp),%al		/ both operands valid?
	orb	tag2(%ebp),%al
	jnz	special_cases_handler	/ no, branch to handler of special cases
valid_scale_case:
	push	%gs:sr_masks			/ save current rounding controls
	orb	rnd_to_zero,%gs:sr_controls	/ institute round by chopping
					/ this "or" works only because
					/ rnd_to_zero sets entire rc field
	call	fix32			/ convert scale factor to int32
	pop	%gs:sr_masks			/ restore rnd control
	jz	add_scale_term				/ if zf=1, no overflow in fix32
	cmpb	positive,sign1(%ebp)
	je	get_least_sf_xtrm_ovfl
	jmp	get_grtst_sf_xtrm_unfl
add_scale_term:
	mov	dword_frac1+frac32(%ebp),%eax	/ int32 scale factor
	cmp	least_sf_xtrm_ovfl,%eax
	jle	check_xtrm_unfl
get_least_sf_xtrm_ovfl:
	mov	least_sf_xtrm_ovfl,%eax
	jmp	add_int32_to_dword_exp
check_xtrm_unfl:
	cmp	grtst_sf_xtrm_unfl,%eax
	jge	add_int32_to_dword_exp
get_grtst_sf_xtrm_unfl:
	mov	grtst_sf_xtrm_unfl,%eax
add_int32_to_dword_exp:
	add	%eax,dword_expon2		/ add scale term to op2's expon
	cmp	$0x7ffe,dword_expon2
	jg	scale_overflow
	cmp	$1,dword_expon2
	jl	scale_underflow
give_op2:
	mov	$offset_operand2,%edi
put_scaled_result:
	mov	offset_result_rec,%esi
	jmp	put_result
scale_overflow:
	mov	$offset_operand2,%esi		/ move op2 to result
	call	move_op_to_result
	call	overflow_response
	jmp	set_up_result
scale_underflow:
	mov	$offset_operand2,%esi		/ move op2 to result
	call	move_op_to_result
	push	%gs:sr_masks
	orb	prec64,%gs:sr_controls		/ "or" works only because prec64
	movb	false,rnd1_inexact		/ sets all bits in the pc field
	call	underflow_response
	pop	%gs:sr_masks
set_up_result:
	mov	$offset_result,%edi
	jmp	put_scaled_result
/scale	endp
/
/a_med	ends
/
/	end
