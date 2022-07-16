/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.file	"dcode.s"

	.ident	"@(#)kern-fp:dcode.s	1.1"

/ *************************************************************************
/
/ 			d c o d e .m o d 
/			================
/
/	================================================================
/               intel corporation proprietary information
/    this software  is  supplied  under  the  terms  of  a  license
/    agreement  or non-disclosure agreement  with intel corporation
/    and  may not be copied nor disclosed except in accordance with
/    the terms of that agreement.                                  
/	================================================================
/
/	function:
/		converts the 80387 instruction into information
/               about the first operand, information about the
/               second operand, information about the result, 
/               and the operation type.  defines mem_operand_ptr.
/
/	public procedures:
/		e80387
/
/ ****************************************************************************
/
/...March 3, 1987...
/
/        .file   "a_mdc"
/$nolist
#include	"e80387.h"
/$list
/
/...declare stack segment...
/
/stack	stackseg	200
/
/...declare status register segment...
/
	.data	/a_msr	segment	rw	public
/	extrn	sr_masks,sr_errors,sr_instr_offset
/	extrn	sr_instr_base,sr_mem_offset,sr_mem_base
/a_msr	ends
/
/...declare status register selector segment...
/
	.data	/a_msrs	segment	rw	public
/ a_msr_selector: .long   a_msr
/a_msrs	ends
/
	.text	/a_med	segment	er	public
/
/	extrn	load,store,chsign,compar
/	extrn	arith,ldcw,save,restore
/	extrn	free_reg,abs_value,init,log
/	extrn	fxtrac,splog,exp,tan
/	extrn	arctan,sqrt,remr,intpt
/	extrn	scale,exchange,restore_status
/	extrn	stenv,stcw,stsw,load_con
/	extrn	decr_top,incr_top,clex,exam
/	extrn	fetch_an_op,sin,cos,sincos
/	extrn	rem1
/
	.globl	e80387

/ --------------------------------------JKD-----------------------

/ --------------------------------------JKD-----------------------

/        .globl  a_msr_selector
/
/ *********************************************************************
/
/	address table for instruction group decoding handlers
/
/ *********************************************************************
						/ 10-9-8  (mod = 3)::1
group_handler:  
	.long   arith_short_real        /  0 0 0  0
	.long	arith_top_reg		/  0 0 0  1
	.long	load_short_real		/  0 0 1  0
	.long	transcendentals		/  0 0 1  1
	.long	arith_short_int		/  0 1 0  0
	.long	double_pop		/  0 1 0  1
	.long	load_short_int		/  0 1 1  0
	.long	administrative		/  0 1 1  1
	.long	arith_long_real		/  1 0 0  0
	.long	arith_reg_top		/  1 0 0  1
	.long	load_long_real		/  1 0 1  0
	.long	store_reg		/  1 0 1  1
	.long	arith_word_int		/  1 1 0  0
	.long	arith_reg_pop		/  1 1 0  1
	.long	load_word_int		/  1 1 1  0
	.long	transfer_status		/  1 1 1  1
/ *********************************************************************
/
/	address table for r/m decoding handlers
/
/ *********************************************************************
rm_handler:	
	.long	rm_0			/ ea = (bx) + (si) + disp
	.long	rm_1			/ ea = (bx) + (di) + disp
	.long	rm_2			/ ea = (bp) + (si) + disp
	.long	rm_3			/ ea = (bp) + (di) + disp
	.long	rm_4			/ ea = (si) + disp
	.long	rm_5			/ ea = (di) + disp
	.long	rm_6			/ ea = (bp) + disp*
	.long	rm_7			/ ea = (bx) + disp
/ *********************************************************************
/
/	address table for 32 bit base register decoding handlers
/
/ *********************************************************************
base32:	
	.long	base_0			/ ea = [eax] + disp
	.long	base_1			/ ea = [ecx] + disp
	.long	base_2			/ ea = [edx] + disp
	.long	base_3			/ ea = [ebx] + disp
	.long	base_4			/ ea = [esp] + disp
	.long	base_5			/ ea = [ebp] + disp*
	.long	base_6			/ ea = [esi] + disp
	.long	base_7			/ ea = [edi] + disp
/ *********************************************************************
/
/	address table for 32 bit index register decoding handlers
/
/ *********************************************************************
index32:	
	.long	index_0			/ ea = [eax*ss] 
	.long	index_1			/ ea = [ecx*ss] 
	.long	index_2			/ ea = [edx*ss] 
	.long	index_3			/ ea = [ebx*ss] 
	.long	index_4			/ ea = [esp*ss] 
	.long	index_5			/ ea = [ebp*ss] 
	.long	index_6			/ ea = [esi*ss] 
	.long	index_7			/ ea = [edi*ss] 
/ *********************************************************************
/
/	address table for instruction operations
/
/ *********************************************************************
op_table:	
	.long	load,store,chsign,compar,free_reg,arith,arith,arith
	.long	arith,ldcw,save,restore,free_reg,abs_value,init
	.long	fxtrac,two_arg_trans,two_arg_trans,exp,tan
	.long	two_arg_trans,sqrt,remr,intpt,scale
	.long	exchange,free_reg,restore_status,stenv,stcw,stsw
	.long	load_con,load_con,load_con,load_con,load_con,load_con
	.long	load_con,decr_top,incr_top,clex,compar,exam,sin,cos,sincos
	.long	remr,compar
/
/ *********************************************************************
/
/	arithmetic operation type decode table
/
/ *********************************************************************
						/          5-4-3
group_0a_0b_2a_4a_4b_6a_6b_op:	
	.byte	add_op	/ fadd     0 0 0
	.byte	mul_op			/ fmul     0 0 1
	.byte	compar_op 		/ fcom     0 1 0
	.byte	compar_pop		/ fcomp    0 1 1
	.byte	sub_op			/ fsub     1 0 0
	.byte	subr_op			/ fsubr    1 0 1
	.byte	div_op			/ fdiv     1 1 0
	.byte	divr_op			/ fdivr    1 1 1
/
/ *********************************************************************
/
/	control word/environment operation type decode table
/
/ *********************************************************************
						/          5-4-3
group_1a_op:	
	.byte	load_op			/ fld      0 0 0
	.byte	error_op		/ reserved 0 0 1
	.byte	store_op		/ fst      0 1 0
	.byte	store_pop		/ fstp     0 1 1
	.byte	ldenv_op		/ fldenv   1 0 0
	.byte	ldcw_op			/ fldcw    1 0 1
	.byte	stenv_op		/ fstenv   1 1 0
	.byte	stcw_op			/ fstsw    1 1 1
/
/ *********************************************************************
/
/	load/exchange/nop operation decode table
/
/ *********************************************************************
						/          5-4-3-2-1-0
group_1ba_op:	
	.byte	load_op			/ fld      0 0 0 r e g
	.byte	exchange_op		/ fxch     0 0 1 r e g
	.byte	null_op			/ fnop     0 1 1 r e g
	.byte	store_pop		/ fstp *   0 1 1 r e g
/
/ *********************************************************************
/
/	transcendental operation decode table
/
/ *********************************************************************
						/          5-4-3-2-1-0
group_1bb_op:	
	.byte	chsign_op		/ fchs     1 0 0 0 0 0
	.byte	abs_op			/ fabs     1 0 0 0 0 1
	.byte	error_op		/ reserved 1 0 0 0 1 0
	.byte	error_op		/ reserved 1 0 0 0 1 1
	.byte	test_op			/ ftst     1 0 0 1 0 0
	.byte	exam_op			/ fxam     1 0 0 1 0 1
	.byte	error_op		/ reserved 1 0 0 1 1 0
	.byte	error_op		/ reserved 1 0 0 1 1 1
	.byte	load_1_op		/ fld1     1 0 1 0 0 0
	.byte	load_l2t_op		/ fldl2t   1 0 1 0 0 1
	.byte	load_l2e_op		/ fldl2e   1 0 1 0 1 0
	.byte	load_pi_op		/ fldpi    1 0 1 0 1 1
	.byte	load_lg2_op		/ fldlg2   1 0 1 1 0 0
	.byte	load_ln2_op		/ fldln2   1 0 1 1 0 1
	.byte	load_0_op		/ fldz     1 0 1 1 1 0
	.byte	error_op		/ reserved 1 0 1 1 1 1
	.byte	exp_op			/ f2xm1    1 1 0 0 0 0
	.byte	log_op			/ fyl2x    1 1 0 0 0 1
	.byte	tan_op			/ fptan    1 1 0 0 1 0
	.byte	arctan_op		/ fpatan   1 1 0 0 1 1
	.byte	fxtrac_op		/ fxtract  1 1 0 1 0 0
	.byte	rem1_op			/ fprem1   1 1 0 1 0 1
	.byte	decstp_op		/ fdecstp  1 1 0 1 1 0
	.byte	incstp_op		/ fincstp  1 1 0 1 1 1
	.byte	remr_op			/ fprem    1 1 1 0 0 0
	.byte	splog_op		/ fyl2xp1  1 1 1 0 0 1
	.byte	sqrt_op			/ fsqrt    1 1 1 0 1 0
	.byte	sincos_op		/ fsincos  1 1 1 0 1 1
	.byte	intpt_op		/ frndint  1 1 1 1 0 0
	.byte	scale_op		/ fscale   1 1 1 1 0 1
	.byte	sin_op			/ fsin     1 1 1 1 1 0
	.byte	cos_op		    / fcos     1 1 1 1 1 1
/
/ *********************************************************************
/
/	tempreal/bcd/short integer/long integer operation decode table
/
/ *********************************************************************
						/          5-4-3
group_3a_7a_op:	
	.byte	load_op			/ fild     0 0 0
	.byte	error_op		/ reserved 0 0 1
	.byte	store_op		/ fist     0 1 0
	.byte	store_pop		/ fistp    0 1 1
	.byte	load_op			/ (fbld)   1 0 0
	.byte	load_op			/ f(i)ld   1 0 1
	.byte	store_pop		/ (fbstp)  1 1 0
	.byte	store_pop		/ f(i)stp  1 1 1
/
/ *********************************************************************
/
/	clear exception/initialize operation decode table
/
/ *********************************************************************
						/          5-4-3-2-1-0
group_3b_op:	
	.byte	null_op			/ feni     1 0 0 0 0 0
	.byte	null_op			/ fdisi    1 0 0 0 0 1
	.byte	cler_op			/ fclex    1 0 0 0 1 0
	.byte	init_op			/ finit    1 0 0 0 1 1
	.byte	null_op			/ fsetpm   1 0 0 1 0 0
	.byte	error_op		/ reserved 1 0 0 1 0 1
	.byte	error_op		/ reserved 1 0 0 1 1 0
	.byte	error_op		/ reserved 1 0 0 1 1 1
/
/ *********************************************************************
/
/	restore/save operation decode table
/
/ *********************************************************************
						/          5-4-3
group_5a_op:	
	.byte	load_op			/ fld      0 0 0
	.byte	error_op		/ reserved 0 0 1
	.byte	store_op		/ fst      0 1 0
	.byte	store_pop		/ fstp     0 1 1
	.byte	restore_op		/ frstor   1 0 0
	.byte	error_op		/ reserved 1 0 1
	.byte	save_op			/ fsave    1 1 0
	.byte	stsw_op			/ fstsw    1 1 1
/
/ *********************************************************************
/
/	free register/store operation decode table
/
/ *********************************************************************
						/          5-4-3
group_5b_op:	
	.byte	free_op			/ ffree    0 0 0
	.byte	exchange_op		/ fxch *   0 0 1
	.byte	store_op		/ fst      0 1 0
	.byte	store_pop		/ fstp     0 1 1
	.byte	ucom_op			/ fucom    1 0 0
	.byte	ucom_pop		/ fucomp   1 0 1
	.byte	error_op		/ reserved 1 1 0
	.byte	error_op		/ reserved 1 1 1
/
/ *********************************************************************
/
/	transfer status operation decode table
/
/ *********************************************************************
						/          5-4-3
group_7b_op:	
	.byte	free_op			/ ffree *  0 0 0
	.byte	exchange_op		/ fxch *   0 0 1
	.byte	store_pop		/ fstp *   0 1 0
	.byte	store_pop		/ fstp *   0 1 1
	.byte	stsw_op			/ fstsw    1 0 0
	.byte	error_op		/ reserved 1 0 1
	.byte	error_op		/ reserved 1 1 0
	.byte	error_op		/ reserved 1 1 1
/
/ *********************************************************************
/
/	load/store environment/control word operand 1 loc/for table
/
/ *********************************************************************
						     /          5-4-3
group_1a_op1_lf:	
	.value  memory_opnd\*0x100+single_real / fld      0 0 0
	.value  null\*0x100+null              / reserved 0 0 1
	.value  stack_top\*0x100+extended_fp   / fst      0 1 0
	.value  stack_top\*0x100+extended_fp   / fstp     0 1 1
	.value  null\*0x100+null               / fldenv   1 0 0
	.value  null\*0x100+null               / fldcw    1 0 1
	.value  null\*0x100+null              / fstenv   1 1 0
	.value  null\*0x100+null              / fstcw    1 1 1
/
/ *********************************************************************
/
/       transcendental operand/result format codes
/
/ *********************************************************************
/
#define	null_fmt	0b00
#define	top_fmt	0b01	
#define	topm1_fmt	0b10
#define	topp1_fmt	0b11
#define	op1	0b1000000
#define	op2	0b10000	
#define	res1	0b100
#define	res2	0b1
/
/	transcendental operand/result form at types
/
/ #define	fprem_type	op1\*top_fmt+op2\*topm1_fmt+res1\*top_fmt
#define	fprem_type	0x64
/ #define	fyl2x_type	op1\*top_fmt+op2\*topm1_fmt+res1\*topm1_fmt
#define	fyl2x_type	0x68
#define	fdecstp_type	null_fmt
#define	fchs_type	op1\*top_fmt+res1\*top_fmt
#define	fld1_type	res1\*topp1_fmt
#define	ftst_type	op1\*top_fmt
/ #define	fptan_type	op1\*top_fmt+res1\*top_fmt+res2\*topp1_fmt
#define	fptan_type	0x47
/ #define	fpatan_type	op1\*topm1_fmt+op2\*top_fmt+res1\*topm1_fmt
#define	fpatan_type	0x98
#define	fscale_type	op1\*topm1_fmt+op2\*top_fmt+res1\*top_fmt
/
/fsin_type   equ op1*top_fmt + res1*top_fmt
/fcos_type   equ op1*top_fmt + res1*top_fmt
/
/	operand/result format/location table
/
group_1bb_lf:	
	.value	null					/ null_fmt
	.value  stack_top\*0x100+extended_fp             / top_fmt
	.value  stack_top_minus_1\*0x100+extended_fp     / topm1_fmt
	.value  stack_top_plus_1\*0x100+extended_fp      / topp1_fmt
/
/ *********************************************************************
/
/	transcendental operand/result format/location type table
/
/ *********************************************************************
						/          5-4-3-2-1-0
group_1bb_type:	
	.byte	fchs_type		/ fchs     1 0 0 0 0 0
	.byte	fchs_type		/ fabs     1 0 0 0 0 1
	.byte	fdecstp_type		/ reserved 1 0 0 0 1 0
	.byte	fdecstp_type		/ reserved 1 0 0 0 1 1
	.byte	ftst_type		/ ftst     1 0 0 1 0 0
	.byte	ftst_type		/ fxam     1 0 0 1 0 1
	.byte	fdecstp_type		/ reserved 1 0 0 1 1 0
	.byte	fdecstp_type		/ reserved 1 0 0 1 1 1
	.byte	fld1_type		/ fld1     1 0 1 0 0 0
	.byte	fld1_type		/ fldl2t   1 0 1 0 0 1
	.byte	fld1_type		/ fldl2e   1 0 1 0 1 0
	.byte	fld1_type		/ fldpi    1 0 1 0 1 1
	.byte	fld1_type		/ fldlg2   1 0 1 1 0 0
	.byte	fld1_type		/ fldln2   1 0 1 1 0 1
	.byte	fld1_type		/ fldz     1 0 1 1 1 0
	.byte	fdecstp_type		/ reserved 1 0 1 1 1 1
	.byte	fchs_type		/ f2xm1    1 1 0 0 0 0
	.byte	fyl2x_type		/ fyl2x    1 1 0 0 0 1
	.byte	fptan_type		/ fptan    1 1 0 0 1 0
	.byte	fpatan_type		/ fpatan   1 1 0 0 1 1
	.byte	fptan_type		/ fxtract  1 1 0 1 0 0
	.byte	fprem_type		/ fprem1   1 1 0 1 0 1
	.byte	fdecstp_type		/ fdecstp  1 1 0 1 1 0
	.byte	fdecstp_type		/ fincstp  1 1 0 1 1 1
	.byte	fprem_type		/ fprem    1 1 1 0 0 0
	.byte	fyl2x_type		/ fyl2xp1  1 1 1 0 0 1
	.byte	fchs_type		/ fsqrt    1 1 1 0 1 0
	.byte	fptan_type		/ fsincos  1 1 1 0 1 1
	.byte	fchs_type		/ frndint  1 1 1 1 0 0
	.byte	fscale_type		/ fscale   1 1 1 1 0 1
	.byte	fchs_type		/ fsin     1 1 1 1 1 0
	.byte	fchs_type		/ fcos     1 1 1 1 1 1
/
/ *********************************************************************
/
/	load/store short integer/tempreal operand 1 format table
/
/ *********************************************************************
						/          5-4-3
group_3a_op1_fmt:	
	.byte	int32			/ fild     0 0 0
	.byte	null			/ reserved 0 0 1
	.byte	extended_fp		/ fist     0 1 0
	.byte	extended_fp		/ fistp    0 1 1
	.byte	null			/ reserved 1 0 0
	.byte	extended_fp		/ fld      1 0 1
	.byte	null			/ reserved 1 1 0
	.byte	extended_fp		/ fstp     1 1 1
/ *********************************************************************
/
/	load/store integer/tempreal/bcd operand 1 location table
/
/ *********************************************************************
						/          5-4-3
group_3a_7a_op1_loc:	
	.byte	memory_opnd		/ fild     0 0 0
	.byte	null			/ reserved 0 0 1
	.byte	stack_top		/ fist     0 1 0
	.byte	stack_top		/ fistp    0 1 1
	.byte	memory_opnd		/ (fbld)   1 0 0
	.byte	memory_opnd		/ f(i)ld   1 0 1
	.byte	stack_top		/ (fbstp)  1 1 0
	.byte	stack_top		/ f(i)stp  1 1 1
/
/ *********************************************************************
/
/	load/store short integer/tempreal result format table
/
/ *********************************************************************
						/          5-4-3
group_3a_res_fmt:	
	.byte	extended_fp		/ fild     0 0 0
	.byte	null			/ reserved 0 0 1
	.byte	int32			/ fist     0 1 0
	.byte	int32			/ fistp    0 1 1
	.byte	null			/ reserved 1 0 0
	.byte	extended_fp		/ fld      1 0 1
	.byte	null			/ reserved 1 1 0
	.byte	extended_fp		/ fstp     1 1 1
/ *********************************************************************
/
/	load/store integer/tempreal/bcd result location table
/
/ *********************************************************************
						/          5-4-3
group_3a_7a_res_loc:	
	.byte	stack_top		/ fild     0 0 0
	.byte	null			/ reserved 0 0 1
	.byte	memory_opnd		/ fist     0 1 0
	.byte	memory_opnd		/ fistp    0 1 1
	.byte	stack_top_plus_1	/ (fbld)   1 0 0
	.byte	stack_top_plus_1	/ f(i)ld   1 0 1
	.byte	memory_opnd		/ (fbstp)  1 1 0
	.byte	memory_opnd		/ f(i)stp  1 1 1
/
/ *********************************************************************
/
/	load/store state/status word operand 1 loc/for table
/
/ *********************************************************************
						     /          5-4-3
group_5a_op1_lf:	
	.value  memory_opnd\*0x100+double_real / fld      0 0 0
	.value  null\*0x100+null              / reserved 0 0 1
	.value  stack_top\*0x100+extended_fp   / fst      0 1 0
	.value  stack_top\*0x100+extended_fp   / fstp     0 1 1
	.value  null\*0x100+null               / frstor   1 0 0
	.value  null\*0x100+null              / reserved 1 0 1
	.value  null\*0x100+null              / fsave    1 1 0
	.value  null\*0x100+null              / fstsw    1 1 1
/ *********************************************************************
/
/	bcd/long integer operand 1 format table
/
/ *********************************************************************
						/          5-4-3
group_7a_op1_fmt:	
	.byte	int16			/ fild     0 0 0
	.byte	null			/ reserved 0 0 1
	.byte	extended_fp		/ fist     0 1 0
	.byte	extended_fp		/ fistp    0 1 1
	.byte	bcd			/ fbld     1 0 0
	.byte	int64			/ fild     1 0 1
	.byte	extended_fp		/ fbstp    1 1 0
	.byte	extended_fp		/ fistp    1 1 1
/
/ *********************************************************************
/
/	bcd/long integer result format table
/
/ *********************************************************************
						/          5-4-3
group_7a_res_fmt:	
	.byte	extended_fp		/ fild     0 0 0
	.byte	null			/ reserved 0 0 1
	.byte	int16			/ fist     0 1 0
	.byte	int16			/ fistp    0 1 1
	.byte	extended_fp		/ fbld     1 0 0
	.byte	extended_fp		/ fild     1 0 1
	.byte	bcd			/ fbstp    1 1 0
	.byte	int64			/ fistp    1 1 1
/ for the return from e80387
	.set    KFL, 0x3bfff            / clear NT bit in flags
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/			e80387:
/
/	function:
/		executive program for 80387 emulator.
/		decode instruction, i.e. separate operation,operand
/		and result info; get memory operand pointer.
/
/	inputs:
/		instruction (in memory).
/
/	outputs:
/		op1, op2, operation, result, result2, and memory operand
/		pointer info
/
/	data accessed:
/		- op1_format			op1_location
/		- op1_use_up			op2_format
/		- op2_location			op2_use_up
/		- operation_type		result_location
/		- result_format			result2_format
/		- result2_location
/
/	data changed:
/		- mem_operand_pointer		op1_location
/		- op1_format			op1_use_up
/		- op2_format			op2_location
/		- op2_use_up			result_location
/		- result_format			result2_format
/		- result2_location
/
/"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
/	assume	%ds:a_med
/	---------------------JKD
/	.data
/LJKD:
/	.string		" ADDRESS is %x %x\n"
/	.text
/
/	---------------------JKD
e80387:	/proc
	pushl	$0			/ push error code and trapno for
	pushl	$7			/ eventual return thru do_ret, ttrap.s
	pusha                           / save other registers
	push	%ds			/ save segment registers
	push	%es
	push	%fs
	push	%gs
	sub     global_reent_seg_length,%esp / reserve stack space
	mov	%esp,%ebp		/set bp to global_reentrant_seg
	/
	/ force ds=es=ss
	/
	movw	%ss, %ax
	movw	%ax, %ds
	movw	%ax, %es

	push	$USER_FP		/ set window to fp registers
	pop	%gs			/ in u area

	cmpb	$0, %gs:fpvalid		/ check if fp has been initialized
	jnz	init_done		/ if fpvalid != 0, it has been
	int	$32			/ use the kernel's fpnoextflt

init_done:
	/
	/ in the case where we have been called by a process
	/ that is running a 286 image, all of the pointers we have
	/ are 16 bit pointers.  they've been padded
	/ out to 32 bits, which is good, but they've been padded
	/ with random half-words, which is bad.  zero those high-
	/ order halfwords.
	/
	movw	%ss, %ax
	cmpw	$USER_DS, %ax
	je	build_grs1		/ assume that USER_DS ==> 32 bit
	cmpw	$KDSSEL, %ax
	je	build_grs1		/ assume that KDSSEL ==> 32 bit
	andl	$0xFFFF, %esp
	andl	$0xFFFF, %ebp
	movb	$1, %bl			/ can't use is16bit(%ebp) until
	jmp	build_grs2		/ after the movsl below.
build_grs1:
	movb	$0, %bl

build_grs2:
	lea	saved_gs(%ebp), %edi / Establishing address for movsl
				     /  to move registers from kernel stack
	lea	[offset_gs](%ebp),%esi	/   to current stack frame
	movl	$12, %ecx		/ 12 longs to be moved.
	rep					/ GS, FS, ES, DS, EDI, ESI, 
	movsl					/ EBP, ESP, EBX, EDX, ECX, EAX

					/ Assume, esi, and edi have been
					/ incremented with movsl.  need to
					/ skip eight bytes for TRAPNO and ERR
					/ then continue for 3 more longs.
	add	$8, %esi
	movl	$3, %ecx
	rep
	movsl					/ EIP, CS, EFL

	movl	%ebp, %ecx
	add	$int_esp, %ecx
	movl	%ecx, (%edi)
	add	$4, %edi
	push	%ss			/ user ss
	pop	(%edi)
	movb	%bl, is16bit(%ebp)	/ record whether we're 286 or 386


/ ----------------------------------------------------------------------

	cld
	mov	$36,%ecx
	mov	$mem_operand_pointer,%edi
	add	%ebp,%edi
	push	%ss
	pop	%es
	mov	$0x0,%eax
	rep
	stosl
	
	movw    %ss,correct_ss(%ebp)
	mov	old_esp(%ebp),%eax
	mov	%eax,correct_esp(%ebp)
continu_init:

	cld				/ d-flag clear for all e80387
	movb	$null,%dl			/ initialize op1 and op2
	movb	%dl,%cl			/  format registers to null
	movw    $do_nothing\*0x100+do_nothing,%bx / initialize useups
	lds	saved_eip(%ebp),%esi / load instruction pointer
/
/	--------------	JKD hack to print out the eip of the user
/			all lines herein can be deleted or commented out
/		*******	ROSHAN, you can uncomment these next few lines of code
/			plus the ones just before e80387: for label LJKD
/			and it will print out the address of the instruction
/			being emulated.  This is helpful sometimes but very
/			slow and writes to the screen a bunch
/	push	%eax
/	push	%edx
/	push	%ecx	/ these destroyed by printf
/	push	%ds	/save current ds
/	movw	saved_ds(%ebp), %ax
/	movw	%ax, %ds / Get old ds back for addressing label LJKD
/	push	%esi	/ This contains user eip
/	push	$LJKD		/this is printf string arg
/	call	printf		/print it out
/	addl	$8, %esp	/restore stack
/	pop	%ds
/	pop	%ecx
/	pop	%edx
/	pop	%eax		/stack back like it was, 
/
/ 	--------------	end of JKD hack,  also, might remove .LJKD

	movb	$1,oprnd_siz32(%ebp)	/set operand size=32
	movb	$1,addrs_siz32(%ebp)	/set address size=32
	lar	saved_cs(%ebp),%eax	/load access right byte of cs
	test	$0x00400000,%eax		/check 32 bit default ?
	jnz	get_opcode		/if yes continue
	decb	oprnd_siz32(%ebp)	/else default is 16bit
	decb	addrs_siz32(%ebp)
get_opcode:	
	movw	(%esi),%ax	/load two bytes

	inc	%esi			/inc pointer
	andb	$0x87,%al			/ is the first byte a prefix
	jns	get_opcode		/ yes skip for now
	addb	$0x40,%ah			/ no, add 1 to mod
	rclb	$1,%al			/ cf set if mod = 3
	rolb	$1,%al			/ form handler table index
	movw	$0x3800,%si		/ form bits 5-4-3 index in si
	andw	%ax,%si
	shrw	$11,%si
	movw	$0x001e,%di		/ form group handler table
	andw	%ax,%di			/  index in di
	shlw	$1,%di			/ take care of dd instead of dw
	push	%cs			/ copy cs to ds for addressing
	pop	%ds			/  all the tables
	movswl  %si,%esi
	movswl  %di,%edi
	call    *%cs:group_handler(%edi)     / call the proper group handler
	andb	$0x07,%ah			/ store register number (r/m)
	movw	%ax,operation_type(%ebp)	/ and operation type
	movw	%dx,op1_format(%ebp) / store operand and result
	movw	%cx,op2_format(%ebp) / formats and locations
	movw	%si,result_format(%ebp)
	movw	%di,result2_format(%ebp)
	movb	%bh,op1_use_up(%ebp)	/ store op1 and op2 useups
	movb	%bl,op2_use_up(%ebp)
	lds	saved_eip(%ebp),%esi / load instruction pointer
	mov	%esi,instruction_pointer(%ebp) / save old pointer
	movw	%ds,instruction_pointer+4(%ebp)
	xorw	%cx,%cx			/ extended addressing = 'false'
	xor	%ebx,%ebx
segment_ds:
	movw	saved_ds(%ebp),%dx	/ assume ds is pointer base
get_mod_rm_byte:
	movw	(%esi),%ax	/ load first two bytes again

	inc	%esi			/ bump instruction pointer
	andb	%al,%al			/ is there an override?
	js	got_mod_rm		/ no, ah contains mod r/m byte
	cmpb	$0x67,%al			/ is there an address overide
	jne	check_prefix66		/ no is it 66
	xorb	$1,addrs_siz32(%ebp)	/yes adjust address size
	jmp	get_mod_rm_byte
check_prefix66:
	cmpb	$0x66,%al		/is it 66 overide
	jne	other_prefix		/no some other prefix
	xorb	$1,oprnd_siz32(%ebp)	/adjust operand size
	jmp	get_mod_rm_byte
other_prefix:
	decw	%cx			/ yes, boolean = 'true'
	movw	saved_fs(%ebp),%dx	/ load fs: override
	cmpb	$0x64,%al			/ is it fs: ?
	je	get_mod_rm_byte		/ yes, scan past prefix byte
	movw	saved_gs(%ebp),%dx	/ load gs: override
	cmpb	$0x65,%al			/ is it gs: ?
	je	get_mod_rm_byte		/ yes, scan past prefix byte
	movw	saved_es(%ebp),%dx	/ load es: override
	cmpb	$0x26,%al			/ is it es: ?
	je	get_mod_rm_byte		/ yes, scan past prefix byte
	movw	saved_cs(%ebp),%dx	/ no, load cs: override
	cmpb	$0x2e,%al			/ is it cs: ?
	je	get_mod_rm_byte		/ yes, scan past prefix byte
	movw	correct_ss(%ebp),%dx			/ no, load ss: override
	cmpb	$0x36,%al			/ is it ss: ?
	je	get_mod_rm_byte		/ yes, scan past prefix byte
	jmp	segment_ds		/ no, assume ds: override
got_mod_rm:

	inc	%esi			/ bump instruction pointer
	movb	%ah,%bl			/ save mod r/m in bl

	testb	$1,addrs_siz32(%ebp)	/check if 32bit addressing
	jnz	addrs_32_bit

	movzwl    (%esi),%eax      / load possible displacement
/
/	assume	%ds:a_msrs
/
/        push    a_msrs                  / push the dynamic selector
/        pop     %ds                     /  for the a?msr segment
/        push    a_msr_selector          / and load it into ds
/        pop     %ds
/
/	assume	%ds:a_msr		; assume ds=a?msr from here on
/
	push	%eax
	movw	saved_ds(%ebp), %ax	/ restore %ds register to it's initial
	movw	%ax, %ds		/ value.
	pop	%eax

	cmpb	$0x0c0,%bl			/ is mod = 3?
	jnc	store_return		/ yes, no memory operand
	shlb    $1,%bl                  / no, is mod = 2?
	jc	mod_2			/ yes, disp = disp-hi: disp-lo
	js	mod_1			/ branch if mod = 1
	andb	$0x0e,%bl			/ mod = 0
	cmpb	$0x0c,%bl			/ does r/m = 6?
	je	store_ea		/ yes, ea = disp-hi: disp-lo
	xorw	%ax,%ax			/ no, disp = 0
	dec	%esi			/ subtract one for return
mod_1:
	dec	%esi			/ subtract one for return
	cbtw                             / extend disp to 16 bits
mod_2:
	and	$0x0000000e,%ebx		/ ax now = real displacement
	shl	$1,%ebx			/ take care of dd instead of dw
	call    *%cs:rm_handler(%ebx)    / call r/m handler (cs <> ds)
store_ea:
	inc	%esi			/ add two for return address
	inc	%esi
store_on:
	movw	%dx,mem_operand_pointer+4(%ebp) / store base
	mov	%eax,mem_operand_pointer(%ebp) / store offset
	testb	$0x80,operation_type(%ebp)	/ update sr_memop pointer?
	jnz	store_return		/ no, must be administrative
	movw	%dx,%gs:sr_mem_base		/ yes, store base in a?msr
	mov	%eax,%gs:sr_mem_offset	/ store offset in a?msr
store_return:
	mov	%esi,saved_eip(%ebp)	/ update return address
	mov	offset_op1_rec,%esi	/set up for fetch_an_op
	mov	$offset_operand1,%edi
	mov	$0, before_error_signals(%ebp)
	mov	$0, extra_dword_reg(%ebp)
	mov	$0, extra_dword_reg+4(%ebp)
	mov	$0, extra_dword_reg+8(%ebp)
	call	fetch_an_op		/ fetch op1
	pushw	%ax			/save stack error flag
	mov	offset_op2_rec,%esi	/set up for fetch_an_op
	mov	$offset_operand2,%edi
	call	fetch_an_op		/ fetch op2
	popw	%bx			/ combine stack error flags
	orw	%bx,%ax			/ set/reset zf, clear cf
	movb	operation_type(%ebp),%bl	/ load operation type
	rclb	$1,%bl			/ shift save ptrs flag to cf
	jnc	pending_unmasked_xcptns_/ all non-administratives must check
	movb	operation_type(%ebp),%cl/ duo-form administratives are all
	cmpb	$ldenv_op,%cl			/ treated as unwaited forms:
	ja	do_operation		/ fnstenv, fnstcw, fnstsw, and fnclex
	cmpb	$init_op,%cl					/ and fninit
	je	do_operation				/ and
	cmpb	$save_op,%cl		/ fnsave all ignore pending unmasked
	je	do_operation			/ exceptions and just execute.
pending_unmasked_xcptns_:
	movb	%gs:sr_masks,%cl
	notb	%cl
	andb	$0x3f,%cl
	andb	%gs:sr_errors,%cl
	jz	no_pending_unmasked_xcptns
	int	$16
	jmp	pending_unmasked_xcptns_
no_pending_unmasked_xcptns:
	testb	$0x80,operation_type(%ebp)
	jnz	do_operation
	push	%eax
	les	instruction_pointer(%ebp),%eax / load instruction ptr
	mov	%eax,%gs:sr_instr_offset	/ store offset in status reg
	movw	%es,%gs:sr_instr_base	/ store base in status reg
	pop	%eax
do_operation:
	movswl  %bx,%ebx
	rol	$1,%ebx			/take care of dd
	orw	%ax,%ax		/ give zf value expected by following call
/				This commented out at Phil's suggestion
	call    *%cs:op_table(%ebx)          / call instruction operator
	movb	%gs:sr_masks,%al		/set zf to false if any
	notb	%al			/ unmasked errors,
	andb	$0x7f,%al	/get rid of es bit
	andb	%gs:sr_errors,%al		/set al to 0 if none
	jz	release_stack
	movb	$0,%al
	movb	operation_type(%ebp),%cl
	testb	$0x80,%cl
	jz	mark_errors
	cmpb	$ldenv_op,%cl
	ja	get_done
mark_errors:
	orw	$0x8080, %gs:sr_errors	/taking care of es and B
	movb	$0x02,%al			/ load nonzero ie mask
	jmp	get_done
release_stack:
	andw	$0x7f7f, %gs:sr_errors
get_done:
/        -------------------------------------JKD
	movl	saved_eip(%ebp), %ecx       	/ restore the eip to the next
	movl	%ecx, [EIP_offset](%ebp)	/ instruction after the just
						/ executed fp instruction.


	add     global_reent_seg_length,%esp	/ release global area
	andb	saved_flags+1(%ebp),%al		/ interrupt enabled?
	jz	no_exception		/ branch if no exception
	int	$16			/ 80387 exception interrupt
no_exception:
	/ this is exactly the code labeled do_ret: in ttrap.s.
	/
	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds
	popa
	addl    $8, %esp        / get TRAPNO and ERROR off the stack

	/ User may have set NT bit before doing a system call.
	/ This would be bad, so we turn it off here.
	/ On a system call, the flags get pushed, then popped.  Since the
	/ VM bit always gets zeroed when pushed, we do not need to clear it.
	pushl	%eax
	pushfl
	movw	%ss, %ax
	cmpw	$USER_DS, %ax
	je	ret32		/ assume that USER_DS ==> 32 bit
	cmpw	$KDSSEL, %ax
	je	ret32		/ assume that KDSSEL ==> 32 bit
	movzwl	%sp, %eax	/ in case of 16 bit, avoid explicit %esp ref.
	andl	$KFL, %ss:(%eax)
	popfl
	popl	%eax
	iret
ret32:
	andl    $KFL, (%esp)    / clear the NT bit in the flags
	popfl
	popl	%eax
	iret

rm_0:
	addw	saved_ebx(%ebp),%ax	/ ea = (bx) + (si) + disp
rm_4:
	addw	saved_esi(%ebp),%ax	/ ea = (si) + disp
	ret
rm_5:
	addw	saved_edi(%ebp),%ax	/ ea = (di) + disp
	ret
rm_1:
	addw	saved_edi(%ebp),%ax	/ ea = (bx) + (di) + disp
rm_7:
	addw	saved_ebx(%ebp),%ax	/ ea = (bx) + disp
	ret
rm_2:
	addw	saved_esi(%ebp),%ax	/ ea = (bp) + (si) + disp
rm_6:
	addw	saved_ebp(%ebp),%ax	/ ea = (bp) + disp
	jcxz	segment_ss		/ branch if no override given
	ret
rm_3:
	addw	saved_edi(%ebp),%ax	/ ea = (bp) + (di) + disp
	jmp	rm_6			/ merge with r/m = 6
segment_ss:
	movw	correct_ss(%ebp),%dx	/ for r/m = 2, 3, or 6, the
	ret				/ standard base = ss
/
addrs_32_bit:	
	xor	%edi,%edi			/clear edi
	cmpb	$0xc0, %ah	
	jae	no_sib
	andb	$0x07,%ah			/check if sib present
	cmpb	$0x04,%ah
	jne	no_sib			/
	movb	(%esi),%bh		/yes get  sib byte
	inc	%esi
no_sib:
	mov	(%esi),%eax		/get displacement

	push	%ds
	pop	%fs			/ save selector
/
/	assume	%ds:a_msrs
/
/        push    a_msrs                  / push the dynamic selector
/        pop     %ds                     /  for the a?msr segment
/        push    a_msr_selector          / and load it into ds
/        pop     %ds
/
/	assume	%ds:a_msr		; assume ds=a?msr from here on
/
	push	%eax
	movw	saved_ds(%ebp), %ax	/ restore %ds register to it's initial
	movw 	%ax, %ds		/segment value.
	pop	%eax

	cmpb	$0x0c0,%bl			/ is mod = 3?
	jnc	store_return		/ yes, no memory operand
	add	$4,%esi
	shlb	$1,%bl			/ no, is mod = 2?
	jc	mod32_2			/ yes, disp = disp-hi: disp-lo
	js	mod32_1			/ branch if mod = 1
	andb	$0x0e,%bl			/ mod = 0
	inc	%edi			/ set flag
	cmpb	$0x0a,%bl			/ does r/m = 5?i
	je	store_on		/ yes, ea = disp-hi: disp-lo
	xor	%eax,%eax			/ no, disp = 0
	dec	%esi			/ subtract one for return
mod32_1:
	sub	$3,%esi			/ adjust esi
	movsbl  %al,%eax                / extend disp to 32 bits
mod32_2:
	andb	$0x0f,%bl			/check if sib present
	cmpb	$0x8,%bl
	je	process_sib
	and	$0x0000000e,%ebx		/no sib get base register
	shl	$1,%ebx			/form index in table
	call    *%cs:base32(%ebx)
	jmp	store_on		/store displacement

process_sib:	
	movb	%bh,%bl			/
	and	%edi,%edi			/edi=1 if mod=0
	jz	get_base		/
	andb	$0x07,%bh			/get last 3 bits
	cmpb	$0x05,%bh			/if mod=0 and basereg=5,no base
	jne	get_base
	mov	%fs:(%esi),%eax		/get displacement
	add	$4,%esi			/ adjust esi
	jmp	get_sir			/get sir

get_base:	
	movw	%bx,%di			/save bx
	and	$0x00000007,%ebx		/form index in the base table
	shl	$2,%ebx
	call    *%cs:base32(%ebx)
	movw	%di,%bx			/restore bx

get_sir:	
	movb	%bl,%bh			/load sib byte
	andb	$0x038,%bh			/get midle 3 bits
	cmpb	$0x020,%bh			/index=4 no index reg
	je	store_on		/if no index jmp out
	movb	%bl,%cl			/save bl
	rolb	$2,%cl			/get high order 2 bits
	andb	$0x03,%cl			/in the low bits
	shrb	$1,%bl			/form index into table
	and	$0x0000001c,%ebx
	call    *%cs:index32(%ebx)               /add sir to displacement
	jmp	store_on

base_0:	
	add	saved_eax(%ebp),%eax
	ret
base_1:
	add	saved_ecx(%ebp),%eax
	ret
base_2:
	add	saved_edx(%ebp),%eax
	ret
base_3:
	add	saved_ebx(%ebp),%eax
	ret
base_4:
	add	correct_esp(%ebp),%eax
	jcxz	segmentss
	ret
base_5:
	add	saved_ebp(%ebp),%eax
	jcxz	segmentss
	ret
segmentss:	
	movw	correct_ss(%ebp),%dx		/ for esp, ebp as base registers
	ret

base_6:
	add	saved_esi(%ebp),%eax
	ret
base_7:
	add	saved_edi(%ebp),%eax
	ret

index_0:	
	mov	saved_eax(%ebp),%edi
	shll    %cl,%edi
	add	%edi,%eax
	ret
index_1:	
	mov	saved_ecx(%ebp),%edi
	shll    %cl,%edi
	add	%edi,%eax
	ret
index_2:	
	mov	saved_edx(%ebp),%edi
	shll    %cl,%edi
	add	%edi,%eax
	ret
index_3:	
	mov	saved_ebx(%ebp),%edi
	shll    %cl,%edi
	add	%edi,%eax
	ret
index_4:
	ret
index_5:	
	mov	saved_ebp(%ebp),%edi
	shll    %cl,%edi
	add	%edi,%eax
	ret
index_6:	
	mov	saved_esi(%ebp),%edi
	shll    %cl,%edi
	add	%edi,%eax
	ret
index_7:	
	mov	saved_edi(%ebp),%edi
	shll    %cl,%edi
	add	%edi,%eax
	ret

/
/	assume	%ds:a_med
/ ****************************************************************************
/
/	instruction group decoding handlers
/
/ ****************************************************************************
/
/	these handlers load all necessary operation, operand, and result
/	information into the following registers for storage into the
/	global data records upon return:
/
/	 dh => op1_location		dl => op1_format (null)
/	 ch => op2_location		cl => op2_format (null)
/	 si => result_location/result_format
/	 di => result2_location/result2_format
/	 bh => op1_use_up (do_nothing)	bl => op2_use_up (do_nothing)
/	 ah => (mod r/m, reg byte)	al => operation_type
/
/	parentheses indicate initial values upon entry.
/
/ ****************************************************************************
/
/	group 0a - standard arithmetic instructions with short-real operands
/
arith_short_real:
	movb	$single_real,%cl		/ op2 format is short real
op1_top_op2_mem_0a:
	movb	$memory_opnd,%ch		/ op2 loc is memory operand
op1_top_stand_arith_op_0a:
	movb	group_0a_0b_2a_4a_4b_6a_6b_op(%esi),%al / arithmetic op
	movw	%dx,%di			/ result2 format is null
	movw    $[stack_top\*0x100+extended_fp],%dx / op1 top extended_fp
	movw	%dx,%si			/ result is same as op1
arith_op_0a:
	cmpb	$subr_op,%al		/ is it fsubr ?
	je	fsubr_0a		/ reverse operands
	cmpb	$divr_op,%al		/ is it fdivr ?
	je	fdivr_0a		/ reverse operands
	cmpb	$compar_pop,%al		/ is it fcomp ?
	jne	exit_0a			/ no, return
	movb	$compar_op,%al		/ yes, op type is compare
	movb	pop_stack,%bh		/ op1 useup is pop stack
exit_0a:
	ret
fsubr_0a:
	movb	$sub_op,%al		/ operation type is subtract
	jmp	swap_0a		/ switch operand locations
fdivr_0a:
	movb	$div_op,%al		/ operation type is division
swap_0a:
	xchgb	%bl,%bh			/ switch operand useups
	xchgw	%cx,%dx			/ switch operand fmt/loc
	ret
/
/	group 2a - standard arithmetic instructions with short integer operands
/
arith_short_int:
	movb	$int32,%cl		/ op2 format is short integer
	jmp	op1_top_op2_mem_0a	/ finish loading information
/
/	group 4a - standard arithmetic instructions with long-real operands
/
arith_long_real:
	movb	$double_real,%cl		/ op2 format is long real
	jmp	op1_top_op2_mem_0a	/ load rest of information
/
/	group 6a - standard arithmetic instructions with word-integer operands
/
arith_word_int:
	movb	$int16,%cl		/ op2 format is word integer
	jmp	op1_top_op2_mem_0a	/ load rest of information
/
/	group 0b - standard arithmetic instructions with inner-stack operands
/
arith_top_reg:
	movw    $[reg\*0x100+extended_fp],%cx / op2 is reg extended_fp
	jmp	op1_top_stand_arith_op_0a / load rest of information
/
/	group 6b - reversed arithmetic with register operand and pop useup
/
arith_reg_pop:
	movb	pop_stack,%bl		/ op2 useup is pop stack
/
/	group 4b - reversed arithmetic instructions with inner-stack operands
/
arith_reg_top:
	movb	group_0a_0b_2a_4a_4b_6a_6b_op(%esi),%al / arithmetic op's
	movw	%dx,%di			/ result2 format is null
	movw    $[stack_top\*0x100+extended_fp],%dx / op1 top extended_fp
	movw    $[reg\*0x100+extended_fp],%cx / op2 is reg extended_fp
	movw	%cx,%si			/ result is same as op2
	jmp	arith_op_0a		/ load rest of information
/
/	group 1a - load/store instructions with short-real operand
/		   load/store environment/control word with string operand
/
load_short_real:
	movb	group_1a_op(%esi),%al	/ load operation type
	shlw	$1,%si			/ change byte index to word
	movswl  %si,%esi
	movw	group_1a_op1_lf(%esi),%dx	/ load op1 loc/for
	movw    $[memory_opnd\*0x100+single_real],%si / result format
load_store_1a:
	movw	%cx,%di			/ result2 format is null
	cmpb	$load_op,%al		/ is it fld?
	je	fld_1a			/ result goes to stack top
	cmpb	$store_pop,%al		/ is it fstp?
	je	fstp_1a			/ op1 useup is pop stack
	cmpb	$store_op,%al		/ is it fst?
	je	fst_1a			/ result goes to memory
	movw	%di,%si			/ else, result format is null
	ret
fld_1a:
	movw    $[stack_top_plus_1\*0x100+extended_fp],%si / result is top
	ret
fstp_1a:
	movb	pop_stack,%bh		/ op1 useup is pop stack
	movb	$store_op,%al		/ operation type is store
fst_1a:
	ret
/
/	group 3a - short integer/tempreal load/store instructions
/
load_short_int:
	movb	group_3a_7a_op(%esi),%al	/ load operation type
	movb	group_3a_res_fmt(%esi),%dl	/ load result format
	movb	group_3a_7a_res_loc(%esi),%dh / load result location
	pushw	%dx			/ push result fmt/loc
	movb	group_3a_op1_fmt(%esi),%dl	/ load op1 format
	movb	group_3a_7a_op1_loc(%esi),%dh / load op1 location
	popw	%si			/ pop result fmt/loc
	jmp	load_store_1a		/ finish loading information
/
/	group 5a - load/store instructions with long-real operand
/                  load/store state/status word instructions
/
load_long_real:
	movb	group_5a_op(%esi),%al	/ load operation type
	shlw	$1,%si			/ change byte index to word
	movswl  %si,%esi
	movw	group_5a_op1_lf(%esi),%dx	/ load op1 loc/for
	movw    $[memory_opnd\*0x100+double_real],%si / result format
	jmp	load_store_1a		/ load rest of information
/
/	group 7a - load/store word-integer/bcd/long-integer instructions
/
load_word_int:
	movb	group_3a_7a_op(%esi),%al	/ load operation type
	movb	group_7a_res_fmt(%esi),%dl	/ load result format
	movb	group_3a_7a_res_loc(%esi),%dh / load result location
	pushw	%dx			/ push result fmt/loc
	movb	group_7a_op1_fmt(%esi),%dl	/ load op1 format
	movb	group_3a_7a_op1_loc(%esi),%dh / load op1 location
	popw	%si			/ pop result fmt/loc
	jmp	load_store_1a		/ finish loading information
/
/	group 1b - load/store/transcendental instructions with inner
/		   stack operands
transcendentals:
	testb	$0x20,%ah			/ does bit 5 = 0?
	jz	group_1ba		/ yes, it's a load/store op
	movw	$0x1f00,%si		/ no, it's a transcendental
	andw	%ax,%si			/ calculate new table index
	shrw	$8,%si			/ shift right one byte
	movswl  %si,%esi
	movb	group_1bb_type(%esi),%al	/ load format/location type
	rolb	$1,%al			/ rotate one left
	mov	$0x0004,%ecx		/ load loop count
load_lf_1bb:
	rolb	$2,%al			/ shift next field into di
	movw	$0x0006,%di		/ form table index
	andw	%ax,%di
	movswl  %di,%edi
	pushw	group_1bb_lf(%edi)	/ stack next format/location
	loop	load_lf_1bb		/ decode four fields
	movb	group_1bb_op(%esi),%al	/ load operation type
	cmpb	$log_op,%al		/ is it fyl2x?
	je	pop_op1_1bb		/ yes, operand 1 gets popped
	cmpb	$splog_op,%al		/ is it fyl2xp1?
	je	pop_op1_1bb		/ yes, operand 1 gets popped
	cmpb	$arctan_op,%al		/ is it fpatan?
	jne	return_1bb		/ no, load return information
pop_op1_1bb:
	movb	pop_stack,%bh		/ pop operand 1
return_1bb:
	popw	%di			/ load operand 1 information
	popw	%si			/ load operand 2 information
	popw	%cx			/ load result 1 information
	popw	%dx			/ load result 2 information
	ret
group_1ba:
	movw	%cx,%di			/ assume result2 always null
	movb	group_1ba_op(%esi),%al	/ load operation type
load_store_1ba:
	cmpb	$load_op,%al		/ is it fld st(i) ?
	je	fld_1ba			/ op1 is a register
	cmpb	$exchange_op,%al		/ is it fxch st(i) ?
	je	fxch_1ba		/ op1 is stack top
	cmpb	$store_pop,%al		/ is it fstp st(i)? *
	je	fstp_1ba		/ pop op1
	cmpb	$store_op,%al		/ is it fst st(i)?
	je	fst_1ba			/ don't pop op1
	movw	%di,%si			/ result format is null
	ret
fld_1ba:
	movw    $[reg\*0x100+extended_fp],%dx / op1 is a stack reg
	movw    $[stack_top_plus_1\*0x100+extended_fp],%si / result pushed
	ret
fxch_1ba:
	movw    $[stack_top\*0x100+extended_fp],%dx / op1 is stack top
	movw    $[reg\*0x100+extended_fp],%cx / op2 is a stack reg
	movw	%cx,%si			/ result is the stack reg
	movw	%dx,%di			/ result2 is the stack top
	ret
fstp_1ba:
	movb	$store_op,%al		/ operation type is store
	movb	pop_stack,%bh		/ op1 useup is pop stack
fst_1ba:
	movw    $[stack_top\*0x100+extended_fp],%dx / op1 is stack top
	movw    $[reg\*0x100+extended_fp],%si / result is stack reg
	ret
/
/	group 5b - free register/store to register instructions
/
store_reg:
	movb	group_5b_op(%esi),%al	/ load operation type
	cmpb	$ucom_pop,%al
	jne	test_ucom_op
	movb	pop_stack,%bh
	movb	$ucom_op,%al
test_ucom_op:
	cmpb	$ucom_op,%al
	jne	test_ffree_5b
	movw    $[reg\*0x100+extended_fp],%cx / op2 is reg extended_fp
	movw	%dx,%di			/ result2 format is null
	movw	%dx,%si			/ result format is null
	movw    $[stack_top\*0x100+extended_fp],%dx / op1 top extended_fp
	ret
test_ffree_5b:
	cmpb	$free_op,%al		/ is it ffree st(i)?
	jne	load_store_1ba		/ no, must be fst(p)/fxch
	movb	free,%bh			/ yes, op1 useup is free
	jmp	load_store_1ba		/ load rest of information
/
/	group 7b - transfer status instruction
/
transfer_status:
	movb	group_7b_op(%esi),%al	/ load operation type
	cmpb	$free_op,%al		/ is it ffreep st(i) ?
	jne	not_ffreep_7b		/ no, useup is do nothing
	movb	pop_stack,%bl		/ yes, op1 useup is pop
not_ffreep_7b:
	cmpb	$stsw_op,%al		/ is it fstsw ax ?
	jne	test_ffree_5b		/ no, handle like group 5b
	movb	$reg,%ch		/ yes, op2 loc is 'reg'
	jmp	test_ffree_5b		/ finish loading information
/
/	group 2b - fucompp
/
double_pop:	
	cmpb	$0x029,%ah			/ is it fucompp
	jne	reserved		/ is a reserved instruction
	movb	pop_stack,%bl		/ op2 useup is pop stack
	movb	pop_stack,%bh
	movb	$ucom_op,%al
	movw    $[reg\*0x100+extended_fp],%cx / op2 is reg extended_fp
	movw	%dx,%di			/ result2 format is null
	movw	%dx,%si			/ result format is null
	movw    $[stack_top\*0x100+extended_fp],%dx / op1 top extended_fp
	ret
/
/	group 2b - reserved
/
reserved:
	movb	$error_op,%al		/ illegal operation
null_results_2b:
	movw	%cx,%si			/ result is null
	movw	%cx,%di			/ result2 is null
	ret
/
/	group 3b - administrative instructions
/
administrative:
	movw	$0x0700,%si		/ calculate new table index
	andw	%ax,%si
	shrw	$8,%si			/ shift right one byte
	movswl  %si,%esi
	movb	group_3b_op(%esi),%al	/ load operation type
	jmp	null_results_2b		/ result and result2 are null
/
/e80387	endp
/
/a_med	ends
/
/
/	fp_emul_present is a routine which will always return 1 indicating
/	the emulator is present, at least in this hacked up version
	.globl	fp_emul_present
fp_emul_present:
	movw	$1, %ax
	ret

/	end
