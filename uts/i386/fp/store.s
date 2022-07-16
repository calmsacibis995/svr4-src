/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file "store.s"

	.ident	"@(#)kern-fp:store.s	1.1"

/$tt("80387	emulator+++s	t	o	r	e+++")
/ ***************************************************************************
/
/			s t o r e . m o d 
/			==================
/
/	=============================================================
/               intel corporation proprietary information
/    this software  is  supplied  under  the  terms  of  a  license
/    agreement  or non-disclosure agreement  with intel corporation
/    and  may not be copied nor disclosed except in accordance with
/    the terms of that agreement.                                  
/	===============================================================
/
/	function:
/		implements 80387 store, fix, and storex instructions.
/
/	public procedures:
/		store	fix16	fix32	fix64	move_op_to_op
/
/	internal procedures:
/		extended_store		bcd_store
/		single_real_store	double_real_store
/		store_valid		int64_store
/		int16_store		int32_store
/
/ ************************************************************************
/
/...december 12, 1986..
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
/	extrn	pop_free,clear_6w,test_3w,subadd
/	extrn	gradual_underflow,right_shift_frac2_cl
/	extrn	special_round_test,directed_round_test
/	extrn	right_shift_result_cl,set_up_indefinite
/	extrn	right_shift_frac1_cl,sticky_right_shift
/	extrn	left_shift_frac1_cl,left_shift_result_cl
/	extrn	put_op1_result,divid,addition_normalize
/	extrn	round,get_precision,get_rnd_control
/	extrn	store_precision,set_i_error,i_masked_
/	extrn	set_o_error,o_masked_,set_p_error
/	extrn	set_u_error,test_4w,subtraction_normalize
/	extrn	p_error_,clear_p_error,add_to_frac
/	extrn	move_constant,set_stk_u_error,norm_denorm
/	extrn	%gs:sr_mem_offset
/	--------to be added for unix
/..		extrn	fpfulong:far,fpfushort:far,fpsulong:far,fpsushort:far
/
	.globl	store
	.globl	fix16
	.globl	fix32
	.globl	fix64
	.globl	move_op_to_op
/
store_routine:	
	.long	single_real_store,double_real_store,int16_store
	.long	int32_store,int64_store,extended_store
	.long	int16_store,int16_store,bcd_store
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			store:
/			""""""
/	function:
/		implements all store instructions .
/
/	inputs:
/		operand1; format of result.
/
/	outputs:
/		operand in "result_format" stored in memory
/		destination pointed to by es:di.
/
/	data accessed:
/		- result_rec_offset	result_format
/		- offset_operand1
/
/	data changed:
/		- result record
/
/	procedures called:
/		set_up_indefinite	store routine [result format]
/		i_masked?
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
store:	/proc
	jz	no_unmasked_stack_error		/ branch if no stack error
	call	set_stk_u_error			/ stack underflow occurred
	testb	invalid_mask,%gs:sr_masks	/ if unmasked, just exit
	jz	return				/ else, store indef set up
no_unmasked_stack_error:			/ in operand1 by fetch_an_op
	andb	$-1! a_mask,%gs:sr_flags	/ clear a-bit
get_result_loc:
	cmpb	$reg,result_location(%ebp)
	jne	store_to_memory				/ branch if memop
	call	put_op1_result				/ give opnd1 as result
	jmp	pop_free
store_to_memory:
	movzbl	result_format(%ebp),%ebx/ call a different rtn
	shll	$2,%ebx					/ for each of the
	cmp	$8,%ebx
	jge	go_to_storer			/ do store of temporary real
	cmpb	unsupp,tag1(%ebp)		/ detect unsupported pattern
	jne	go_to_storer
	orb	invalid_mask,%gs:sr_errors	/ set i_error
	testb	invalid_mask,%gs:sr_masks	/ if unmasked, just exit
	jz	return					/ else, set up indef in
	mov	$offset_operand1,%edi		/ operand1. 
	call	set_up_indefinite
go_to_storer:
	jmp	*%cs:store_routine(%ebx)			/ output formats
/store	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			move_op_to_op:
/
/	function:
/		moves operand record (op1,op2,result_op,result2_op) to
/		another op.
/
/	inputs:
/		ss:esi points to the source, and
/		ss:edi points to the destination
/	outputs:
/		operand record in destination
/
/	data accessed:
/		operand or result variables
/
/	data changed:
/		operand or result variables
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
move_op_to_op:	/proc
	push	%ds			/ save a?msr
	push	%ss
	pop	%ds
	push	%ss
	pop	%es
	mov	$[[sign2-sign1]\/4],%ecx / set record length
	rep	
	movsl
	pop	%ds			/ reload a?msr
return:
	ret
/move_op_to_op	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			extended_store:
/			"""""""""""""""
/	function:
/		stores into double extended format.
/
/	inputs:
/		operand1
/
/	outputs:
/		double extended operand in memory.
/		stack popped if indicated
/
/	data accessed:
/		- mem_operand_pointer		sign1
/		- expon1			word_frac1
/
/	data changed:
/
/	procedures called:
/		pop_free
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
extended_store:	/proc
	les	mem_operand_pointer(%ebp),%edi	/move frac to memory
	push	%ds				/ save a?msr
	push	%ss
	pop	%ds				/ set up source pointer
	lea	2+word_frac1,%esi
	mov	$0x0002,%ecx
	rep	
	movsl
/..
/..		pushad
/..		push	es
/..extend_loop:
/..		push	edi
/..		push	[esi]
/..		call	fpsulong
/..		pop	eax
/..		pop	edi
/..		add	edi,4
/..		add	esi,4
/..		loop	extend_loop
/..		pop	es
/..		popad
/..		cld
/..
	pop	%ds						/ reload a?msr
	movb	sign1(%ebp),%ah			/ merge sign & exponent
	andw	$0x8000,%ax				/ and move to memory
	orw	expon1(%ebp),%ax
	movw	%ax,%es:(%edi)

/..
/..		pushad
/..		push	es
/..		push	edi
/..		push	ax
/..		call	fpsushort
/..		add	esp,12
/..		popad
/..		cld
	jmp	pop_free					/ pop stack, if needed
/extended_store	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			bcd_store:
/			""""""""""
/	function:
/		stores into bcd format
/
/	inputs:
/		operand1
/
/	outputs:
/		bcd operand in memory; stack popped
/
/	data accessed:
/		- mem_operand_pointer		operation_type
/		- offset_operand1		sign1
/		- tag1				word_frac1
/		- msb_frac1			offset_operand1
/		- offset_operand2		sign2
/		- expon2			word_frac2
/		- offset_operand2		offset_result
/		- result_expon			result_word_frac
/		- msb_result			offset_result
/
/	data changed:
/		- operation_type		word_frac1
/		- msb_frac1			sign2
/		- expon2			word_frac2
/		- result_word_frac		msb_result
/
/	procedures called:
/		move_op_to_op			right_shift_frac1_cl
/		right_shift_frac2_cl	right_shift_result_cl
/		left_shift_frac1_cl		left_shift_result_cl
/		divid					clear_6w
/		pop_free				test_4w
/		add_to_frac				move_constant
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
ten_18:	
	.value	0x0000,0x0000,0x0000,0x7640,0x6b3a,0x0de0b/ frac for 10**18
/
bcd_store:	/proc
	push	%gs:sr_masks				/ save current precision
	orb	prec64,%gs:sr_controls			/ and set to 64-bit
	cmpb	special,tag1(%ebp)			/ separate bcd cases
	jne	finite_value_
	jmp	store_signed_bcd
finite_value_:
	cmp	$0x07fff,dword_expon1            / is it a finite number?
	jne	check_bcd_denorm			/ yes, branch to valid
invalid_bcd:
	orb	invalid_mask,%gs:sr_errors	/ set i_error
	testb	invalid_mask,%gs:sr_masks	/ is i_error unmasked?
	jnz	store_bcd_indef
	jmp	bcd_store_done			/ no, so exit at once
store_bcd_indef:				/ yes, store bcd_indef
	mov	$0, dword_frac1+frac80(%ebp)	/ clear low frac
	mov	$0x80000000, dword_frac1+6(%ebp)/ set only ms bit of hi frac
	movw	$0x0ffff, 8+word_frac1		/set msw to all ones,
	jmp	store_bcd_result		/ and give as answer
check_bcd_denorm:
	cmpb	denormd,tag1(%ebp)			/ separate bcd cases
	jne	round_to_int
	mov	$0x0001,dword_expon1	/ if masked d-error, make valid
	movb	valid,tag1(%ebp)
round_to_int:
	mov	$0x403e,%eax
	cmp	%eax,dword_expon1		/ if expon >= 63, then number
	jge	invalid_bcd			/ is already too big for bcd
	mov	$offset_operand1,%edi	/ gradual uflow until expon=63
	push	%edi
	call	gradual_underflow
	pop	%edi				/ round to precision 64
	movb	prec64,%dl
	movb	false,%al
	call	round
detect_zero:
	xor	%eax,%eax
	mov	$dword_frac1+4,%edi	/ if fraction = 0, result = 0
	call	test_4w
	jnz	below_10_to_18_
	mov	%eax,dword_expon1		/ set result to true zero
	movb	special,tag1(%ebp)
	jmp	store_bcd_round_data
below_10_to_18_:
	mov	4(%ebp,%edi),%edx
	cmp	$0x0de0b6b3,%edx
	ja	invalid_bcd
	mov	(%ebp,%edi),%eax
	jb	divide_by_10_to_9
	cmp	$0x0a7640000,%eax
	jae	invalid_bcd
divide_by_10_to_9:
	mov	$0,4(%ebp,%edi)	/ clear high dword of frac1
	mov	$0,(%ebp,%edi)/ clear  low dword of 8 byte frac1 
	sub	$2,%edi			/ make edi point to 10 byte frac1
	mov	$0x3b9aca00,%ebx		/ ten to the 9th, as hex ordinal
	div	%ebx
	push	%eax						/ save quotient
	mov	$4,%ecx
	mov	$100,%ebx
next_pair_nibbles:
	mov	%edx,%eax
	xor	%edx,%edx
	div	%ebx
	xchgl	%edx,%eax
	aam
second_half_loop_start:
	shlb	$4,%ah
	orb	%ah,%al
	movb	%al,(%ebp,%edi)
	inc	%edi
	loop	next_pair_nibbles
do_second_half_:
	pop	%eax
	or	%eax,%eax					/ is eax 0?
	movb	%dl,(%ebp,%edi)
	jz	store_bcd_round_data
	push	%edx
	xor	%edx,%edx
	mov	$10,%ecx
	div	%ecx
	xchgl	%edx,%eax
	pop	%ecx
	movb	%al, %ah
	movb	%cl, %al
	push	$0
	mov	$5, %ecx
	jmp	second_half_loop_start
store_bcd_round_data:
	cmpb	true,rnd1_inexact
	jne	store_signed_bcd
	orb	$inexact_mask,%gs:sr_errors
	cmpb	true,added_one
	jne	store_signed_bcd
	orb	$a_mask,%gs:sr_flags
store_signed_bcd:
	movb	sign1(%ebp),%ah			/ set sign in bcd number
	andb	$0x080,%ah
	movb	%ah,msb_frac1
store_bcd_result:
	les	%gs:sr_mem_offset,%edi	/move frac1 to memory
	push	%ds				/ save a?msr
	push	%ss
	pop	%ds
	lea	word_frac1,%esi
	mov	$5,%ecx
	rep	
	movsw

/..
/..		pushad
/..		push	es
/..bcdst_loop:
/..		push	edi
/..		push	[esi]
/..		call	fpsushort
/..		add	esp,8
/..		add	edi,2
/..		add	esi,2
/..		loop	bcdst_loop
/..		pop	es
/..		popad
/..		cld
/..

	pop	%ds					/ reload a?msr
	call	pop_free
bcd_store_done:
	pop	%gs:sr_masks				/ restore old setting
	ret
/bcd_store	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			single_real_store:
/			""""""""""""""""""
/	function:
/		implements store to single-precision real format
/
/	inputs:
/		operand1
/
/	outputs:
/		single_precision real in memory;
/		stack popped if indicated.
/
/	data accessed:
/		- mem_operand_pointer		offset_operand1
/		- sign1				tag1
/		- expon1			word_frac
/		- msb_frac1
/
/	data changed:
/
/	procedures called:
/		round				store_valid
/		addition_normalized		special_round_test
/		directed_round_test		gradual_underflow
/		pop_free			test_3w
/		u_masked?			set_o_error
/		o_masked?			set_p_error
/		set_u_error			set_i_masked?
/		store_denormd
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
single_real_store:	/proc
	push	$pop_free				/ return to pop_free
	les	mem_operand_pointer(%ebp),%edi	/set es:di to memop
	mov	low_extended_expon_for_single,%esi
	movb	prec24,%dl 				/ 24-bit precision
	movb	tag1(%ebp),%al			/ load op1 tag
	cmpb	valid,%al
	je	single_valid				/ detect valid, infinity,
	cmpb	denormd,%al				/ denormd, invalid, and
	je	single_denormd				/ special cases
	cmpb	inv,%al					/ check whether op1 is nan
	jne	single_special				/ branch unless op1 is nan
	testb	$0x40,msb_frac1				/ check whether nan is quiet
	jnz	single_special				/ branch if nan is quiet
	orb	invalid_mask,%gs:sr_errors	/ set i_error for snan
	testb	invalid_mask,%gs:sr_masks	/ if unmasked, just exit
	jz	special_return				/ else, change to qnan
	orb	$0x40,msb_frac1
single_special:
	mov	dword_frac1+frac32+1(%ebp),%ecx
	and	$0x807fffff,%ecx
	movzbl	expon1(%ebp),%eax
	shl	$23,%eax
	or	%ecx,%eax
	mov	%eax,%es:(%edi)
/..
/..		pushad
/..		push	es
/..		push	edi
/..		push	eax
/..		call	fpsulong
/..		add		esp,	12
/..		popad
/..		cld
/..
special_return:
	ret

single_denormd:
	mov	$offset_operand1,%edi	/ op1 is denormal or pseudo-denormal
	call	norm_denorm		/ don't set d_error; just normalize
single_valid:
	mov	high_extended_expon_for_single,%edi / load parameter
	call	store_valid
	les	mem_operand_pointer(%ebp),%edi	/ es:edi --> memop
	jz	single_infinity		/ zf = 1 => store infinity
	js	single_max			/ sf = 1 => store max
	jc	single_qnan			/ cf = 1 => store qnan
single_store:
	mov	dword_frac1+frac32+1(%ebp),%eax
	and	$0x80ffffff,%eax
	test	$0x00ffffff,%eax
	jz	single_store_it
	and	$0x807fffff,%eax
	movzbl	expon1(%ebp),%ecx
	addb	$0x80,%cl
	shl	$23,%ecx
	or	%ecx,%eax
single_store_it:
	mov	%eax,%es:(%edi)
/..
/..		pushad
/..		push	es
/..		push	edi
/..		push	eax
/..		call	fpsulong
/..		add	esp,12
/..		popad
/..		cld
/..
	ret					/do needed pop or free

single_max:
	mov	$0x7fffff7f,%eax
	jmp	single_sign_on
single_infinity:
	mov	$0x8000007f,%eax
single_sign_on:
	orb	sign1(%ebp),%al
	ror	$8,%eax
	jmp	single_store_it
single_qnan:
	mov	$0xffffffff,%es:(%edi)
	pop	%eax				/ skip pop or free
	ret					/   since this happens on error

store_valid:
	push	%edi				/ stack high_expon
	push	%esi				/ stack low_expon
	push	%edx				/ stack prec control
	movb	false,%al			/ not second rounding
	mov	$offset_operand1,%edi	/ round to prec (dl)
/	push	edi					; round preserves edi
	call	round
/		pop		edi
	call	addition_normalize		/ and renormalize
	pop	%edx				/ unstack prec control
	pop	%esi				/ unstack low_expon
	pop	%edi				/ unstack high_expon
	cmp	%esi,dword_expon1		/ is underflow possible?
	jl	store_underflow_	/ < low_expon, so underflow possible
	je	decrement_exponent		/ no, = low_expon?
	cmp	%edi,dword_expon1		/ no, > high_expon?
	jle	do_store_valid			/ no, store number
	orb	overflow_mask,%gs:sr_errors	/ set o_error
	testb	overflow_mask,%gs:sr_masks	/ if unmasked, violate ieee standard
	jnz	masked_overflow		/ and abort without rounding source
unmasked_ov_underflow:
/
/ the commentized instructions immediately following are appropriate for
/ unmasked over/underflow response in accord with the ieee standard.  on the
/ 80387, unmasked over/underflows on conversion leave for the trap handler the
/ source operand, unmodified by rounding, exponent rebiasing, or anything else.
/ therefore, to report that preliminary rounding was inexact or achieved by
/ increasing the source seems inconsistent with the 80387's philosophy
/ of entering an over/underflow trap handler with the 80387's state changed
/ only insofar as the over/underflow flag has been set, so skip that report.
/
/	cmp	rnd1_inexact,	true		; detect inexact result
/	jne	move_op1_to_87_tos  		; if exact, branch to replace source
/	or	%gs:sr_errors,	inexact_mask	; else, set i_error
/	cmp	added_one,	true		; was rounding done by adding one?
/	jne	move_op1_to_87_tos		; if not, branch to replace source
/	or	%gs:sr_flags,	a_mask		; else, set a_bit
/move_op1_to_87_tos:
/	mov	[ebp].result_location,	stack_top
/	pop	%edi		; discard caller's return address
/	pop	%edi		; discard pop_free address
/	jmp	put_op1_result
/
/ Instead, we return a special error condition (sf=0 zf=0 cf=1)
/ to our caller, to cause a quiet NaN to be generated, as the 80387 does.
	movb	$1, %ah
	sahf
	ret

masked_overflow:
	orb	$inexact_mask,%gs:sr_errors	/ set inexact_error
	movb	%gs:sr_controls,%al			/ load control byte
	andb	$rnd_control_mask,%al	/ mask in rounding control
	movb	rnd_up,%ah
	cmpb	%al,%ah
	js	store_valid_done		/ branch if zf = 0 & sf = 1
	movb	sign1(%ebp),%al				/ get sign of op1
	call	special_round_test			/ after round test, cf=0
	pushf				/  (zf=0 sf=1)/(zf=1 sf=0)
	js	pop_store_valid_done		/ sf = 1 => store max
	orb	$a_mask,%gs:sr_flags			/ set a_bit
pop_store_valid_done:
	popf						/ reload flags
store_valid_done:
	ret									/ exit
store_underflow_:
	testb	underflow_mask,%gs:sr_masks	/ if underflow masked,
	jnz	check_masked_underflow	/ then branch to check for inexact
	orb	underflow_mask,%gs:sr_errors/ else, set u_error
/
/ after the underflow exception has been set, the response to unmasked
/ underflow ought to be the same as that to unmasked overflow.
	jmp	unmasked_ov_underflow

check_masked_underflow:
	mov	$offset_operand1,%edi		/ load op1 offset
	mov	%esi,%eax				/ load low exponent
	push	%edi					/ save op1 offset
	push	%edx					/ save prec control
	call	gradual_underflow			/ do gradual underflow
	pop	%edx					/ reload prec control
	pop	%edi					/ reload op1 offset
	movb	true,%al				/ perform second round
	call	round
	cmpw	$0,rnd_history			/ signal_u_error?
	je	decrement_exponent
	orb	underflow_mask+inexact_mask,%gs:sr_errors
decrement_exponent:
	testb	$0x80,msb_frac1				/ test msb of fraction
	jnz	do_store_valid		/ don't decrement if 800..0h
	decw	expon1(%ebp)				/ else, decrement expon
do_store_valid:
	cmpw	$0,rnd_history			/ signal_p_error?
	je	adjust_flags
	orb	$inexact_mask,%gs:sr_errors	/ signal_p_error.
	cmpb	true,added_one		/ was rounding done by adding one?
	jne	adjust_flags		/ if not, branch to adjust flags
	orb	$a_mask,%gs:sr_flags		/ else, set a_bit
adjust_flags:
	xor	%eax,%eax		/ cf = 0, sf = 0, zf = 0 for
	inc	%eax			/  store valid exit
	ret

/single_real_store	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			double_real_store:
/			""""""""""""""""""
/	function:
/		implements store to double-precision real format
/
/	inputs:
/		operand1
/
/	outputs:
/		double precision real in memory;
/		stack popped if indicated.
/
/	data accessed:
/		- mem_operand_pointer		offset_operand1
/		- sign1				tag1
/		- expon1			word_frac1
/		- msb_frac1			offset_operand1
/
/	data changed:
/		- sign1				word_frac1
/
/	procedures called:
/		test_3w				store_valid
/		right_shift_frac1_cl		pop_free
/		double_zero			double_fill
/		store_denormd			double_frac_mem
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
double_real_store:	/proc
	push	$pop_free				/ return to pop_free
	mov	low_extended_expon_for_double,%esi
	movb	prec53,%dl 				/ 53-bit precision
	movb	tag1(%ebp),%al			/ separate special cases
	cmpb	valid,%al
	je	double_valid
	cmpb	denormd,%al
	je	double_denormd
	cmpb	special,%al
	jne	inv_or_inf
	movw	$0x3c00,expon1(%ebp)
	jmp	double_frac_mem
inv_or_inf:
	cmpb	inv,%al
	jne	double_special
	testb	$0x40,msb_frac1			/ check whether nan is quiet
	jnz	double_special				/ branch if nan is quiet
	orb	invalid_mask,%gs:sr_errors	/ set i_error for snan
	testb	invalid_mask,%gs:sr_masks	/ if unmasked, just exit
	jnz	make_dbl_qnan				/ else, change to qnan
dbl_special_return:
	ret
make_dbl_qnan:
	orb	$0x40,msb_frac1
double_special:
	movw	$0x3c00+0x07ff,expon1(%ebp)
	jmp	double_frac_mem			/ store frac to memory
double_denormd:
	mov	$offset_operand1,%edi	/ op1 is denormal or pseudo-denormal
	call	norm_denorm		/ don't set d_error; just normalize
double_valid:
	mov	high_extended_expon_for_double,%edi
	call	store_valid
	jz	double_infinity			/ zf = 1 ==> store infinity
	js	double_max				/ sf = 1 ==> store max
	jc	double_qnan				/ cf = 1 ==> store qnan
double_store:
	movb	3+lsb_frac1,%al	/ else store double
	cbtw
	mov	$dword_frac1+6,%edi
	call	test_3w
	jz	double_zero				/ store signed zero
double_frac_mem:
	movb	$3,%cl					/ start by shifting
	call	right_shift_frac1_cl		/ fraction right 3 bits
	les	mem_operand_pointer(%ebp),%edi	/ es:edi points to memop
	push	%ds				/ save a?msr
	push	%ss
	pop	%ds				/ set up source pointer
	lea	3+word_frac1,%esi		/ move frac to memory
	mov	$3,%ecx
	rep	
	movsw
/..
/..		pushad
/..		push	es
/..doublst_loop:
/..		push	edi
/..		push	[esi]
/..		call	fpsushort
/..		add	esp,8
/..		add	esi,2
/..		add	edi,2
/..		loop	doublst_loop
/..		add	esp,4
/..		popad
/..		cld
/..
	pop	%ds					/ reload a?msr
	movw	expon1(%ebp),%ax		/ fetch high-order word,
	subw	$0x3c00,%ax				/ with expon, sign, and
	shlw	$4,%ax					/ top 4 bits of fraction
	andb	$0x0f,msb_frac1
	orb	msb_frac1,%al
	andb	$0x80,sign1(%ebp)
	orb	sign1(%ebp),%ah
	movw	%ax,%es:(%edi)				/ es:di points to expon
/..
/..		pushad
/..		push	es
/..		push	edi
/..		push	ax
/..		call	fpsushort
/..		add	esp,12
/..		popad
/..		cld
/..

	ret					/ do pop or free

double_zero:
	xor	%eax,%eax			/ store +/- 0 to memory
double_fill:
	les	mem_operand_pointer(%ebp),%edi	/ es:edi points to memop
	mov	%eax,%es:(%edi)
/..
/..		pushad
/..		push	es
/..		push	edi
/..		push	eax
/..		call	fpsulong
/..		add	esp,12
/..

	movw	%ax,%es:4(%edi)
/..
/..		push	es
/..		push	edi+4
/..		push	ax
/..		call	fpsushort
/..		add	esp,12
/..		popad
/..		cld
/..
	movb	sign1(%ebp),%ah
	andb	$0x80,%ah
double_expon:
	movw	%ax,%es:6(%edi)			/ store the exponent
/..
/..		pushad
/..		push	es
/..		push	edi+12
/..		push	ax
/..		call	fpsushort
/..		add	esp,12
/..		popad
/..		cld
/..
	ret

double_max:
	mov	$0x0ffffffff,%eax			/ set fraction to ff..f
	call	double_fill
	movw	$0x7fef,%ax				/ set exponent to 7feh
	orb	sign1(%ebp),%ah			/ or sign bit
	jmp	double_expon
double_infinity:
	call	double_zero			/ first clear the memop
	orw	$0x7ff0,%es:6(%edi)	/ es:di -> memop
/..
/..		pushad
/..		push	es
/..		push	edi+6
/..		call	fpfushort
/..		or	ax,7ff0h
/..		push	ax
/..		call	fpsushort
/..		add	esp,12
/..		popad
/..		cld
/..
	ret
double_qnan:
	mov	$0xffffffff,%eax		/ set fraction to ff..f
	call	double_fill
	movw	$0xffff,%es:6(%edi)	/ es:di -> memop (set exp to ff..f)
	pop	%eax				/ skip pop or free
	ret					/   since this happens on error
/double_real_store	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/		fix16:
/
/	function:
/		 fixes valid number to a 16-bit integer.
/
/	inputs:
/		 operand1
/
/	outputs:
/		16-bit integer in word_frac1 + 8;
/		overflow indication in zf (zf:1 => no overflow)
/
/	data accessed:
/		- offset_operand1		sign1
/		- expon1			word_frac1
/		- offset_operand1
/
/	data changed:
/		- word_frac1			extra_word_reg
/
/	procedures called:
/		sticky_right_shift		round
/		careful_round			p_error?
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
fix16:	/proc
	mov	high_int16_exponent,%ecx		/ calc shift amount
	sub	dword_expon1,%ecx			/ if expon1 > high expon
	js	no_fix16_ovf			/ then overflow (zf=0)
	cmp	max_int16_shift,%ecx			/ don't let shift amount
	jbe	do_shift_16			/ exceed max_int16_shift
	mov	max_int16_shift,%ecx
do_shift_16:
	mov	$offset_operand1,%edi		/ shift frac1 by cl
	xorb	%al,%al
	call	sticky_right_shift
	mov	$offset_operand1,%edi		/ round to 16-bit prec
	movb	prec16,%dl
	movb	false,%al
	call	round
	cmpb	false,%al				/ overflow if al=true
	jnz	no_fix16_ovf				/ round cy out (zf=1)
	cmpw	$0x8000,8+word_frac1
	jb	check_sign_16				/ no overflow if < 8000h
	ja	no_fix16_ovf				/ else, overflow (zf=0)
	cmpb	negative,sign1(%ebp)	/ oflow if = 8000h and
	jnz	no_fix16_ovf				/ sign1 = pos (zf=0)
check_sign_16:
	movb	positive,%al			/ al is overflow flag
	cmpb	sign1(%ebp),%al			/ negate integer if
	jz	no_fix16_ovf				/ sign is neg (zf=1)
	negw	8+word_frac1
	xorb	%al,%al					/ no overflow (zf=1)
no_fix16_ovf:
	ret
/fix16	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			int16_store:
/
/	function:
/		stores into 16-bit integer format
/
/	inputs:
/		operand1
/
/	outputs:
/		16-bit integer in memory operand location
/
/	data accessed:
/		- mem_operand_pointer		tag1
/		- word_frac1			msb_frac1
/		- offset_operand1		extra_word_reg
/
/	data changed:
/
/	procedures called:
/		fix16				pop_free
/		invalid_or_special		get_rnd_control
/		set_i_error			i_masked?
/		set_p_error			clear_p_error
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
int16_store:	/proc
/
	cmpb	valid,tag1(%ebp)
	je	valid_16
	cmpb	denormd,tag1(%ebp)
	jne	special_16
valid_16:
	call	fix16				/ make op1 a 16-bit int
	jnz	special_16			/ if zf=0, oflow on fix
	cmpw	$0,rnd_history
	je	store_16
	orb	$inexact_mask,%gs:sr_errors
	cmpb	true,added_one
	jne	store_16
	orb	$a_mask,%gs:sr_flags
store_16:
	movw	8+word_frac1,%ax	/ move integer to memory
pointer_16:
	les	mem_operand_pointer(%ebp),%edi	/set es:di to location
finish_16:
	movw	%ax,%es:(%edi)		/ store integer to mem
/..
/..		pushad
/..		push	es
/..		push	edi
/..		push	ax
/..		call	fpsushort
/..		add	esp,12
/..		popad
/..		cld
/..
	jmp	pop_free			/ pop stack and go home
special_16:
	xorw	%ax,%ax
	cmpb	special,tag1(%ebp)		/ is operand tagged special?
	je	pointer_16				/ yes, load memop ptr and exit
	orb	invalid_mask,%gs:sr_errors/ no, set invalid error
	movw	$0x8000,%ax			/ load inv constant into ax
	testb	invalid_mask,%gs:sr_masks/ is invalid error masked?
	jnz	pointer_16				/ yes, store invalid constant
	ret
/int16_store	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/		fix32:
/
/	function:
/		 fixes valid number to a 32-bit integer.
/
/	inputs:
/		 operand1
/
/	outputs:
/		32-bit integer in word_frac1 + 6;
/		overflow indication in zf (zf:1 => no overflow)
/
/	data accessed:
/		- offset_operand1		sign1
/		- dword_expon1			word_frac1
/		- offset_operand1
/
/	data changed:
/		- word_frac1
/
/	procedures called:
/		sticky_right_shift		round
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
fix32:	/proc
	mov	high_int32_exponent,%ecx		/ calc shift amount
	sub	dword_expon1,%ecx			/ if expon1 > high expon
	js	no_fix32_ovf					/ then overflow (zf=0)
	cmp	max_int32_shift,%ecx			/ don't let shift amount
	jbe	do_shift_32						/ exceed max_int32_shift
	mov	max_int32_shift,%ecx
do_shift_32:
	mov	$offset_operand1,%edi		/ shift frac1 by cl
	xorb	%al,%al
	call	sticky_right_shift
	mov	$offset_operand1,%edi		/ round to 16-bit prec
	movb	prec32,%dl
	movb	false,%al
	call	round
	cmpb	false,%al				/ overflow if al=true
	jnz	no_fix32_ovf				/ round cy out (zf=1)
	cmp	$0x080000000,dword_frac1+frac32(%ebp)
	jb	check_sign_32				/ no overflow if < 80000000h
	ja	no_fix32_ovf				/ else, overflow (zf=0)
	cmpb	negative,sign1(%ebp)	/ oflow if = 80000000h and
	jnz	no_fix32_ovf				/ sign1 = pos (zf=0)
check_sign_32:
	movb	positive,%al			/ al is overflow flag
	cmpb	sign1(%ebp),%al			/ negate integer if
	jz	no_fix32_ovf				/ sign is neg (zf=1)
	neg	dword_frac1+frac32(%ebp)
	xorb	%al,%al					/ no overflow (zf=1)
no_fix32_ovf:
	ret
/fix32	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/		int32_store:
/
/	function:
/		stores into 32-bit integer format
/
/	input:
/		operand1
/
/	output:
/		32-bit integer in location pointed to by es:di.
/
/	data accessed:
/		- mem_operand_pointer		offset_operand1
/		- sign1				tag1
/		- expon1			word_frac1
/		- msb_frac1			offset_operand1
/
/	data changed:
/		- word_frac1
/
/	procedures called:
/		pop_free
/		fix32
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
int32_store:	/proc
	cmpb	valid,tag1(%ebp)
	je	valid_32
	cmpb	denormd,tag1(%ebp)
	jne	special_32
valid_32:
	call	fix32				/ make op1 a 32-bit int
	jnz	special_32			/ if zf=0, oflow on fix
	cmpw	$0,rnd_history
	je	store_32
	orb	$inexact_mask,%gs:sr_errors
	cmpb	true,added_one
	jne	store_32
	orb	$a_mask,%gs:sr_flags
store_32:
	mov	dword_frac1+frac32(%ebp),%eax	/ move integer to eax
pointer_32:
	les	mem_operand_pointer(%ebp),%edi	/set es:di to location
finish_32:
	mov	%eax,%es:(%edi)		/ store integer to mem
/..
/..		pushad
/..		push	es
/..		push	edi
/..		push	eax
/..		call	fpsulong
/..		add	esp,12
/..		popad
/..		cld
/..
	jmp	pop_free			/ pop stack and go home
special_32:
	xor	%eax,%eax
	cmpb	special,tag1(%ebp)		/ is operand tagged special?
	je	pointer_32				/ yes, load memop ptr and exit
	orb	invalid_mask,%gs:sr_errors/ no, set invalid error
	mov	$0x080000000,%eax	/ load inv constant into ax
	testb	invalid_mask,%gs:sr_masks/ is invalid error masked?
	jnz	pointer_32				/ yes, store invalid constant
	ret
/int32_store	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/		fix64:
/
/	function:
/		 fixes valid number to a 64-bit integer.
/
/	inputs:
/		 operand1
/
/	outputs:
/		64-bit integer in word_frac1 + 2;
/		overflow indication in zf (zf:1 => no overflow)
/
/	data accessed:
/		- offset_operand1		sign1
/		- dword_expon1			word_frac1
/		- offset_operand1
/
/	data changed:
/		- word_frac1
/
/	procedures called:
/		sticky_right_shift		round
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
fix64:	/proc
	mov	high_int64_exponent,%ecx		/ calc shift amount
	sub	dword_expon1,%ecx			/ if expon1 > high expon
	js	no_fix64_ovf				/ then overflow (zf=0)
	cmp	max_int64_shift,%ecx			/ don't let shift amount
	jbe	do_shift_64				/ exceed max_int64_shift
	mov	max_int64_shift,%ecx
do_shift_64:
	mov	$offset_operand1,%edi		/ shift frac1 by cl
	xorb	%al,%al
	call	sticky_right_shift
	mov	$offset_operand1,%edi		/ round to 64-bit prec
	movb	prec64,%dl
	movb	false,%al
	call	round
	cmpb	false,%al				/ overflow if al=true
	jnz	no_fix64_ovf				/ round cy out (zf=1)
	cmp	$0x080000000,dword_frac1+frac32(%ebp)
	jb	check_sign_64			/ no overflow if < 80000000h
	ja	no_fix64_ovf				/ else, overflow (zf=0)
	cmp	$0,dword_frac1+frac64(%ebp)
	jnz	no_fix64_ovf
	cmpb	negative,sign1(%ebp)	/ oflow if = 800..0h and
	jnz	no_fix64_ovf				/ sign1 = pos (zf=0)
check_sign_64:
	movb	positive,%al			/ al is overflow flag
	cmpb	sign1(%ebp),%al			/ negate integer if
	jz	no_fix64_ovf				/ sign is neg (zf=1)
	xor	%eax,%eax
	neg	dword_frac1+frac64(%ebp)
	sbb	dword_frac1+frac32(%ebp),%eax
	mov	%eax,dword_frac1+frac32(%ebp)
	xorb	%al,%al					/ no overflow (zf=1)
no_fix64_ovf:
	ret
/fix64	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			int64_store:
/			""""""""""""
/	function:
/		stores into 64-bit integer format
/
/	input:
/		operand1
/
/	output:
/		64-bit integer in memory location pointed to by es:edi.
/
/	data accessed:
/		- mem_operand_pointer		offset_operand1
/		- sign1				tag1
/		- expon1			word_frac1
/		- msb_frac1			offset_operand1
/
/	data changed:
/		- word_frac1
/
/	procedures called:
/		pop_free
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
int64_store:	/proc
	cmpb	valid,tag1(%ebp)
	je	valid_64
	cmpb	denormd,tag1(%ebp)
	jne	special_64
valid_64:
	call	fix64				/ make op1 a 64-bit int
	jnz	special_64			/ if zf=0, oflow on fix
	cmpw	$0,rnd_history
	je	store_64
	orb	$inexact_mask,%gs:sr_errors
	cmpb	true,added_one
	jne	store_64
	orb	$a_mask,%gs:sr_flags
store_64:
	mov	dword_frac1+frac64(%ebp),%eax	/ move integer to edx:eax
	mov	dword_frac1+frac32(%ebp),%edx	/ move integer to edx:eax
pointer_64:
	les	mem_operand_pointer(%ebp),%edi	/set es:edi to location
finish_64:
	mov	%eax,%es:(%edi)		/ store low integer to low mem
/..
/..		pushad
/..		push	es
/..		push	edi
/..		push	eax
/..		call	fpsulong
/..		add	esp,12
/..		popad
/..		cld
/..
	mov	%edx,%es:4(%edi)		/ store high integer to high mem
/..
/..		pushad
/..		push	es
/..		push	edi
/..		push	edx
/..		call	fpsulong
/..		add	esp,12
/..		popad
/..		cld
/..
	jmp	pop_free			/ pop stack and go home
special_64:
	xor	%eax,%eax
	cltd
	cmpb	special,tag1(%ebp)		/ is operand tagged special?
	je	pointer_64				/ yes, load memop ptr and exit
	orb	invalid_mask,%gs:sr_errors/ no, set invalid error
	mov	$0x080000000,%edx	/ load inv constant into ax
	testb	invalid_mask,%gs:sr_masks/ is invalid error masked?
	jnz	pointer_64		/ yes, store invalid constant
	ret

/int64_store	endp
/
/a_med	ends
/
/	end
