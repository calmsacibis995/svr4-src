#ident	"@(#)sdb:libsymbol/common/Coffbuild.C	1.8"
#include	"Coffbuild.h"
#include	"Locdesc.h"
#include	"Syminfo.h"
#include	"Type.h"
#include	"Tag.h"
#include	"builder.h"
#include	<string.h>
#include	<malloc.h>

#define equal(x,y)	(strcmp((x),(y)) == 0)

Coffbuild::Coffbuild( int fd ): coff(fd)
{
	linedisp = coff.line_info();
	file_offset = 0;
	global_offset = 0;
	string_offset = 0;
}

long
Coffbuild::past_ef( long offset )
{
	struct syment	sym3;
	union auxent	aux3;
	long		ofs,nxt;

	ofs = offset;
	while ( (nxt = coff.get_symbol( ofs, sym3, aux3 )) != 0 )
	{
		if ( equal( sym3.n_name, ".ef" ) )
		{
			return nxt;
		}
		ofs = nxt;
	}
	return 0;
}

void
Coffbuild::find_arcs( long & sibofs, long & childofs )
{
	name = sym.n_name;
	file_offset = coff.first_symbol();
	if (sym.n_sclass == C_FILE)
	{
		childofs = nextofs;
		sibofs = file_offset + sym.n_value*SYMESZ;
	}
	else if ( equal(name,".bf") && (sym.n_scnum < 0) )
	{
		sibofs = past_ef( nextofs );
		childofs = 0;
	}
	else if ( ((sym.n_type >> 4) & 0x3) == DT_FCN)
	{
		sibofs = file_offset + aux.x_sym.x_fcnary.x_fcn.x_endndx*SYMESZ;
		if ( sym.n_scnum < 0 )
		{
			sibofs = nextofs;
			childofs = 0;
		}
		else if (sibofs != (nextofs))
		{
			childofs = nextofs;
			sibofs = file_offset + aux.x_sym.x_fcnary.x_fcn.x_endndx*SYMESZ;
		}
		else
		{
			sibofs = nextofs;
			childofs = 0;
		}
	}
	else if (sym.n_sclass == C_STRTAG ||
		sym.n_sclass == C_UNTAG  ||
		sym.n_sclass == C_ENTAG)
	{
		childofs = nextofs;
		sibofs = file_offset + aux.x_sym.x_fcnary.x_fcn.x_endndx*SYMESZ;
	}
	else if ((sym.n_sclass == C_EOS) ||
		 ((sym.n_sclass == C_FCN) && equal(name,".ef")) ||
		 ((sym.n_sclass == C_BLOCK) && equal(name,".eb")))
	{
		sibofs = 0;
		childofs = 0;
	}
	else if (((sym.n_sclass == C_FCN) && equal(name,".bf")) ||
		equal(name,".target") ||
		equal(name,".comment") || equal(name,".data") ||
		equal(name,".text") || equal(name,".bss") ||
		equal(name,".init")) 
	{
		sibofs = nextofs;
		childofs = 0;
	}
	else if ( (sym.n_sclass == C_BLOCK) && equal(name,".bb") )
	{
		childofs = nextofs;
		sibofs = file_offset + aux.x_sym.x_fcnary.x_fcn.x_endndx*SYMESZ;
	}
	else
	{
		sibofs = nextofs;
		childofs = 0;
	}
}

void
Coffbuild::get_arcs()
{
	long		sibofs,childofs;

	find_arcs(sibofs,childofs);
	if ( sym.n_sclass == C_FILE )
	{
		fetalrec.add_attr(an_sibling,af_cofffile,sibofs);
		fetalrec.add_attr(an_child,af_coffrecord,childofs);
		fetalrec.add_attr(an_scansize,af_int,sibofs-childofs);
	}
	else if ( sibofs && childofs )
	{
		fetalrec.add_attr(an_parent,af_coffrecord,0L);
		fetalrec.add_attr(an_sibling,af_coffrecord,sibofs);
		fetalrec.add_attr(an_child,af_coffrecord,childofs);
		fetalrec.add_attr(an_scansize,af_int,sibofs-childofs);
	}
	else if ( sibofs )
	{
		fetalrec.add_attr(an_parent,af_coffrecord,0L);
		fetalrec.add_attr(an_sibling,af_coffrecord,sibofs);
	}
	else if ( childofs )
	{
		fetalrec.add_attr(an_parent,af_coffrecord,0L);
		fetalrec.add_attr(an_child,af_coffrecord,childofs);
	}
	else
	{
		fetalrec.add_attr(an_parent,af_coffrecord,0L);
	}
}

void
Coffbuild::get_data()
{
	long		offset;
	struct syment	sym2;
	union auxent	aux2;
	Iaddr		lopc,hipc;
	long		size,sibofs,childofs;
	int		textsectno;

	textsectno = coff.sectno( ".text" );
	switch (sym.n_sclass)
	{
		case C_FILE:
			fetalrec.add_attr(an_tag,af_tag,t_sourcefile);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(aux.x_file.x_fname));
			find_arcs( sibofs, childofs );
			fetalrec.add_attr(an_lineinfo,af_coffline,childofs);
			size =  sibofs - childofs + 2*SYMESZ;
			coff.cache( childofs, size );
			get_pc_info( childofs, lopc, hipc );
			fetalrec.add_attr(an_lopc,af_addr,lopc);
			fetalrec.add_attr(an_hipc,af_addr,hipc);
			break;
		case C_AUTO:
		case C_REG:
			fetalrec.add_attr(an_tag,af_tag,t_variable);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			break;
		case C_ARG:
		case C_REGPARM:
			fetalrec.add_attr(an_tag,af_tag,t_argument);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			break;
		case C_STAT:
		case C_EXT:
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			if ( sym.n_scnum != textsectno )
			{
				fetalrec.add_attr(an_tag,af_tag,t_global);
			}
			else if ( sym.n_numaux != 0 )
			{
				fetalrec.add_attr(an_tag,af_tag,t_entry);
				fetalrec.add_attr(an_lopc,af_addr,sym.n_value);
				fetalrec.add_attr(an_hipc,af_addr,sym.n_value
					+ aux.x_sym.x_misc.x_fsize);
			}
			else
			{
				fetalrec.add_attr(an_tag,af_tag,t_entry);
				fetalrec.add_attr(an_lopc,af_addr,sym.n_value);
				fetalrec.add_attr(an_hipc,af_addr,0L);
			}
			break;
		case C_STRTAG:
			fetalrec.add_attr(an_tag,af_tag,t_structuretype);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			fetalrec.add_attr(an_bytesize,af_int,aux.x_sym.x_misc.x_lnsz.x_size);
			break;
		case C_UNTAG:
			fetalrec.add_attr(an_tag,af_tag,t_uniontype);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			fetalrec.add_attr(an_bytesize,af_int,aux.x_sym.x_misc.x_lnsz.x_size);
			break;
		case C_ENTAG:
			fetalrec.add_attr(an_tag,af_tag,t_enumtype);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			fetalrec.add_attr(an_bytesize,af_int,aux.x_sym.x_misc.x_lnsz.x_size);
			break;
		case C_TPDEF:
			fetalrec.add_attr(an_tag,af_tag,t_typedef);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			break;
		case C_MOS:
			fetalrec.add_attr(an_tag,af_tag,t_structuremem);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			break;
		case C_MOE:
			fetalrec.add_attr(an_tag,af_tag,t_enumlittype);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			fetalrec.add_attr(an_litvalue,af_int,sym.n_value);
			break;
		case C_MOU:
			fetalrec.add_attr(an_tag,af_tag,t_unionmem);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			break;
		case C_FIELD:
			fetalrec.add_attr(an_tag,af_tag,t_bitfield);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			fetalrec.add_attr(an_bitsize,af_int
				,aux.x_sym.x_misc.x_lnsz.x_size);
			break;
		case C_BLOCK:
			fetalrec.add_attr(an_tag,af_tag,t_block);
			fetalrec.add_attr(an_lopc,af_addr,sym.n_value);
			offset = file_offset + (aux.x_sym.x_fcnary.x_fcn.x_endndx-2)
						* SYMESZ;
			if ( coff.get_symbol(offset,sym2,aux2) != 0 )
			{
				fetalrec.add_attr(an_hipc,af_addr,sym2.n_value);
			}
			break;
		case C_LABEL:
		case C_ULABEL:
			fetalrec.add_attr(an_tag,af_tag,t_label);
			fetalrec.add_attr(an_name,af_stringndx,
				make_string(coff.get_name(sym)));
			break;
	}
}

void
Coffbuild::get_type_C()
{
	int		typ;
	int		dt,deriv;
	Attr_value	value;
	Attr_form	form;
	int		i;
	int		bound;
	short		d[7];

	typ = sym.n_type & 0xf;
	form = af_none;
	switch (typ)
	{
		case T_NULL:
			form = af_fundamental_type;
			value.word = ft_none;
			break;
		case T_CHAR:
			value.word = ft_char;
			form = af_fundamental_type;
			break;
		case T_SHORT:
			form = af_fundamental_type;
			value.word = ft_short;
			break;
		case T_INT:
			form = af_fundamental_type;
			value.word = ft_int;
			break;
		case T_LONG:
			form = af_fundamental_type;
			value.word = ft_long;
			break;
		case T_FLOAT:
			form = af_fundamental_type;
			value.word = ft_sfloat;
			break;
		case T_DOUBLE:
			form = af_fundamental_type;
			value.word = ft_lfloat;
			break;
		case T_STRUCT:
		case T_UNION:
		case T_ENUM:
			if (aux.x_sym.x_tagndx == 0)
			{
				form = af_none;
				value.word = 0;
			}
			else
			{
				form = af_coffrecord;
				value.word = file_offset + aux.x_sym.x_tagndx * SYMESZ;
			}
			break;
		case T_MOE:
			form = af_none;
			value.word = 0;
			break;
		case T_UCHAR:
			form = af_fundamental_type;
			value.word = ft_uchar;
			break;
		case T_USHORT:
			form = af_fundamental_type;
			value.word = ft_ushort;
			break;
		case T_UINT:
			form = af_fundamental_type;
			value.word = ft_uint;
			break;
		case T_ULONG:
			form = af_fundamental_type;
			value.word = ft_ulong;
			break;
	}
	deriv = (sym.n_type & 0xffff) >> 4;
	bound = 0;
	for ( i = 1 ; i <= 6 ; i ++ )
	{
		dt = deriv & 0x3;
		if (dt == DT_ARY) bound++ ;
		d[i] = dt;
		deriv = deriv >> 2;
	}
	for ( i = 6 ; i > 0 ; i -- )
	{
		switch (d[i])
		{
			case DT_PTR:
				fetaltype.add_attr(an_tag,af_tag,t_pointertype);
				fetaltype.add_attr(an_basetype,form,value);
				fetaltype.add_attr(an_bytesize,af_int,4L);
				value.ptr = fetaltype.put_record();
				form = af_symbol;
				break;
			case DT_ARY:
				fetaltype.add_attr(an_tag,af_tag,t_arraytype);
				fetaltype.add_attr(an_elemtype,form,value);
				fetaltype.add_attr(an_bytesize,af_int,
					aux.x_sym.x_misc.x_lnsz.x_size);
				fetaltype.add_attr(an_subscrtype,af_fundamental_type,ft_sint);
				fetaltype.add_attr(an_lobound,af_int,0L);
				fetaltype.add_attr(an_hibound,af_int,
					aux.x_sym.x_fcnary.x_ary.x_dimen[--bound]);
				value.ptr = fetaltype.put_record();
				form = af_symbol;
				break;
			case DT_FCN:
				fetaltype.add_attr(an_tag,af_tag,t_functiontype);
				fetaltype.add_attr(an_resulttype,form,value);
				value.ptr = fetaltype.put_record();
				form = af_symbol;
				break;
			case DT_NON:
				break;
		}
	}
	fetalrec.add_attr(an_type,form,value);
}

void
Coffbuild::get_type()
{
	switch(sym.n_sclass)
	{
		case C_STRTAG:
		case C_UNTAG:
		case C_ENTAG:
		case C_FILE:
		case C_BLOCK:
		case C_LABEL:
		case C_ULABEL:
			break;
		default:
			get_type_C();
	}
}

void
Coffbuild::get_addr_C_3B()
{
	Locdesc	locdesc;

	switch(sym.n_sclass)
	{
		case C_ARG:
			locdesc.clear().basereg(REG_AP).offset(sym.n_value).add();
			fetalrec.add_attr(an_location, af_locdesc,
				make_chunk(locdesc.addrexp(),locdesc.size()));
			break;
		case C_REG:
		case C_REGPARM:
			locdesc.clear().reg(sym.n_value);
			fetalrec.add_attr(an_location,af_locdesc,
				make_chunk(locdesc.addrexp(),locdesc.size()));
			break;
		case C_LABEL:
		case C_EXT:
		case C_FCN:
		case C_BLOCK:
		case C_STAT:
		case C_ULABEL:
		case C_USTATIC:
			locdesc.clear().addr(sym.n_value);
			fetalrec.add_attr(an_location,af_locdesc,
				make_chunk(locdesc.addrexp(),locdesc.size()));
			break;
		case C_MOU:
		case C_MOS:
			locdesc.clear().offset(sym.n_value).add();
			fetalrec.add_attr(an_location,af_locdesc,
				make_chunk(locdesc.addrexp(),locdesc.size()));
			break;
		case C_AUTO:
			locdesc.clear().basereg(REG_FP).offset(sym.n_value).add();
			fetalrec.add_attr(an_location,af_locdesc,
				make_chunk(locdesc.addrexp(),locdesc.size()));
			break;
		case C_FIELD:
			locdesc.clear().offset(sym.n_value >> 3).add();
			fetalrec.add_attr(an_location,af_locdesc,
				make_chunk(locdesc.addrexp(),locdesc.size()));
			fetalrec.add_attr(an_bitoffs,af_int,(sym.n_value % 8));
			break;
	}
}

void
Coffbuild::get_addr_C()
{
	get_addr_C_3B();
}

void
Coffbuild::get_addr()
{
	get_addr_C();
}

int
Coffbuild::get_syminfo( long offset, Syminfo & syminfo )
{
	int	textsectno;

	if ( (nextofs = coff.get_symbol( offset, sym, aux )) == 0 )
	{
		syminfo.bind = sb_weak;
		return 0;
	}
	else if ( sym.n_sclass == C_FILE )
	{
		syminfo.name = (long) aux.x_file.x_fname;
		syminfo.lo = 0;
		syminfo.hi = 0;
	}
	else if (((sym.n_type >>4) & 0x3 ) == DT_FCN )
	{
		syminfo.name = (long) coff.get_name( sym );
		syminfo.lo = sym.n_value;
		syminfo.hi = sym.n_value + aux.x_sym.x_misc.x_fsize;
	}
	else if ( equal( sym.n_name, ".text" ) )
	{
		syminfo.name = (long) sym.n_name;
		syminfo.lo = sym.n_value;
		syminfo.hi = sym.n_value + aux.x_scn.x_scnlen;
	}
	else
	{
		syminfo.name = (long) coff.get_name( sym );
		syminfo.lo = sym.n_value;
		syminfo.hi = sym.n_value;
	}
	switch ( sym.n_sclass )
	{
		case C_FILE:	syminfo.bind = sb_global;	break;
		case C_EXT:	syminfo.bind = sb_global;	break;
		case C_STAT:	syminfo.bind = sb_local;	break;
		default:	syminfo.bind = sb_weak;		break;
	}
	textsectno = coff.sectno( ".text" );
	switch ( sym.n_sclass )
	{
		case C_FILE:
			syminfo.type = st_file;
			break;
		case C_EXT:
		case C_STAT:
			if ( sym.n_scnum == textsectno )
			{
				syminfo.type = st_func;
			}
			else
			{
				syminfo.type = st_object;
			}
			break;
		default:
			syminfo.type = st_none;
			break;
	}
	find_arcs( syminfo.sibling, syminfo.child );
	syminfo.resolved = (sym.n_scnum > 0);
	return 1;
}

int
Coffbuild::find_record( long offset, int want_file )
{
	Syminfo		syminfo;
	long		disp;

	disp = offset;
	while ( get_syminfo( disp, syminfo ) != 0 )
	{
		if ( want_file && (syminfo.type == st_file) )
		{
			return 1;
		}
		else if ( !want_file && (syminfo.type == st_file) )
		{
			return 0;
		}
		name = (char*)syminfo.name;
		if ( equal( name, ".ef" ) || equal( name, ".eb" ) )
		{
			return 0;
		}
		else if (equal(name,".bf") || equal(name,".target") ||
			equal(name,".comment") || equal(name,".data") ||
			equal(name,".text") || equal(name,".bss") ||
			equal(name,".init"))
		{
			disp = syminfo.sibling;
		}
		else
		{
			return 1;
		}

	}
}

Attribute *
Coffbuild::make_record( long offset, int want_file )
{
	Attribute *	attribute;

	if ( offset == 0 )
	{
		return 0;
	}
	else if ( find_record( offset, want_file ) == 0 )
	{
		return 0;
	}
	else if ( reflist.lookup( offset, attribute ) )
	{
		return attribute;
	}
	else if ( sym.n_sclass == C_EOS )
	{
		return 0;
	}
	get_arcs();
	get_data();
	get_type();
	get_addr();
	attribute = fetalrec.put_record();
	reflist.add(offset,attribute);
	return attribute;
}

void
Coffbuild::get_pc_info( long offset, Iaddr & lopc, Iaddr & hipc )
{
	long		size;
	Syminfo		syminfo;

	lopc = 0;
	hipc = 0;
	while ( get_syminfo( offset , syminfo ) )
	{
		name = (char*) syminfo.name;
		if ( syminfo.type == st_file )
		{
			size = syminfo.sibling - syminfo.child + 2*SYMESZ;
			coff.cache( syminfo.child, size );
			offset = syminfo.child;
		}
		else if ( equal(name,".text") )
		{
			lopc = syminfo.lo;
			hipc = syminfo.hi;
			return;
		}
		else
		{
			offset = syminfo.sibling;
		}
	}
}

long
Coffbuild::first_symbol()
{
	if  ( file_offset == 0 )
	{
		file_offset = coff.first_symbol();
	}
	return file_offset;
}

long
Coffbuild::first_global()
{
	long		offset;
	Syminfo		syminfo;

	if  ( global_offset != 0 )
	{
		return global_offset;
	}
	else if ( (offset = coff.first_symbol()) == 0 )
	{
		return 0;
	}
	file_offset = offset;
	while ( get_syminfo( offset, syminfo ) != 0 )
	{
		if ( syminfo.type != st_file )
		{
			break;
		}
		else
		{
			offset = syminfo.sibling;
		}
	}
	global_offset = offset;
	return global_offset;
}

char *
Coffbuild::make_string(char * p)
{
	char *		s;
	register int	len;

	len = ::strlen(p)+1;
	s = ::malloc(len);
	::memcpy(s,p,len);
	return s;
}

void *
Coffbuild::make_chunk(void * p, int len)
{
	char *		s;

	s = ::malloc(len);
	::memcpy(s,(char*)p,len);
	return s;
}

char *
x_get_lineno( char * b, struct lineno & line )
{
	::memcpy((char*)&line,b,LINESZ);
	return b+LINESZ;
}

char *
Coffbuild::get_fcn_lineinfo( char * b, char * end )
{
	long		pcval;
	int		lowline;
	struct lineno	line;
	int		i;

	pcval = sym.n_value;
	coff.get_symbol( nextofs, sym, aux );
	lowline = aux.x_sym.x_misc.x_lnsz.x_lnno;
	i = (end - b)/ LINESZ;
	b = x_get_lineno(b,line);
	for ( ; i > 0 && line.l_lnno != 0 ; i-- )
	{
		fetalline.add_line(pcval, lowline+line.l_lnno-1);
		b = x_get_lineno(b,line);
		pcval = line.l_addr.l_paddr;
	}
	return b;
}

Lineinfo *
Coffbuild::line_info( long offset, long size )
{
	int	notext,nofcn;
	long	p,ignore;
	long	loffset,foffset,lncnt;
	Iaddr	last_hipc;
	Lineinfo *	lineinfo;

	p = offset;
	loffset = foffset = 0;
	notext = 1;
	nofcn = 1;
	coff.cache( offset, size );
	while ( notext || nofcn )
	{
		if ( (nextofs = coff.get_symbol(p, sym, aux )) == 0 )
		{
			notext = nofcn = 0;
		}
		else if ( nofcn && ( ((sym.n_type >> 4) & 0x3) == DT_FCN ) )
		{
			foffset = p;
			loffset = aux.x_sym.x_fcnary.x_fcn.x_lnnoptr;
			nofcn = 0;
			find_arcs(p,ignore);
		}
		else if (sym.n_sclass == C_FILE)
		{
			notext = nofcn = 0;
		}
		else if ( notext && equal(sym.n_name,".text"))
		{
			lncnt = aux.x_scn.x_nlinno;
			last_hipc = sym.n_value + aux.x_scn.x_scnlen;
			notext = 0;
			find_arcs(p,ignore);
		}
		else
		{
			find_arcs(p,ignore);
		}
	}
	if ( (loffset >= linedisp) && (loffset < file_offset) && (lncnt > 0) )
	{
		get_lineinfo( loffset, lncnt, foffset );
		lineinfo = fetalline.put_line(last_hipc);
	}
	else
	{
		lineinfo = 0;
	}
	return lineinfo;
}

void
Coffbuild::get_lineinfo( long loffset, long lncnt, long foffset )
{
	char *	b;
	char *	cofflbuf;
	long	ignore;
	long	offset;

	cofflbuf = malloc( lncnt * LINESZ );
	cofflbuf = coff.get_lineno( loffset, lncnt );
	b = cofflbuf + LINESZ;
	offset = foffset;
	while ( (nextofs = coff.get_symbol(offset, sym, aux )) != 0 )
	{
		find_arcs( offset, ignore );
		if ( ((sym.n_type >> 4) & 0x3) == DT_FCN )
		{
			b = get_fcn_lineinfo( b, cofflbuf + lncnt * LINESZ );
		}
		else if ( sym.n_sclass == C_FILE )
		{
			break;
		}
	}
}

void
Coffbuild::cache_globals()
{
	coff.cache( global_offset, string_offset - global_offset );
}

void
Coffbuild::de_cache()
{
	coff.de_cache();
}

void
Coffbuild::cache( long offset, long size )
{
	coff.cache( offset, size );
}
