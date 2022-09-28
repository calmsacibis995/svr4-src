/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dump:common/debug.c	1.11"

/* This file contains all of the functions necessary
 * to interpret debugging information and line number information.
 */

#include        <stdio.h>
#include        <malloc.h>

#include	"libelf.h"
#include        "sgs.h"
#include	"dwarf.h"
#include	"dump.h"
#include 	"ccstypes.h"
#include <sys/elf_M32.h>
#include <sys/elf_386.h>


extern char *prog_name;
extern VOID_P get_scndata();
extern int p_flag;
extern SCNTAB *p_debug, *p_line;


static		unsigned char	*p_debug_data, *p_line_data, *ptr;
static 		long		length = 0;
static 		int		no_elemtype = 0;
static		long		current, nextoff;

static		long	get_long();
static		short	get_short();
static		char	get_byte();
static unsigned char	*get_string();
static		void	print_record(),
			not_interp(),
			user_def_type(),
			mod_fund_type(),
			location(),
			mod_u_d_type();
static unsigned	char	*the_string;

static char *	lookuptag();
static char *	lookupattr();
static char *	lookupfmt();
static char *	lang();
static char *	fund_type();
static char *	modifier();
static char *	op();
static int      has_arg();
static char *	order();
static char *	vis();
static void     element_list();
static void     subscr_data();
static void     line_info();
static void     print_line();



/*
 * Get the debugging data and call print_record to process it.
 */

void
dump_debug(elf, filename)
Elf *elf;
char * filename;
{

	extern SCNTAB *p_debug, *p_line;
        Elf_Scn         *scn;
	size_t		dbg_size;

	if(!p_flag)
	{
		(void)printf("    ***** DEBUGGING INFORMATION *****\n");
		(void)printf("%-12s%-16s%-20s%s\n",
			"EntSize",
			"Tag",
			"Attribute",
			"Value");
	}

	if ( (p_debug_data = (unsigned char *)get_scndata(p_debug->p_sd, &dbg_size)) == NULL)
	{
		(void)fprintf(stderr, "%s: %s: no data in %s section\n",
			prog_name, filename, p_debug->scn_name);
		return;
	}

	ptr = p_debug_data;
	print_record(p_debug->p_shdr->sh_offset, dbg_size);

}


/*
 * Process debugging information by reading in set numbers of
 * bytes.  Since the debugging information is not contained
 * in structures, each set of bytes provides information for the
 * interpretation and size of the next set of bytes.
 * This code is machine dependent and will require updates to
 * any functions that read bytes (get_long, get_short, get_byte,
 * get_string) to be valid on machines with different byte ordering.
 * 
 * Debugging information is organized into records.  The first 4
 * bytes of a record provide the total size of that record.  The
 * next two bytes contain a tag value.  The next two bytes contain
 * an attribute value.  Each attribute has an associated data format
 * and type of information.  The number of bytes to be read following
 * the attribute information is determined by the data format and
 * can be seen by looking through the case statement of attributes
 * (AT_...).  The relevation functions are called for each attribute
 * listed and a not_interp function is provided which gives
 * uninterpreted numerical output for those attributes not in
 * the case statement.  Some attributes may not have been implemented
 * and it is easy to determine them since the only function called
 * is not_interp.
 */

static void
print_record( offset, size)
Elf32_Off	offset;
size_t		size;
{
	long	word;
	short	sword;
	short 	tag;
	short	attrname;
	short   len2;

  current = 0;
  while ( current < size)
  { 
	(void)printf("\n");
	word = get_long();

	if ( word <= 8)
	{
		(void)printf("\n0x%-10lx\n", word);
		if(word < 4)
		{
			current += 4;
		}
		else
		{
			current += word;
			ptr += word - 4;
		}
		continue;
	}
	else
		current += word;

	(void)printf("\n0x%-10lx", word);
	length = word - 6;
	nextoff = offset + word;

	tag = get_short();
	(void)printf("%-16s", lookuptag(tag) );

	
	while(length > 0)
	{
		attrname = get_short();
		(void)printf("%-20s", lookupattr(attrname) );
		switch( attrname )
		{
			case AT_padding:
				(void)printf("(FORM_NONE)\n");
                                break;
                        case AT_sibling:
                                word = get_long();
				if(word != 0)
					(void)printf("0x%lx\n", word);
				else
					(void)printf("\n");
				break;
			case AT_location:
			case AT_string_length:
				location();
                                break;
                        case AT_name:
				the_string = get_string();
                                (void)printf("%s\n",the_string);
                                break;
                        case AT_dimensions:
                                len2 = get_short();
				(void)printf("0x%x\n", len2);
                                break;
                        case AT_fund_type:
                                sword = get_short();
				(void)printf("%s\n", fund_type(sword) );
				break;
                        case AT_mod_fund_type:
                                mod_fund_type();
				break;
                        case AT_user_def_type:
                                user_def_type();
				break;
                        case AT_mod_u_d_type:
                                mod_u_d_type();
				break;
			case AT_ordering:
				sword = get_short();
				(void)printf("%s\n", order(sword) );
				break;
			case AT_byte_size:
                                word = get_long();
				(void)printf("0x%lx\n", word);
                                break;
                        case AT_bit_offset:
                                sword = get_short();
				(void)printf("0x%lx\n", sword);
                                break;
                        case AT_bit_size:
                                word = get_long();
				(void)printf("0x%lx\n", word);
                                break;
                        case AT_stmt_list:
				word = get_long();
				(void)printf("0x%lx\n", word);
                                break;
                        case AT_low_pc:
                                word = get_long();
				(void)printf("0x%lx\n", word);
                                break;
                        case AT_high_pc:
                                word = get_long();
				(void)printf("0x%lx\n", word);
                                break;
                        case AT_language:
                                word = get_long();
				(void)printf("%s\n", lang(word) );
                                break;
			case AT_visibility:
				sword = get_short();
				(void)printf("%s\n", vis(sword) );
				break;
                        case AT_element_list:
				element_list();
				break;
                        case AT_subscr_data:
				subscr_data(); 
				break;
			case AT_deriv_list:
			case AT_member:
			case AT_discr:
			case AT_discr_value:
			case AT_import:
				not_interp( attrname );
                                break;
                        default:
                                not_interp( attrname );
                                break;
                }
		if(length)
			(void)printf("%-28s", " ");
	}
  }
  return;
}


/*
 * Returns a string name for the tag value.  This function
 * needs to be updated any time that a tag value is added or
 * changed.
 */

static char *
lookuptag( tag )
short tag;
{
	static char buf[16];

	switch ( tag ) {
	default:
		sprintf(buf, "0x%x", tag);
		return buf;
	case TAG_padding:		return "padding";
	case TAG_array_type:		return "array type";
	case TAG_class_type:		return "class type";
	case TAG_entry_point:		return "entry point";
	case TAG_enumeration_type:	return "enum type";
	case TAG_formal_parameter:	return "formal param";
	case TAG_global_subroutine:	return "global subrtn";
	case TAG_global_variable:	return "global var";
	case TAG_imported_declaration:	return "imported decl";
	case TAG_inline_subroutine:	return "inline subrtn";
	case TAG_label:			return "label";
	case TAG_lexical_block:		return "lexical blk";
	case TAG_local_variable:	return "local var";
	case TAG_member:		return "member";
	case TAG_member_function:	return "member func";
	case TAG_pointer_type:		return "pointer type";
	case TAG_reference_type:	return "ref type";
	case TAG_source_file:		return "source file";
	case TAG_string_type:		return "string type";
	case TAG_structure_type:	return "struct type";
	case TAG_subroutine:		return "subroutine";
	case TAG_subroutine_type:	return "subrtn type";
	case TAG_typedef:		return "typedef";
	case TAG_union_type:		return "union type";
	case TAG_unspecified_parameters:return "unspec parms";
	case TAG_variant:		return "variant";
	}
}


/*
 * Return a string name for an attribute value.  This function
 * needs to be updated any time that an attribute value is added
 * or changed
 */

static char *
lookupattr( attr )
short attr;
{
	static char buf[20];

	switch ( attr ) {
	default:
		sprintf(buf, "0x%x", attr);
		return buf;
	case AT_padding:	return "padding";
	case AT_sibling:	return "sibling";
	case AT_location:	return "location";
	case AT_name:		return "name";
	case AT_dimensions:	return "dimensions";
	case AT_fund_type:	return "fund_type";
	case AT_mod_fund_type:	return "mod_fund_type";
	case AT_user_def_type:	return "user_def_type";
	case AT_mod_u_d_type: 	return "mod_u_d_type";
	case AT_ordering:	return "ordering";
	case AT_subscr_data:	return "subscr_data";
	case AT_byte_size:	return "byte_size";
	case AT_bit_offset:	return "bit_offset";
	case AT_bit_size:	return "bit_size";
	case AT_deriv_list:	return "deriv_list";
	case AT_element_list:	return "element_list";
	case AT_stmt_list:	return "stmt_list";
	case AT_low_pc:		return "low_pc";
	case AT_high_pc:	return "high_pc";
	case AT_language:	return "language";
	case AT_member:		return "member";
	case AT_discr:		return "discr";
	case AT_discr_value:	return "discr_value";
	case AT_visibility:	return "visibility";
	case AT_import:		return "import";
	case AT_string_length:	return "string_length";
	}
}


/*
 * Get 1 byte of data.  Decrement the length by 1 byte
 * and the no_elemtype by 1 byte.  The length is used
 * globally by all functions. no_elemtype is used globally
 * only by subscr_data.
 */

static char 
get_byte()
{
	unsigned char 	*p;

	p = ptr; 
	++ptr;
	length -= 1;
	no_elemtype -= 1;
	return *p;
}

/*
 * Get 2 bytes of data.  Decrement the length by 2 bytes
 * and the no_elemtype by 2 bytes.  The length is used
 * globally by all functions. no_elemtype is used globally
 * only by subscr_data.
 */

static short
get_short()
{
	short x;
	unsigned char    *p = (unsigned char *)&x;

	*p = *ptr; ++ptr; ++p;
        *p = *ptr; ++ptr;
        length -= 2;
        no_elemtype -= 2;
        return x;

}

/*
 * Get 4 bytes of data.  Decrement the length by 4 bytes
 * and the no_elemtype by 4 bytes.  The length is used
 * globally by all functions. no_elemtype is used globally
 * only by subscr_data.
 */

static long
get_long()
{
	long 	x;
	unsigned char	*p = (unsigned char *)&x;

	*p = *ptr; ++ptr; ++p;
        *p = *ptr; ++ptr; ++p;
        *p = *ptr; ++ptr; ++p;
        *p = *ptr; ++ptr;
        length -= 4;
        no_elemtype -= 4;
        return x;
}

/*
 * Get string_length bytes of data.  Decrement the length by
 * string_length bytes
 * and the no_elemtype by string_length bytes.  The length is used
 * globally by all functions. no_elemtype is used globally
 * only by subscr_data.
 */

static unsigned char *
get_string()
{
	unsigned char	*s;
	register 	int	len;
	
	len = strlen(ptr) +1;
	s = (unsigned char *)malloc(len);
	memcpy(s,ptr,len);
	ptr += len;
	length -= len;
	no_elemtype -= len;
	return s;

}


/*
 * Print out the uninterpreted numerical value
 * of the associated attribute data depending on
 * the data format.
 */

static void
not_interp( attrname ) 
short	attrname;
{
        short   len2;
        long    word;

        switch( attrname & FORM_MASK )
        {
                case FORM_NONE: break;
                case FORM_ADDR:
                case FORM_REF:  word = get_long();
				(void)printf("<0x%lx>\n", word);
				break;
                case FORM_BLOCK2:       len2 = get_short();
                                        length -= len2;
                                        ptr += len2;
					(void)printf("0x%x\n", len2);
					break;
                case FORM_BLOCK4:       word = get_long();
                                        length -= word;
                                        ptr += word;
					(void)printf("0x%lx\n", word);
					break;
                case FORM_DATA2:        len2 = get_short();
					(void)printf("0x%x\n", len2);
					break;
                case FORM_DATA8:        word = get_long();
					(void)printf("0x%lx ", word);
					break;
                case FORM_DATA4:        word = get_long();
					(void)printf("0x%lx\n", word);
					break;
                case FORM_STRING:       word = strlen(ptr) + 1;
                                        length -= word;
					(void)printf("%s\n", ptr);
                                        ptr += word;
					break;
                default:
                        printf("<unknown form: 0x%x>\n", (attrname & FORM_MASK) );
			length = 0;
        }
}


/*
 * The following functions are called to interpret the debugging
 * information depending on the attribute of the debugging entry.
 */

static void
user_def_type()
{
	long	x;
	x = get_long();
	(void)printf("0x%lx\n", x);
}

static void
mod_fund_type()
{
	short len2, x;
	int modcnt;
	char *p;
	int i;

	len2 = get_short();
	modcnt = len2 - 2;
	while(modcnt--)
	{
		(void)printf("%s ", modifier(*ptr++) );
		length--;
	}
	x = get_short();
	(void)printf("\n%-48s%s\n", " ", fund_type(x) );
	
}

static void
mod_u_d_type()
{
	short len2;
        int modcnt;
        char *p;
        int i;
	long	x;

	len2 = get_short();
	modcnt = len2 - 4;
	while(modcnt--)
	{
		(void)printf("%s ", modifier(*ptr++) );
		length--;
	}
	x = p_debug->p_shdr->sh_offset + get_long() - p_debug->p_shdr->sh_addr;
	(void)printf("\n%-48s0x%lx\n", " ",  x);
	
}

static void
location()
{
	short           len2;
        int             o, a;
	long		x;

        len2 = get_short();
        while ( len2 > 0 )
        {
                o = get_byte();
		len2 -= 1;
		(void)printf("%s ", op(o) );
		if( has_arg(o) )
		{
			a = get_long();
			len2 -= 4;
			(void)printf("0x%x\n", a);
		}
		if(len2)
			(void)printf("%-48s", " ");
        }
}


static char *
lang( l )
long l;
{
	static char buf[20];

	switch ( l ) {
	default:
		sprintf(buf, "<LANG_0x%lx>", l);
		return buf;
	case LANG_UNK:			return "LANG_UNK";
	case LANG_ANSI_C_V1:		return "LANG_ANSI_C_V1";
	}
}

static char *
fund_type( f )
short f;
{
	static char buf[20];

	switch ( f ) {
	default:
		sprintf(buf, "<FT_0x%x>", f);
		return buf;
	case FT_none:			return "FT_none";
	case FT_char:			return "FT_char";
	case FT_signed_char:		return "FT_unsigned_char";
	case FT_unsigned_char:		return "FT_unsigned_char";
	case FT_short:			return "FT_short";
	case FT_signed_short:		return "FT_signed_short";
	case FT_unsigned_short:		return "FT_unsigned_short";
	case FT_integer:		return "FT_integer";
	case FT_signed_integer:		return "FT_signed_integer";
	case FT_unsigned_integer:	return "FT_unsigned_integer";
	case FT_long:			return "FT_long";
	case FT_signed_long:		return "FT_signed_long";
	case FT_unsigned_long:		return "FT_unsigned_long";
	case FT_pointer:		return "FT_pointer";
	case FT_float:			return "FT_float";
	case FT_dbl_prec_float:		return "FT_dbl_prec_float";
	case FT_ext_prec_float:		return "FT_ext_prec_float";
	case FT_complex:		return "FT_complex";
	case FT_dbl_prec_complex:	return "FT_dbl_prec_complex";
	case FT_set:			return "FT_set";
	case FT_void:			return "FT_void";
	}
}

static
char *modifier( m )
char m;
{
	static char buf[20];

	switch ( m ) {
	default:
		sprintf(buf, "<MOD_0x%x>", m);
		return buf;
	case MOD_none:		return "MOD_none";
	case MOD_pointer_to:	return "MOD_pointer_to";
	case MOD_reference_to:	return "MOD_reference_to";
	}
}

static char *
op( a )
char a;
{
	static char buf[20];

	switch ( a ) {
	default:
		sprintf(buf, "<OP_0x%x>", a);
		return buf;
	case OP_UNK:		return "OP_UNK";
	case OP_REG:		return "OP_REG";
	case OP_BASEREG:	return "OP_BASEREG";
	case OP_ADDR:		return "OP_ADDR";
	case OP_CONST:		return "OP_CONST";
	case OP_DEREF2:		return "OP_DEREF2";
	case OP_DEREF4:		return "OP_DEREF4";
	case OP_ADD:		return "OP_ADD";
	}
}

static int 
has_arg( op )
char op;
{
	switch ( op ) {
	default:
		return 0;
	case OP_UNK:		return 0;
	case OP_REG:		return 1;
	case OP_BASEREG:	return 1;
	case OP_ADDR:		return 1;
	case OP_CONST:		return 1;
	case OP_DEREF2:		return 0;
	case OP_DEREF4:		return 0;
	case OP_ADD:		return 0;
	}
}

static char *
order(a)
short a;
{
	static char buf[20];

	switch (a)
	{
		case ORD_row_major:	return "ORD_row_major";
		case ORD_col_major:	return "ORD_col_major";
		default:		sprintf(buf, "<OP_0x%x>", a);
			 		return buf;
	}
}

static char *
vis(a)
short a;
{
	static char buf[20];

	switch (a)
	{
		case VIS_local:		return "VIS_local";
		case VIS_exported:	return "VIS_exported";
		default:		sprintf(buf, "<OP_0x%x>", a);
			 		return buf;
	}
}

static void
element_list()
{
	short len2;
	long  word;
	unsigned char *the_string;

	len2 = get_short();

	while ( len2 > 0 )
	{
		word = get_long();
		len2 -= 4;
		len2 -= (strlen(ptr) + 1);
		the_string = get_string();
		(void)printf("0x%lx %s\n", word, the_string);

		if(len2)
			(void)printf("%-48s", " ");
	}
}

static void
subscr_data()
{
	static char buf[20];
	char    fmt;
	short   et_name;
	short	sword;
	long	word;

	no_elemtype = get_short();
	while (no_elemtype)
	{
		fmt = get_byte();
		(void)printf("%s\n", lookupfmt(fmt) );
		switch (fmt)
		{
			case FMT_FT_C_C:
				et_name = get_short();
				(void)printf("%-48s", " ");
				(void)printf("%s\n", fund_type(et_name) );
				word = get_long();
				(void)printf("%-48s", " ");
				(void)printf("lobound = 0x%lx\n", word);
				word = get_long();
				(void)printf("%-48s", " ");
				(void)printf("hibound = 0x%lx\n", word);
				break;
			case FMT_FT_C_X:
				et_name = get_short();
				(void)printf("%-48s", " ");
				(void)printf("%s\n", fund_type(et_name) );
				word = get_long();
				(void)printf("%-48s", " ");
				(void)printf("lobound = 0x%lx\n", word);
				(void)printf("%-48s", " ");
				location();
				break;
			case FMT_FT_X_C:
				et_name = get_short();
				(void)printf("%-48s", " ");
				(void)printf("%s\n", fund_type(et_name) );
				(void)printf("%-48s", " ");
				location();
				word = get_long();
				(void)printf("%-48s", " ");
				(void)printf("hibound = 0x%lx\n", word);
				break;
			case FMT_FT_X_X:
				et_name = get_short();
				(void)printf("%-48s", " ");
				(void)printf("%s\n", fund_type(et_name) );
				(void)printf("%-48s", " ");
				location();
				(void)printf("%-48s", " ");
				location();
				break;
			case FMT_UT_C_C:
				(void)printf("%-48s", " ");
				user_def_type();
				word = get_long();
				(void)printf("%-48s", " ");
				(void)printf("lobound = 0x%lx\n", word);
				word = get_long();
				(void)printf("%-48s", " ");
				(void)printf("hibound = 0x%lx\n", word);
				break;
			case FMT_UT_C_X:
				(void)printf("%-48s", " ");
				user_def_type();
				word = get_long();
				(void)printf("%-48s", " ");
				(void)printf("lobound = 0x%lx\n", word);
				(void)printf("%-48s", " ");
				location();
				break;
			case FMT_UT_X_C:
				(void)printf("%-48s", " ");
				user_def_type();
				(void)printf("%-48s", " ");
				location();
				word = get_long();
				(void)printf("%-48s", " ");
				(void)printf("hibound = 0x%lx\n", word);
				break;
			case FMT_UT_X_X:
				(void)printf("%-48s", " ");
				user_def_type();
				(void)printf("%-48s", " ");
				location();
				(void)printf("%-48s", " ");
				location();
				break;
			case FMT_ET:
				et_name = get_short();
				switch(et_name)
				{
					case AT_fund_type:
						sword = get_short();
						(void)printf("%-48s", " ");
						(void)printf("%s\n", fund_type(sword) );
						break;
					case AT_mod_fund_type:
						(void)printf("%-48s", " ");
						mod_fund_type();
						no_elemtype--;
						break;
					case AT_user_def_type:
						(void)printf("%-48s", " ");
						user_def_type();
						break;
					case AT_mod_u_d_type:
						(void)printf("%-48s", " ");
						mod_u_d_type();
						break;
					default:
						(void)printf("%-48s", " ");
						(void)printf("<unknown element type 0x%x>\n", et_name);
						break;
				}
				break;
			default:
				(void)printf("%-48s", " ");
				(void)printf("<unknown format 0x%x>\n", et_name);
				no_elemtype = 0;
				break;
		}
		if(no_elemtype)
			(void)printf("%-48s", " ");
	}	/* end while */
}


static char *
lookupfmt( fmt )
char fmt;
{
	static char buf[10];

	switch ( fmt )
	{
		default:
			sprintf(buf, "0x%x", fmt);
			return buf;
		case FMT_FT_C_C:	return "FMT_FT_C_C";
		case FMT_FT_C_X:	return "FMT_FT_C_X";
		case FMT_FT_X_C:	return "FMT_FT_X_C";
		case FMT_FT_X_X:	return "FMT_FT_X_X";
		case FMT_UT_C_C: 	return "FMT_UT_C_C";
		case FMT_UT_C_X:	return "FMT_UT_C_X";
		case FMT_UT_X_C:	return "FMT_UT_X_C";
		case FMT_UT_X_X:	return "FMT_UT_X_X";
		case FMT_ET:		return "FMT_ET";
	}
}


/*
 * Print line number information.  Get the line number data
 * and call print_line to print it out.  Input is an ELF file
 * descriptor and the filename.
 */

void
dump_line(elf, filename)
Elf *elf;
char * filename;
{

	extern SCNTAB *p_line;
        Elf_Scn         *scn;
	size_t		l_size;

	if(!p_flag)
	{
		(void)printf("    ***** LINE NUMBER INFORMATION *****\n");
		(void)printf("%-12s%-12s%s\n",
			"LineNo",
			"LinePos",
			"Pcval");
	}

	if ( (p_line_data = (unsigned char *)get_scndata(p_line->p_sd, &l_size)) == NULL)
	{
		(void)fprintf(stderr, "%s: %s: no data in %s section\n",
			prog_name, filename, p_line->scn_name);
		return;
	}

	ptr = p_line_data;
	print_line(p_line->p_shdr->sh_offset, l_size, filename);
}


/*
 * Print line number information.  Input is section header offset of
 * the line number section, the size, and the filename.  The first 4
 * bytes contain the length of the line number information for the first
 * source file linked in if the file is an executable, and the total
 * size if the file is a relocatable object.  The size is the size of
 * the entire section.  Print out line information until length is 0.
 * If size > 0, read in the next 4 bytes for the length of the next 
 * part of the line number information.  There will be one sub-section
 * for each file that was linked together, including library files.
 */

static void
print_line(off, size, filename)
long off;
long size;
char *filename;
{
	long  line;
	long  pcval;
	long  base_address;
	short delta;
	short stmt;
	
	while (size > 0)
	{
		length = get_long();
		length -= 4;
		size -= 4;
		base_address = get_long();
		size -= 4;
	
		if(size < length-4)
		{
			(void)fprintf(stderr, "%s: %s: bad line info section -  size=%ld length-4=%ld\n", prog_name, filename, size, length-4);
			return;
		}
		while(length > 0)
		{
			line = get_long();
			size -= 4;
			stmt = get_short();
			size -= 2;
			delta = get_long();
			size -= 4;
			pcval = base_address + delta;
	
			(void)printf("%-12ld%-12d0x%lx\n",
				line,
				stmt,
				pcval);
		}
	}
}
