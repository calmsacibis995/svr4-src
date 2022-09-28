#ident	"@(#)sdb:libutil/common/run.C	1.6"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
run( Process * process, int clearsig, Location * location, int count )
{
	Iaddr	addr;

	if ( process == 0 )
	{
		printe("internal error: ");
		printe("process pointer was zero\n");
		return 0;
	}
	else if ( get_addr( process, location, addr ) == 0 )
	{
		printe("could not run to specified location\n");
		return 0;
	}
	else
	{
		return process->run( clearsig, addr, count, 0 );
	}
}
