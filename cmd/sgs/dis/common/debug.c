/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dis:common/debug.c	1.3"

#include        <stdio.h>
#include        <malloc.h>

#include	"dis.h"
#include	"structs.h"
#include	"libelf.h"
#include        "sgs.h"
#include	"dwarf.h"
#include	"ccstypes.h"

extern	int	trace; /* for debugging */


static		int		no_elemtype = 0;
static		Elf_Data	*debug_data;
static		Elf32_Shdr      *shdr;
static		unsigned char	*p_debug_data, *ptr;
static 		long		length = 0;
static		long		current;

static		short		tag;
static		long 		tag_address;
static unsigned char    	*the_string;

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

static char *		lookuptag();
static char *		lookupattr();
static char *		lookupfmt();
static char *	lang();
static char *	fund_type();
static char *	modifier();
static char *	op();
static int      has_arg();
static char *	order();
static char *	vis();
static void     element_list();
static void     subscr_data();

void	get_debug_info(),
	get_line_info(),	 
	print_line();

void
get_debug_info()
{

	extern  Elf             *elf;
        extern  int             debug;
	extern	char		*fname;
        Elf_Scn         *scn;

	if ( (scn = elf_getscn(elf, debug)) == NULL)
	{
		(void) fprintf(stderr, 
		"%dis: %s: failed to get section .debug; limited functionality\n"
		, SGS, fname);
		debug = 0;
		return;
	}
	else
		if ((shdr = elf32_getshdr(scn)) != 0)
		{
			debug_data = 0;
			if ((debug_data=elf_getdata(scn, debug_data))==0 || debug_data->d_size == 0)
                        {
				(void) fprintf(stderr,
                                "%sdis: no data in section .debug\n", SGS);
				debug = 0;
                                return;		
			}
			else
			{
				p_debug_data = (unsigned char *)debug_data->d_buf;
				ptr = p_debug_data;
				print_record( shdr->sh_offset , shdr->sh_size);

			}
		}
}

static void
print_record( offset, size)
Elf32_Off	offset;
size_t		size;
{
	extern	void	build_labels();
	long	word;
	short	sword;
	short	attrname;
	short   len2;
	unsigned	char	*tag_source_name;

  current = 0;
  while ( current < size)
  { 
	tag_source_name = NULL;

	word = get_long();

	if ( word <= 8)
	{
		if (trace) (void)printf("\n0x%-10lx\n", word);
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

	if (trace) (void)printf("\n0x%-10lx", word);

	length = word - 6;

	tag = get_short();

	if (trace) (void)printf("%-16s", lookuptag(tag));

	
	while(length > 0)
	{
		attrname = get_short();
		if (trace) (void)printf("%-20s\n", lookupattr(attrname) );
		switch( attrname )
		{
			case AT_padding:
				if (trace) (void)printf("(FORM_NONE)\n");
                                break;
                        case AT_sibling:
                                word = get_long();
				if (trace)
				{	
					if(word != 0)
						(void)printf("0x%lx\n", 
						shdr->sh_offset + word - shdr->sh_addr);
					else
						(void)printf("\n");
				}	
				break;
			case AT_location:
			case AT_string_length:
				location();
                                break;
                        case AT_name:
				if ( tag == TAG_label)
					tag_source_name = get_string();
				else
				{
					the_string = get_string();
					if (trace)
                                		(void)printf("%s\n",the_string);
				}
                                break;
                        case AT_dimensions:
                                len2 = get_short();
				if (trace) (void)printf("0x%x\n", len2); 
                                break;
                        case AT_fund_type:
                                sword = get_short();
				(void)fund_type(sword);	
				if (trace) (void)printf("%s\n", fund_type(sword) );
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
				if (trace) (void)printf("%s\n", order(sword) );
				break;
			case AT_byte_size:
                                word = get_long();
				if (trace) (void)printf("0x%lx\n", word);
                                break;
                        case AT_bit_offset:
                                sword = get_short();
				if (trace) (void)printf("0x%lx\n", sword);
                                break;
                        case AT_bit_size:
                                word = get_long();
				if (trace) (void)printf("0x%lx\n", word);
                                break;
                        case AT_stmt_list:
                                word = shdr->sh_offset + get_long() - shdr->sh_addr;
				if (trace) (void)printf("0x%lx\n", word); 
                                break;
                        case AT_low_pc:
                                word = get_long();
				if ( tag == TAG_label)
					tag_address = word;
				if (trace) (void)printf("0x%lx\n", word);
                                break;
                        case AT_high_pc:
                                word = get_long();
				if (trace) (void)printf("0x%lx\n", word);
                                break;
                        case AT_language:
                                word = get_long();
				if (trace) (void)printf("%s\n", lang(word) );
                                break;
			case AT_visibility:
				sword = get_short();
				if (trace) (void)printf("%s\n", vis(sword) );
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

		if(length && trace )
			(void)printf("%-28s", " ");
	}
	if ( tag == TAG_label) 
		build_labels(tag_source_name, tag_address);	
  }
  return;
}

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

static unsigned char *
get_string()
{
	unsigned char	*s;
	register 	int	len;
	
	len = strlen(ptr) +1;
	s = (unsigned char *)malloc(len);
	(void)memcpy(s,ptr,len);
	ptr += len;
	length -= len;
	no_elemtype -= len;
	return s;

}


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
				if (trace) (void)printf("<0x%lx>\n", word);
				break;
                case FORM_BLOCK2:       len2 = get_short();
                                        length -= len2;
                                        ptr += len2;
					if (trace) (void)printf("0x%x\n", len2);
					break;
                case FORM_BLOCK4:       word = get_long();
                                        length -= word;
                                        ptr += word;
					if (trace) (void)printf("0x%lx\n", word);
					break;
                case FORM_DATA2:        len2 = get_short();
					if (trace) (void)printf("0x%x\n", len2);
					break;
                case FORM_DATA8:        word = get_long();
					if (trace) (void)printf("0x%lx ", word);
					break;
                case FORM_DATA4:        word = get_long();
					if (trace) (void)printf("0x%lx\n", word);
					break;
                case FORM_STRING:       word = strlen(ptr) + 1;
                                        length -= word;
					if (trace) (void)printf("%s\n", ptr);
                                        ptr += word;
					break;
                default:
			if (trace)
                        printf("<unknown form: 0x%x>\n", (attrname & FORM_MASK) );
			length = 0;
        }
}

static void
user_def_type()
{
	long	x;
	x = shdr->sh_offset + get_long() - shdr->sh_addr;
	if (trace) (void)printf("0x%lx\n", x);
}

static void
mod_fund_type()
{
	short len2, x;
	int modcnt;

	len2 = get_short();
	modcnt = len2 - 2;
	while(modcnt--)
	{
		if (trace)	
			(void)printf("%s ", modifier(*ptr++) );
		else
			*ptr++;

		length--;
	}
	x = get_short();
	if (trace)
		(void)printf("\n%-48s%s\n", " ", fund_type(x) ); 
	return;
	
}

static void
mod_u_d_type()
{
	short len2;
        int modcnt;
	long	x;

	len2 = get_short();
	modcnt = len2 - 4;
	while(modcnt--)
	{
		if (trace)
			(void)printf("%s ", modifier(*ptr++) );
		else
			*ptr++;

		length--;
	}
	x = shdr->sh_offset + get_long() - shdr->sh_addr;
	if (trace) (void)printf("\n%-48s0x%lx\n", " ",  x);
	return;
	
}

static void
location()
{
	short           len2;
        int             o, a;

        len2 = get_short();
        while ( len2 > 0 )
        {
                o = get_byte();
		len2 -= 1;
		if (trace) (void)printf("%s ", op(o) );
		if( has_arg(o) )
		{
			a = get_long();
			len2 -= 4;
			if (trace) (void)printf("0x%x\n", a);
		}
		if(len2 && trace )
			(void)printf("%-48s", " ");
        }
}

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

static char *
modifier( m )
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
		len2 -= (short) (strlen(ptr) + 1);
		the_string = get_string();
		if (trace) (void)printf("0x%lx %s\n", word, the_string);

		if(len2)
			if (trace) (void)printf("%-48s", " ");
	}
}

static void
subscr_data()
{
	char    fmt;
	short   et_name;
	short	sword;
	long	word;

	no_elemtype = get_short();
	while (no_elemtype)
	{
		fmt = get_byte();
		if (trace) (void)printf("%s\n", lookupfmt(fmt) );
		switch (fmt)
		{
			case FMT_FT_C_C:
				et_name = get_short();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("%s\n", fund_type(et_name) );
				word = get_long();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("lobound = 0x%lx\n", word);
				word = get_long();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("hibound = 0x%lx\n", word);
				break;
			case FMT_FT_C_X:
				et_name = get_short();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("%s\n", fund_type(et_name) );
				word = get_long();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("lobound = 0x%lx\n", word);
				if (trace) (void)printf("%-48s", " ");
				location();
				break;
			case FMT_FT_X_C:
				et_name = get_short();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("%s\n", fund_type(et_name) );
				if (trace) (void)printf("%-48s", " ");
				location();
				word = get_long();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("hibound = 0x%lx\n", word);
				break;
			case FMT_FT_X_X:
				et_name = get_short();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("%s\n", fund_type(et_name) );
				if (trace) (void)printf("%-48s", " ");
				location();
				if (trace) (void)printf("%-48s", " ");
				location();
				break;
			case FMT_UT_C_C:
				if (trace) (void)printf("%-48s", " ");
				user_def_type();
				word = get_long();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("lobound = 0x%lx\n", word);
				word = get_long();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("hibound = 0x%lx\n", word);
				break;
			case FMT_UT_C_X:
				if (trace) (void)printf("%-48s", " ");
				user_def_type();
				word = get_long();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("lobound = 0x%lx\n", word);
				if (trace) (void)printf("%-48s", " ");
				location();
				break;
			case FMT_UT_X_C:
				if (trace) (void)printf("%-48s", " ");
				user_def_type();
				if (trace) (void)printf("%-48s", " ");
				location();
				word = get_long();
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("hibound = 0x%lx\n", word);
				break;
			case FMT_UT_X_X:
				if (trace) (void)printf("%-48s", " ");
				user_def_type();
				if (trace) (void)printf("%-48s", " ");
				location();
				if (trace) (void)printf("%-48s", " ");
				location();
				break;
			case FMT_ET:
				et_name = get_short();
				switch(et_name)
				{
					case AT_fund_type:
						sword = get_short();
						if (trace) (void)printf("%-48s", " ");
						if (trace) (void)printf("%s\n", fund_type(sword) );
						break;
					case AT_mod_fund_type:
						if (trace) (void)printf("%-48s", " ");
						mod_fund_type();
						break;
					case AT_user_def_type:
						if (trace) (void)printf("%-48s", " ");
						user_def_type();
						break;
					case AT_mod_u_d_type:
						if (trace) (void)printf("%-48s", " ");
						mod_u_d_type();
						break;
					default:
						if (trace) (void)printf("%-48s", " ");
						if (trace) (void)printf("<unknown element type 0x%x>\n", et_name);
						break;
				}
				break;
			default:
				if (trace) (void)printf("%-48s", " ");
				if (trace) (void)printf("<unknown format 0x%x>\n", et_name);
				no_elemtype = 0;
				break;
		}
		if(no_elemtype)
			if (trace) (void)printf("%-48s", " ");
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

void
get_line_info()
{
	extern 	Elf	*elf;
	extern	int	line;
	extern	unsigned	char *ptr_line_data;
	extern	size_t		size_line;

	Elf_Scn	*scn;
	Elf_Data	*line_data;

	if ( (scn = elf_getscn(elf, line)) == NULL)
	{
		(void) fprintf(stderr,
		"%sdis: failed to get section .line; limited functioality\n"
		,SGS);
		line = 0;
		return;
	}
	else
	if ((shdr = elf32_getshdr(scn)) != 0)
	{
		line_data = 0;	
		if ((line_data=elf_getdata(scn, line_data))==0 || line_data->d_size ==0)
		{
			(void) fprintf(stderr,
			"%sdis: no data in section .line\n", SGS);
			line = 0;
			return;
		}
		else
		{
			ptr_line_data = (unsigned char *)line_data->d_buf;
			size_line	= line_data->d_size;
		}
	}
	return;
	
}

void
print_line(current, ptr_line, size_line)
long current;
unsigned	char *ptr_line;
size_t	size_line;
{
	extern	char	*fname;
	long  line;
	long  pcval;
	long  base_address;
	long  size;
	short delta;


	ptr = ptr_line;
	size = size_line;
	
	while (size > 0)
	{
		length = get_long();
		length -= 4;
		size -= 4;
		base_address = get_long();
		size -= 4;
	
		if(size < length-4)
		{
			(void)fprintf(stderr, "%sdis: %s: bad line info section -  size=%ld length-4=%ld\n", SGS, fname, size, length-4);
			return;
		}
		while(length > 0)
		{
			line = get_long();
			size -= 4;
			(void)get_short();
			size -= 2;
			delta = get_long();
			size -= 4;
			pcval = base_address + delta;

			if (current == pcval)
			 (void)printf("[%ld]", line);
			else
			if (current < pcval)
				return; /* can return because line
					   number info in ascending
					   order */
		}
	}
}
