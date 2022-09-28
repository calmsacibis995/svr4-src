#ident	"@(#)sdb:libsymbol/common/Bdibuild.C	1.13.2.1"
#include	"Bdibuild.h"
#include	"Interface.h"
#include	"Locdesc.h"
#include	"Syminfo.h"
#include	"Tag.h"
#include	"dwarf.h"
#include	"builder.h"
#include	<malloc.h>
#include	<string.h>

Bdibuild::Bdibuild( int fd ) : bdi(fd)
{
	ptr = 0;
	length = 0;
	nextoff = 0;
	bdi.entry_info( entry_offset, entry_base );
	bdi.stmt_info( stmt_offset, stmt_base );
}

char
Bdibuild::get_byte()
{
	char *	p;

	p = ptr; ++ptr;
	length -= 1;
	return *p;
}

short
Bdibuild::get_short()
{
	short	x;
	char *	p = (char*)&x;

	*p = *ptr; ++ptr; ++p;
	*p = *ptr; ++ptr;
	length -= 2;
	return x;
}

long
Bdibuild::get_long()
{
	long	x;
	char *	p = (char*)&x;

	*p = *ptr; ++ptr; ++p;
	*p = *ptr; ++ptr; ++p;
	*p = *ptr; ++ptr; ++p;
	*p = *ptr; ++ptr;
	length -= 4;
	return x;
}

char *
Bdibuild::get_string()
{
	char *		s;
	register int	len;

	len = ::strlen(ptr)+1;
	s = ::malloc(len);
	::memcpy(s,ptr,len);
	ptr += len;
	length -= len;
	return s;
}

void *
Bdibuild::make_chunk(void * p, int len)
{
	char *		s;

	s = ::malloc(len);
	::memcpy(s,(char*)p,len);
	return s;
}

static Tag	tagname[] =	{
					t_none,			// 00
					t_arraytype,		// 01
					0,			// 02
					t_entry,		// 03
					t_enumtype,		// 04
					t_argument,		// 05
					t_entry,		// 06
					t_global,		// 07
					0,			// 08
					0,			// 09
					t_label,		// 0a
					t_block,		// 0b
					t_variable,		// 0c
					t_structuremem,		// 0d
					0,			// 0e
					t_pointertype,		// 0f
					t_reftype,		// 10
					t_sourcefile,		// 11
					t_stringtype,		// 12
					t_structuretype,	// 13
					t_entry,		// 14
					t_functiontype,		// 15
					t_typedef,		// 16
					t_uniontype,		// 17
					t_unspecargs,		// 18
					0,			// 19
				};

static Attr_name	typename[] = {
					an_nomore,
					an_elemtype,
					an_nomore,
					an_resulttype,
					an_nomore,
					an_type,
					an_resulttype,
					an_type,
					an_nomore,
					an_resulttype,
					an_nomore,
					an_nomore,
					an_type,
					an_type,
					an_resulttype,
					an_basetype,
					an_basetype,
					an_nomore,
					an_nomore,
					an_nomore,
					an_nomore,
					an_resulttype,
					0,
					an_nomore,
					an_nomore,
					an_type,
				};


void
Bdibuild::skip_attribute( short attrname )
{
	short	len2;
	long	word;

	switch( attrname & FORM_MASK )
	{
		case FORM_NONE:					break;
		case FORM_ADDR:
		case FORM_REF:		get_long();		break;
		case FORM_BLOCK2:	len2 = get_short();
					length -= len2;
					ptr += len2;		break;
		case FORM_BLOCK4:	word = get_long();
					length -= word;
					ptr += word;		break;
		case FORM_DATA2:	get_short();		break;
		case FORM_DATA8:	get_long();
		case FORM_DATA4:	get_long();		break;
		case FORM_STRING:	word = strlen(ptr) + 1;
					length -= word;
					ptr += word;		break;
		default:
			printf("wierd attrname %#.4x\n", attrname);
			length = 0;
	}
}

void
Bdibuild::get_ft( Attr_form & form, Attr_value & value )
{
	short	x;

	form = af_fundamental_type;
	x = get_short();
	value.word = x;
}

void
Bdibuild::fund_type()
{
	Attr_value	value;
	Attr_form	form;

	get_ft( form, value );
	fetalrec.add_attr( typename[tag], form, value );
}

void
Bdibuild::get_udt( Attr_form & form, Attr_value & value )
{
	form = af_bdioffs;
	value.word = entry_offset + get_long() - entry_base;
}

void
Bdibuild::user_def_type()
{
	Attr_value	value;
	Attr_form	form;

	get_udt( form, value );
	fetalrec.add_attr( typename[tag], form, value );
}

void
Bdibuild::get_mft( Attr_form & form, Attr_value & value )
{
	short		len2;
	int		modcnt;
	char *		p;
	int		i;

	len2 = get_short();
	modcnt = len2 - 2;
	ptr += modcnt;
	value.word = get_short();
	form = af_fundamental_type;
	p = ptr - 3;
	for ( i = 0 ; i < modcnt ; i++ )
	{
		switch (*p)
		{
			case MOD_pointer_to:
				fetaltype.add_attr(an_tag,af_tag,t_pointertype);
				fetaltype.add_attr(an_basetype,form,value);
				fetaltype.add_attr(an_bytesize, af_int, 4L );
				value.ptr = fetaltype.put_record();
				form = af_symbol;
				break;
			case MOD_reference_to:
				fetaltype.add_attr(an_tag,af_tag,t_reftype);
				fetaltype.add_attr(an_basetype,form,value);
				fetaltype.add_attr(an_bytesize, af_int, 4L );
				value.ptr = fetaltype.put_record();
				form = af_symbol;
				break;
		}
		--p;
	}
}

void
Bdibuild::mod_fund_type()
{
	Attr_value	value;
	Attr_form	form;

	get_mft( form, value );
	fetalrec.add_attr( typename[tag], form, value );
}

void
Bdibuild::get_mudt( Attr_form & form, Attr_value & value )
{
	short		len2;
	int		modcnt;
	char *		p;
	int		i;

	len2 = get_short();
	modcnt = len2 - 4;
	ptr += modcnt;
	value.word = entry_offset + get_long() - entry_base;
	form = af_bdioffs;
	p = ptr - 5;
	for ( i = 0 ; i < modcnt ; i++ )
	{
		switch (*p)
		{
			case MOD_pointer_to:
				fetaltype.add_attr(an_tag,af_tag,t_pointertype);
				fetaltype.add_attr(an_basetype,form,value);
				fetaltype.add_attr(an_bytesize, af_int, 4L );
				value.ptr = fetaltype.put_record();
				form = af_symbol;
				break;
			case MOD_reference_to:
				fetaltype.add_attr(an_tag,af_tag,t_reftype);
				fetaltype.add_attr(an_basetype,form,value);
				fetaltype.add_attr(an_bytesize, af_int, 4L );
				value.ptr = fetaltype.put_record();
				form = af_symbol;
				break;
		}
		--p;
	}
}

void
Bdibuild::mod_u_d_type()
{
	Attr_value	value;
	Attr_form	form;

	get_mudt( form, value );
	fetalrec.add_attr( typename[tag], form, value );
}

void
Bdibuild::sibling()
{
	long	word;
	long	siboff;

	word = get_long();
	siboff = entry_offset + word - entry_base;
	fetalrec.add_attr(an_sibling,af_bdioffs,siboff);
	if ( (nextoff != siboff) && (tag != TAG_enumeration_type) )
	{
		fetalrec.add_attr(an_scansize, af_int, siboff - nextoff);
		fetalrec.add_attr(an_child, af_bdioffs, nextoff);
	}
}

void
Bdibuild::name()
{
	fetalrec.add_attr( an_name, af_stringndx, get_string() );
}

void
Bdibuild::get_location( Attr_form & form, Attr_value & value )
{
	Locdesc		locdesc;
	short		len2;
	char 		op;

	locdesc.clear();
	len2 = get_short();
	while ( len2 > 0 )
	{
		op = get_byte();
		switch( op )
		{
			case OP_REG:
				locdesc.reg( get_long() );
				len2 -= 5;
				break;
			case OP_BASEREG:
				locdesc.basereg( get_long() );
				len2 -= 5;
				break;
			case OP_ADDR:
				locdesc.addr( get_long() );
				len2 -= 5;
				break;
			case OP_CONST:
				locdesc.offset( get_long() );
				len2 -= 5;
				break;
			case OP_DEREF4:
				locdesc.deref4();
				len2 -= 1;
				break;
			case OP_ADD:
				locdesc.add();
				len2 -= 1;
				break;
			default:
				len2 -= 1;
				break;
		}
	}
	value.ptr = make_chunk(locdesc.addrexp(),locdesc.size());
	form = af_locdesc;
}

void
Bdibuild::location()
{
	Attr_form	form;
	Attr_value	value;
	
	get_location( form, value );
	fetalrec.add_attr( an_location, form, value );
}

void
Bdibuild::byte_size()
{
	long	word;

	word = get_long();
	fetalrec.add_attr( an_bytesize, af_int, word );
}

void
Bdibuild::bit_offset()
{
	long	word;

	word = get_short();
	fetalrec.add_attr( an_bitoffs, af_int, word );
}

void
Bdibuild::bit_size()
{
	long	word;

	word = get_long();
	fetalrec.add_attr( an_bitsize, af_int, word );
}

void
Bdibuild::stmt_list()
{
	long	word;

	word = stmt_offset + get_long() - stmt_base;
	fetalrec.add_attr(an_lineinfo,af_bdiline,word);
}

void
Bdibuild::low_pc()
{
	long	word;

	word = get_long();
	fetalrec.add_attr( an_lopc, af_addr, word );
}

void
Bdibuild::high_pc()
{
	long	word;

	word = get_long();
	fetalrec.add_attr( an_hipc, af_addr, word );
}

void
Bdibuild::element_list()
{
	Attribute	*prev;
	short		len2;
	long		word;
	Attr_value	value;
	Attr_form	form;

	prev = 0;
	len2 = get_short();
	while ( len2 > 0 )
	{
		fetaltype.add_attr( an_tag, af_tag, t_enumlittype );
		word = get_long();
		fetaltype.add_attr( an_litvalue, af_int, word );
		len2 -= 4;
		len2 -= (::strlen(ptr)+1);
		fetaltype.add_attr( an_name, af_stringndx, get_string() );
		fetaltype.add_attr( an_sibling, af_symbol, prev );
		prev = fetaltype.put_record();
	}
	form = af_symbol;
	value.ptr = prev;
	fetalrec.add_attr( an_child, form, value );
}

void
Bdibuild::next_item( Attribute * a )
{
	Attribute	attr[5];

	if ( subscr_list( attr ) )
	{
		fetaltype.add_attr( attr[0].name, attr[0].form, attr[0].value );
		fetaltype.add_attr( attr[1].name, attr[1].form, attr[1].value );
		fetaltype.add_attr( attr[2].name, attr[2].form, attr[2].value );
		fetaltype.add_attr( attr[3].name, attr[3].form, attr[3].value );
		fetaltype.add_attr( attr[4].name, attr[4].form, attr[4].value );
		a[0].value.ptr = fetaltype.put_record();
		a[0].form = af_symbol;
		a[0].name = an_elemtype;
	}
	else
	{
		a[0] = attr[0];
	}
}

int
Bdibuild::subscr_list( Attribute * a )
{
	char		fmt;
	short		et_name;
	Attr_form	form;
	Attr_value	value;

	a[0].name = an_elemtype;
	a[1].name = an_subscrtype;
	a[2].name = an_lobound;
	a[3].name = an_hibound;
	a[4].name = an_tag;
	a[4].form = af_tag;
	a[4].value.word = t_arraytype;
	fmt = get_byte();
	switch( fmt )
	{
		case FMT_FT_C_C:
			get_ft( form, value );
			a[1].form = form;
			a[1].value = value;
			a[2].form = af_int;
			a[2].value.word = get_long();
			a[3].form = af_int;
			a[3].value.word = get_long();
			next_item( a );
			return 1;
		case FMT_FT_C_X:
			get_ft( form, value );
			a[1].form = form;
			a[1].value = value;
			a[2].form = af_int;
			a[2].value.word = get_long();
			get_location( form, value );
			a[3].form = form;
			a[3].value = value;
			next_item( a );
			return 1;
		case FMT_FT_X_C:
			get_ft( form, value );
			a[1].form = form;
			a[1].value = value;
			get_location( form, value );
			a[2].form = form;
			a[2].value = value;
			a[3].form = af_int;
			a[3].value.word = get_long();
			next_item( a );
			return 1;
		case FMT_FT_X_X:
			get_ft( form, value );
			a[1].form = form;
			a[1].value = value;
			get_location( form, value );
			a[2].form = form;
			a[2].value = value;
			get_location( form, value );
			a[3].form = form;
			a[3].value = value;
			next_item( a );
			return 1;
		case FMT_UT_C_C:
			get_udt( form, value );
			a[1].form = form;
			a[1].value = value;
			a[2].form = af_int;
			a[2].value.word = get_long();
			a[3].form = af_int;
			a[3].value.word = get_long();
			next_item( a );
			return 1;
		case FMT_UT_C_X:
			get_udt( form, value );
			a[1].form = form;
			a[1].value = value;
			a[2].form = af_int;
			a[2].value.word = get_long();
			get_location( form, value );
			a[3].form = form;
			a[3].value = value;
			next_item( a );
			return 1;
		case FMT_UT_X_C:
			get_udt( form, value );
			a[1].form = form;
			a[1].value = value;
			get_location( form, value );
			a[2].form = form;
			a[2].value = value;
			a[3].form = af_int;
			a[3].value.word = get_long();
			next_item( a );
			return 1;
		case FMT_UT_X_X:
			get_udt( form, value );
			a[1].form = form;
			a[1].value = value;
			get_location( form, value );
			a[2].form = form;
			a[2].value = value;
			get_location( form, value );
			a[3].form = form;
			a[3].value = value;
			next_item( a );
			return 1;
		case FMT_ET:
			et_name = get_short();
			switch( et_name )
			{
				case AT_fund_type:
					get_ft( form, value );
					a[0].form = form;
					a[0].value = value;
					break;
				case AT_user_def_type:
					get_udt( form, value );
					a[0].form = form;
					a[0].value = value;
					break;
				case AT_mod_fund_type:
					get_mft( form, value );
					a[0].form = form;
					a[0].value = value;
					break;
				case AT_mod_u_d_type:
					get_mudt( form, value );
					a[0].form = form;
					a[0].value = value;
					break;
				default:
					skip_attribute( et_name );
					break;
			}
			return 0;
		default:
			return 0;
	}
}

void
Bdibuild::subscr_data()
{
	Attribute	attr[5];

	get_short();
	if ( subscr_list( attr ) )
	{
		fetalrec.add_attr( attr[0].name, attr[0].form, attr[0].value );
		fetalrec.add_attr( attr[1].name, attr[1].form, attr[1].value );
		fetalrec.add_attr( attr[2].name, attr[2].form, attr[2].value );
		fetalrec.add_attr( attr[3].name, attr[3].form, attr[3].value );
	}
}

Attribute *
Bdibuild::build_record( long offset )
{
	short		attrname;
	long		word;

	word = get_long();
	if ( word <= 8 )
		return 0;
	length = word - 6;
	nextoff = offset + word;
	tag = get_short();
	fetalrec.add_attr( an_tag, af_tag, tagname[tag] );
	if ( tag != TAG_source_file )
		fetalrec.add_attr( an_parent, af_symbol, 0L );
	while ( length > 0 )
	{
		attrname = get_short();
		switch( attrname )
		{
			case AT_padding:
				break;
			case AT_sibling:
				sibling();
				break;
			case AT_location:
				location();
				break;
			case AT_name:
				name();
				break;
			case AT_fund_type:
				fund_type();
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
			case AT_byte_size:
				byte_size();
				break;
			case AT_bit_offset:
				bit_offset();
				break;
			case AT_bit_size:
				bit_size();
				break;
			case AT_stmt_list:
				stmt_list();
				break;
			case AT_low_pc:
				low_pc();
				break;
			case AT_high_pc:
				high_pc();
				break;
			case AT_element_list:
				element_list();
				break;
			case AT_subscr_data:
				subscr_data();
				break;
			case AT_language:
			case AT_ordering:
			case AT_dimensions:
			default:
				skip_attribute( attrname );
				break;
		}
	}
	return fetalrec.put_record();
}

long
Bdibuild::first_file()
{
	return bdi.entry_offset();
}

Attribute *
Bdibuild::make_record( long offset )
{
	Attribute *	attribute;

	if ( offset == 0 )
	{
		return 0;
	}
	else if ( reflist.lookup(offset,attribute) )
	{
		return attribute;
	}
	else if( (ptr = bdi.get_entry(offset)) == 0 )
	{
		return 0;
	}
	else if ( (attribute = build_record( offset )) == 0 )
	{
		return 0;
	}
	else
	{
		reflist.add( offset, attribute );
		return attribute;
	}
}

int
Bdibuild::get_syminfo( long offset, Syminfo & syminfo )
{
	long	word;
	short	tag;
	int	len;
	short	attrname;

	if ( (ptr = bdi.get_entry( offset )) == 0 )
	{
		return 0;
	}
	word = get_long();
	if ( word <= 8 )
	{
		syminfo.type = st_none;
		syminfo.bind = sb_weak;
		syminfo.name = 0;
		syminfo.sibling = offset + word;
		syminfo.child = 0;
		syminfo.lo = 0;
		syminfo.hi = 0;
		return 1;
	}
	nextoff = offset + word;
	length = word - 6;
	tag = get_short();
	switch ( tag )
	{
		case TAG_padding:
			return 0;
		case TAG_global_variable:
			syminfo.bind = sb_global;
			syminfo.type = st_object;
			break;
		case TAG_global_subroutine:
			syminfo.bind = sb_global;
			syminfo.type = st_func;
			break;
		case TAG_subroutine:
			syminfo.type = st_func;
			syminfo.bind = sb_local;
			break;
		case TAG_local_variable:
		case TAG_formal_parameter:
			syminfo.type = st_object;
			syminfo.bind = sb_local;
			break;
		case TAG_source_file:
			syminfo.type = st_file;
			syminfo.bind = sb_global;	// meaningful?
			break;
		default:
			syminfo.type = st_none;
			syminfo.bind = sb_weak;
			break;
	}
	syminfo.name = 0;
	syminfo.sibling = nextoff;
	syminfo.child = 0;
	syminfo.lo = 0;
	syminfo.hi = 0;
	while ( length > 0 )
	{
		attrname = get_short();
		switch ( attrname )
		{
			case AT_name:
				syminfo.name = (long)ptr;
				len = ::strlen(ptr) + 1;
				ptr += len;
				length -= len;
				break;
			case AT_sibling:
				word = get_long();
				syminfo.sibling = entry_offset + word - entry_base;
				syminfo.child = nextoff;
				break;
			case AT_low_pc:
				syminfo.lo = get_long();
				break;
			case AT_high_pc:
				syminfo.hi = get_long();
				break;
			default:
				skip_attribute( attrname );
		}
	}
	return 1;
}

char *
Bdibuild::get_name( long p )
{
	return (char*)p;
}

int
Bdibuild::cache( long offset, long size )
{
	return bdi.cache( offset, size );
}

Lineinfo *
Bdibuild::line_info( long offset )
{
	Lineinfo *	lineinfo;
	long		line;
	long		pcval;
	long		base_address;
	short		delta;

	lineinfo = 0;
	if ( (ptr = bdi.get_lineno( offset )) != 0 )
	{
		length = get_long();
		base_address = get_long();
		while ( length > 0 )
		{
			line = get_long();
			get_short();
			delta = get_long();
			pcval =  base_address + delta;
			if ( line != 0 )
			{
				fetalline.add_line( pcval, line );
			}
			else
			{
				lineinfo = fetalline.put_line( pcval );
				break;
			}
		}
	}
	return lineinfo;
}
