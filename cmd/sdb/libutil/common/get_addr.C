#ident	"@(#)sdb:libutil/common/get_addr.C	1.5"
#include	"utility.h"
#include	"Process.h"
#include	"Source.h"
#include	"Symbol.h"
#include	"Symtab.h"
#include	"Tag.h"
#include	"Interface.h"
#include	"Expr.h"

int
get_addr( Process * process, Location * location, Iaddr & addr )
{
	Symbol		symbol;
	Iaddr		pc;
	Source		source;
	Symtab *	symtab;

	if ( location == 0 )
	{
		addr = 0;
		return 1;
	}
	switch ( location->kind )
	{
		case lk_addr:
			addr = location->locn.addr;
			return 1;
		case lk_stmt:
			if ( location->file_name == 0 )
			{
				printe("no filename specified\n");
				return 0;
			}
			else if ( process->find_source(location->file_name,symbol) == 0 )
			{
				printe("No source information for file %s\n",
						location->file_name);
				return 0;
			}
			else if ( symbol.source( source ) == 0 )
			{
				printe("No source information for file %s\n",
						location->file_name);
				return 0;
			}
			else
			{
				source.stmt_to_pc( location->locn.line, addr );
				if ( addr == 0 )
				{
					printe("No code for %s:%d\n",
						location->file_name,
						location->locn.line);
					return 0;
				}
				else
				{
					return 1;
				}
			}
			break;
		case lk_fcn:
			if ( location->fcn_name == 0 )
			{
				printe("no function name specified\n");
				return 0;
			}
			symbol = function_symbol(location->fcn_name, process,
						process->pc_value());
			if ( symbol.isnull() )
			{
				printe("no entry '%s' exists.\n",
					location->fcn_name);
				return 0;
			}
			else if ( symbol.tag() != t_entry )
			{
				printe("'%s' is not a function.\n",
					location->fcn_name);
				return 0;
			}
			else if ( location->locn.line == 0 )
			{
				addr = symbol.pc(an_lopc);
				return 1;
			}
			pc = symbol.pc( an_lopc );
			if ( (symtab = process->find_symtab( pc )) == 0 )
			{
				printe("No source info for function %s\n",location->fcn_name);
				return 0;
			}
			else if ( symtab->find_source( pc, symbol ) == 0 )
			{
				printe("No source info for function %s\n",location->fcn_name);
				return 0;
			}
			else if ( symbol.source( source ) == 0 )
			{
				printe("No source info for file %s\n",location->file_name);
				return 0;
			}
			else
			{
				source.stmt_to_pc( location->locn.line, addr );
				if ( addr == 0 )
				{
					printe("No code for %s:%d\n",
						location->fcn_name,
						location->locn.line);
					return 0;
				}
				else
				{
					return 1;
				}
			}
			break;
		case lk_pc:
			addr = process->pc_value();
			return 1;
		default:
			printe("internal error : not ready for that location");
			printe(" discriminant yet\n");
			return 0;
	}
}
