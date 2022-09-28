#ident	"@(#)sdb:libint/common/Interface.C	1.14"

#include	<stdio.h>
#include	<stdarg.h>
#include	"Interface.h"
#include	"Itype.h"
#include	"Process.h"
#include	"Source.h"
#include	"SrcFile.h"
#include	"Symtab.h"

// define the Interface globals ---
Interface *	IP = &ln_interface;

CmdType	lastcom = NOCOM;
Iaddr	instaddr = 0;

// Interface definitions for line-mode

int
ln_show_location( Process * process, Iaddr pc, int showsrc )
{
	Source		source;
	Symtab *	symtab;
	Symbol		symbol;
	Symbol		entry;
	SrcFile *	srcfile;
	char *		s;

	long		line;

	if ( process == 0 )
	{
		return 0;
	}
	else if ( !showsrc )
	{
		printx("%s\n",process->stateinfo() );
		if ( process->disassemble( pc, 1, &instaddr ) )
			lastcom = DSICOM;
		else
			lastcom = NOCOM;
		return 1;
	}
	else if ( (symtab = process->find_symtab( pc )) == 0 )
	{
		printx("%s\n",process->stateinfo() );
		if ( process->disassemble( pc, 1, &instaddr ) )
			lastcom = DSICOM;
		else
			lastcom = NOCOM;
		return 1;
	}
	entry = symtab->find_entry( pc );
	if ( symtab->find_source( pc, symbol ) == 0 )
	{
		printx("%s\n",process->stateinfo() );
		if ( process->disassemble( pc, 1, &instaddr ) )
			lastcom = DSICOM;
		else
			lastcom = NOCOM;
		return 1;
	}
	else if ( symbol.source( source ) == 0 )
	{
		if ( entry.isnull() )
			printx("%s in %s\n",process->stateinfo(),
				symbol.name() );
		else
			printx("%s function %s() in %s\n",process->stateinfo(),
				entry.name(), symbol.name() );
		if ( process->disassemble( pc, 1, &instaddr ) )
			lastcom = DSICOM;
		else
			lastcom = NOCOM;
		return 1;
	}
	source.pc_to_stmt( pc, line );
	if ( entry.isnull() )
		printx("%s in %s\n",process->stateinfo(),
			symbol.name() );
	else
		printx("%s function %s() in %s\n",process->stateinfo(),
			entry.name(), symbol.name() );
	if ( line == 0 )
	{
		if ( process->disassemble( pc, 1, &instaddr ) )
			lastcom = DSICOM;
		else
			lastcom = NOCOM;
	}
	else if ( (srcfile = find_srcfile( symbol.name() )) == 0 )
	{
		printx("%d:\t<no source text available>\n",line );
		lastcom = NOCOM;
	}
	else if ( (s = srcfile->line( line )) == 0 )
	{
		printx("%d:\t<no source text available>\n",line );
		lastcom = NOCOM;
	}
	else
	{
		printx("%d:\t%s",line, s );
		lastcom = PRCOM;
	}
	return 1;
}

Interface ln_interface = {
	ln_show_location,
	_printx,
	_printe
};	

/* ABORT() -- abort with diagnostic
 *
 * This routine provides abort() with a printf-stype diagnostic
 * issued through printe. It also can be used within the debugger
 * to insure ``orderly'' cleanup.
 */

void
ABORT( char *format ... )
{
	va_list 	arglist;

	va_start( arglist, format );
	vprintf( format, arglist );
	va_end( arglist );
	abort();
}
