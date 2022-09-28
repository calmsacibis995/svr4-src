#ident	"@(#)sdb:libexecon/common/Watchlist.C	1.5"
#include	"Flags.h"
#include	"Interface.h"
#include	"Locdesc.h"
#include	"Watchlist.h"
#include	<memory.h>
#include	<string.h>

Watchlist::Watchlist()	{}

Watchlist::~Watchlist()
{
	Watchpoint *	w;
	Watchpoint *	next;

	for ( w = (Watchpoint*)tfirst(); w != 0 ; w = next )
	{
		next = (Watchpoint*)w->next();
		delete w;
	}
}

Watchpoint *
Watchlist::add( Expr & e )
{
	Watchpoint	watchpoint( e );

	return (Watchpoint*)tinsert( watchpoint );
}

int
Watchlist::remove( Expr & e )
{
	Watchpoint	watchpoint( e );

	return tdelete( watchpoint );
	
}

Watchpoint *
Watchlist::lookup( Expr & e )
{
	Symbol		symbol;
	Watchpoint	watchpoint( e );

	symbol = e.lhs_symbol();
	watchpoint.symname = symbol.name();
	return (Watchpoint*)tlookup( watchpoint );
}

Watchpoint::Watchpoint( Expr & e ) : expr(e)
{
	Symbol	symbol;

	expr.lvalue( lvalue );
	symbol = expr.lhs_symbol();
	symname = symbol.name();
	flags = 0;
	ENABLE( flags );
}

Watchpoint::~Watchpoint() {}

int
Watchpoint::set_value( Frame * frame )
{
	Symbol		symbol,parent;
	Locdesc		locdesc;
	Place		place;

	symbol = expr.lhs_symbol();
	if ( symbol.isnull() )
	{
		lopc = 0;
		hipc = ~0;
		frameid.null();
		return expr.rvalue( rvalue );
	}
	else if ( symbol.locdesc(locdesc) == 0 )
	{
		printe("internal error: ");
		printe("no location attribute for %s\n", symbol.name());
		return 0;
	}
	place = locdesc.place( 0, 0 );
	if ( place.isnull() || place.kind == pRegister )
	{
		parent = symbol.arc( an_parent );
		lopc = parent.pc( an_lopc );
		hipc = parent.pc( an_hipc );
		frameid = frame->id();
	}
	else
	{
		lopc = 0;
		hipc = ~0;
		frameid.null();
	}
	return expr.rvalue( rvalue );
}
