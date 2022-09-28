#ident	"@(#)sdb:libexecon/common/EventTable.C	1.2"

#include	"EventTable.h"
#include	"Object.h"
#include	"Process.h"

EventTable *
find_et( int fdobj )
{
	EventTable *	et;
	Object *	s;

	if ( fdobj == -1 )
	{
		return 0;
	}
	else if ( (s = find_object(fdobj, &et, 0 )) == 0 )
	{
		return 0;
	}
	else if ( et->process == s->protop )
	{
		et->process = 0;
		return et;
	}
	else if ( et->process == 0 )
	{
		return et;		// found it and it's not in use
	}
	else
	{
		return new EventTable;	// found it and it's in use
	}
}

EventTable *
dispose_et( EventTable * e )
{
	if ( e == 0 )
	{
		return 0;
	}
	else if ( e->object != 0 )
	{
		e->process = e->object->protop;
		if ( e->process ) e->process->etable = e;
		return 0;
	}
	else
	{
		delete e;
		return 0;
	}
}
