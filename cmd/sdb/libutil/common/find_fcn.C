#ident	"@(#)sdb:libutil/common/find_fcn.C	1.3"
#include	"Interface.h"
#include	"Process.h"
#include	"Source.h"
#include	"Symtab.h"
#include	"Tag.h"
#include	"utility.h"
#include	"Expr.h"

char *
find_function( Process * process, char * fname, long * line )
{
	Symbol		symbol;
	Iaddr		pc;
	Symtab *	symtab;
	Source		source;

	if ( process == 0 )
	{
		printe("internal error: ");
		printe("null pointer to find_function()\n");
		return 0;
	}
	symbol = function_symbol( fname, process, process->pc_value() );
	if ( symbol.isnull() )
	{
		return 0;
	}
	else if ( symbol.tag() != t_entry )
	{
		return 0;
	}
	else if ( line == 0 )
	{
		return symbol.name();
	}
	pc = symbol.pc( an_lopc );
	if ( (symtab = process->find_symtab( pc )) == 0 )
	{
		return 0;
	}
	else if ( symtab->find_source( pc, symbol ) == 0 )
	{
		return 0;
	}
	else if ( symbol.source( source ) == 0 )
	{
		return 0;
	}
	else
	{
		source.pc_to_stmt( pc, *line );
		return symbol.name();
	}
}
