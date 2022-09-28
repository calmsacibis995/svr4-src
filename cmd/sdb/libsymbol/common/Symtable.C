#ident	"@(#)sdb:libsymbol/common/Symtable.C	1.4"
#include	"Symtable.h"
#include	"Evaluator.h"
#include	"Tag.h"
#include	"Tagcheck.h"
#include	"builder.h"
#include	<string.h>

#define equal(x,y)	(strcmp((x),(y))==0)

Symtable::Symtable( int fd )
{
	evaluator = new Evaluator( fd );
}

Symbol
Symtable::find_entry( Iaddr addr )
{
	Symbol		symbol;
	Attribute *	a;

	symbol = find_scope( addr );
	a = symbol.attribute(an_tag);
	while(a != 0 && a->form == af_tag && !stacktag(a->value.word))
	{
		symbol = symbol.arc(an_parent);
		a = symbol.attribute(an_tag);
	}
	if ( symbol.isnull() )
	{
		symbol.attrlist = evaluator->lookup_addr( addr );
	}
	symbol.evaluator = evaluator;
	return symbol;
}

Symbol
Symtable::find_scope ( Iaddr addr )
{
	Symbol	i,nearest;

	i = first_symbol();
	while ( !i.isnull() )
	{
		if ( addr < i.pc(an_lopc) )
		{
			i = i.arc(an_sibling);
		}
		else if ( addr >=  i.pc(an_hipc) )
		{
			i = i.arc(an_sibling);
		}
		else
		{
			nearest = i;
			i = i.arc(an_child);
		}
	}
	return nearest;
}

Symbol
Symtable::first_symbol()
{
	Symbol	symbol;

	symbol.evaluator = evaluator;
	symbol.attrlist = evaluator->first_file();
	return symbol;
}

int
Symtable::find_source( Iaddr pc, Symbol & symbol )
{
	Symbol		x;

	x = first_symbol();
	while ( !x.isnull() )
	{
		if ( pc < x.pc(an_lopc) )
		{
			x = x.arc(an_sibling);
		}
		else if ( pc >= x.pc(an_hipc) )
		{
			x = x.arc(an_sibling);
		}
		else
		{
			symbol = x;
			return 1;
		}
	}
	return 0;
}

int
Symtable::find_source( char * name, Symbol & symbol )
{
	Symbol		x;
	char *		s;

	x = first_symbol();
	while ( !x.isnull() )
	{
		if ( equal(name,x.name()) )
		{
			symbol = x;
			return 1;
		}
		else if ( (s = ::strrchr( x.name(), '/' )) == 0 )
		{
			x = x.arc( an_sibling );
		}
		else if ( equal( s+1, name ) )
		{
			symbol = x;
			return 1;
		}
		else
		{
			x = x.arc(an_sibling);
		}
	}
	return 0;
}

Symbol
Symtable::find_global( char * name )
{
	Symbol		symbol;
	Attribute *	a;

	if ( (a=evaluator->find_global( name )) != 0 )
	{
		symbol.attrlist = a;
		symbol.evaluator = evaluator;
	}
	return symbol;
}

NameEntry *
Symtable::first_global()
{
	return evaluator->first_global();
}

NameEntry *
Symtable::next_global()
{
	return evaluator->next_global();
}

Symbol
Symtable::global_symbol( NameEntry * n )
{
	Symbol	symbol;

	if ( n != 0 )
	{
		symbol.attrlist = evaluator->evaluate(n);
		symbol.evaluator = evaluator;
	}
	return symbol;
}

// These attribute lists are constructed on demand,
// and shared among all Symtables

static Attribute *ptr_to_ft[ ft_void + 1 ];

Symbol
Symtable::ptr_type ( Fund_type ft )
{
	Symbol	symbol;

	if ( ft < 0 || ft > ft_void ) {
		return symbol;		// caller must print message
	}

	if ( !ptr_to_ft[ ft ] ) {	// build attribute list by hand
		Fetalrec fetalrec;
		fetalrec.add_attr( an_tag, af_tag, t_pointertype );
		fetalrec.add_attr( an_basetype, af_fundamental_type, ft );
		fetalrec.add_attr( an_bytesize, af_int, sizeof( char *) );
		ptr_to_ft[ ft ] = fetalrec.put_record();
	}
	symbol.evaluator = evaluator;
	symbol.attrlist = ptr_to_ft[ft];

	return symbol;
}
