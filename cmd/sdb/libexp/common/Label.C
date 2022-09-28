#ident	"@(#)sdb:libexp/common/Label.C	1.2"

//	Label.C -- implements class Label

#include "Label.h"
#include "str.h"
#include <string.h>
#include <stdlib.h>

Labelpart::Labelpart( char *s )
{
	if ( s ) {
		len = ::strlen( s );
		p = ::str( s );
	} else {
		len = 0;
		p = "";
	}
}

Labelpart::~Labelpart()
{
}


Label::Label()
{
	buf    = 0;
	buflen = 0;
	len    = 0;
	first  = 0;
}

Label::Label( Label& l )
{
	*this = l;
}

Label &
Label::operator=( Label &l )
{
	buf    = new char[ l.buflen ];
	strcpy( buf, l.buf );
	buflen = l.buflen;
	len    = 0;
	first  = 0;
	for ( Labelpart *p = l.first ; p ; p = p->next() ) {
		push( p->str() );
		if ( p->next() == l.first )
			break;
	}
	return *this;
}

Label::~Label()
{
	clear();
	delete buf;
}

void
Label::push( char *str )
{
	Labelpart *part = new Labelpart( str );
	if ( first )
		first->prepend( part );
	else
		first = part;
	len += part->length();
}

void
Label::pop( int count )
{
	Labelpart *p;

	while ( count-- && first ) {
		p = (Labelpart *) first->prev()->unlink();
		if ( p == first )
			first = 0;
		len -= p->length();
		delete p;
	}
	if ( !first ) {
		if ( len != 0 )
			abort();	// can't happen!
	}
}

void
Label::clear()
{
	pop( -1 );
}

char *
Label::str()
{
	if ( buflen < len ) {
		delete buf;
		buf = 0;
	}

	if ( buflen == 0 )
		buflen = 40;

	while ( buflen <= len )
		buflen *= 2;

	if ( !buf )
		buf = new char[buflen];

	char *q = buf;
	*q = 0;

	for ( Labelpart *p = first ; p ; p = p->next() ) {
		::strcpy( q, p->str() );
		q += p->length();
		if ( p->next() == first )	// wrap-around
			break;
	}

	return buf;
}
