#ident	"@(#)sdb:libsymbol/common/Evaluator.C	1.10"
#include	"Evaluator.h"
#include	"Syminfo.h"
#include	"builder.h"
#include	"Machine.h"
#include	<osfcn.h>
#include	<string.h>

enum ftyp {	ftyp_unk, ftyp_coff, ftyp_elf	};

Evaluator::Evaluator( int fd ) : bdibuild(fd),coffbuild(fd),elfbuild(fd)
{
	fdesc = fd;
	first_offset = 0;
	first_record = 0;
	next_disp = 0;
	elf_disp = 0;
	at_end = 0;
	noglobals = 1;
	no_elf_syms = 1;
	current_entry = 0;
	file_type = ftyp_unk;
}

enum ftyp
Evaluator::get_file_type()
{
	union	{
		long	lmagic;
		short	smagic;
	} magic;

	int	len = sizeof( magic );

	if ( ::lseek( fdesc, 0L, 0 ) == -1 )
	{
		return ftyp_unk;
	}
	else if ( ::read( fdesc, (char*)&magic, len ) != len )
	{
		return ftyp_unk;
	}
	else if ( magic.lmagic == ELFMAGIC )
	{
		return ftyp_elf;
	}
	else if ( magic.smagic == COFFMAGIC )
	{
		return ftyp_coff;
	}
	else
	{
		return ftyp_unk;
	}
}

Attribute *
Evaluator::first_file()
{
	if ( first_record != 0 )
	{
		return first_record;
	}
	else if ( (file_type = get_file_type()) == ftyp_unk )
	{
		return 0;
	}
	else if ( file_type == ftyp_coff )
	{
		first_offset = coffbuild.first_symbol();
		first_record = coffbuild.make_record( first_offset, 1 );
	}
	else if ( file_type != ftyp_elf )
	{
		return 0;
	}
	else if ( (first_offset = bdibuild.first_file()) != 0 )
	{
		first_record = bdibuild.make_record( first_offset );
	}
	else if ( (first_offset = elfbuild.first_symbol()) != 0 )
	{
		first_record = elfbuild.make_record( first_offset );
	}
	return first_record;
}

Attribute *
Evaluator::arc( Attribute * attrlist, Attr_name attrname )
{
	Attribute *	a;

	if ( (a = find_attr( attrlist, attrname )) == 0 )
	{
		return 0;
	}
	else if ( a->form == af_symbol )
	{
		return a;
	}
	else if ( a->name == an_child )
	{
		return add_children( a, attrlist );
	}
	else if ( a->name == an_parent )
	{
		return add_parent( a, attrlist );
	}
	else
	{
		return add_node( a );
	}
}

Attribute *
Evaluator::add_node( Attribute * a )
{
	long	offset;

	offset = a->value.word;
	switch ( a->form )
	{
		case af_symbol:
			return a;
		case af_coffrecord:
			a->value.ptr = coffbuild.make_record( offset, 0 );
			a->form = af_symbol;
			return a;
		case af_cofffile:
			a->value.ptr = coffbuild.make_record( offset, 1 );
			a->form = af_symbol;
			return a;
		case af_bdioffs:
			a->value.ptr = bdibuild.make_record( offset );
			a->form = af_symbol;
			return a;
		case af_elfoffs:
			a->value.ptr = elfbuild.make_record( offset );
			a->form = af_symbol;
			return a;
		default:
			return a;
	}
}

Attribute *
Evaluator::add_parent( Attribute * b, Attribute * ancestor )
{
	Attribute *	p;

	if ( (p = find_attr( b, an_parent )) != 0 )
	{
		p->value.ptr = ancestor;
		p->form = af_symbol;
	}
	return p;
}

Attribute *
Evaluator::add_children( Attribute * a, Attribute * ancestor )
{
	long		offset, limit, scansize;
	Attribute	*b, *x;

	if ( (b = find_attr( ancestor, an_scansize )) == 0 )
	{
		return 0;
	}
	offset = a->value.word;
	scansize = b->value.word;
	switch ( file_type )
	{
		case ftyp_coff:
			coffbuild.cache( offset, scansize );
			break;
		case ftyp_elf:
			bdibuild.cache( offset, scansize );
			break;
		default:
			return 0;
	}
	limit = offset + scansize;
	x = a;
	while ( x != 0 )
	{
		offset = x->value.word;
		if ( (x->form == af_bdioffs) && (offset < limit) )
		{
			b = bdibuild.make_record( offset );
			add_parent( b, ancestor );
			x->value.ptr = b;
			x->form = af_symbol;
			x = find_attr( b, an_sibling );
		}
		else if ( (x->form == af_coffrecord) && (offset < limit) )
		{
			b = coffbuild.make_record( offset, 0 );
			add_parent( b, ancestor );
			x->value.ptr = b;
			x->form = af_symbol;
			x = find_attr( b, an_sibling );
		}
		else
		{
			x->value.ptr = 0;
			x->form = af_symbol;
			x = 0;
		}
	}
	return a;
}

Attribute *
Evaluator::attribute( Attribute * attrlist, Attr_name attr_name )
{
	Attribute *	a;
	Attribute *	b;
	Attribute *	c;
	Iaddr		lopc,hipc;
	long		size;

	a = find_attr(attrlist,attr_name);
	if ( a == 0 ) return a;
	switch (a->form)
	{
		case af_bdioffs:
			a->value.ptr = bdibuild.make_record(a->value.word);
			a->form = af_symbol;
			break;
		case af_bdiline:
			a->value.ptr = bdibuild.line_info(a->value.word);
			a->form = af_lineinfo;
			break;
         	case af_coffrecord:
                        a->value.ptr = coffbuild.make_record( a->value.word,0 );
                        a->form = af_symbol;
                        break;
		case af_coffline:
                        c = find_attr(attrlist,an_scansize);
                        size = (c != 0) ? c->value.word : 0 ;
                        a->value.ptr = coffbuild.line_info(a->value.word,size);
                        a->form = af_lineinfo;
                        break;
		case af_coffpc:
                        c = find_attr(attrlist,an_scansize);
                        size = (c != 0) ? c->value.word : 0 ;
                        coffbuild.get_pc_info(a->value.word,lopc,hipc);
                        if (attr_name == an_lopc)
                        {
                         	b = find_attr(attrlist,an_hipc);
                        }
                        else if (attr_name == an_hipc)
			{
                                b = a;
                                a = find_attr(attrlist,an_lopc);
			}
			if (a != 0 )
			{
                                a->value.word = lopc;
                                a->form = af_addr;
			}
			if (b != 0 )
			{
                                b->value.word = hipc;
                                b->form = af_addr;
			}
			break;
		default:
			break;
	}
	return a;
}

NameEntry *
Evaluator::get_global( char * name )
{
	char *		nom;
	Syminfo		syminfo;
	long		scansize;
	NameEntry *	n;

	if ( (file_type == ftyp_coff) && (next_disp != -1) )
	{
		if ( next_disp == 0 ) next_disp = coffbuild.first_symbol();
		while ( coffbuild.get_syminfo( next_disp, syminfo ) != 0 )
		{
			nom = (char*)syminfo.name;
			if ( syminfo.type == st_file )
			{
				scansize = syminfo.sibling - syminfo.child;
				coffbuild.cache( syminfo.child, scansize );
				next_disp = syminfo.child;
			}
			else if ( syminfo.bind != sb_global )
			{
				next_disp = syminfo.sibling;
			}
			else if ( !syminfo.resolved )
			{
				next_disp = syminfo.sibling;
			}
			else if ( name == 0 )
			{
				n = namelist.add( nom, next_disp, af_coffrecord );
				next_disp = syminfo.sibling;
				return n;
			}
			else if ( ::strcmp(name,nom) == 0 )
			{
				n = namelist.add( nom, next_disp, af_coffrecord );
				next_disp = syminfo.sibling;
				return n;
			}
			else
			{
				namelist.add( nom, next_disp, af_coffrecord );
				next_disp = syminfo.sibling;
			}
		}
		next_disp = -1;
	}

	if ( (file_type == ftyp_elf) && (next_disp != -1) )
	{
		if ( next_disp == 0 ) next_disp = bdibuild.first_file();
		while ( bdibuild.get_syminfo( next_disp, syminfo ) != 0 )
		{
			nom = bdibuild.get_name( syminfo.name );
			if ( syminfo.type == st_file )
			{
				scansize = syminfo.sibling - syminfo.child;
				bdibuild.cache( syminfo.child, scansize );
				next_disp = syminfo.child;
			}
			else if ( syminfo.bind != sb_global )
			{
				next_disp = syminfo.sibling;
			}
			else if ( name == 0 )
			{
				n = namelist.add( nom, next_disp, af_bdioffs );
				next_disp = syminfo.sibling;
				return n;
			}
			else if ( ::strcmp(name,nom) == 0 )
			{
				n = namelist.add( nom, next_disp, af_bdioffs );
				next_disp = syminfo.sibling;
				return n;
			}
			else
			{
				namelist.add( nom, next_disp, af_bdioffs );
				next_disp = syminfo.sibling;
			}
		}
		next_disp = -1;
	}

	if ( (file_type == ftyp_elf) && (elf_disp != -1) )
	{
		if ( elf_disp == 0 ) elf_disp = elfbuild.first_symbol();
		while ( elfbuild.get_syminfo( elf_disp, syminfo ) != 0 )
		{
			nom = elfbuild.get_name( syminfo.name );
			if ( ((syminfo.bind != sb_weak) &&
				(syminfo.bind != sb_global)) ||
				((syminfo.type != st_object) &&
				(syminfo.type != st_func)) ||
				(!syminfo.resolved) )
			{
				elf_disp = syminfo.sibling;
			}
			else if ( name == 0 )
			{
				n = namelist.add( nom, elf_disp, af_elfoffs );
				elf_disp = syminfo.sibling;
				if ( n != 0 ) return n;
			}
			else if ( ::strcmp( nom, name ) == 0 )
			{
				n = namelist.add( nom, elf_disp, af_elfoffs );
				elf_disp = syminfo.sibling;
				if ( n != 0 ) return n;
			}
			else
			{
				namelist.add( nom, elf_disp, af_elfoffs );
				elf_disp = syminfo.sibling;
			}
		}
		elf_disp = -1;
	}
	return 0;
}

NameEntry *	
Evaluator::first_global()
{
	NameEntry *	entry;

	at_end = 0;
	entry = (NameEntry*)namelist.tfirst();
	if ( entry != 0 )
	{
		current_entry = entry;
	}
	else if ( (entry = get_global(0)) != 0 )
	{
		current_entry = entry;
	}
	else
	{
		current_entry = 0;
	}
	return entry;
}

NameEntry *
Evaluator::next_global()
{
	NameEntry	* n;
	NameEntry	* entry;

	n = current_entry;
	if ( n == 0 )
	{
		entry = 0;
	}
	else if ( !at_end && ((entry = (NameEntry*)(n->next())) != 0 ) )
	{
		current_entry = entry;
	}
	else if ( (entry = get_global(0)) != 0 )
	{
		at_end = 1;
		current_entry = entry;
	}
	else
	{
		at_end = 1;
		current_entry = 0;
	}
	return entry;
}

Attribute *
Evaluator::evaluate( NameEntry * n )
{
	Attribute *	a;

	if ( n == 0 )
	{
		a = 0;
	}
	else if ( n->form == af_bdioffs )
	{
		a = bdibuild.make_record( n->value.word );
		n->value.ptr = a;
		n->form = af_symbol;
	}
	else if ( n->form == af_coffrecord )
	{
		a = coffbuild.make_record( n->value.word, 0 );
		n->value.ptr = a;
		n->form = af_symbol;
	}
	else if ( n->form == af_elfoffs )
	{
		a = elfbuild.make_record( n->value.word );
		n->value.ptr = a;
		n->form = af_symbol;
	}
	else
	{
		a = (Attribute *) n->value.ptr;
	}
	return a;
}

// Search for a NameEntry with the specified name
Attribute *
Evaluator::find_global( char * name )
{
	NameEntry *	n;
	NameEntry	node;
	Attribute *	a;

	if ( file_type == ftyp_unk )
	{
		file_type = get_file_type();
	}
	node.namep = name;
	n = (NameEntry*)namelist.tlookup(node);
	if ( n != 0 )
	{
		a = evaluate(n);
	}
	else if ( (n = get_global(name)) != 0 )
	{
		a = evaluate(n);
	}
	else
	{
		a = 0;
	}
	return a;
}
		
// Search for a AddrEntry containing a specified address
Attribute *
Evaluator::lookup_addr( Iaddr addr )
{
	AddrEntry	node;
	AddrEntry *	a;
	Attribute *	attrlist;
	long		offset;
	Syminfo		syminfo;

	node.loaddr = addr;
	node.hiaddr = addr;
	if ( (file_type == ftyp_coff) && no_elf_syms )
	{
		no_elf_syms = 0;
		offset = coffbuild.first_global();
		while ( coffbuild.get_syminfo( offset, syminfo ) != 0 )
		{
			if ( ((syminfo.bind == sb_local) ||
				(syminfo.bind == sb_global)) &&
				((syminfo.type == st_object) ||
				(syminfo.type == st_func)) &&
				syminfo.resolved )
                        {
                                addrlist.add( syminfo.lo, syminfo.hi, offset, af_coffrecord );
			}
			offset = syminfo.sibling;
		}
		addrlist.complete();
	}
	else if ( (file_type == ftyp_elf) && no_elf_syms )
	{
		no_elf_syms = 0;
		offset = elfbuild.first_symbol();
		while ( elfbuild.get_syminfo( offset, syminfo ) != 0 )
		{
			if ( ((syminfo.bind == sb_local) ||
				(syminfo.bind == sb_global)) &&
				((syminfo.type == st_object) ||
				(syminfo.type == st_func)) &&
				syminfo.resolved )
			{
				addrlist.add( syminfo.lo, syminfo.hi, offset, af_elfoffs );
			}
			offset = syminfo.sibling;
		}
		addrlist.complete();
	}
	a = (AddrEntry*)addrlist.tlookup(node);
	if ( a == 0 )
	{
		attrlist = 0;
	}
	else if ( a->form == af_symbol )
	{
		attrlist = (Attribute*)a->value.ptr;
	}
	else if ( a->form == af_elfoffs )
	{
		a->value.ptr = elfbuild.make_record(a->value.word);
		a->form = af_symbol;
		attrlist = (Attribute*)a->value.ptr;
	}
	else if ( a->form == af_coffrecord )
	{
		a->value.ptr = coffbuild.make_record( a->value.word, 0 );
		a->form = af_symbol;
		attrlist = (Attribute*)a->value.ptr;
	}
	else
	{
		attrlist = 0;
	}
	return attrlist;
}
