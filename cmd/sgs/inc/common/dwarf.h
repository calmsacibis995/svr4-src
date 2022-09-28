/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-inc:common/dwarf.h	1.1"

#ifndef DWARF_H
#define DWARF_H

/* dwarf.h - manifest constants used in the .debug section of ELF files */


/* the "tag" - the first short of any legal record */

#define	TAG_padding			0x0000
#define	TAG_array_type			0x0001
#define	TAG_class_type			0x0002
#define	TAG_entry_point			0x0003
#define	TAG_enumeration_type		0x0004
#define	TAG_formal_parameter		0x0005
#define	TAG_global_subroutine		0x0006
#define	TAG_global_variable		0x0007
#define	TAG_imported_declaration	0x0008	
#define	TAG_inline_subroutine		0x0009
#define	TAG_label			0x000a
#define	TAG_lexical_block		0x000b
#define	TAG_local_variable		0x000c
#define	TAG_member			0x000d
#define	TAG_member_function		0x000e
#define	TAG_pointer_type		0x000f
#define	TAG_reference_type		0x0010
#define	TAG_source_file			0x0011
#define	TAG_string_type			0x0012
#define	TAG_structure_type		0x0013
#define	TAG_subroutine			0x0014
#define	TAG_subroutine_type		0x0015
#define TAG_typedef			0x0016
#define	TAG_union_type			0x0017
#define	TAG_unspecified_parameters	0x0018
#define	TAG_variant			0x0019


/* attribute forms are encoded as part */
/* of the attribute name and must fit */
/* into 4 bits */

#define	FORM_MASK	0xf

#define	FORM_NONE	0x0	/* error */
#define	FORM_ADDR	0x1	/* relocated address */
#define	FORM_REF	0x2	/* reference to another .debug entry */
#define	FORM_BLOCK2	0x3	/* block with 2-byte length */
#define	FORM_BLOCK4	0x4	/* block with 4-byte length (unused) */
#define	FORM_DATA2	0x5	/* 2 bytes */
#define	FORM_DATA4	0x6	/* 4 bytes */
#define	FORM_DATA8	0x7	/* 8 bytes (two 4-byte values) */
#define	FORM_STRING	0x8	/* NUL-terminated string */


/* attribute names, halfwords with low 4 bits indicating the form */

#define	AT_padding	 (0x0000|FORM_NONE)	/* just padding */
#define	AT_sibling	 (0x0010|FORM_REF)	/* next owned declaration */
#define	AT_location	 (0x0020|FORM_BLOCK2)	/* location description */
#define	AT_name		 (0x0030|FORM_STRING)	/* symbol name */
#define	AT_dimensions	 (0x0040|FORM_DATA2)	/* array dimensions */
#define	AT_fund_type	 (0x0050|FORM_DATA2)	/* fund type enum */
#define	AT_mod_fund_type (0x0060|FORM_BLOCK2)	/* modifiers & fund type enum */
#define	AT_user_def_type (0x0070|FORM_REF)	/* type entry */
#define	AT_mod_u_d_type  (0x0080|FORM_BLOCK2)	/* modifiers & type entry ref */
#define	AT_ordering	 (0x0090|FORM_DATA2)	/* array row/column major */
#define	AT_subscr_data	 (0x00a0|FORM_BLOCK2)	/* list of array dim info */
#define	AT_byte_size	 (0x00b0|FORM_DATA4)	/* number bytes per instance */
#define	AT_bit_offset	 (0x00c0|FORM_DATA2)	/* number bits padding */
#define	AT_bit_size	 (0x00d0|FORM_DATA4)	/* number bits per instance */
#define	AT_deriv_list	 (0x00e0|FORM_BLOCK2)	/* list of base class refs */
#define	AT_element_list	 (0x00f0|FORM_BLOCK2)	/* list of enum data elements */
#define	AT_stmt_list	 (0x0100|FORM_DATA4)	/* offset in .line sect */
#define	AT_low_pc	 (0x0110|FORM_ADDR)	/* first machine instr */
#define	AT_high_pc	 (0x0120|FORM_ADDR)	/* beyond last machine instr */
#define	AT_language	 (0x0130|FORM_DATA4)	/* compiler enumeration */
#define	AT_member	 (0x0140|FORM_REF)	/* class description */
#define	AT_discr	 (0x0150|FORM_REF)	/* discriminant entry */
#define	AT_discr_value	 (0x0160|FORM_BLOCK2)	/* value of discr */
#define	AT_visibility	 (0x0170|FORM_DATA2)	/* visibility enumeration */
#define	AT_import	 (0x0180|FORM_REF)	/* imported declaration */
#define	AT_string_length (0x0190|FORM_BLOCK2)	/* runtime string size */


/* atoms which compose a location description; must fit in a byte */

#define	OP_UNK		0x00	/* error */
#define	OP_REG		0x01	/* push register (number) */
#define	OP_BASEREG	0x02	/* push value of register (number) */
#define	OP_ADDR		0x03	/* push address (relocated address) */
#define	OP_CONST	0x04	/* push constant (number) */
#define	OP_DEREF2	0x05	/* pop, deref and push 2 bytes (as a long) */
#define	OP_DEREF4	0x06	/* pop, deref and push 4 bytes (as a long) */
#define	OP_ADD		0x07	/* pop top 2 items, add, push result */

/* fundamental types; must fit in two bytes */

#define	FT_none			0x0000	/* error */
#define	FT_char			0x0001	/* "plain" char */
#define	FT_signed_char		0x0002
#define	FT_unsigned_char	0x0003
#define	FT_short		0x0004	/* "plain" short */
#define	FT_signed_short		0x0005
#define	FT_unsigned_short	0x0006
#define	FT_integer		0x0007	/* "plain" integer */
#define	FT_signed_integer	0x0008
#define	FT_unsigned_integer	0x0009
#define	FT_long			0x000a	/* "plain" long */
#define	FT_signed_long		0x000b
#define	FT_unsigned_long	0x000c
#define	FT_pointer		0x000d	/* (void *) */
#define	FT_float		0x000e
#define	FT_dbl_prec_float	0x000f
#define	FT_ext_prec_float	0x0010
#define	FT_complex		0x0011
#define	FT_dbl_prec_complex	0x0012
#define	FT_set			0x0013
#define	FT_void			0x0014


/* type modifiers; must fit in a byte */

#define	MOD_none		0x00	/* error */
#define	MOD_pointer_to		0x01
#define	MOD_reference_to	0x02


/* the "format" byte for array descriptions; formed from three */
/* one-bit fields */

#define	FMT_FT	0		/* fundamental type */
#define	FMT_UDT	1		/* user-defined type */

#define	FMT_CONST	0	/* 4-byte constant */
#define	FMT_EXPR	1	/* block with 2-byte length (loc descr) */

#define	FMT_FT_C_C	( (FMT_FT <<2) | (FMT_CONST<<1) | (FMT_CONST) )
#define	FMT_FT_C_X	( (FMT_FT <<2) | (FMT_CONST<<1) | (FMT_EXPR)  )
#define	FMT_FT_X_C	( (FMT_FT <<2) | (FMT_EXPR <<1) | (FMT_CONST) )
#define	FMT_FT_X_X	( (FMT_FT <<2) | (FMT_EXPR <<1) | (FMT_EXPR)  )
#define	FMT_UT_C_C	( (FMT_UDT<<2) | (FMT_CONST<<1) | (FMT_CONST) )
#define	FMT_UT_C_X	( (FMT_UDT<<2) | (FMT_CONST<<1) | (FMT_EXPR)  )
#define	FMT_UT_X_C	( (FMT_UDT<<2) | (FMT_EXPR <<1) | (FMT_CONST) )
#define	FMT_UT_X_X	( (FMT_UDT<<2) | (FMT_EXPR <<1) | (FMT_EXPR)  )

#define	FMT_ET		8	/* element type */


/* ordering of arrays */

#define	ORD_row_major	0
#define ORD_col_major	1


/* visibility values */

#define	VIS_local	0	/* for static functions in C */
#define VIS_exported	1	/* for Modula */


/* language/compiler enumeration */

typedef enum _LANG {
	LANG_UNK = 0,
	LANG_ANSI_C_V1 = 1
} LANG;

#endif /* DWARF_H */
