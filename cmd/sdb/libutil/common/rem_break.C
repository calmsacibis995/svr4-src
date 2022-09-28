#ident	"@(#)sdb:libutil/common/rem_break.C	1.4"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
remove_break( Process * process, Location * location )
{
	Iaddr		addr;

	if ( process == 0 )
	{
		printe("internal error : null Process pointer\n");
		return 0;
	}
	else if ( get_addr( process, location, addr ) == 0 )
	{
		printe("internal error : no code at specified location\n");
		return 0;
	}
	else
	{
		return process->remove_bkpt( addr );
	}
}
