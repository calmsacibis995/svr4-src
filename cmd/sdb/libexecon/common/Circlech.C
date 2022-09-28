#ident	"@(#)sdb:libexecon/common/Circlech.C	1.1"

#include	"Circlech.h"

Circlech::Circlech( void * thing )
{
	prev = succ = this;
	item = thing;
}

Circlech::~Circlech()
{
	remove();
}

int
Circlech::unconnected()
{
	return prev == this || succ == this;
}

void
Circlech::add( Circlech * c )
{
	succ->prev = c;
	c->succ = succ;
	succ = c;
	c->prev = this;
}

void
Circlech::add_prev( Circlech * c )
{
	prev->succ = c;
	c->prev = prev;
	prev = c;
	c->succ = this;
}

void
Circlech::add_succ( Circlech * c )
{
	succ->prev = c;
	c->succ = succ;
	succ = c;
	c->prev = this;
}

void
Circlech::remove()
{
	prev->succ = succ;
	succ->prev = prev;
	prev = this;
	succ = this;
}
