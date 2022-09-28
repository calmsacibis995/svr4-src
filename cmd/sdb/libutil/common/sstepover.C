#ident	"@(#)sdb:libutil/common/sstepover.C	1.4"
#include	"utility.h"
#include	"Process.h"
#include	"Interface.h"

int
stmt_step_over( Process * p, int clearsig, Location * loc, int cnt, int talk )
{
	Iaddr	addr;

	if ( p == 0 )
	{
		printe("internal error: ");
		printe("process pointer was zero\n");
		return 0;
	}
	else if ( get_addr( p, loc, addr ) == 0 )
	{
		printe("no code at specified location\n");
		return 0;
	}
	else
	{
		return p->stmt_step_over( clearsig, addr, cnt, talk );
	}
}
