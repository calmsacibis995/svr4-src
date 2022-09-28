#ident	"@(#)sdb:libsymbol/common/Reflist.C	1.1"
#include	"Reflist.h"
#include	<memory.h>
#include	<malloc.h>
#include	<stdio.h>

int
Refnode::operator>( Avlnode & t )
{
	Refnode &	refnode = *(Refnode*)(&t);

	return diskloc > refnode.diskloc;
}

int
Refnode::operator<( Avlnode & t )
{
	Refnode &	refnode = *(Refnode*)(&t);

	return diskloc < refnode.diskloc;
}

// Add a new refnode 
void
Reflist::add(long l, Attribute * s)
{
	Refnode		newnode(l,s);
	Avlnode *	t = tinsert(newnode);

	if ( t == 0 )
		printf("can not add a refnode\n");

}

// Search for a refnode with a specified key
// Return succeed or fail.

int
Reflist::lookup(long offset, Attribute * & r)
{
	Refnode		keynode(offset);
	Avlnode	 *	t = tlookup(keynode);

	if ( t == 0 )
	{
		r = 0;
		return 0;
	}
	else
	{
		r = ((Refnode *)t)->nodeloc;
		return 1;
	}
}

Avlnode *
Refnode::makenode()
{
	char *	s;

	s = ::malloc(sizeof(Refnode));
	::memcpy(s,(char*)this,sizeof(Refnode));
	return (Avlnode*)s;
}

void
Reflist::tdump()	{}
