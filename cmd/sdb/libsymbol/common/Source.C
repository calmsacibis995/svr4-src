#ident	"@(#)sdb:libsymbol/common/Source.C	1.3"
#include	"Source.h"
#include	"Lineinfo.h"
#include	"builder.h"
#include	<string.h>
#include	<osfcn.h>

Source::Source()
{
	lineinfo = 0;
	lastpcsub = 0;
	ss_base = 0;
}

Source::Source( Source& source )
{
	lineinfo = source.lineinfo;
	lastpcsub = source.lastpcsub;
	ss_base = source.ss_base;
}

Source&
Source::operator=( const Source & source )
{
	lineinfo = source.lineinfo;
	lastpcsub = source.lastpcsub;
	ss_base = source.ss_base;
	return *this;
}

void
Source::pc_to_stmt( Iaddr pc, long & line, Iaddr * stmtpc, long * sz, int slide )
{
	int		first,middle,last;
	int		found;
	long		entrycount;
	LineEntry *	pcinfo;

	if ( lineinfo == 0 )
	{
		line = 0;
		if (stmtpc) *stmtpc = 0;
		if (sz) *sz = 0;
		return;
	}
	else
	{
		entrycount = lineinfo->entrycount;
		pcinfo = lineinfo->addrpart;
	}
	pc -= ss_base;
	found = 0;
	first = 0;
	last = entrycount;
	while ( (first <= last) && (last != 0 ) && !found )
	{
		middle = ( first + last ) / 2;
		if ( pcinfo[middle].addr == pc )
		{
			found = 1;
		}
		else if ( pcinfo[middle].addr > pc )
		{
			last = middle - 1;
		}
		else
		{
			first = middle + 1;
		}
	}
	if ( found )
	{
		line = pcinfo[middle].linenum;
		if (stmtpc) *stmtpc = pcinfo[middle].addr + ss_base;
		if (sz) *sz = pcinfo[middle+1].addr - pc;
		lastpcsub = middle;
	}
	else if ( ( slide < 0 ) && ( first < entrycount ) )
	{
		line = pcinfo[last].linenum;
		if (stmtpc) *stmtpc = pcinfo[last].addr + ss_base;
		if (sz) *sz = pcinfo[last+1].addr - pc;
		lastpcsub = last;
	}
	else if ( ( slide > 0 ) && ( first < entrycount ) )
	{
		line = pcinfo[first].linenum;
		if (stmtpc) *stmtpc = pcinfo[first].addr + ss_base;
		if (sz) *sz = pc - pcinfo[first-1].addr;
		lastpcsub = first;
	}
	else
	{
		line = 0;
		if (stmtpc) *stmtpc = 0;
		if (sz) *sz = 0;
		lastpcsub = -1;
	}
}

void
Source::stmt_to_pc( long line, Iaddr & pc, long * stmtline, long * sz, int slide )
{
	int		first,middle,last;
	int		found;
	long		entrycount;
	LineEntry *	stmtinfo;

	if ( lineinfo == 0 )
	{
		pc = 0;
		if (stmtline) *stmtline = 0;
		if (sz) *sz = 0;
		return;
	}
	else
	{
		entrycount = lineinfo->entrycount;
		stmtinfo = lineinfo->linepart;
	}
	found = 0;
	first = 0;
	last = entrycount;
	while ( (first <= last) && !found )
	{
		middle = ( first + last ) / 2;
		if ( stmtinfo[middle].linenum == line )
		{
			found = 1;
		}
		else if ( stmtinfo[middle].linenum > line )
		{
			last = middle - 1;
		}
		else
		{
			first = middle + 1;
		}
	}
	if ( found )
	{
		// find first entry for that line number here
		pc = stmtinfo[middle].addr + ss_base;
		if (stmtline) *stmtline = stmtinfo[middle].linenum;
		if (sz) *sz = stmtinfo[middle+1].addr - pc;
	}
	else if ( slide < 0 )
	{
		// find first entry for that line number here
		pc = stmtinfo[last].addr + ss_base;
		if (stmtline ) *stmtline = stmtinfo[last].linenum;
		if (sz) *sz = stmtinfo[last+1].addr - pc;
	}
	else if ( slide > 0 )
	{
		pc = stmtinfo[first].addr + ss_base;
		if (stmtline ) *stmtline = stmtinfo[first].linenum;
		if (sz) *sz = pc - stmtinfo[first-1].addr;
	}
	else
	{
		pc = 0;
		if (stmtline ) *stmtline = 0;
		if (sz) *sz = 0;
	}
}

Stmt
Source::first_stmt()
{
	Stmt		stmt;
	LineEntry *	stmtinfo;

	if ( lineinfo != 0 )
	{
		stmtinfo = lineinfo->linepart;
		stmt.line = stmtinfo->linenum;
		stmt.pos = 0;
	}
	return stmt;
}

void
Source::next_stmt( Iaddr & addr, long & line, long & size )
{
	LineEntry *	pcinfo;

	if ( lineinfo == 0 )
	{
		addr = 0;
		line = -1;
		size = 0;
		return;
	}
	else
	{
		pcinfo = lineinfo->addrpart;
	}

	if ( lastpcsub == -1 )
	{
		addr = 0;
		line = -1;
		size = 0;
	}
	else if ( pcinfo[++lastpcsub].linenum != BIG_LINE )
	{
		addr = pcinfo[lastpcsub].addr + ss_base;
		line = pcinfo[lastpcsub].linenum;
		size = pcinfo[lastpcsub+1].addr - addr;
	}
	else
	{
		addr = pcinfo[lastpcsub].addr;
		line = -1;
		size = 0;
		lastpcsub = -1;
	}
}
