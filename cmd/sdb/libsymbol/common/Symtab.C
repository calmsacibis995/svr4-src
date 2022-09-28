#ident	"@(#)sdb:libsymbol/common/Symtab.C	1.4"
#include	"Evaluator.h"
#include	"Symtab.h"
#include	"Symtable.h"
#include	"Tagcheck.h"
#include	"builder.h"
#include	<string.h>

#define equal(x,y)	(strcmp((x),(y))==0)

Symbol
Symtab::find_entry( Iaddr addr )
{
	Symbol		symbol;

	if ( symtable != 0 )
	{
		symbol = symtable->find_entry( addr - ss_base );
		symbol.ss_base = ss_base;
	}
	return symbol;
}

Symbol
Symtab::find_scope ( Iaddr addr )
{
	Symbol		symbol;

	if ( symtable != 0 )
	{
		symbol = symtable->find_scope( addr - ss_base );
		symbol.ss_base = ss_base;
	}
	return symbol;
}

Symbol
Symtab::first_symbol()
{
	Symbol	symbol;

	if ( symtable != 0 )
	{
		symbol = symtable->first_symbol();
		symbol.ss_base = ss_base;
	}
	return symbol;
}

int
Symtab::find_source( Iaddr pc, Symbol & symbol )
{
	int	result;

	result = 0;
	if ( symtable != 0 )
	{
		result = symtable->find_source( pc - ss_base, symbol );
		symbol.ss_base = ss_base;
	}
	return result;
}

int
Symtab::find_source( char * name, Symbol & symbol )
{
	int	result;

	result = 0;
	if ( symtable != 0 )
	{
		result = symtable->find_source( name, symbol );
		symbol.ss_base = ss_base;
	}
	return result;
}

Symbol
Symtab::find_global( char * name )
{
	Symbol	symbol;

	if ( symtable != 0 )
	{
		symbol = symtable->find_global( name );
		symbol.ss_base = ss_base;
	}
	return symbol;
}

NameEntry *
Symtab::first_global()
{
	if ( symtable != 0 )
	{
		return symtable->first_global();
	}
	else
	{
		return 0;
	}
}

NameEntry *
Symtab::next_global()
{
	if ( symtable != 0 )
	{
		return symtable->next_global();
	}
	else
	{
		return 0;
	}
}

Symbol
Symtab::global_symbol( NameEntry * n )
{
	Symbol	symbol;

	if ( symtable != 0 )
	{
		symbol = symtable->global_symbol( n );
		symbol.ss_base = ss_base;
	}
	return symbol;
}

Symbol
Symtab::ptr_type( Fund_type ft )
{
	Symbol	symbol;

	if ( symtable != 0 )
	{
		symbol = symtable->ptr_type( ft );
		symbol.ss_base = ss_base;
	}
	return symbol;
}
