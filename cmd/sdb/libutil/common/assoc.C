#ident	"@(#)sdb:libutil/common/assoc.C	1.2"
#include	"Assoccmds.h"
#include	"utility.h"
#include	"Interface.h"

Assoccmds *
new_assoc()
{
	return new Assoccmds;
}

int
add_assoc( Assoccmds * a, char * p )
{
	if ( a == 0 )
	{
		return 0;
	}
	else if ( p == 0 )
	{
		return 0;
	}
	else if (*p == '\n') 
	{
		return 0;
	}
	else
	{
		a->add_cmd( p );
		return 1;
	}
}
