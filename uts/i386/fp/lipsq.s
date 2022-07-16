/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file 	"lipsq.s"

	.ident	"@(#)kern-fp:lipsq.s	1.1"

/ ***************************************************************************
/
/			l i p s q . m o d
/			=================
/
/   ===============================================================
/               intel corporation proprietary information
/    this software  is  supplied  under  the  terms  of  a  license
/    agreement  or non-disclosure agreement  with intel corporation
/    and  may not be copied nor disclosed except in accordance with
/    the terms of that agreement.                                  
/   ===============================================================
/
/	functions:
/		implements the loading of the constants:
/		 one, log base 2 of ten, log base 2 of e, pi,
/		 log base 10 of 2, log base e of 2, and zero.
/		implements the 80387 square root instruction.
/		implements the 80387 integer part instruction.
/
/	public procedures:
/		load_con		sqrt			intpt
/
/ *****************************************************************************
/
/...september 16, 1983...
/
/	.file	"a_mli"
/
/$nolist
#include	"e80387.h"
/$list
	.text	/a_med	segment	er	public
/
/	extrn	put_si_result,sticky_right_shift,round
/	extrn	addition_normalize,left_shift,getx
/	extrn	set_up_indefinite,clear_6w,set_6w
/	extrn	test_4w,left_shift_result_cl
/	extrn	left_shift_frac1_cl,left_shift_frac2_cl
/	extrn	add_to_frac_2,gradual_underflow
/	extrn	subtraction_normalize,put_indefinite
/	extrn	i_masked_,set_i_masked_,set_d_masked_
/	extrn	affine_infinity_,get_precision
/
	.globl	load_con
	.globl	intpt
	.globl	sqrt
/
/	temp real floating point numbers for push constant instructions
/
treal_table:
	.value	0x00000,0x00000,0x00000,0x00000,0x08000,0x03fff	/treal_one
	.value	0x04000,0x08afe,0x0cd1b,0x0784b,0x0d49a,0x04000	/treal_l2t
	.value	0x0c000,0x0f0bb,0x05c17,0x03b29,0x0b8aa,0x03fff	/treal_l2e
	.value	0x0c000,0x0c234,0x02168,0x0daa2,0x0c90f,0x04000	/treal_pi
	.value	0x0a000,0x0f798,0x0fbcf,0x09a84,0x09a20,0x03ffd	/treal_lg2
	.value	0x0e000,0x079ab,0x0d1cf,0x017f7,0x0b172,0x03ffe	/treal_ln2
	.value	0x00000,0x00000,0x00000,0x00000,0x00000,0x00000	/treal_0
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			load_con
/			""""""
/	function:
/		implements the load constants instructions.
/
/	inputs:
/		offset of fpn pointer in bx register.
/
/	outputs:
/		constant value on top of stack
/
/	data accessed:
/		- offset_result
/
/	data changed:
/		- mem_operand_pointer
/
/	procedures called:
/		getx
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/  all load constant instructions use the same entry point.
/
load_con:	/proc	near
/
/   the contents of bx is used as an index to treal_table.
/   getx unpacks the extended floating point number and put_result
/   pushes the value to the top of the 80387 stack.
/
	shrb	$1,%bl				/take care of extra shi
	mov	$0x06,%eax				/treal_table ptr = cs:
	mulb	%bl				/ index * 6 - (6 * 2)
	add	$[treal_table-[load_1_op\*12]],%eax / * (load_1_op)
	mov	%eax, %esi
	mov	$0, result_dword_frac(%ebp)
	movw	%cs:(%esi), %ax
	movw	%ax, result_dword_frac+frac80(%ebp)
	mov	%cs:2(%esi), %eax
	mov	%eax, result_dword_frac+frac64(%ebp)
	mov	%cs:6(%esi), %eax
	mov	%eax, result_dword_frac+frac32(%ebp)
	movzwl	%cs:10(%esi), %eax
	mov	%eax, dword_result_expon
	xor	%eax, %eax
	mov	%eax, result_sign(%ebp)
	movb	prec64, %dl
	mov	$offset_result, %edi
	call	round
	mov	$offset_result, %edi
	call	addition_normalize
	andb	$ -1 ! a_mask, %gs:sr_flags
	xor	%eax, %eax
	mov	before_error_signals(%ebp), %eax
	lahf
	mov	$offset_result, %edi
	jmp	put_si_result
/load_con	endp
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			intpt:
/			"""""
/	function:
/		implements 80387 integer part instruction
/
/	inputs:
/
/	outputs:
/
/	data accessed:
/		- offset_result			offset_operand1
/		- tag1				expon1
/		- word_frac1
/
/	data changed:
/		- tag1				expon1
/		- result
/
/	procedures called:
/		get_operand			gradual_underflow
/		round				subtraction_normalize
/		put_si_result			test_4w
/		i_masked?			set_i_masked?
/		set_d_masked?
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
intpt:	/proc
/	movb	true,%ah			/ load constant
	jz	separate_cases		/ branch if no stack error
	call	set_stk_u_error		
	testb	invalid_mask, %gs:sr_masks
	jz	intpt_done
	jmp	put_op1_result		/math stack error, so return indef
separate_cases:
	andb	$ -1 ! a_mask, %gs:sr_flags /initialize a-bit to zero
	movb	tag1(%ebp), %al		/ load op1 tag
	cmpb	valid,%al
	je	valid_case
	cmpb	denormd,%al
	je	denormalized_operand
	cmpb	unsupp,%al
	jne	check_nan
	movl	$offset_operand1, %edi
	call	set_up_indefinite
set_i_err:
	orb	invalid_mask, %gs:sr_errors
	testb	invalid_mask, %gs:sr_masks
	jz	intpt_done
	jmp	put_op1_result	/ masked_i error so return indef
intpt_done:
	ret
check_nan:
	cmpb	inv, %al
	je	kind_of_nan_	
	jmp	put_op1_result 	/infinity or zero, same answer
kind_of_nan_:
	testb	$0x40, msb_frac1
	jz	make_qnan
	jmp	put_op1_result	/ op1 is a qnan so pass it through
make_qnan:
	orb	$0x40, msb_frac1 /op1 is an snan so make it a qnan
	jmp	set_i_err	/and signal i_error
denormalized_operand:
	orb	denorm_mask, %gs:sr_errors
	testb	denorm_mask, %gs:sr_masks
	jz	intpt_done
	movw	$0x0001,expon1(%ebp)	/if masked d-error, make valid
	movb	valid,tag1(%ebp)
valid_case:
	movl	$0x403e,%eax
	cmpl	%eax,dword_expon1 /if expon >=63, then number
	jge	give_op1		/ is already an integer
	mov	$offset_operand1,%edi	/gradual uflow until expon=63
	push	%edi
	call	gradual_underflow
	pop	%edi			/round to precision 64
	movb	prec64,%dl
	movb	false,%al
	call	round
	cmpb	true, rnd1_inexact
	jne	detect_zero
	orb	$inexact_mask, %gs:sr_errors
	cmpb	true,added_one
	jne	detect_zero
	orb	$a_mask, %gs:sr_flags
detect_zero:
	xor	%eax,%eax
	movl	$dword_frac1+4,%edi /if fraction = 0, result = 0
	call	test_4w
	jz	zero_result
	mov	$offset_operand1,%edi	/normalize
	call	subtraction_normalize
	jmp	put_op1_result
zero_result:
	movl	%eax,dword_expon1 		/set result to true zero
	movb	special,tag1(%ebp)
give_op1:
	jmp	put_op1_result
/intpt	endp
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			sqrtx:
/
/	function:
/		 fractional square root routine.
/
/	inputs:
/		assumes valid, non-zero,positive, normalized
/		fraction is in frac1.
/
/	outputs
/		leaves fractional square root in result.
/
/	data accessed:
/		- offset_operand1		lsb_frac1
/		- offset_operand2		word_frac2
/		- offset_result			lsb_frac2
/		- result_word_frac		lsb_result
/		- msb_frac1
/
/	data changed:
/		- word_frac2			lsb_frac2
/		- result_word_frac		lsb_result
/
/	procedures called:
/		left_shift_result_cl		left_shift_frac1_cl
/		left_shift_frac2_cl		clear_6w
/		set_6w				add_to_frac_2
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
sqrtx:	/proc
	movl	$result_dword_frac,%edi /during this computation,
	call	clear_6w		/the lsb of the result will
	mov	$dword_frac2,%edi	/ hold g and s, and the msb
	call	set_6w			/ will hold carry-out bits
	and	$0x000000ff,%ecx		/clear high word
	stc				/cf holds the quotient bit
	push	$65			/ iterate 65 times
	pushf				/ stack the quotient bit
	jmp	enter_sqrt_loop
sqrt_loop:
	push	%ecx
	pushf				/ stack the q_bit
	movb	$1,%cl
	call	left_shift_result_cl	/shift result left one bit
	popf				/ inject the new q_bit into
	pushf				/ the least significant byte
	adcb	%cl,%cl
	orb	%cl,1+lsb_result
	movb	$2,%cl			/ into the lsb
	call	left_shift_frac2_cl	/shift frac2 left 2 bits
enter_sqrt_loop:
	movb	msb_frac1,%al		/g and s bits of frac2 <--
	andb	$0x0c0,%al			/ top 2 bits of frac1
	movb	%al,lsb_frac2
	movb	$2,%cl			/ shift frac1 left 2
	call	left_shift_frac1_cl
	movb	$0x0c0,%al
	movb	1+lsb_result,%ah
	popf				/test q_bit
	jc	q_bit_set
	shl	$16,%eax
	call	add_to_frac2		/frac2.gs <-- frac2.gs +
	jmp	set_q_bit		/result.11
q_bit_set:
	notb	%ah			/frac2.gs <-- frac2.gs +
	shl	$16,%eax			/take care of extra low word
	add	%eax,dword_frac2(%ebp)	/not(result).11
	mov	result_dword_frac+frac64(%ebp),%eax
	not	%eax
	adc	%eax,dword_frac2+frac64(%ebp)
	mov	result_dword_frac+frac32(%ebp),%eax
	not	%eax
	adc	%eax,dword_frac2+frac32(%ebp)
set_q_bit:
	pop	%ecx			/ reload loop count
	loop	sqrt_loop		/ loop until done
	rcrb	$1,%cl			/ set g bit of result to q_bit
	orb	%cl,lsb_result
	movw	result_word_frac,%ax 	/frac2 <- frac2 +
	incb	%ah			/ result + 1
	shl	$16,%eax
	call	add_to_frac2
	mov	$dword_frac2+frac64,%edi /if frac2 = 0 then
	xor	%eax,%eax			/s_bit of result = 0,
	call	test_4w			/otherwise 1.
	orb	1+lsb_frac2,%al
	jz	left_adjust_result
	orb	$0x40,lsb_result
left_adjust_result:
	movb	$8,%cl			/shift result left 8 bits
	jmp	left_shift_result_cl	/ to eliminate carry
add_to_frac2:
	mov	$result_dword_frac,%esi / si points to the addend
	mov	$dword_frac2,%edi	/ di points to the result
	jmp	add_to_frac_2		/ add result frac to frac2
/sqrtx	endp
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			sqrt:
/
/	function:
/		implements the 80387 sqrt instruction
/
/	inputs:
/		assume op1 is set up
/
/	outputs:
/		result
/
/	data accessed:
/		- result_record_offset		result_expon
/		- offset_operand1		tag1
/		- sign1				expon1
/		- msb_frac1			offset_operand1
/		- offset_result			result_sign
/		- result_tag
/
/	data changed:
/		- expon1			result_sign
/		- result_tag			result_expon
/
/	procedures called:
/		set_up_indefinite		sticky_right_shift
/		sqrtx				round
/		addition_normalize		affine_infinity?
/		set_i_masked?			get_precision
/		set_d_masked?			put_si_result
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
sqrt:	/proc
	jz	sqrt_cont		/ if stack error, sqrt done
	call	set_stk_u_error
	testb	invalid_mask, %gs:sr_masks
	jnz	put_op1		/masked stack error, so return indef
sqrt_done:
	ret
sqrt_cont:
	andb	$ -1 ! a_mask, %gs:sr_flags
	movb	tag1(%ebp),%al		/ load tag for op1
	cmpb	valid,%al
	jne	op1_denorm_
	cmpb	positive, sign1(%ebp)
	jne	i_error
	jmp	sqrt_valid_case
op1_denorm_:
	cmpb	denormd,%al		/if op1 denormalized, then give
	jne	op1_zero_
	cmpb	positive, sign1(%ebp)
	jne	i_error
	jmp	d_error
op1_zero_:
	cmpb	special,%al		/if op1 = 0, then give 0 as the
	je	put_op1			/result
	cmpb	unsupp,%al			
	je	i_error
	cmpb	inv,%al			/if op1 inv, then give i_error
	jne	inf_op1
	jmp	kind_of_nan_
inf_op1:
	cmpb	positive, sign1(%ebp)	/infinity case
	je	put_op1			/ -infinity is invalid
i_error:
	call	set_i_masked_		/denormalized, unnormalized,
	jz	sqrt_done		/ negative, or proj +infinity
	mov	$offset_result,%edi
	call	set_up_indefinite	/if masked i_error, then give
	jmp	sqrt_give_result	/indefinite
put_op1:					/if op1 = zero, nan, or +inf,
	jmp	give_op1		/ then give op1 as the result
d_error:
	call	set_d_masked_		/op1 is denormalized
	jz	sqrt_done
	movl	$offset_operand1, %edi
	call 	norm_denorm		/if d_error masked, make valid
sqrt_valid_case:
	sub	$exponent_bias,dword_expon1
	test	$0x0001,dword_expon1
	jz	even_expon
	dec	dword_expon1	/if expon1 odd, then expon1 <--
	jmp	halve_exponent	/ expon1 - 1
even_expon:
	movb	$1,%cl			/if expon1 even, then shift
	xorb	%al,%al			/ frac1 right one bit
	mov	$offset_operand1,%edi
	call	sticky_right_shift
halve_exponent:
	movl	dword_expon1,%eax
	sarl	$1,%eax
	addl	$exponent_bias,%eax
	movl	%eax,dword_result_expon
	call	sqrtx			/ calculate fraction
sqrt_round_result:
	xorl	%eax,%eax			/ not second rounding
	movl	%eax,result_sign(%ebp) / set sign and tag
	movl	$offset_result,%edi	/ round result
	call	get_precision
	call	round
	mov	$offset_result,%edi
	call	addition_normalize	/(possible renormalize)
	cmpb	true, rnd1_inexact
	jne	sqrt_give_result
	orb	$inexact_mask, %gs:sr_errors
	cmpb	true, added_one
	jne	sqrt_give_result
	orb	$a_mask, %gs:sr_flags
sqrt_give_result:
	mov	$offset_result,%edi
	jmp	put_si_result
/sqrt	endp
/
/a_med	ends
/
/	end
