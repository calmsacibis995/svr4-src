#ident	"@(#)sdb:libutil/common/sstep.C	1.4"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
stmt_step( Process * process, int clearsig, Location * loc, int cnt, int talk )
{
	Iaddr	addr;

	if ( process == 0 )
	{
		printe("internal error: ");
		printe("process pointer was zero\n");
		return 0;
	}
	else if ( get_addr( process, loc, addr ) == 0 )
	{
		printe("no code at specified location\n");
		return 0;
	}
	else
	{
		return process->stmt_step( clearsig, addr, cnt, talk );
	}
}
