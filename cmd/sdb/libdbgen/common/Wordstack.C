#ident	"@(#)sdb:libdbgen/common/Wordstack.C	1.1"

#include	"Wordstack.h"

void
Wordstack::push( unsigned long word )
{
	vector.add(&word,sizeof(unsigned long));
	++count;
}

unsigned long
Wordstack::pop()
{
	unsigned long *	p;
	unsigned long 	top_item = 0;

	if ( count > 0 )
	{
		p = (unsigned long *)vector.ptr();
		--count;
		top_item = p[count];
		vector.drop(sizeof(unsigned long *));
	}
	return top_item;
}

unsigned long
Wordstack::item( int n )
{
	unsigned long *	p;
	unsigned long 	nth_item = 0;

	if ( count > 0 && n <= count && n >= 0 )
	{
		p = (unsigned long *)vector.ptr();
		nth_item = p[count-n];
	}
	return nth_item;
}

