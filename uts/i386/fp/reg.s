/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"reg.s"

	.ident	"@(#)kern-fp:reg.s	1.1"

/ ****************************************************************************
/
/			r e g . m o d
/			=============
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
/		operation cluster for the 80387 register stack.
/
/	public procedures:
/		put_result	stack_full?	float_int16
/		save_regs	restore_regs	set_up_indefinite
/		getx		fetch_an_op	exchange	exam
/		fxtrac		abs_value	chsign		put_op1_result
/		put_si_result	do_exchange
/
/	internal procedures:
/		extend_single	extend_double	get_bcd		fetch_an_op
/		float16		float32		float64
/
/ *****************************************************************************
/
/...March 3, 1987...
/
/$nolist
#include	"e80387.h"
/$list
/...declare status register segment...
/
	.data	/a_msr	segment	rw	public
/	extrn	%gs:sr_masks,%gs:sr_errors,%gs:sr_flags,%gs:sr_regstack
/a_msr	ends
/
/	assume	%ds:a_msr
/
	.text	/a_med	segment	er	public
/
/	extrn	store_reg_tag,decr_top,left_shift
/	extrn	subtraction_normalize,clear_6w,test_3w
/	extrn	move_op_to_result,save_status,set_z_bit
/	extrn	restore_status,init,test_4w,test_6w
/	extrn	left_shift_frac2_cl,left_shift_frac1_1
/	extrn	left_shift_result_cl,get_top,set_i_error
/	extrn	get_reg_tag,set_s_bit,set_d_error
/	extrn	i_masked_,clear_c_bit,clear_cond_bits
/	extrn	set_c_bit,set_a_bit,set_i_masked_
/	extrn	move_op_to_op,set_stk_u_error,set_stk_o_error
/	extrn	norm_denorm,correct_tag_word
/	--------to be added for unix
/..		extrn	fpfulong:far,fpfushort:far,fpsulong:far,fpsushort:far
/
	.globl	getx
	.globl	fetch_an_op
	.globl	put_result
	.globl	stack_full_
	.globl	fxtrac
	.globl	float_int16
	.globl	save
	.globl	save_regs
	.globl	restore_regs
	.globl	restore
	.globl	set_up_indefinite
	.globl	exchange
	.globl	exam
	.globl	decompose
	.globl	load
	.globl	abs_value
	.globl	chsign
	.globl	put_op1_result
	.globl	do_exchange
	.globl	put_si_result
	.globl	do_exchange_leave_a_bit
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			extend_single:
/			""""""""""""""
/	function:
/		retrieves a single real operand from memory,
/		extends its parts, and puts them in the operand
/
/	inputs:
/		assumes mem_operand_pointer set up
/		edi points to operand1 or operand2
/
/	outputs:
/		extended operand in operand
/
/	data accessed:
/		- mem_operand_pointer
/
/	data changed:
/		- operands
/
/	procedures called:
/		set_d_error
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
extend_single:	/proc
	les	mem_operand_pointer(%ebp),%ebx
	xor	%eax,%eax
	mov	%eax,frac64(%ebp,%edi) 		/clear low 5 bytes of
	movb	%al,frac32(%ebp,%edi)	/fraction to 0
/	---- replace following line for unix
	mov	%es:(%ebx),%eax				/ move 3 fraction

/..		
/..		pushad
/..		push	es
/..		push	ebx
/..		call	fpfulong
/..		add	esp,8
/..		popad
/..		cld
/..
	mov	%eax,frac32+1(%ebp,%edi)	/ bytes from memory
	sar	$23,%eax		/ get short real exponent in al
	movb	%ah,sign(%ebp,%edi) 	/ transfer sign to operand
	and	$0x000000ff,%eax	/ strip sign bits (here, b8-b31)
	mov	$0,before_error_signals(%ebp)	/ initialize all to false
	jnz	single_invalid_
	movb	frac64+7(%ebp,%edi),%al	/ zero or denormalized?
	orw	frac64+5(%ebp,%edi),%ax
	jnz	single_denormalized
zero_return:
	movb	special,tag(%ebp,%edi)	/ set tag to special,
	mov	%eax,expon(%ebp,%edi)			/ expon to 0
	ret
single_denormalized:
	movb	denormd,tag(%ebp,%edi)	/ set tag to denormd
	movb	true,signal_d_error_
	testb	denorm_mask,%gs:sr_masks		/ short cut for unmasked d-error
	jnz	sngl_d_error_masked
	ret
sngl_d_error_masked:
	mov	single_exp_offset+1,expon(%ebp,%edi)
	bsrl	frac32(%ebp,%edi),%ecx
	sub	$31,%ecx
	neg	%ecx
	shll	%cl,frac32(%ebp,%edi)
	sub	%ecx,expon(%ebp,%edi)	/ adjust exponent
	ret
single_invalid_:
	cmpb	true,%al
	jne	single_operand_is_valid
	mov	$0x7fff,expon(%ebp,%edi)	/operand is inv/inf
	test	$0x7fffffff,frac32(%ebp,%edi)
	movb	infinty,tag(%ebp,%edi)
	jz	sngl_exit
	movb	inv,tag(%ebp,%edi)		/invalid operand
	testb	$0x40,msb(%ebp,%edi)
	jnz	sngl_qnan
	movb	true,signal_i_error_
sngl_qnan:
sngl_exit:
	ret													/set tag to invalid
single_operand_is_valid:
	movb	valid,tag(%ebp,%edi)		/set tag to valid
	add	single_exp_offset,%eax		/single_exp_offset,
	mov	%eax,expon(%ebp,%edi)		/implicit bit to 1
	orb	$0x80,frac64+7(%ebp,%edi)
	ret
/extend_single	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			extend_double:
/			""""""""""""""
/	function:
/		retrieves a double real operand from memory,
/		extends its parts, and puts them in the operand
/
/	inputs:
/		assumes mem_operand_pointer is set up
/		edi points to operand1 or operand2
/
/	outputs:
/		expanded operand in operand
/
/	data accessed:
/		- mem_operand_pointer
/
/	data changed:
/		- operand
/
/	procedures called:
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
extend_double:	/proc
	push	%ds											/ save a?msr
	lds	mem_operand_pointer(%ebp),%ebx
	movb	$0,frac64(%ebp,%edi)			/clear low byte of
	mov	(%ebx),%eax		/fraction to 0

/..
/..		pushad
/..		push	ds
/..		push	ebx
/..		call	fpfulong
/..		pop		ebx
/..
	mov	%eax,frac64+1(%ebp,%edi)		/move 7 fraction bytes
	mov	4(%ebx),%eax		/ from memory
/..
/..		push	ebx+4
/..		call	fpfulong
/..		add		esp,	8
/..		popad
/..		cld

	pop	%ds									/ reload a?msr
	mov	%eax,frac32+1(%ebp,%edi)
	sar	$20,%eax
	cwtd
	movb	%dl,sign(%ebp,%edi) 	/ transfer sign to operand
	and	$0x000007ff,%eax	/ strip sign bits (here,
					/ b11 - b31)
	mov	$0,before_error_signals(%ebp)	/ initialize both to false
	jnz	double_invalid_
	mov	frac64(%ebp,%edi),%eax		/ exp=0 : is fraction 0?
	or	frac32(%ebp,%edi),%eax
	jnz	double_denormalized
	jmp	zero_return
double_denormalized:
	movb	denormd,tag(%ebp,%edi)	/ set tag to denormd
	movb	true,signal_d_error_
	testb	denorm_mask,%gs:sr_masks		/ short cut for unmasked d-error
	jnz	dubl_d_error_masked
	ret
dubl_d_error_masked:
	mov	double_exp_offset+1+3,expon(%ebp,%edi)
	bsrl	frac32(%ebp,%edi),%ecx
	jz	top_28_zero
	mov	frac64(%ebp,%edi),%eax
	sub	$31,%ecx
	neg	%ecx
	shldl	%eax,frac32(%ebp,%edi)
	shll	%cl,frac64(%ebp,%edi)
dubl_denorm_expon:
	sub	%ecx,expon(%ebp,%edi)	/ adjust exponent
	ret
top_28_zero:
	mov	frac64(%ebp,%edi),%eax
	bsrl	%eax,%ecx
	sub	$31,%ecx
	neg	%ecx
	shll	%cl,%eax
	mov	%eax,frac32(%ebp,%edi)
	mov	$0,frac64(%ebp,%edi)
	add	$32,%ecx
	jmp	dubl_denorm_expon
double_invalid_:
	cmpw	$0x07ff,%ax
	jne	double_operand_is_valid
	movl	$0x7fff,expon(%ebp,%edi)	  / operand is nan or
	and	$0x0fffffff,frac32(%ebp,%edi) / inf. set exponent.
	mov	frac32(%ebp,%edi),%eax	  / mask expon bits
	or	frac64(%ebp,%edi),%eax
	movb	infinty,tag(%ebp,%edi)
	jz	dubl_msb_set
	movb	inv,tag(%ebp,%edi)		/ invalid operand
	testb	$0x08,msb(%ebp,%edi)
	jnz	dubl_qnan
	movb	true,signal_i_error_
	testb	invalid_mask,%gs:sr_masks
	jz	dubl_exit
dubl_qnan:
	jmp	norm_dubl_sgnfcnd
double_operand_is_valid:
	add	double_exp_offset,%eax		/ double_exp_offset
	mov	%eax,expon(%ebp,%edi)
	movb	valid,tag(%ebp,%edi)		/ set operand tag
norm_dubl_sgnfcnd:
	mov	frac64(%ebp,%edi),%eax
	.byte	0x0f,0xa4,0x44,0x3d,0x08,0x03  /implements following inst.
/	shldl	$3,%eax,frac32(%ebp,%edi)
	shl	$3,frac64(%ebp,%edi)
dubl_msb_set:
	orb	$0x80,frac64+7(%ebp,%edi)	/set leading bit
dubl_exit:
	ret
/extend_double	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			getx:
/			"""""
/	function:
/		fetches extended-format fp number from memory, unpacks it,
/		and sets its tag
/
/	inputs:
/		assumes mem_operand_pointer is set up
/		edi points to operand1 or operand2
/
/	outputs:
/		if number is valid, then exponent and msb set to all ones
/
/	data accessed:
/		- mem_operand_pointer		extra_word_reg
/
/	data changed:
/		- operand			extra_word_reg
/
/	procedures called:
/		test_4w				test_3w
/		set_d_error
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
getx:	/proc
	push	%ds									/ save a?msr
	lds	mem_operand_pointer(%ebp),%esi	/ ds:esi ==> memop
/
getx_stackreg:
	push	%ss
	pop	%es			/ es:edi ==> operand

	add	$frac64,%edi		/ bump di to frac64
	push	%edi			/ save operand offset
	lea	(%ebp,%edi),%edi
	mov	$0x0002,%ecx		/ move first four words
	rep	
	movsl					/ to frac64
	movw	(%esi),%ax			/ get sign/expon word
/..		pushad
/..		push	ds
/..rep_loop:	
/..		push	esi
/..		call	fpfulong
/..		pop	esi
/..		stosd
/..		add	esi,4
/..		loop	rep_loop
/..		push	esi
/..		call	fpfushort
/..		add	esp,8
/..		popad
/..		cld
/..
	pop	%edi		/ reload operand offset
	pop	%ds		/ reload a?msr
	cwtd			/ transmit sign to dx
	movb	%dl,sign-frac64(%ebp,%edi)/ and to operand
	and	$0x7fff,%eax		/ strip off sign bit
	mov	%eax,expon-frac64(%ebp,%edi)		/ store to operand
	mov	$0,before_error_signals(%ebp)	/ initialize both to false
	jz	expon_zero			/ branch if exponent 0
	testb	$0x80,msb-frac64(%ebp,%edi)
	jnz	max_expon_
	movb	unsupp,%dl				/ we have an
	jmp	getx_set_tag			/ unsupported format
max_expon_:
	cmpw	$0x7fff,%ax			/ check for invalid
	je	not_validx			/ or infinity
	movb	valid,%dl		 	/ operand is valid
getx_set_tag:
	movb	%dl,tag-frac64(%ebp,%edi) / set operand tag
	xor	%eax,%eax			/ (for stackreg return)
	ret
not_validx:
	movb	infinty,%dl
	mov	frac32-frac64(%ebp,%edi),%eax
	and	$0x07fffffff,%eax
	or	(%ebp,%edi),%eax
	jz	getx_set_tag
	movb	inv,%dl				/ set tag to invalid
	jmp	getx_set_tag
expon_zero:
	mov	frac32-frac64(%ebp,%edi),%eax
	or	(%ebp,%edi),%eax
	movb	special,%dl			/ set tag to special
	jz	getx_set_tag			/ if number is +/- 0
	movb	denormd,%dl			/ set tag to denormd
	jmp	getx_set_tag
/getx	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			fetch_an_op:
/			"""""""""""
/	function:
/		fetches one operand(op1 or op2) and puts its parts
/ 		in operand1 or operand2
/
/	inputs:
/		location of 	op info in esi
/		location of the operand in edi
/
/	outputs:
/		operand parts in operand1 or operand2
/
/	procedures called:
/		mem_fetch_routine		set_up_indefinite
/		get_top
/		getx_stackreg
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
fetch_location:	
	.long	from_memory,from_stack_top,from_stack_minus_1
	.long	from_stack_plus_1,from_reg
mem_fetch_routine:	
	.long	extend_single,extend_double,float16,float32
	.long	float64,getx,float16,float16,get_bcd
fetch_an_op:	/proc
	cmpb	$null,format(%ebp,%esi)/ if format = null,
	je	no_stack_error_on_fetch		/ then no operand to get
	xor	%ebx,%ebx
	mov	%ebx,(%ebp,%edi)		/ set low order dword of
	movb	location(%ebp,%esi),%bl	/ fraction to zero
	shl	$2,%ebx				/ handle each operand
	jmp	*%cs:fetch_location(%ebx)		/ location case separate
from_memory:
	movb	format(%ebp,%esi),%bl	/ form table offset
	shl	$2,%ebx
	call	*%cs:mem_fetch_routine(%ebx)	/ call routine to fetch
no_stack_error_on_fetch:
	xor	%eax,%eax			/ eax = 00000000h
	ret
from_stack_top:
	call	get_top			/ get reg num in al
unpack_reg:
	call	reg_full_		/ zf=1 (empty) cx=top
	jz	from_stack_plus_1	/ error if reg empty
	mov	%ebx,%esi		/ reg_full returns ebx as pointer
					/ to stackoffset =
					/  (10*regnum) + offset regstack
					/ thus, ds:esi ==> regstack
/
	push	%ds			/ stack a copy of a?msr
	push	%gs			/ set ds to address register stack
	pop	%ds
	jmp	getx_stackreg		/ unpack register
from_stack_minus_1:
	call	get_top
	incw	%ax
	jmp	unpack_reg
from_reg:
	call	get_top
	addb	reg_num(%ebp),%al
	jmp	unpack_reg
from_stack_plus_1:
	call	set_up_indefinite		/ store indefinite opr
	incw	%ax				/ indicate stack error on return
	ret

/fetch_an_op	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			get_bcd:
/			""""""""
/	function:
/		fetches a bcd number and converts it to an unpacked 
/		tempreal in operand1.
/	inputs:
/		assumes mem_operand_pointer is set up.
/		ignores di and uses operand2 and result fields.
/
/	outputs:
/		unpacked operand in operand1
/
/	data accessed:
/		- mem_operand_pointer		expon1
/		- offset_operand1		sign1
/		- tag1				word_frac1
/		- offset_operand1			word_frac2
/		- lsb__frac2			offset_operand2
/		- result_word_frac		offset_result
/
/	data changed:
/		- sign1				tag1
/		- expon1			word_frac1
/		- word_frac2			result_word_frac
/
/	procedures called:
/		left_shift			move_op_to_result
/		subtraction_normalize		test_6w
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
get_bcd:	/proc
	push	%ds			/ save a?msr
	lds	mem_operand_pointer(%ebp),%esi
	push	%ss			/ set up destination pointer
	pop	%es
	lea	word_frac2,%edi		/ es:di ==> word_frac2
	mov	$0x0005,%ecx
	rep	
	movsw				/ move bcd fraction to frac2
/..
/..		pushad
/..		push	ds
/..getbcd_loop:
/..		push	esi
/..		call	fpfushort
/..		pop	esi
/..		stosw
/..		add	esi,2
/..		loop	getbcd_loop
/..		pop	ds
/..		popad
/..		cld
/..
	pop	%ds			/ reload a?msr
	cmpw	$0x0ffff,dword_frac1+10(%ebp)
	jne	accept_bcd
	mov	$offset_operand1,%edi
	call	set_up_indefinite
	ret
accept_bcd:
	movb	msb_frac2,%al	/ get most significant byte
	cbtw				/  and transmit sign to ah
	movb	%ah,sign1(%ebp)		/ store sign into operand
	mov	$offset_operand1,%edi	/ set frac1 to 0
	call	clear_6w
	movb	$18,%cl			/set loop count to 18 digits
convert_loop:
	push	%ecx
	movb	$4,%cl			/shift frac2 left 4 bits to
	call	left_shift_frac2_cl	/ the next digit
	and	$0x0f,msb_frac2	/clear previous digit
	call	left_shift_frac1_1	/ frac1 <= 10*frac1 + frac2(9):
	mov	$offset_operand1,%esi	/ 1) frac1 <= frac1 * 2
	call	move_op_to_result	/ 2) result_frac <= frac1 * 4
	movb	$2,%cl
	call	left_shift_result_cl
/		mov	ax,[ebp].result_word_frac ; 3) frac1 <-- frac1 +
/		add	word_frac1,ax		;	result_frac
/		mov	ax,[ebp].result_word_frac + 2
/		adc	[ebp].word_frac1 + 2,ax
/		mov	ax,[ebp].result_word_frac + 4
/		adc	[ebp].word_frac1 + 4,ax
/		mov	ax,[ebp].result_word_frac + 6
/		adc	[ebp].word_frac1 + 6,ax
/		mov	ax,[ebp].result_word_frac + 8
/		adc	[ebp].word_frac1 + 8,ax
/
	mov	result_dword_frac(%ebp),%eax / 3) frac1 <-- frac1 +
	xorw	%ax,%ax
	add	%eax,dword_frac1(%ebp)		/	result_frac
	mov	result_dword_frac+4(%ebp),%eax
	adc	%eax,dword_frac1+4(%ebp)
	mov	result_dword_frac+8(%ebp),%eax
	adc	%eax,dword_frac1+8(%ebp)
	xor	%eax,%eax			/ 4) frac1 <-- frac1 + frac(9)
	movb	msb_frac2,%al
	addw	%ax,word_frac1
	movb	$0,%al
	adc	%eax,dword_frac1+4(%ebp)
	adc	%eax,dword_frac1+8(%ebp)
	pop	%ecx			/and loop
	loop	convert_loop
	mov	$dword_frac1,%edi
	call	test_6w			/frac1 = 0?
	jnz	make_floating_point
	mov	%eax,dword_expon1 		/set expon and tag for zero
	movb	special,tag1(%ebp)
	ret
make_floating_point:
	mov	$0x404e,dword_expon1	/set expon1 and normalize
	mov	$offset_operand1,%edi
	call	subtraction_normalize
	movb	valid,tag1(%ebp)		/set tag to valid
	ret
/get_bcd	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			put_result:
/
/	function:
/		puts result on 80387 stack or in 80387 register.
/		implements 80387 load instruction.
/
/	inputs:
/		address of data parts  (in edi register)
/		address of result info (in esi).
/		[ebp].before_error_signals
/	outputs:
/		stack error indication (in al register).
/
/	data accessed:
/
/	data changed:
/
/	procedures called:
/		decr_top			stack_full?
/		set_up_indefinite	store_reg_tag
/		get_top
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
location_case:	
	.long	push_onto_stack
	.long	put_in_stack_top
	.long	put_in_stack_top_minus_1
	.long	push_onto_stack
	.long	put_in_reg
/
load:	/proc
	jz	put_zf_in_ah			/ if zf set, no stack error
	call	set_stk_u_error
	testb	invalid_mask,%gs:sr_masks	/ else, stack underflow error
	jnz	put_zf_in_ah			/ if i_error masked, continue
	ret					/ else, return
put_zf_in_ah:				/ if zf is set here, no stack
	lahf				/ underflow occurred.  however,
					/ if zf is clear, stack underflow
					/ occurred and invalid is masked.
	jnz	put_op1_result
common_clear_a_bit:
	andb	$ -1 ! a_mask,%gs:sr_flags
put_op1_result:
	mov	$offset_operand1,%edi		/ edi points to result record
put_si_result:
	mov	offset_result_rec,%esi	/ merge with put_result
put_result:
	movzbl	location(%ebp,%esi),%ebx	/handle cases separately
	shl	$2,%ebx
	jmp	*%cs:location_case(%ebx)
put_in_stack_top_minus_1:
	call	get_top				/ put reg num of top-1 in al
	incw	%ax
	jmp	mask_top
push_onto_stack:
	sahf			/ zf clear iff stack underflow occurred with
	jnz	push_value	/   invalid exception masked 
	call	stack_full_			/ is top - 1 full?
	jz	other_before_error_	/ no, so detect other before errors.
	call	set_stk_o_error		/ set invalid error, stack bit, and
					/ c1 (a) bit
	testb	invalid_mask,%gs:sr_masks
	jz	exit_load			/ unmasked, so done with load
	call	set_up_indefinite	/ push indefinite as the result
	jmp	push_value
other_before_error_:
	mov	before_error_signals(%ebp),%eax
	or	%eax,%eax
	jz	push_value
	orb	%al,%al
	jz	check_z_error
	orb	invalid_mask,%gs:sr_errors
	testb	invalid_mask,%gs:sr_masks
	jz	exit_load			/ unmasked, so done with load
	orb	$0x40,msb(%ebp,%edi)
	jmp	push_value
check_z_error:
	orb	%ah,%ah
	jz	signal_d_error
	orb	zero_divide_mask,%gs:sr_errors
	testb	zero_divide_mask,%gs:sr_masks
	jz	exit_load			/ unmasked, so done with load
	jmp	push_value
signal_d_error:
	orb	denorm_mask,%gs:sr_errors
	testb	denorm_mask,%gs:sr_masks
	jz	exit_load			/ unmasked, so done with load
/	the following seven instructions were added to cope with a load from
/	memory to (new) '87 stack top of a short_real or long_real operand.
/	otherwise, such loads of a denormal source operand while the denormal
/	exception is masked would incorrectly tag the loaded, normalized
/	temporary_real operand as invalid, instead of valid.  the first four
/	of these seven instructions have been commented out, because the
/	executive procedure e80387 in the module "dcode" always initializes
/	the op1_format field, even when its contents are deducible from the
/	operation_type and op1_location fields, so it is enough to check
/	whether the format field is single_real or double_real.
/		cmp		[ebp].operation_type,	load_op
/		jne		push value
/		cmp		[ebp].op1_location,	memory_opnd
/		jne		push_value
	cmpb	$double_real,op1_format(%ebp)
	ja	push_value
	movb	valid,tag(%ebp,%edi)
push_value:
	call	decr_top			/ decrement top for push
put_in_stack_top:
	call	get_top				/ put reg num of stack top in al
	jmp	mask_top
put_in_reg:
	call	get_top				/ put reg num specified in
	addb	reg_num(%ebp),%al	/ instruction in al
mask_top:
	and	$0x0007,%eax
	mov	%eax,%ecx		/ save reg num word in cx
	movb	tag(%ebp,%edi),%al	/ put new tag in al
	call	store_reg_tag
	lea	frac64(%ebp,%edi),%esi/ ds:esi ==> operand frac64
	mov	$sr_regstack,%edi	/ form offset within %gs:sr_regstack
	add	%ecx,%edi		/ for result (10 * reg num)
	shl	$2,%ecx
	add	%ecx,%edi		/ es:edi ==>  treal st(reg num)
	mov	$0x02,%ecx			/ count = 2 dwords
	push	%gs				/ set segment registers
	pop	%es
	push	%ds
	push	%ss
	pop	%ds
	rep	
	movsl				/ fraction => %gs:sr_regstack
	movb	(%esi),%ah			/ ah = sign of operand
	andw	$0x8000,%ax		/ form tempreal exponent
	orw	4(%esi),%ax
	pop	%ds			/ reload a?msr into ds
	movw	%ax,%gs:(%edi)		/ exponent => %gs:sr_regstack
exit_load:
	ret
/load	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			stack_full?:
/
/	function:
/		checks stack to see if an attempted push will
/		generate a stack error.
/
/	inputs:
/		tag for register on top of stack
/
/	outputs:
/		zf = true if stack is empty; zf = false if stack is full.
/
/	procedures called:
/		get_top			get_reg_tag
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
stack_full_:	/proc
	call	get_top			/ determine reg no. of (top-1)
	decw	%ax
reg_full_:
	andb	$0x07,%al
	call	get_reg_tag
	cmpb	$empty,%al	/ if (top-1) is not empty,
return:
	ret					/ then stack is full
/stack_full_	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			decompose:
/
/	function:
/		decomposes op1 into two parts
/
/	inputs:
/		assumes op1 is set up, with tag1 /= unsupp and tag1 /= inv
/
/	outputs:
/		puts the exponent, floated in op1; the fraction in op2
/
/	data accessed:
/		- offset_operand		sign1
/		- tag1				expon1
/		- offset_operand2		sign2
/		- tag2				expon2
/
/	data changed:
/		- sign2				expon2
/
/	procedures called:
/		float_int32			move_op_to_op
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
decompose:	/proc
	lea	dword_frac1(%ebp),%esi	/ move op1 to op2
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op
	cmpb	valid,tag1(%ebp)
	jne	is_op1_denorm_
put_sig_in_op2:
	mov	$exponent_bias,dword_expon2	/ set op2 exponent=bias
put_exp_in_op1:
	mov	dword_expon1,%eax	/ float exp into op1
unbias_expon:
	sub	$exponent_bias,%eax	/ first unbias it
	mov	$offset_operand1,%edi
	jmp	float_int32
is_op1_denorm_:
	cmpb	denormd,tag1(%ebp)
	jne	op1_zero_or_inf
	movb	true,signal_d_error_
	testb	denorm_mask,%gs:sr_masks		/ short cut if unmasked d-error
	jz	interim_exit
	mov	$offset_operand2,%edi
	call	norm_denorm
	mov	dword_expon2,%eax
	mov	$exponent_bias,dword_expon2	/ set op2 exponent=bias
	jmp	unbias_expon
op1_zero_or_inf:
	cmpb	special,tag1(%ebp)
	je	op1_zero			/ op1 and op2 are infinite
	movb	positive,sign1(%ebp)
interim_exit:
	ret
op1_zero:
	movb	true,signal_z_error_		/ if number=0, signal z-error
	testb	zero_divide_mask,%gs:sr_masks	/ if z_error is unmasked
	jz	interim_exit			/ then just return
make_op1_negative_inf:
	mov	$0x07fff,dword_expon1		/ else, make op1 -infinity
	movb	$0x80,msb_frac1
	mov	infinty\*0x10000+NEGATIVE,sign1(%ebp)
	ret
/decompose	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			float16:
/
/	function:
/		retrieves a 16-bit, 2's complement integer from memory,
/		and fills in the operand
/
/	inputs:
/		assumes mem_operand_pointer is set up
/		edi points to operand1 or operand2
/
/	outputs:
/		operand
/
/	data accessed:
/		- mem_operand_pointer
/
/	data changed:
/		- operand
/
/	procedures called:
/		float_int32
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
float16:	/proc
	les	mem_operand_pointer(%ebp),%ebx	/ fetch the
						/ 16-bit integer
	movw	%es:(%ebx),%ax
/..
/..		pushad
/..		push	es
/..		push	ebx
/..		call	fpfushort
/..		add	esp,8
/..		popad
/..		cld
/..
float_int16:
	movswl	%ax,%eax
	jmp	float_int32
/
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			float32:
/
/	function:
/		retrieves a 32-bit, 2's complement integer from memory
/		and fills in the operand
/
/	inputs:
/		assumes mem_operand_pointer is set up
/		edi points to operand1 or operand2
/
/	outputs:
/		operand
/
/	data accessed:
/		- mem_operand_pointer
/
/	data changed:
/		- operand
/
/	procedures called:
/
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
float32:
	les	mem_operand_pointer(%ebp),%ebx

	mov	%es:(%ebx),%eax			/ move from memory to eax

/..
/..		pushad
/..		push	es
/..		push	ebx
/..		call	fpfulong
/..		add	esp,8
/..		popad
/..		cld

float_int32:
	mov	%eax,%edx			/ save integer in edx
	xor	%eax,%eax			/ clear low 8 bytes
	mov	%eax,(%ebp,%edi)
	mov	%eax,frac64(%ebp,%edi)
	or	%edx,%eax			/ if integer is zero,
	jnz	get_sign_and_mag_32		/ then load +0
	mov	%eax,frac32(%ebp,%edi)
	movb	%al,sign(%ebp,%edi)
	movb	special,tag(%ebp,%edi)
	mov	%eax,expon(%ebp,%edi)
	ret
get_sign_and_mag_32:
	cltd					/ transmit sign to edx
	movb	%dl,sign(%ebp,%edi)		/ and store to operand
	jns	positive_int				/ branch if positive
	neg	%eax					/ negate if negative
positive_int:
	bsrl	%eax,%ecx
	sub	$31,%ecx
	neg	%ecx
	shl 	%cl,%eax
	mov	%eax,frac32(%ebp,%edi)	/ store normalized{abs(int)}
	mov	$0x401e,%eax				/ load valid exponent
	sub	%ecx,%eax
	mov	%eax,expon(%ebp,%edi)
	movb	valid,tag(%ebp,%edi)
	ret
/float16	endp
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			float64:
/
/	function:
/		retrieves a 64-bit, 2's complement integer from memory
/		and fills in the operand
/
/	inputs:
/		assumes mem_operand_pointer is set up
/		edi points to operand1 or operand2
/
/	outputs:
/		operand
/
/	data accessed:
/		- mem_operand_pointer
/
/	data changed:
/		- operand
/
/	procedures called:
/
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
float64:	/proc
	push	%ds					/ save a?msr
	lds	mem_operand_pointer(%ebp),%esi
	push	%ss
	pop	%es				/ es:edi ==> operand
	push	%edi				/ save op? offset
	lea	frac64(%ebp,%edi),%edi	/ set operand offset
	mov	$0x0002,%ecx		/ move fraction from
	rep	
	movsl				/  memory to operand

/..
/..		pushad
/..		push	ds
/..float64_loop:
/..		push	esi
/..		call	fpfulong
/..		pop	esi
/..		add	esi,4
/..		stosd
/..		loop	float64_loop
/..		pop	ds
/..		popad
/..		cld
/..
	pop	%edi			/ reload operand offset
	pop	%ds					/ reload a?msr
	mov	frac32(%ebp,%edi),%eax		/ get long integer sign
	cltd			/ transmit sign to edx
	movb	%dl,sign(%ebp,%edi)	/ and store to operand
	inc	%edx			/ test for '-' fract
	jnz	detect_zero_64
	not	frac64(%ebp,%edi)		/ '-', so form 2's
	add	$1,frac64(%ebp,%edi)	/ complement
	not	frac32(%ebp,%edi)
	adc	$0,frac32(%ebp,%edi)
	jmp	set_expon_64
detect_zero_64:
	or	frac64(%ebp,%edi),%eax
	jnz	set_expon_64
	mov	%eax,expon(%ebp,%edi)			/ set expon = zero
	movb	special,tag(%ebp,%edi)
	ret
set_expon_64:
	mov	$0x403e,expon(%ebp,%edi)	/set expon = 63
	bsrl	frac32(%ebp,%edi),%ecx
	jz	top_32_zero
	mov	frac64(%ebp,%edi),%eax
	sub	$31,%ecx
	neg	%ecx
	shldl	%eax,frac32(%ebp,%edi)
	shl 	%cl,frac64(%ebp,%edi)
adjust_norm_expon:
	sub	%ecx,expon(%ebp,%edi)	/ adjust exponent
	ret
top_32_zero:
	mov	frac64(%ebp,%edi),%eax
	bsrl	%eax,%ecx
	sub	$31,%ecx
	neg	%ecx
	shl 	%cl,%eax
	mov	%eax,frac32(%ebp,%edi)
	mov	$0,frac64(%ebp,%edi)
	add	$32,%ecx
	jmp	adjust_norm_expon
/float64	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			save:
/
/	function:
/		implements 80387 save instruction
/
/	inputs:
/
/	outputs:
/
/	data accessed:
/		- mem_operand_pointer		result_rec_offset
/
/	data changed:
/
/	procedures called:
/		save_status		save_regs	init
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
save:	/proc
	call	save_status
	call	save_regs
	jmp	init			/ initialize environment
/save	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			save_regs:
/
/	function:
/		saves all registers in contiguous memory locations.
/
/	inputs:
/		memory operand pointer (in es:[di])
/
/	outputs:
/
/	data accessed:
/
/	data changed:
/
/	procedures called:
/		get_top
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
save_regs:	/proc
	movb	$5,%cl				/ 5 words per register
	call	get_top				/ ax = top * 5
	push	%ds				/ set source seg register
	push	%gs
	pop	%ds
	mulb	%cl
	movw	$40,%cx				/ number words to move
	subw	%ax,%cx				/ from top of stack =
	mov	$sr_regstack,%esi		/  40 - top*5
	push	%esi				/ starting address =
	cwtl
	add	%eax,%esi			/  %gs:sr_regstack + top*5
	add	%eax,%esi			/  *2 bytes/word
	movzwl	%cx,%ecx
	rep	
	movsw					/ move top of stack
	movw	%ax,%cx				/ move (top * 5) words
	pop	%esi				/ load starting address
	rep	
	movsw					/ move rest of stack
	pop	%ds				/ restore %ds
	ret
/save_regs	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			restore_regs:
/
/	function:
/		loads all registers from contiguous memory locations
/		implements 80387 restore instruction
/
/	inputs:
/		mem_operand_pointer
/
/	outputs:
/		loads the %gs:sr_regstack area of a?msr from memory
/
/	data accessed:
/
/	data changed:
/		- operand
/
/	procedures called:
/		restore_status			get_top
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
restore:	/proc
	call	restore_status			/ this call defines es
restore_regs:
	movb	$5,%cl				/ 5 words per register
	call	get_top
	push	%ds				/ save a?msr
/	pop	%es
/	push	%ds
	mulb	%cl				/ ax = top*5
	mov	$40,%ecx				/ number words to move
	subw	%ax,%cx				/ from top of stack =
	mov	$sr_regstack,%edi		/  40 - top*5
	push	%edi				/ starting address =
	cwtl
	add	%eax,%edi				/  %gs:sr_regstack + top*5
	add	%eax,%edi				/  *2 bytes/word
	lds	mem_operand_pointer(%ebp),%esi	/ source is in memory
	add	$14,%esi
	cmpb	$1,oprnd_siz32(%ebp)	/is it a 32 bit operand
	jne	status16		/no restore 16 bit status
	add	$14,%esi				/ skip the environment
/	--------replace following line for unix
status16:
	rep	
	movsw					/ move top of stack

/..
/..		pushad
/..		push	ds
/..restor_loop:
/..		push	esi
/..		call	fpfushort
/..		pop	esi
/..		stosw
/..		add	esi,2
/..		loop	restor_loop
/..		pop	ds
/..		popad
/..		cld
/..
	movw	%ax,%cx				/ move (top * 5) words
	pop	%edi				/ load starting address
	rep	
	movsw					/ move rest of stack
	pop	%ds				/ reload a?msr
	jmp	correct_tag_word

	ret
/restore	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			set_up_indefinite:
/
/	function:
/		puts indefinite in the operand pointed to by edi
/
/	input:
/		edi points to operand
/
/	output:
/		indefinite in operand,	eax = 0, zf set (zf = 1)
/
/	data accessed:
/		- operand
/
/	data changed:
/		- operand
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
set_up_indefinite:	/proc
	xor	%eax,%eax
	mov	%eax,frac64(%ebp,%edi) 	/set up indef fraction
	mov	$0x0c0000000,frac32(%ebp,%edi)
	movb	negative,sign(%ebp,%edi) /set sign negative
	mov	$0x7fff,expon(%ebp,%edi)	 /set exponent to 7fffh
	movb	inv,tag(%ebp,%edi)			 /set tag to invalid
common_return:
	ret
/set_up_indefinite	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			exchange
/
/
/	function:
/		implements 80387 exchange instruction
/
/	inputs:
/		operand(s) in operands
/
/	outputs:
/		result(s) on stack
/
/	data accessed:
/		- result_rec_offset		offset_operand1
/
/	data changed:
/
/	procedures called:
/		put_result			set_stk_u_error
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
exchange:	/proc
	jz	do_exchange		/ if zf clear, then stack error
	call	set_stk_u_error
	testb	invalid_mask,%gs:sr_masks
	jz	common_return			/ if unmasked error, do nothing
do_exchange:
/	call	common_clear_a_bit		/ put op1(top) in result(reg)
	and	$ -1 ! a_mask,%gs:sr_flags
do_exchange_leave_a_bit:
	call	put_op1_result
set_zf_put_second_result:
	xor	%eax,%eax				/ set '86 zf
	lahf					/ save set zf in ah
put_second_result:
	mov	$offset_operand2,%edi	/ put op2(reg) in res2(top)
	mov	offset_result2_rec,%esi
	jmp	put_result
/exchange	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			exam:
/
/	function:
/		implements 80387 fxam instruction
/
/	inputs:
/		operand1
/
/	outputs:
/		operand type indicators set
/
/	data accessed:
/		- tag1				sign1
/		- msb_frac1
/
/	data changed:
/
/	procedures called:
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
exam:	/proc
	jz	something_on_stack	/ branch on no stack error
	orb	$[zero_mask+sign_mask],%gs:sr_flags	/ set s and z bits
	andb	$ -1 ! [a_mask+c_mask],%gs:sr_flags	/ clear a and c bits
	jmp	check_a_bit
something_on_stack:
	andb	$ -1 ![zero_mask+c_mask+a_mask+sign_mask],%gs:sr_flags
					/ clear condition code bits
	movb	tag1(%ebp),%al
	cmpb	unsupp,%al
	je	check_a_bit
	cmpb	inv,%al				/ remove special cases
	je	check_z_s_a_bits
	cmpb	infinty,%al
	je	set_c_to_one
	cmpb	denormd,%al
	jne	valid_or_zero
	orb	$[zero_mask+c_mask],%gs:sr_flags	/ set s, z, c to 0, 1, 1
					/ for denormalized op
	jmp	check_a_bit
valid_or_zero:
	testb	$0x80,msb_frac1		/ for valid or zero,
	jz	check_z_s_a_bits	/ set c to msb of frac
set_c_to_one:
	orb	$c_mask,%gs:sr_flags
check_z_s_a_bits:
	rorb	$1,%al				/ set z-bit to lsb of tag1
	jnc	check_s_a_bits
	orb	$zero_mask,%gs:sr_flags
check_s_a_bits:
	rorb	$1,%al				/ set s-bit to lsb-1 of tag1
	jnc	check_a_bit
	orb	$sign_mask,%gs:sr_flags
check_a_bit:
	testb	$0x01,sign1(%ebp)	/ set a-bit to sign bit
	jz	exchange_return
	orb	$a_mask,%gs:sr_flags	/ set_a_bit
exchange_return:
	ret
/exam	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			fxtrac:
/
/	function:
/		implements the 80387 fxtrac instruction
/
/	inputs:
/
/	outputs:
/		decomposed results in results
/
/	data accessed:
/		- result_rec_offset		result2_rec_offset
/		- offset_operand1		tag1
/		- offset_operand2
/
/	data changed:
/
/	procedures called:
/		set_up_indefinite		decompose
/		do_exchange	
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
fxtrac:	/proc
	jz	detect_i_error	/ branch if no stack error
	call	set_stk_u_error	/ set invalid error and stack bit, but
						/ clear c1 (a) bit
	testb	invalid_mask,%gs:sr_masks
	jz	quick_out			/ branch if error unmasked
	mov	$offset_operand2,%edi		/ for masked invalid error,
	call	set_up_indefinite			/ set op2 to indefinite
	orb	true,%al			/ clear zf to indicate that
	lahf					/ stack underflow occurred.
	jmp	put_second_result
detect_i_error:
	call	stack_full_
	jz	other_i_error_
	jmp	set_zf_put_second_result
other_i_error_:
	andb	$-1 ! a_mask,%gs:sr_flags	/ clear a-bit
	mov	$0,before_error_signals(%ebp)	/ initialize all to zero
	movb	tag1(%ebp),%al				/ load operand tag
	cmpb	unsupp,%al
	jne	is_op1_nan_
	testb	invalid_mask,%gs:sr_masks	/ short cut if unmasked i-error
	jnz	put_indef_in_op1
unmasked_i_error:
	orb	invalid_mask, %gs:sr_errors
quick_out:
	ret			/ return if unmasked error
put_indef_in_op1:
	mov	$offset_operand1, %edi
	call	set_up_indefinite
	jmp	masked_i_error
is_op1_nan_:
	cmpb	inv,%al
	jne	do_decompose
	testb	$0x40,msb_frac1			/ is op1 a signaling nan?
	jnz	op1_is_qnan
	testb	invalid_mask,%gs:sr_masks
	jz	unmasked_i_error
	orb	$0x40,msb_frac1				/ make op1 a quiet nan
masked_i_error:
	movb	true, signal_i_error_
op1_is_qnan:
	lea	dword_frac1(%ebp),%esi	/ move op1 to op2
	lea	dword_frac2(%ebp),%edi
	call	move_op_to_op
	call	put_op1_result				/ no push occurs here
	jmp	set_zf_put_second_result		/ put both results
do_decompose:
	call	decompose
	jmp	do_exchange		/ put both results
/fxtrac	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			abs_value:
/
/	function:
/		implements the 80387 absolute value instruction
/
/	inputs:
/		assumes operand is set up
/
/	outputs:
/		absolute value of operand in result
/
/	procedures called:
/		set_stk_u_error
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
abs_value:	/proc
	jz	do_abs_value			/ branch if no stack error
	call	set_stk_u_error		/ set invalid error and stack bit,
					/ but clear c1 (a) bit
	testb	invalid_mask,%gs:sr_masks
	jnz	abs_value_result	/ masked error, so give indef already
common_exit:				/ set up in operand1 by fetch_an_op.
	ret				/ unmasked error
do_abs_value:
	movb	positive,sign1(%ebp)/ set sign to positive
abs_value_result:
	jmp	common_clear_a_bit		/ put result on stack
/abs_value	endp
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			chsign:
/
/	function:
/		implement the 80387 chsign instruction
/
/	inputs:
/		operand1
/
/	outputs:
/		result <==  -(operand1)
/
/	procedures called:
/		set_stk_u_error
/
/""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
chsign:	/proc
	jz	do_chsign			/ branch if no stack error
	call	set_stk_u_error	/ set invalid error and stack bit, but
						/ clear c1 (a) bit
	testb	invalid_mask,%gs:sr_masks
	jnz	chsign_result		/ put op1, set up as qnan indefinite
	ret				/ by fetch_an_op, into result
do_chsign:
	notb	sign1(%ebp)			/ complement sign
chsign_result:
	jmp	common_clear_a_bit		/ put op1 into result
/chsign	endp
/
/a_med	ends
/
/	end
