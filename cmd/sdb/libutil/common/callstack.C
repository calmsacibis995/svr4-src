#ident	"@(#)sdb:libutil/common/callstack.C	1.14"

#include "utility.h"
#include "Interface.h"
#include "Process.h"
#include "Symtab.h"
#include "Frame.h"
#include "Reg.h"
#include "Expr.h"
#include "Rvalue.h"
#include "Attribute.h"
#include "Tag.h"
#include <string.h>

static char *
basename( register char *path )
{
	register char *p = strrchr( path, '/' );
	if ( p ) return p+1;
	else	 return path;
}

static void show_call( Process *, Frame *, Symtab *, int quick );

int
print_stack( Process *process, int how_many, int quick )
{
	int i = 0;
	if ( !process || process->is_proto() ) {
		printe("no process\n");
		return 0;
	}
	Frame *frame = process->topframe();
	if ( how_many == 0 )
		how_many = ~0;		// big enough

	while ( frame && how_many-- && !interrupted ) {
		Iaddr pc = frame->pc_value();
		Symtab *symtab = process->find_symtab(pc);
		Symbol symbol;
		Source source;
		char *filename = 0;
		long line = 0;

		if ( symtab && symtab->find_source( pc, symbol ) ) {
			filename = basename( symbol.name() );
			symbol.source( source );
			source.pc_to_stmt( pc, line );
		}

		show_call( process, frame, symtab, quick > 0 );

		if ( filename && *filename ) {
			printx( "\t[%s", filename );
			if ( line )
				printx( ":%d]\n", line );
			else
				printx( "]\n" );
		} else {
			printx( "\n" );
		}

		frame = frame->caller();
	}
	return 1;
}

static void
show_call( Process *process, Frame *frame, Symtab *symtab, int quick )
{
	Iaddr pc = frame->pc_value();
	int i = 0;
	int nbytes = frame->nargwds() * sizeof(int);
	int done = 0;
	if ( symtab ) {
		Symbol symbol = process->find_entry( pc );
		char *name = process->symbol_name(symbol);
		if ( !name || !*name )
			name = "?";
		printx( "%s(", name );
		if ( (nbytes > 0) && !quick && frame->caller() ) {
			Symbol arg = symbol.child();
			Attribute *attr = arg.attribute( an_tag );
			Tag tag = t_none;
			if ( attr && attr->form == af_tag )
				tag = attr->value.word;
			Rvalue rval;
			while ( !arg.isnull() && tag == t_argument ) {
				if ( i++ > 0 ) printx(",");
				Expr expr( arg );
				expr.eval( EV_RHS, process, pc, frame );
				if ( expr.rvalue( rval ) )
					rval.print(arg.name(), 0);
				else
					printx("%s=???", arg.name());
				//
				// round to word boundary
				//
				done += ( (rval.type().size() + 3) & ~0x3 );
				arg = arg.sibling();
				attr = arg.attribute( an_tag );
				if ( attr && attr->form == af_tag )
					tag = attr->value.word;
				else
					tag = t_none;
			}
		}
	} else {
		printx( "?(" );
	}
	if ( !quick && frame->caller() ) {
		int n = done/sizeof(int);
		while ( done < nbytes ) {
			printx( "%s%#x", (i++>0)?",":"", frame->argword(n++) );
			done += sizeof(int);
		}
	}
	printx(")");
}
