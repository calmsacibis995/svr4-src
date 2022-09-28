#ident	"@(#)sdb:libsymbol/common/Fetalline.C	1.3"
#include	"Fetalline.h"
#include	"Lineinfo.h"
#include	<malloc.h>
#include	<memory.h>

Fetalline &
Fetalline::add_line( Iaddr addr, long linenum )
{
	LineEntry	entry;

	entry.addr = addr;
	entry.linenum = linenum;
	vector.add(&entry,sizeof(LineEntry));
	count++;
	return *this;
}

int asc_addr( LineEntry * e1, LineEntry * e2 )
{
	if ( e1->addr > e2->addr )
		return 1;
	else if ( e1->addr < e2->addr )
		return -1;
	else
		return 0;
}

int asc_line( LineEntry * e1, LineEntry * e2 )
{
	if ( e1->linenum > e2->linenum )
		return 1;
	else if ( e1->linenum < e2->linenum )
		return -1;
	else if ( e1->addr > e2->addr )
		return 1;
	else if ( e1->addr < e2->addr )
		return -1;
	else
		return 0;
}

extern void	qsort( void * ... );

Lineinfo *
Fetalline::put_line( Iaddr hiaddr )
{
	Lineinfo *	lineinfo;

	if ( count != 0 )
	{
		lineinfo = new Lineinfo;
		add_line( hiaddr, BIG_LINE );
		qsort( vector.ptr(), count, sizeof(LineEntry), asc_addr );
		lineinfo->addrpart = (LineEntry*)::malloc(vector.size());
		::memcpy((char*)lineinfo->addrpart,(char*)vector.ptr(),vector.size());
		qsort( vector.ptr(), count, sizeof(LineEntry), asc_line );
		lineinfo->linepart = (LineEntry*)::malloc(vector.size());
		::memcpy((char*)lineinfo->linepart,(char*)vector.ptr(),vector.size());
		lineinfo->entrycount = count;
	}
	else
	{
		lineinfo = 0;
	}
	vector.clear();
	count = 0;
	return lineinfo;
}

