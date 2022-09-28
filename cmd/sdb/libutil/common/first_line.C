#ident	"@(#)sdb:libutil/common/first_line.C	1.2"
#include	"Interface.h"
#include	"Process.h"
#include	"Source.h"
#include	"SrcFile.h"
#include	"utility.h"

long
first_exec_line( Process * process )
{
	Stmt		stmt;
	Symbol		symbol;
	Source		source;
	char *		name;

	if ( process == 0 )
	{
		printe("internal error: ");
		printe("null pointer to first_exec_line()\n");
		return 0;
	}
	name = process->current_srcfile;
	if ( process->find_source( name, symbol ) == 0 )
	{
		printe("file %s is not a source file of this process\n",name);
		return 0;
	}
	else if ( symbol.source( source ) == 0 )
	{
		printe("internal error: ");
		printe("could not get source for file %s\n",name);
		return 0;
	}
	else
	{
		stmt = source.first_stmt();
		return stmt.line;
	}
}
