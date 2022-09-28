#ident	"@(#)sdb:libutil/common/set_break.C	1.8"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
set_break( Process * process, Location * location, Assoccmds * a, int announce )
{
	Iaddr	addr;

	if ( process == 0 )
	{
		printe("internal error : null Process pointer\n");
		return 0;
	}
	else if ( get_addr( process, location, addr ) == 0 )
	{
		printe("no code at specified location\n");
		return 0;
	}
	else
	{
		return process->set_bkpt( addr, a, announce );
	}
}
