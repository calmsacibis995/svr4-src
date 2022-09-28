#ident	"@(#)sdb:libexecon/common/Process.st.C	1.10.3.1"
#include	"Process.h"
#include	"Symtab.h"
#include	"str.h"
#include	<string.h>
#include	<memory.h>
#include	<sys/stat.h>

static int find_filename( char * name );

Symtab *
Process::find_symtab( Iaddr addr )
{
	return seglist.find_symtab( key, addr );
}

Symbol
Process::find_entry( Iaddr addr )
{
	Symtab *	symtab;
	Symbol		symbol;

	if ( (symtab = seglist.find_symtab(key,addr)) == 0 )
	{
		symbol.null();
	}
	else
	{
		symbol = symtab->find_entry( addr );
	}
	return symbol;
}

Symbol
Process::find_scope( Iaddr addr )
{
	Symtab *	symtab;
	Symbol		symbol;

	if ( (symtab = seglist.find_symtab(key,addr)) == 0 )
	{
		symbol.null();
	}
	else
	{
		symbol = symtab->find_scope( addr );
	}
	return symbol;
}

int
Process::find_source( char * name, Symbol & symbol )
{
	return seglist.find_source( key, name, symbol );
}

NameEntry *
Process::first_global()
{
	return seglist.first_global( key );
}

NameEntry *
Process::next_global()
{
	return seglist.next_global( key );
}

Symbol
Process::find_global( char * name )
{
	return seglist.find_global( key, name );
}

long
Process::current_line()
{
	return currstmt.line;
}

int
Process::set_current_stmt( char * filename, long line )
{
	Symbol	symbol;

	if ( seglist.find_source( key, filename, symbol ) != 0 )
	{
		current_srcfile = symbol.name();
		currstmt.line = line;
		return 1;
	}
	else if ( find_filename(filename) )
	{
		current_srcfile = str( filename );
		currstmt.line = line;
		return 1;
	}
	return 0;
}

//
// get symbol name
//
char *
Process::symbol_name( Symbol symbol )
{
	char	*ptr, *name;
	int	offset;
	Iaddr	addr, newaddr;
	Symbol	newsymbol;

	name = symbol.name();
	//
	// if there are no static shared libs, return symbol name unchanged
	//
	if ( seglist.has_stsl() == 0 )
		return name;
	//
	// if name is ".bt?" get real name of function from the branch table.
	//
	ptr = name;
	if ( (name[0] == '.') && (name[1] == 'b') && (name[2] == 't') ) {
		extern int atoi(const char *);
		offset  = atoi(ptr+3);		// offset in branch table
		addr = symbol.pc(an_lopc);
		newaddr = instr.fcn2brtbl( addr, offset);
		newsymbol = find_entry(newaddr);
		if ( newsymbol.isnull() == 0 )
			name = newsymbol.name();
	}	
	return name;
}	

Symbol
Process::ptr_type( Fund_type basetype )
{
	Symbol sym;
	seglist.ptr_type( key, basetype, sym );
	return sym;
}

Symbol
Process::current_function()
{
	Symbol	symbol;
	Iaddr	backpc, forwpc;
	Symbol	sym2, sym3, sym4;
	Source	source;

	if ( current_srcfile == 0 )
	{	
		return symbol;
	}
	else if ( currstmt.is_unknown() )
	{	
		return symbol;
	}
	else if ( find_source( current_srcfile, sym2 ) == 0 )
	{	
		return symbol;
	}
	else if ( sym2.source( source ) == 0 )
	{	
		return symbol;
	}
	source.stmt_to_pc( currstmt.line, backpc, 0, 0, -1 );
	source.stmt_to_pc( currstmt.line, forwpc, 0, 0, 1 );
	sym3 = find_entry( backpc );
	sym4 = find_entry( forwpc );
	if ( sym3 == sym4 )
	{
		return sym3;
	}
	else
	{
		return symbol;
	}
}

Symbol
Process::current_scope()
{
	Symbol	symbol, sym2;
	Iaddr	pc;
	Source	source;

	if ( current_srcfile == 0 )
	{	
		return symbol;
	}
	else if ( currstmt.is_unknown() )
	{	
		return symbol;
	}
	else if ( find_source( current_srcfile, sym2 ) == 0 )
	{	
		return symbol;
	}
	else if ( sym2.source( source ) == 0 )
	{	
		return symbol;
	}
	source.stmt_to_pc( currstmt.line, pc, 0, 0, -1 );
	symbol = find_scope( pc );
	return symbol;
}

Symbol
Process::first_file()
{
	return seglist.first_file( key );
}

Symbol
Process::next_file()
{
	return seglist.next_file( key );
}

extern char *	global_path;
extern int	pathage;

static int
find_filename( char * name )
{
	static char *		pathtmp;
	static int		tmplen, pathlen;
	static char		pathbuf[256];
	static struct stat	stbuf;

	if ( ::stat(name, &stbuf) != -1 ) return 1;

	if( global_path == 0 ) return 0;

	pathlen = ::strlen( global_path );
	pathtmp = global_path;
	while ( pathlen > 0 )
	{
		tmplen = ::strcspn( pathtmp, ":" );
		::memcpy( pathbuf, pathtmp, tmplen );
		pathbuf[tmplen] = '/';
		pathbuf[tmplen+1] = '\0';
		::strcat( pathbuf, name );
		if ( ::stat(pathbuf, &stbuf) != -1 )
			return 1;
		else {
			pathtmp += ( tmplen + 1 );
			pathlen -= ( tmplen + 1 );
		}
	}
	return 0;
}
