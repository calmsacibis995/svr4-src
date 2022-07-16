/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-fp:e80387.h	1.1"

#include "symvals.h"
#include "sizes.h"

/* ...definition of global reentrant data segment      */

/* global_record_1         struc */

#define	mem_operand_pointer     	0	/* dp */
#define	instruction_pointer     	6	/* dp */
#define	operation_type          	12	/* .byte */
#define	reg_num                 	13	/* .byte */

#define	op1_format              	14	/* .byte */
#define	op1_location            	15	/* .byte */
#define	op1_use_up              	16	/* .byte */

#define	op2_format              	17	/* .byte */
#define	op2_location            	18	/* .byte */
#define	op2_use_up              	19	/* .byte */

#define	result_format           	20	/* .byte */
#define	result_location         	21	/* .byte */

#define	result2_format          	22	/* .byte */
#define	result2_location        	23	/* .byte */

#define	dword_frac1             	24	/* 3 dup(_) */
#define	sign1                   	36	/* .byte */
#define	sign1_ext               	37	/* .byte */
#define	tag1                    	38	/* .byte */
#define	tag1_ext                	39	/* .byte */
#define	expon1                  	40	/* .value */
#define	expon1_ext              	42	/* .value */

#define	dword_frac2             	44	/* 3 dup(_) */
#define	sign2                   	56	/* .byte */
#define	sign2_ext               	57	/* .byte */
#define	tag2                    	58	/* .byte */
#define	tag2_ext                	59	/* .byte */
#define	expon2                  	60	/* .value */
#define	expon2_ext              	62	/* .value */

#define	before_error_signals		64	/* .long  */
#define	extra_dword_reg         	68	/* 3 dup(_) */
#define	result_dword_frac       	80	/* 3 dup(_) */
#define	result_sign             	92	/* .byte */
#define	result_sign_ext         	93	/* .byte */
#define	result_tag              	94	/* .byte */
#define	result_tag_ext          	95	/* .byte */
#define	result_expon            	96	/* .value */
#define	result_expon_ext        	98	/* .value */

#define	result2_dword_frac      	100	/* 3 dup(_) */
#define	result2_sign            	112	/* .byte */
#define	result2_sign_ext        	113	/* .byte */
#define	result2_tag             	114	/* .byte */
#define	result2_tag_ext         	115	/* .byte */
#define	result2_expon           	116	/* .value */
#define	result2_expon_ext       	118	/* .value */
/  global_record_1         ends

/  global_record_2         struc

#define	dword_cop               	120	/* 3 dup(_) */
#define	dword_dop               	132	/* 3 dup(_) */

#define	oprnd_siz32             	144	/* .byte */
#define	addrs_siz32             	145	/* .byte */
#define	correct_ss              	146	/* .value */
#define	correct_esp             	148	/* dd */

#define	saved_gs                	152	/* .value */
#define	is16bit                  	154	/* .value */
#define	fill_1                  	155	/* .value */
#define	saved_fs                	156	/* .value */
#define	fill_2                  	158	/* .value */
#define	saved_es                	160	/* .value */
#define	fill_3                  	162	/* .value */
#define	saved_ds                	164	/* .value */
#define	fill_4                  	166	/* .value */
#define	saved_edi               	168	/* dd */
#define	saved_esi               	172	/* dd */
#define	saved_ebp               	176	/* dd */
#define	saved_esp               	180	/* dd */
#define	saved_ebx               	184	/* dd */
#define	saved_edx               	188	/* dd */
#define	saved_ecx               	192	/* dd */
#define	saved_eax               	196	/* dd */
#define	saved_eip               	200	/* dd */
#define	saved_cs                	204	/* .value */
#define	fill_5                  	206	/* .value */
#define	saved_flags             	208	/* dd */
#define	old_esp                  	212	/* .value */
#define	old_ss                  	216	/* .value */
#define	fill_6                  	218	/* .value */
/  global_record_2         ends

#define	global_reent_seg_length	$GRSL

/*	offset_gs and offset_eax are offsets from ebp for the registers
	saved on entry to the emulator.  these are the user's registers,
	and will be reinstated on exit from the emulator.  therefore,
	any writes to these locations will be reflected in the 
	restored environment.
*/

#define		offset_gs	0 + 220		/* global_reent_seg_length */
#define		offset_eax	11 \* 4 + 220	/* offset of user's eax */
#define		int_esp		17 \* 4 + 220	/* user esp at int. */
#define	KDSSEL	0x160		/* This is the value assigned to the kernel DS
				  and needs to be modified if the kernel value
				  ever changes		*/

/*     convenience definitions     */
/*  new ones -387     */

#define         word_frac1                      [dword_frac1+2](%ebp)
#define         word_frac2                      [dword_frac2+2](%ebp)
#define         result_word_frac                [result_dword_frac+2](%ebp)
#define         result2_word_frac               [result2_dword_frac+2](%ebp)

#define		dword_expon1			[expon1](%ebp)
#define		dword_expon2			[expon2](%ebp)
#define		dword_result_expon		[result_expon](%ebp)
#define		dword_result2_expon		[result2_expon](%ebp)

#define         dop                             [dword_dop+2](%ebp)
#define         cop                             [dword_cop+2](%ebp)
#define         extra_word_reg                  [extra_dword_reg+2](%ebp)
#define		signal_i_error_			[before_error_signals](%ebp)
#define		signal_z_error_			[before_error_signals+1](%ebp)
#define		signal_d_error_			[before_error_signals+2](%ebp)

#define         offset_op1_rec                  $op1_format
#define         offset_op2_rec                  $op2_format
#define         offset_result_rec               $result_format
#define         offset_result2_rec              $result2_format
#define         offset_operand1                 dword_frac1
#define         lsb_frac1                       word_frac1
#define         msb_frac1                       9+word_frac1
#define         offset_operand2                 dword_frac2
#define         lsb_frac2                       word_frac2
#define         msb_frac2                       9+word_frac2
#define         offset_result                   result_dword_frac
#define         lsb_result                      result_word_frac
#define         msb_result                      9+result_word_frac
#define         offset_result2                  result2_dword_frac
#define         lsb_result2                     result2_word_frac
#define         msb_result2                     9+result2_word_frac
#define         offset_cop                      dword_cop
#define         lsb_cop                         cop
#define         msb_cop                         9+lsb_cop
#define         offset_dop                      dword_dop
#define         lsb_dop                         dop
#define         msb_dop                         9+lsb_dop
#define         q                               [mem_operand_pointer](%ebp)
#define         hi_q                            [mem_operand_pointer+1](%ebp)
#define         exp_tmp                         [mem_operand_pointer](%ebp)
#define         siso                            [mem_operand_pointer+4](%ebp)
#define         loop_ct                         [mem_operand_pointer+2](%ebp)
#define		log_loop_ct			[mem_operand_pointer](%ebp)
#define         bit_ct                          [instruction_pointer](%ebp)
#define         added_one                       [instruction_pointer+1](%ebp)
#define         rnd_history                     [instruction_pointer+2](%ebp)
#define         rnd1_inexact                    [instruction_pointer+2](%ebp)
#define         rnd2_inexact                    [instruction_pointer+3](%ebp)

#define		dword_exp_tmp			[before_error_signals](%ebp)
#define		trans_info			[extra_dword_reg](%ebp)
#define		op_type				[extra_dword_reg](%ebp)
#define		sin_sign			[extra_dword_reg+1](%ebp)
#define		cos_sign			[extra_dword_reg+2](%ebp)
#define		cofunc_flag			[extra_dword_reg+3](%ebp)
#define		res_signs			[extra_dword_reg+1](%ebp)
/* ...define structure of operand info and result info structures...     */

#define         format          0
#define         location        1
#define         use_up          2

/* ...define format codes...     */

#define         single_real     0
#define         double_real     1
#define         int16           2
#define         int32           3
#define         int64           4
#define         extended_fp     5
#define         null            7
#define         bcd             8
#define         op_no_result    9
#define         no_op_result    10
#define         op2_pop         11
#define         op2_no_pop      12
#define         result2         13
#define         op1_top_op2     14

/* ...define operation type codes...     */

#define         save_ptrs       0x80
#define         load_op         0
#define         store_op        1
#define         chsign_op       2
#define         compar_op       3
#define         error_op        4+save_ptrs
#define         add_op          5
#define         sub_op          6
#define         mul_op          7
#define         div_op          8
#define         ldcw_op         9+save_ptrs
#define         save_op         10+save_ptrs
#define         restore_op      11+save_ptrs
#define         null_op         12+save_ptrs
#define         abs_op          13
#define         init_op         14+save_ptrs
#define         fxtrac_op       15
#define         log_op          16
#define         splog_op        17
#define         exp_op          18
#define         tan_op          19
#define         arctan_op       20
#define         sqrt_op         21
#define         remr_op         22
#define         intpt_op        23
#define         scale_op        24
#define         exchange_op     25
#define         free_op         26
#define         ldenv_op        27+save_ptrs
#define         stenv_op        28+save_ptrs
#define         stcw_op         29+save_ptrs
#define         stsw_op         30+save_ptrs
#define         load_1_op       31
#define         load_l2t_op     32
#define         load_l2e_op     33
#define         load_pi_op      34
#define         load_lg2_op     35
#define         load_ln2_op     36
#define         load_0_op       37
#define         decstp_op       38
#define         incstp_op       39
#define         cler_op         40+save_ptrs
#define         test_op         41
#define         exam_op         42
#define         sin_op          43
#define         cos_op          44
#define         sincos_op       45
#define         rem1_op         46
#define         ucom_op         47
#define         compar_pop      48
#define         subr_op         49
#define         divr_op         50
#define         store_pop       51
#define         ucom_pop        52

/* ...definition of register tag codes...     */

#define         valid                    $0
#define         VALID                    0
#define         special                  $1
#define		SPECIAL			 1
#define         inv                      $2
#define         empty                    3
#define         denormd                  $6
#define         infinty                  $0x0a
#define		INFINITY		 0x0a
#define         unsupp                   $0x12 

/* ...infinity_control settings...     */

#define         projective              $0
#define         affine                  $1

/* ...define location codes...     */

#define         memory_opnd             0
#define         stack_top               1
#define         stack_top_minus_1       2
#define         stack_top_plus_1        3
#define         reg                     4

/* ...define use_up codes...     */

#define         free            $0
#define         pop_stack       $1
#define         do_nothing      2

/* ...definition of operand and result records in global reent seg...     */

#define         frac80  2
#define         frac64  4
#define         frac32  8
#define         msb     11
#define         sign    12
#define         tag     14
#define         expon   16

/* ...definition of precision codes...     */

#define         prec24                   $0
#define         prec32                   $1
#define         prec53                   $2
#define         prec64                   $3
#define         prec16                   $4

/* ...define round control codes...     */

#define         rnd_to_even                      $0x0000
#define         rnd_down                         $0x0004
#define         rnd_up                           $0x0008
#define         rnd_to_zero                      $0x000c

/* ...define positive and negative...     */

#define         positive                        $0x0000
#define		POSITIVE			 0x0000
#define         negative                        $0x00ff
#define         NEGATIVE                         0x00ff

/* ...definition of true and false...     */

#define         true                    $0x00ff
#define         false                   $0x0000

#define         wrap_around_constant            $0x6000
#define         no_change                       $0x00f0
#define         exponent_bias                   0x3fff

#define         single_exp_offset               $0x3f80
#define         double_exp_offset               $0x3c00

#define         zero_mask                       0x40
#define         sign_mask                       0x01
#define         a_mask                          0x02
#define         c_mask                          0x04
#define         inexact_mask                    0x20
#define         underflow_mask                  $0x10
#define         overflow_mask                   $0x08
#define         zero_divide_mask                $0x04
#define         invalid_mask                    $0x01
#define         denorm_mask                     $0x02
#define         top_mask                        0x38
#define         precision_mask                  0x03
#define         rnd_control_mask                0x0c
#define         infinity_control_mask           $0x10
#define         high_extended_expon_for_single  $0x407e
#define         low_extended_expon_for_single   $0x3f81
#define         high_extended_expon_for_double  $0x43fe
#define         low_extended_expon_for_double   $0x3c01
#define         high_int16_exponent     $0x400e
#define         max_int16_shift         $17
#define         high_int32_exponent     $0x401e
#define         max_int32_shift         $33
#define         high_int64_exponent     $0x403e
#define         max_int64_shift         $65
/*	the least scale term for which extreme overflow is certain */
#define		least_sf_xtrm_ovfl	$0x0000e03d
/*	the greatest scale term for which extreme underflow is certain */
#define		grtst_sf_xtrm_unfl	$0xffff2002

/*	These values set up for referencing the status registers in u_block */

/*
/       The following structure depicts the Status Register structure
/       for the 80387 emulator.
/
/               +-----------------------------------------------+
/            0  |       sr_masks        |       sr_controls     |
/               +-----------------------------------------------+
/            2  |               sr_reserved1                    |
/               +-----------------------------------------------+
/            4  |       sr_errors       |       sr_flags        |
/               +-----------------------------------------------+
/            6  |               sr_reserved2                    |
/               +-----------------------------------------------+
/            8  |                    sr_tags                    |
/               +-----------------------------------------------+
/           10  |               sr_reserved3                    |
/               +-----------------------------------------------+
/           12  |               sr_instr_offset                 |
/               +                                               +
/           14  |                                               |
/               +-----------------------------------------------+
/           16  |               sr_instr_base                   |
/               +-----------------------------------------------+
/           18  |               sr_reserved4                    |
/               +-----------------------------------------------+
/           20  |               sr_mem_offset                   |
/               +                                               +
/           22  |                                               |
/               +-----------------------------------------------+
/           24  |               sr_mem_base                     |
/               +-----------------------------------------------+
/           26  |               sr_reserved5                    |
/               +-----------------------------------------------+
/           28  |               sr_regstack                     |
/               +-----------------------------------------------+
/           30  |                                               |
/               +-----------------------------------------------+
/                                       .
/
/                                       .
/
/                                       .
/               +-----------------------------------------------+
/           106 |                                               |
/               +-----------------------------------------------+
/
/
/
/
/	The status register memory allocation for the emulator is found at
/	offset u_fpstate into the user's u block structure.
/
/
/...define the 80387 status register...
/								*/
/ #define sr_masks 	u + u_fpstate + 0 	
/ #define sr_controls	u + u_fpstate + 1 
/ #define sr_reserved1 	u + u_fpstate + 2 	
/ #define sr_errors 	u + u_fpstate + 4 	
/ #define sr_flags 	u + u_fpstate + 5 	
/ #define sr_reserved2 	u + u_fpstate + 6 	
/ #define sr_tags 	u + u_fpstate + 8 	
/ #define sr_reserved3 	u + u_fpstate + 10 	
/ #define sr_instr_offset u + u_fpstate + 12
/ #define sr_instr_base 	u + u_fpstate + 16
/ #define sr_reserved4 	u + u_fpstate + 18
/ #define sr_mem_offset 	u + u_fpstate + 20	
/ #define sr_mem_base 	u + u_fpstate + 24	
/ #define sr_reserved5 	u + u_fpstate + 26	
/ #define sr_regstack 	u + u_fpstate + 28	
#define sr_masks 	u_fps - u_fpvalid + 0 	
#define sr_controls	u_fps - u_fpvalid + 1 
#define sr_reserved1 	u_fps - u_fpvalid + 2 	
#define sr_errors 	u_fps - u_fpvalid + 4 	
#define sr_flags 	u_fps - u_fpvalid + 5 	
#define sr_reserved2 	u_fps - u_fpvalid + 6 	
#define sr_tags 	u_fps - u_fpvalid + 8 	
#define sr_reserved3 	u_fps - u_fpvalid + 10 	
#define sr_instr_offset u_fps - u_fpvalid + 12
#define sr_instr_base 	u_fps - u_fpvalid + 16
#define sr_reserved4 	u_fps - u_fpvalid + 18
#define sr_mem_offset 	u_fps - u_fpvalid + 20	
#define sr_mem_base 	u_fps - u_fpvalid + 24	
#define sr_reserved5 	u_fps - u_fpvalid + 26	
#define sr_regstack 	u_fps - u_fpvalid + 28	
#define fpvalid		0
/

#define	EIP_offset	14\*4 + 220  /* EIP that will be loaded on iret */

/	.globl	sr_mem_offset
/	.globl	sr_mem_base
/	.globl	a_msr_data
/	.globl	sr_regstack
/	.globl	sr_masks
/	.globl	sr_errors
/	.globl	sr_instr_offset
/	.globl	sr_instr_base

/	WARNING, if the values in immu.h or seg.h change,
/		 these values may have to change as well.

#define USER_DS	0x1F
#define MINUVADR 0x00000000
#define MAXUVADR 0xC0000000
