#ident	"@(#)sdb:libdbgen/common/Link.C	1.1"

#include "Link.h"

// Link -- a doubly-linked circular list with a distinguished head


Link *
Link::append( Link *other )
{
	Link *this_next = this->_next;
	this->_next = other;
	Link *other_prev = other->_prev;
	other->_prev = this;
	this_next->_prev = other_prev;
	other_prev->_next = this_next;
	return this;
}

Link *
Link::prepend( Link *other )
{
	Link *this_prev = this->_prev;
	Link *other_prev = other->_prev;
	this->_prev = other_prev;
	other_prev->_next = this;
	this_prev->_next = other;
	other->_prev = this_prev;
	return this;
}

Link *
Link::unlink()
{
	if ( !this )
		return 0;
	_prev->_next = _next;
	_next->_prev = _prev;
	_next = this;
	_prev = this;
	return this;
}
