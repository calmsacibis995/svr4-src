#ident	"@(#)sdb:libexecon/common/TSClist.C	1.2"
#include	"TSClist.h"
#include	"SCmap.h"
#include	"Assoccmds.h"

static int
scmask( int i, sysset_t &set, int off = 0 )
{
	if ( off )
	{
		prdelset(&set, i);
	}
	else
	{
		praddset(&set, i);
	}
	return 1;
}

static int
sc_biton( int i, sysset_t &set )
{
	return prismember(&set, i);
}


TSClist::TSClist()
{
	int	i;

	premptyset(&entrymask)
	premptyset(&exitmask)
	for ( i = 0 ; i < lastone ; i++ )
	{
		entryassoc[i] = 0;
		exitassoc[i] = 0;
	}
}

TSClist::~TSClist()
{
	int	i;

	premptyset(&entrymask)
	premptyset(&exitmask)
	for ( i = 0 ; i < lastone ; i++ )
	{
		entryassoc[i] = ::dispose_assoc( entryassoc[i] );
		exitassoc[i] = ::dispose_assoc( exitassoc[i] );
	}
}

int
TSClist::add( int i, int exit, Assoccmds * assoc )
{
	if ( exit && sc_biton(i, exitmask) )
	{
		return 0;
	}
	else if ( exit && scmask(i,exitmask) )
	{
		exitassoc[i] = assoc;
		return 1;
	}
	else if ( scmask(i,entrymask,1) )
	{
		entryassoc[i] = assoc;
		return 1;
	}
	else
	{
		return 0;
	}
}

int
TSClist::remove( int i, int exit )
{
	if ( exit && sc_biton(i,exitmask) )
	{
		scmask( i, exitmask );
		exitassoc[i] = dispose_assoc( exitassoc[i] );
		return 1;
	}
	else if ( !exit && sc_biton(i,entrymask) )
	{
		scmask( i, entrymask );
		entryassoc[i] = dispose_assoc( entryassoc[i] );
		return 1;
	}
	else
	{
		return 0;
	}
}

int
TSClist::disable( int i, int exit )
{
	if ( exit && sc_biton(i,exitmask) )
	{
		scmask( i, exitmask );
		return 1;
	}
	else if ( !exit && sc_biton(i,entrymask) )
	{
		scmask( i, entrymask );
		return 1;
	}
	else
	{
		return 0;
	}
}

int
TSClist::enable( int i, int exit )
{
	if ( exit && !sc_biton(i,exitmask) )
	{
		return scmask( i, exitmask, 1 );
	}
	else if ( !exit && !sc_biton(i,entrymask) )
	{
		return scmask( i, entrymask, 1 );
	}
	else
	{
		return 0;
	}
}

sysset_t *
TSClist::tracemask( int exit )
{
	if ( exit )
		return &exitmask;
	else
		return &entrymask;
}

Assoccmds *
TSClist::assoccmds( int i, int exit )
{
	if ( exit )
		return exitassoc[i];
	else
		return entryassoc[i];
}
