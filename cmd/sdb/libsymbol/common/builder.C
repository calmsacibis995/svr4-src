#ident	"@(#)sdb:libsymbol/common/builder.C	1.3"
#include	"builder.h"
#include	"Interface.h"
#include	"Tag.h"

Attribute *
find_attr(Attribute * a, Attr_name name)
{
	register Attribute *	p;
	Attribute *		rec;
	static long		last;

	if ( a != 0 )
	{
		p = rec = a;
		last = p->value.word - 1;
		rec[last].name = name;
		while (p->name != name) p++ ;
		rec[last].name = an_nomore;
		return (p == rec+last) ? 0 : p ;
	}
	else
		return 0;
}

char * attrname[] = {
			"an_nomore",
			"an_tag",
			"an_name",
			"an_child",
			"an_sibling",
			"an_parent",
			"an_count",
			"an_type",
			"an_elemtype",
			"an_elemspan",
			"an_subscrtype",
			"an_lobound",
			"an_hibound",
			"an_basetype",
			"an_resulttype",
			"an_argtype",
			"an_bytesize",
			"an_bitsize",
			"an_bitoffs",
			"an_litvalue",
			"an_stringlen",
			"an_lineinfo",
			"an_location",
			"an_lopc",
			"an_hipc",
			"an_hipc",
			"an_visibility",
			"an_scansize",
		};

char *	attrform[] = {
			"af_none",
			"af_tag",
			"af_int",
			"af_locdesc",
			"af_stringndx",
			"af_coffrecord",
			"af_coffline",
			"af_coffpc",
			"af_spidoffs",
			"af_fundamental_type",
			"af_symndx",
			"af_reg",
			"af_addr",
			"af_local",
			"af_visibility",
			"af_lineinfo",
			"af_attrlist",
			"af_cofffile",
			"af_symbol",
			"af_bdioffs",
			"af_bdiline",
			};

char *	typename[] = {
			"none",
			"signed 1 byte integer",
			"unsigned 1 byte integer",
			"signed 2 byte integer",
			"unsigned 2 byte integer",
			"signed 3 byte integer",
			"unsigned 3 byte integer",
			"signed 4 byte integer",
			"unsigned 4 byte integer",
			"1 byte boolean",
			"2 byte boolean",
			"3 byte boolean",
			"4 byte boolean",
			"signed character",
			"unsigned character",
			"short float",
			"long float",
			"address",
			"short address",
			"encoded offset",
			"short complex",		// Fortran COMPLEX
			"long complex",			// Fortran DCOMPLEX
			"charstring",
			"language specific type",
		};

#undef DEFTAG
char *	tagname[] = {
#define DEFTAG(VAL, NAME)	NAME,
#include "Tag1.h"
#undef DEFTAG
		};

char *
name_string( Attr_name x )
{
	return attrname[x];
}

char *
form_string( Attr_form x )
{
	return attrform[x];
}

char *
type_string( Fund_type x )
{
	if ( x < 24 )
		return typename[x];
	else
		return "NO DESCRIPTION FOR THIS TYPE";
}

char *
tag_string( Tag x )
{
	return tagname[x];
}

void
print_attr( Attribute * x )
{
	if (x != 0 )
	{
		printx("--name:\t%s\tform:\t%s\tvalue:\t%#x\n",attrname[x->name],
			attrform[x->form],x->value);
	}
}

void
print_attrlist( Attribute * attrlist )
{
	Attribute *	x;
	char *		s;

	printx("attribute list is at address %#x\n",attrlist);
	if (attrlist != 0)
	{
		x = attrlist;
		do {
			switch (x->form)
			{
				case af_stringndx:
					s = (char*)x->value.ptr;
					printx("name:\t%s\t\tform:\t%s\tvalue:\t%s\n",
					attrname[x->name],attrform[x->form],s);
					break;
				case af_tag:
					printx("name:\t%s\tform:\t%s\tvalue:\t%s\n",
					attrname[x->name],attrform[x->form], tagname[x->value.word]);
					break;
				default:
					printx("name:\t%s\tform:\t%s\tvalue:\t%#x\n",
					attrname[x->name],attrform[x->form],x->value);
			}
		} while ((x++)->name != an_nomore);
	}
}
