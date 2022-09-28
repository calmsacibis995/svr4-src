#ident	"@(#)sdb:libexecon/common/Assoccmds.C	1.4"
#include	"Assoccmds.h"
#include	<string.h>

char *
Assoccmds::get_cmd()
{
	char *	p;

	if ( vector.size() == 0 )
	{
		p = 0;
	}
	else if ( (next_cmd-(char*)vector.ptr()) >= vector.size() )
	{
		p = 0;
	}
	else
	{
		p = next_cmd;
		next_cmd += ::strlen(p) + 1;
	}
	return p;
}

void
Assoccmds::add_cmd( char * cmd )
{
	vector.add( cmd, ::strlen(cmd)+1 );
}

void
Assoccmds::reset()
{
	++refcount;
	next_cmd = (char*)vector.ptr();
}

char *
Assoccmds::string()
{
	return (char*) vector.ptr();
}

Assoccmds *
dispose_assoc( Assoccmds * a )
{
	if ( a == 0 )
	{
		return 0;
	}
	else if ( a->refcount < 1 )
	{
		delete a;
		return 0;
	}
	else
	{
		a->refcount--;
		return 0;
	}
}

#define QSIZE	10

Assoccmds *	queue[QSIZE];

static int	loend = 0;
static int	hiend = 0;

static int
incr( int v )
{
	++v;
	if ( v >= QSIZE )
		return 0;
	else
		return v;
}

int
queue_assoc( Assoccmds * a )
{
	int	next;

	if ( (next = incr(hiend)) == loend )
	{
		return 0;
	}
	else
	{
		queue[hiend] = a;
		hiend = next;
		return 1;
	}
}

Assoccmds *
dequeue()
{
	Assoccmds *	a;

	if ( loend == hiend )
	{
		return 0;
	}
	else
	{
		a = queue[loend];
		loend = incr(loend);
		return a;
	}
}
