#ident	"@(#)sdb:libexecon/common/Sigset.C	1.2"

#include	"Sigset.h"

#define bit(x) ( 1 << ((x)-1) )
int
all_on( Sigset *s )
{
	s->mem[0] = ~0;
	s->mem[1] = ~0;
	return 1;
}

int
all_off( Sigset *s )
{
	s->mem[0] = 0;
	s->mem[1] = 0;
	return 1;
}

int
turn_on( int sig, Sigset *s )
{
	if ( (sig < 1) || (sig > 64) )
	{
		return 0;
	}
	else if ( sig <= 32 )
	{
		s->mem[0] |= bit(sig);
		return 1;
	}
	else
	{
		s->mem[1] |= bit(sig - 32);
		return 1;
	}
}

int
turn_off( int sig, Sigset *s )
{
	if ( (sig < 1) || (sig > 64) )
	{
		return 0;
	}
	else if ( sig <= 32 )
	{
		s->mem[0] &= ~bit(sig);
		return 1;
	}
	else
	{
		s->mem[1] &= ~bit(sig - 32);
		return 1;
	}
}

int
is_on( int sig, Sigset *s )
{
	if ( (sig <1) || (sig> 64) )
		return 0;
	else if ( sig < 33 )
		return s->mem[0] & bit( sig );
	else
		return s->mem[1] & bit( sig - 32 );
}

int
copy_sigset( Sigset *s1, Sigset *s2 )
{
	*s1 = *s2;
	return 1;
}
