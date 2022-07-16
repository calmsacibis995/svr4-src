/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"divmul.s"

	.ident	"@(#)kern-fp:divmul.s	1.1"

/ ***********************************************************************
/
/			m u l d i v . m o d 
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
/		preforms floating-point divide of unpacked
/		non-zero, valid numbers.
/		performs floating-point multiply of unpacked
/		non-zero, valid numbers.
/
/	public procedures:
/		mulx			mult
/		divx			divid
/
/ ****************************************************************************
/
/...september 16, 1983...
/
/	.file	"a_mdm"
/$nolist
#include	"e80387.h"
/$list
	.text	/a_med	segment	er	public
/
/	extrn	sticky_right_shift,one_left_normalize
/	extrn	clear_5w,test_5w,test_3w,set_5w
/	extrn	get_precision,set_i_error
/	extrn	left_shift_result_cl
/
	.globl	accel_divx
	.globl	divx
	.globl	divid
	.globl	log_divx
	.globl	mulx
	.globl	mult
/
quotient_length:	
	.byte	28,36,57,68	/ incremented and changed to
log_quotient_length:	
	.byte	28,36,60,68	/ a byte table on 12/02/82.
						/ for unknown reasons, divx
						/ doesn't work with 53-bit
						/ precision from log function
low_quotient_byte:	
	.byte	offset_result+6,offset_result+4
	.byte	offset_result+2,offset_result+1
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			accel_divx:
/			""""""""""
/	function:
/		 fractional divide. result_frac <-- frac1/frac2.
/
/	inputs:
/		frac2 is assumed to be normalized and non-zero.
/
/	outputs:
/		the sticky bit is set for result_frac
/		the remainder for ((frac1)/2)mod(frac2) is left in frac1
/
/	data accessed:
/		- dword_frac1		offset_operand2
/		- dword_frac2		result_dword_frac
/		- lsb_result		offset_operand1
/
/	data changed:
/		- dword_frac1		result_dword_frac
/		- lsb_result
/
/	procedures called:
/		sticky_right_shift		get_precision
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
divisor_dwords:	
	.byte	1,2,2,2
quotient_dwords:	
	.byte	1,2,2,3

#define	r_apprx_j	extra_dword_reg(%ebp)
#define	q_apprx_j	extra_dword_reg+4(%ebp)
#define	carry	extra_dword_reg+8(%ebp)

accel_divx:	/proc
clr_quo:
	mov	$offset_result,%edi
	call	clear_6w            		/ clears eax as well.
	mov	$offset_operand1,%edi	/ shift frac1 (here, the dividend)
	movb	$1,%cl					/ right by 1 bit
	call	sticky_right_shift
	call	get_precision
	cmpb	prec64,%dl
	je	prec_to_base
	movb	dword_frac1+3(%ebp),%cl
	movb	%cl,lsb_result
prec_to_base:
	movzbl	%dl,%ebx
	movl	$quotient_dwords,%edi 	/[edi] has offset quotient_dwords' table
	movzbl	%cs:(%ebx,%edi),%ecx	/ q holds the number of dwords
	movb	%cl,q   	    	/ to be produced for the quotient.
	lea	dword_frac1+frac64(%ebp),%esi
	push	%ss:4(%esi)

	testb	$1, is16bit(%ebp)
	jz	prec_32
	movzwl	%sp, %edx
	jmp	prec_32c	
prec_32:
	mov	%esp,%edx         / edx holds the offset relative to ss of the
                                    / interim partial remainder's highest dword.
prec_32c:
	dec	%ecx
	jnz	push_dvdnd
	inc	%ecx
push_dvdnd:
	push	%ss:(%esi)
	sub	$4,%esi
	loop	push_dvdnd
set_up_push_dvsr:
	mov	%ecx,%eax		/ ecx is 0 here.
	movl	$divisor_dwords,%edi
	movb	%cs:(%ebx,%edi),%cl	/ bit_ct holds the number of
	movb	%cl,bit_ct   	    		/ dwords in the divisor.
push_zeroes:
	push	%eax
	loop	push_zeroes
	mov	%edx,%ebx         / ebx holds the offset relative to ss of the
                                    / interim partial remainder's highest word.
	mov	%eax,%edi	/ edi is index to words of quotient.
	movb	q,%cl
main_loop:
	push	%ecx
	mov	%ss:(%ebx),%edx
	cmp	dword_frac2+frac32(%ebp),%edx / is dvsr(1) = prem(j)?
	jne	do_divide
	movl	$0x0ffffffff,%eax	/ eax <-- apprx_q(j) = 2**32 - 1
	movl	%ss:-4(%ebx),%edx	/ edx <-- prem(j+1)
	jmp	get_r_apprx_j
do_divide:
	movl	%ss:-4(%ebx),%eax	/ (edx,eax) <-- prem(j) * 2**32 + prem(j+1)
	div	dword_frac2+frac32(%ebp)
	mov	%eax,q_apprx_j
	mov	%edx,r_apprx_j
	jmp	test_q_apprx_j
dec_q_apprx_j:
	mov	q_apprx_j,%eax
	dec	%eax
	mov	r_apprx_j,%edx
get_r_apprx_j:
	mov	%eax,q_apprx_j
	add	dword_frac2+frac32(%ebp),%edx / r_apprx_j <-- edx + dvsr(1)
	jc	adjst_prem
	mov	%edx,r_apprx_j
test_q_apprx_j:
	mov	q_apprx_j,%eax
	mul	dword_frac2+frac64(%ebp) / (edx,eax) <-- dvsr(2)*q_apprx_j
	cmp	r_apprx_j,%edx
	jb	adjst_prem
	ja	dec_q_apprx_j
	cmp	%ss:-8(%ebx),%eax	/ is eax > prem(j+2)?
	ja	dec_q_apprx_j
adjst_prem:
	xor	%eax,%eax
	mov	%eax,carry
	movb	bit_ct,%al
	mov	%eax,%ecx
	shl	$2,%eax
	neg	%eax
	mov	%eax,%esi
prem_loop:
	mov	sign2(%ebp,%esi),%eax
	mul	q_apprx_j
	sub	%eax,%ss:(%ebx,%esi)
	jnc	sbtrct_carry
	inc	%edx
sbtrct_carry:
	mov	carry,%eax
	sub	%eax,%ss:(%ebx,%esi)
	jnc	next_carry
	inc	%edx
next_carry:
	mov	%edx,carry
	add	$4,%esi
	loop	prem_loop
/		
	sub	%edx,%ss:(%ebx,%esi)   / here, esi = 0.
	jnc	next_j
	dec	q_apprx_j
	movb	bit_ct,%cl
	mov	%ecx,%eax
	shl	$2,%eax
	neg	%eax
	mov	%eax,%esi
	clc
fix_prem_loop:
	mov	sign2(%ebp,%esi),%eax
	adc	%eax,%ss:(%ebx,%esi)
	add	$4,%esi
	loop	fix_prem_loop
/
	adc	%ecx,%ss:(%ebx,%esi)
next_j:
	mov	q_apprx_j,%eax
	mov	%eax,result_dword_frac+frac32(%ebp,%edi)
	pop	%ecx
	dec	%ecx
	jz	get_sticky_bit
	sub	$4,%ebx
	sub	$4,%edi
	jmp	main_loop
/
get_sticky_bit:
	mov	%ecx,%eax
	mov	%eax,%esi
	movb	bit_ct,%cl
	shlb	$1,%cl
sticky_loop:
/        or	ax, 	word ptr ss:[ebx + esi - 2] bug in asm386 hardcoded
	.byte	0x066,0x36,0x00b,0x44,0x33,0x0fe

	sub	$2,%esi
	loop	sticky_loop
	orb	%al,%ah
	orb	%ah,lsb_result
	mov	%ecx,%esi
	movb	bit_ct,%cl
/	
stor_rmndr:
/        mov 	eax, 	dword ptr ss:[ebx + esi - 4] bug in asm386 hard coded
	.byte	0x36,0x8b,0x44,0x33,0x0fc

	mov	%eax,dword_frac1+frac32(%ebp,%esi)
	sub	$4,%esi
	loop	stor_rmndr
/
	movb	q,%cl
	addb	bit_ct,%cl
	cmpb	$2,%cl
	jne	get_stack_dwords
	inc	%ecx
get_stack_dwords:
	shl	$2,%ecx
	add	%ecx,%esp                  /   restore stack
/
	ret
/accel_divx	endp
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			divx:
/			""""
/	function:
/		 fractional divide. result_frac <-- frac1/frac2.
/
/	inputs:
/		frac2 is assumed to be normalized and non-zero.
/
/	outputs:
/		the sticky bit is set for result_frac
/		the remainder is left in frac1
/
/	data accessed:
/		- word_frac1		offset_operand2
/		- word_frac2		result_word_frac
/		- lsb_result
/
/	data changed:
/		- word_frac1		result_word_frac
/		- lsb_result
/
/	procedures called:
/		sticky_right_shift		get_precision
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
divx:	/proc
	push	$quotient_length		/ set normal entry ptr
	jmp	clear_quotient
log_divx:
	push	$log_quotient_length	/ set log entry ptr
clear_quotient:
	mov	$offset_result,%edi
	call	clear_6w
	mov	$offset_operand2,%edi		/shift frac2 (divisor)
	movb	$1,%cl				/right by 1 bit
	call	sticky_right_shift
	call	get_precision
	movzbl	%dl, %ebx
	pop	%edi		/retrieve offset quotient length table
	movb	%cs:(%ebx,%edi),%al			/ bit_ct holds
	movb	%al,bit_ct			/ the quotient bits
	movzbl	%cs:low_quotient_byte(%ebx),%eax	/[edi] points to low-
					/order byte of quotient
	mov	%eax,%edi
	movl	dword_frac1(%ebp),%eax		/load dividend (frac1)
	movl	dword_frac1+frac64(%ebp),%ebx  / in frac1
	movl	dword_frac1+frac32(%ebp),%edx
	.long	0x02d8ac0f	/shrd   eax, ebx, 2
	.long	0x02d3ac0f  	/shrd   ebx, edx, 2
	shrl	$2,%edx				/shift dvdnd rt 2 bits
	jmp	subtract_divisor
frac_divide_loop:
	shll	$1,%eax				/shift dvdnd (partial
	rcll	$1,%ebx				/remainder) lft one bit
	rcll	$1,%edx
	jc	quotient_bit_0			/jump if cy from shift
	orb	$0x20,frac80(%ebp,%edi)		/"shift in" a 1-bit
subtract_divisor:
	subl	dword_frac2(%ebp),%eax		/subtract divisor from
	sbbl	dword_frac2+frac64(%ebp),%ebx		/partial remainder
	sbbl	dword_frac2+frac32(%ebp),%edx
	jmp	shift_quotient_left
quotient_bit_0:
	call	add_divisor			/ add divisor to prem
shift_quotient_left:
	call	shift_result_left
	decb	bit_ct				/ decrement bit count
	jnz	frac_divide_loop		/  next quotient bit
	andl	%edx,%edx			/ branch if remainder
	js	adjust_remainder		/  is negative
	orb	$0x20,frac80(%ebp,%edi)		/"shift in" last 1-bit
	jmp	store_remainder
adjust_remainder:
	call	add_divisor			/add divisor to prem
store_remainder:
	movl	%eax,dword_frac1(%ebp)		/store partial rmndr
	movl	%ebx,dword_frac1+frac64(%ebp)  / in frac1
	movl	%edx,dword_frac1+frac32(%ebp)
	orl	%ebx,%eax			/set sticky bits if
	orl	%edx,%eax			/partial rmndr non-zero
	.long	0x10c2a40f		/ shld edx, eax, 16
	orw	%dx, %ax
	orb	%al,%ah
	movb	%ah,lsb_result
	ret
add_divisor:
	addl	dword_frac2(%ebp),%eax		/store partial rmndr
	adcl	dword_frac2+frac64(%ebp),%ebx  / in frac1
	adcl	dword_frac2+frac32(%ebp),%edx
	ret
shift_result_left:
	shl	$1,result_dword_frac(%ebp)
	rcl	$1,result_dword_frac+frac64(%ebp)
	rcl	$1,result_dword_frac+frac32(%ebp)
	ret
/divx	endp
/$eject
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			divid:
/			""""""
/	function:
/		floating-point divide.
/
/	inputs:
/		assumes operands are unpacked, non-zero, and valid
/
/	outputs:
/		calculates unpacked result and returns with al
/		set to true if underflow is possible, false if
/		overflow is possible.  the quotient is left in
/		the result, and the remainder is left in frac1.
/
/	data accessed:
/		- expon1			expon2
/		- msb_frac2			offset_result
/		- result_sign			result_expon
/		- result_word_frac
/
/	data changed:
/		- result_sign			result_expon
/		- result_word_frac
/
/	procedures called:
/		divx				one_left_normalize
/		set_i_error			get_precision
/		left_shift_result_cl
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
divid:	/proc
	movl	dword_expon1,%eax		/ stack underflow possible
	subl	dword_expon2,%eax		/ flag (sign bit)
	addl	$exponent_bias,%eax	/form biased exponent
	movl	%eax,dword_result_expon
	movb	sign1(%ebp),%al		/ sign = '+' if sign1 = sign2
	xorb	sign2(%ebp),%al
	movb	%al,result_sign(%ebp)
fractional_divide:
	call	accel_divx
/		The following five lines are not to be used when accel_divx
/	does the division instead of divx.
/	call	get_precision
/	cmpb	prec53,%dl		/if double precision, shift
/	jne	norm_quotient		/ quotient left 3 bits
/	movb	$3,%cl
/	call	left_shift_result_cl
norm_quotient:
	mov	$offset_result,%edi	/normalize by 1 left
	jmp	one_left_normalize	/ shift, if unnormalized
/divid	endp
/$eject
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			mult:
/			""""
/	function:
/		floating point multiply.
/
/	inputs:
/		assumes operands are unpacked, valid, non-zero.
/
/	outputs:
/		calculates unpacked result and returns with al set
/		to true if underflow is possible, false if overflow
/		is possible.
/
/	data accessed:
/		- sign1			expon1
/		- sign2			expon2
/		- offset_result		result_sign
/		- result_expon
/
/	data changed:
/		- result
/
/	procedures called:
/		mulx				norm_quotient
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
mult:	/proc
	movl	dword_expon1,%eax		/form doubly-biased exponent
	addl	dword_expon2,%eax		/if high bit set, underflow
	subl	$exponent_bias-1,%eax	/form singly-biased exponent
	movl	%eax,dword_result_expon
	movb	sign1(%ebp),%al		/ result sign = sign1 xor sign2
	xorb	sign2(%ebp),%al
	movb	%al,result_sign(%ebp)
	call	mulx
	movl	$offset_result, %edi	/normalize by 1 left
	jmp	one_left_normalize	/ shift if unnormalized 
/mult	endp
/$eject
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			mulx:
/			"""""
/	function:
/		fractional multiply. result_frac <-- frac1 * frac2.
/
/	inputs:
/		assumes the operands are unpacked, valid, non-zero.
/
/	outputs:
/		product in result_frac (sticky indicator left in low bit)
/
/	data accessed:
/		- word_frac1			offset_operand1
/		- word_frac2			offset_operand2
/		- extra_word_reg		lsb_result
/		- offset_result
/
/	data changed:
/		- extra_word_reg		offset_result
/
/	procedures called:
/		clear_5w			set_5w
/		test_3w				test_5w
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
mulx:	/proc
	push	%ds			/ save a?msr
	movl	$extra_dword_reg,%edi / ss:bp+di => extra_word_reg
	push	%ss
	pop	%ds
	lea	(%ebp,%edi),%ebx		/ ds:bx also => extra_word_reg
	call	clear_6w		/ clear extra_word_reg
	movl	$offset_result,%edi	/ clear result_frac
	call	set_6w
	movl	$frac32,%ecx		/ load s.p. offset
	movl	$offset_operand2,%edi	/di => to multiplier
	call	test_4w			/if low 3 words <> zero,
	jnz	examine_frac1		/ branch if non single
	add	%ecx,%edi			/frac2 is s. p.,
	add	%ecx,%ebx			/so adjust pointers
examine_frac1:
	movl	$offset_operand1,%esi	/si points to multiplicand
	movl	frac64(%ebp,%esi),%eax	/if low 3 words = zero,
	orl	(%ebp,%esi),%eax	/then single precision
	jnz	do_frac_multiply
	add	%ecx,%esi			/frac1 is s. p.,
	add	%ecx,%ebx			/so adjust pointers
do_frac_multiply:
	push	%edi			/ save frac2 offset
	movl	(%ebp,%edi),%edi		/ load multiplier
	xorl	%ecx,%ecx			/clear cx
	movl	(%ebp,%esi),%eax		/ multiply first word
	mull	%edi
	addl	%eax,(%ebx)			/add to partial product
	adcl	%edx,%ecx			/ cx initially 0
	cmpl	$offset_operand1, %esi
	je	mult_scnd
	movl	%ecx, 4(%ebx)		/multiplicand is s. p. so
	jmp	end_of_mul_loop		/go to multipliers next word
mult_scnd:
	movl	4(%ebp,%esi),%eax	/ multiply second word
	mull	%edi
	addl	%ecx,%eax
	adcl	$0,%edx
	xorl	%ecx,%ecx
	addl	%eax,4(%ebx)		/add to partial product
	adcl	%edx,%ecx
	movl	8(%ebp,%esi), %eax
	mull	%edi
	addl	%ecx, %eax
	adcl	$0, %edx
	addl	%eax, 8(%ebx)
	adcl	$0, %edx
	movl	%edx, 12(%ebx)
end_of_mul_loop:
	pop	%edi			/ reload frac2 offset
	addl	$4, %ebx		/adjust pointers for next iteration
	addl	$4, %edi
	cmp	$[offset_operand2+12],%edi
	jne	do_frac_multiply
	movl	$extra_dword_reg,%edi /set sticky bit if any extra
	call	test_6w			/ reg words are nonzero
	jz	frac_mult_done
	orb	$0x01,lsb_result
frac_mult_done:
	pop	%ds			/ restore a?msr
	ret
/mulx	endp
/
/a_med	ends
/
/	end
