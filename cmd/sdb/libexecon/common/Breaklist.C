#ident	"@(#)sdb:libexecon/common/Breaklist.C	1.7"
#include	"Breaklist.h"
#include	"Assoccmds.h"
#include	"Flags.h"
#include	<memory.h>

Breaklist::Breaklist()	{}

Breaklist::~Breaklist()
{
	Breakpoint *	b;
	Breakpoint *	next;

	for ( b = (Breakpoint*)tfirst(); b != 0 ; b = next )
	{
		b->_assoccmds = dispose_assoc( b->_assoccmds );
		next = (Breakpoint*)b->next();
		delete b;
	}
}

Breakpoint *
Breaklist::add( Iaddr a, Assoccmds * assoc, int announce )
{
	Breakpoint	breakpoint(a,assoc,announce);
	Avlnode *p;

	p = tinsert( *((Avlnode *)&breakpoint) );
	return (Breakpoint*)p;
}

int
Breaklist::remove( Iaddr a )
{
	Breakpoint	breakpoint( a );
	Breakpoint *	b;

	if ( (b=(Breakpoint*)tlookup(breakpoint)) == 0 )
	{
		return 0;
	}
	else
	{
		b->_assoccmds = dispose_assoc( b->_assoccmds );
		return tdelete( breakpoint );
	}
}

int
Breaklist::disable( Iaddr a )
{
	Breakpoint	breakpoint( a );
	Breakpoint *	b;

	if ( (b=(Breakpoint*)tlookup(breakpoint)) == 0 )
	{
		return 0;
	}
	else if ( IS_DISABLED(b->_flags) )
	{
		return 0;
	}
	else
	{
		DISABLE( b->_flags );
		return 1;
	}
}

int
Breaklist::enable( Iaddr a )
{
	Breakpoint	breakpoint( a );
	Breakpoint *	b;

	if ( (b=(Breakpoint*)tlookup(breakpoint)) == 0 )
	{
		return 0;
	}
	else if ( IS_ENABLED(b->_flags) )
	{
		return 0;
	}
	else
	{
		ENABLE( b->_flags );
		return 1;
	}
}

Breakpoint *
Breaklist::lookup( Iaddr a )
{
	Breakpoint	breakpoint( a );

	return (Breakpoint*)tlookup( breakpoint );
}

Breakpoint::Breakpoint( Iaddr a, Assoccmds * assoc, int announce )
{
	_addr = a;
	_assoccmds = assoc;
	_flags = 0;
	ENABLE(_flags);
	if ( announce )
	{
		SET_ANNOUNCE( _flags );
	}
}

Iaddr
Breakpoint::addr()
{
	return _addr;
}

char *
Breakpoint::oldtext()
{
	return _oldtext;
}

Assoccmds *
Breakpoint::assoccmds()
{
	return _assoccmds;
}

int
Breakpoint::operator>( Avlnode & node )
{
	Breakpoint &	b = *((Breakpoint*)&node);

	return _addr > b._addr;
}

int
Breakpoint::operator<( Avlnode & node )
{
	Breakpoint &	b = *((Breakpoint*)&node);

	return _addr < b._addr;
}

Breakpoint &
Breakpoint::operator=( Breakpoint & b )
{
	_addr = b._addr;
	::memcpy( _oldtext, b._oldtext, BKPTSIZE );
	_flags = b._flags;
	_assoccmds = dispose_assoc( _assoccmds );
	_assoccmds = b._assoccmds;
	return *this;
}

void
Breakpoint::value_swap( Avlnode * node )
{
	Breakpoint	temp( 0 );
	Breakpoint *	b;

	b = (Breakpoint*)node;

	temp._addr = b->_addr;
	temp._flags = b->_flags;
	temp._assoccmds = b->_assoccmds;
	memcpy( temp._oldtext, b->_oldtext, BKPTSIZE );

	b->_addr = _addr;
	b->_flags = _flags;
	b->_assoccmds = _assoccmds;
	memcpy( b->_oldtext, _oldtext, BKPTSIZE );

	_addr = temp._addr;
	_flags = temp._flags;
	_assoccmds = temp._assoccmds;
	memcpy( _oldtext, temp._oldtext, BKPTSIZE );
}

Avlnode *
Breakpoint::makenode()
{
	Breakpoint *	b;

	b = new Breakpoint( 0 );
	*b = *this;
	return b;
}

int
Breakpoint::set( char * text )
{
	if ( text != 0 )
	{
		::memcpy( _oldtext, text, BKPTSIZE );
		return 1;
	}
	else
	{
		return 0;
	}
}
