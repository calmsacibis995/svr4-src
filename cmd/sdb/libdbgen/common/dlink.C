#ident	"@(#)sdb:libdbgen/common/dlink.C	1.1"
/*
// NAME
//	dlink
//
// ABSTRACT
//	implementation for the dlink class
//
*/

#include "dlink.h"

extern int write(int, const char *, int);
extern void abort();

void
dlink::prepend(dlink *elem)
{
	if(!elem) {
		write(2, "Can't link into empty list.\n",
			sizeof("Can't link into empty list.\n"));
		abort();
	}

	if(prev_elem = elem->prev_elem)
		prev_elem->next_elem = this;
	elem->prev_elem = this;
	next_elem = elem;
}

void
dlink::append(dlink *elem)
{
	if(!elem) {
		write(2, "Can't link into empty list.\n",
			sizeof("Can't link into empty list.\n"));
		abort();
	}

	if(next_elem = elem->next_elem)
		next_elem->prev_elem = this;
	prev_elem = elem;
	elem->next_elem = this;
}

void
dlink::unlink()
{
	if(next_elem)
		next_elem->prev_elem = prev_elem;
	if(prev_elem)
		prev_elem->next_elem = next_elem;
	next_elem = 0;
	prev_elem = 0;
}

void
dlink::rjoin(dlink *elem)
{
	next_elem = elem;
	if ( elem )
		elem->prev_elem = this;
}

void
dlink::ljoin(dlink *elem)
{
	prev_elem = elem;
	if ( elem )
		elem->next_elem = this;
}
