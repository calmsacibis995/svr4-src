#ident	"@(#)sdb:libexecon/common/Siglist.C	1.4"
#include	"Siglist.h"
#include	"Assoccmds.h"

long
default_mask()
{
	return ~0;
}

Siglist::Siglist()
{
	int	i;

	for ( i = 0 ; i < NSIG ; i++ )
	{
		assoccmd[i] = 0;
	}
}

Siglist::~Siglist()
{
	int	i;

	prfillset( &_sigset );
	for ( i = 0 ; i < NSIG ; i++ )
	{
		assoccmd[i] = dispose_assoc( assoccmd[i] );
	}
}

Assoccmds *
Siglist::assoccmds( int signo )
{
	if ( (signo <= 0) || (signo > NSIG) )
		return 0;
	else
		return assoccmd[signo-1];
}

long
Siglist::sig_mask()
{
	return 0;
}

int
Siglist::ignored( int signo )
{
	return !prismember( &_sigset, signo );
}

int
Siglist::set_sigset( sigset_t s )
{
	_sigset = s;
	return 1;
}

sigset_t
Siglist::sigset()
{
	return _sigset;
}
